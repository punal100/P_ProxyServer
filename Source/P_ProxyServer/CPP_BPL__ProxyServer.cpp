/*
 * @Author: Punal Manalan
 * @Description: Proxy Server Plugin.
 * @Date: 22/11/2025
 */

#include "CPP_BPL__ProxyServer.h"
#include "JsonObjectConverter.h"
#include "HAL/PlatformMisc.h"
#include "Misc/ScopeLock.h"
#include <stdint.h>
#include <string.h>

// OpenSSL Includes
#define UI UI_STUB
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "openssl/err.h"
#include "openssl/bio.h"
#include "openssl/bn.h"
#undef UI

FVector UCPP_BPL__ProxyServer::GetClosestPointOnLineSegment(
    const FVector &Point,
    const FVector &LineStart,
    const FVector &LineEnd)
{
    FVector LineVec = LineEnd - LineStart;
    FVector PointVec = Point - LineStart;

    float LineLength = LineVec.Size();
    if (LineLength < KINDA_SMALL_NUMBER)
    {
        return LineStart; // Line segment is too short
    }

    FVector LineDir = LineVec / LineLength;
    float ProjectedDistance = FVector::DotProduct(PointVec, LineDir);

    // Clamp to line segment bounds
    ProjectedDistance = FMath::Clamp(ProjectedDistance, 0.0f, LineLength);

    return LineStart + (LineDir * ProjectedDistance);
}

bool UCPP_BPL__ProxyServer::SessionJoinToken_FromJson(const FString &JsonString, FSessionJoinToken &OutToken)
{
    if (JsonString.IsEmpty())
    {
        return false;
    }

    // Use JsonObjectConverter to deserialize into the struct
    return FJsonObjectConverter::JsonObjectStringToUStruct<FSessionJoinToken>(JsonString, &OutToken, 0, 0);
}

bool UCPP_BPL__ProxyServer::SessionJoinToken_ToJson(const FSessionJoinToken &Token, FString &OutJsonString, bool bPrettyPrint)
{
    // Use JsonObjectConverter to serialize the struct to a JSON string
    // The UStructToJsonObjectString helper returns true on success
    return FJsonObjectConverter::UStructToJsonObjectString(FSessionJoinToken::StaticStruct(), &Token, OutJsonString, 0, 0);
}

// Minimal public-domain SHA256 implementation (adapted for plugin use)
namespace
{
    // rotate right
    inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }

    static const uint32_t K256[64] = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};

    static void sha256_compute(const uint8_t *data, size_t len, uint8_t out[32])
    {
        uint64_t bitlen = (uint64_t)len * 8u;
        size_t rem = len % 64u;
        size_t padded_len = len + 1 + ((rem < 56) ? (56 - rem) : (120 - rem));

        uint8_t *buf = (uint8_t *)FMemory::Malloc(padded_len);
        if (!buf)
        {
            // out as zeros
            FMemory::Memzero(out, 32);
            return;
        }

        FMemory::Memcpy(buf, data, len);
        buf[len] = 0x80;
        if (padded_len > len + 1)
        {
            FMemory::Memzero(buf + len + 1, padded_len - (len + 1));
        }
        // write big-endian bit length at end
        for (int i = 0; i < 8; ++i)
        {
            buf[padded_len - 8 + i] = (uint8_t)((bitlen >> ((7 - i) * 8)) & 0xFFu);
        }

        uint32_t h0 = 0x6a09e667u;
        uint32_t h1 = 0xbb67ae85u;
        uint32_t h2 = 0x3c6ef372u;
        uint32_t h3 = 0xa54ff53au;
        uint32_t h4 = 0x510e527fu;
        uint32_t h5 = 0x9b05688cu;
        uint32_t h6 = 0x1f83d9abu;
        uint32_t h7 = 0x5be0cd19u;

        size_t chunks = padded_len / 64u;
        for (size_t c = 0; c < chunks; ++c)
        {
            const uint8_t *chunk = buf + (c * 64u);
            uint32_t w[64];
            for (int t = 0; t < 16; ++t)
            {
                w[t] = (uint32_t)chunk[t * 4] << 24 | (uint32_t)chunk[t * 4 + 1] << 16 | (uint32_t)chunk[t * 4 + 2] << 8 | (uint32_t)chunk[t * 4 + 3];
            }
            for (int t = 16; t < 64; ++t)
            {
                uint32_t s0 = rotr(w[t - 15], 7) ^ rotr(w[t - 15], 18) ^ (w[t - 15] >> 3);
                uint32_t s1 = rotr(w[t - 2], 17) ^ rotr(w[t - 2], 19) ^ (w[t - 2] >> 10);
                w[t] = w[t - 16] + s0 + w[t - 7] + s1;
            }

            uint32_t a = h0;
            uint32_t b = h1;
            uint32_t c2 = h2;
            uint32_t d = h3;
            uint32_t e = h4;
            uint32_t f = h5;
            uint32_t g = h6;
            uint32_t h = h7;

            for (int t = 0; t < 64; ++t)
            {
                uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
                uint32_t ch = (e & f) ^ ((~e) & g);
                uint32_t temp1 = h + S1 + ch + K256[t] + w[t];
                uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
                uint32_t maj = (a & b) ^ (a & c2) ^ (b & c2);
                uint32_t temp2 = S0 + maj;

                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c2;
                c2 = b;
                b = a;
                a = temp1 + temp2;
            }

            h0 += a;
            h1 += b;
            h2 += c2;
            h3 += d;
            h4 += e;
            h5 += f;
            h6 += g;
            h7 += h;
        }

        FMemory::Free(buf);

        // store big-endian
        uint32_t hs[8] = {h0, h1, h2, h3, h4, h5, h6, h7};
        for (int i = 0; i < 8; ++i)
        {
            out[i * 4 + 0] = (uint8_t)((hs[i] >> 24) & 0xFFu);
            out[i * 4 + 1] = (uint8_t)((hs[i] >> 16) & 0xFFu);
            out[i * 4 + 2] = (uint8_t)((hs[i] >> 8) & 0xFFu);
            out[i * 4 + 3] = (uint8_t)(hs[i] & 0xFFu);
        }
    }
} // anonymous namespace

FString UCPP_BPL__ProxyServer::Sha256String(const FString &Input)
{
    // Convert to UTF-8 bytes
    FTCHARToUTF8 Converter(*Input);
    const ANSICHAR *BytesAnsi = Converter.Get();
    int32 Length = Converter.Length();

    const uint8_t *Data = reinterpret_cast<const uint8_t *>(reinterpret_cast<const void *>(BytesAnsi));
    uint8_t Hash[32];
    sha256_compute(Data, (size_t)Length, Hash);

    // Convert to lowercase hex
    FString Out;
    Out.Reserve(64);
    for (int i = 0; i < 32; ++i)
    {
        uint8_t b = Hash[i];
        Out += FString::Printf(TEXT("%02x"), b);
    }
    return Out;
}

FString UCPP_BPL__ProxyServer::RsaEncryptString(const FString &Content, const FString &PublicKeyPEM)
{
    return RsaEncryptString_Cpp(Content, PublicKeyPEM);
}

FString UCPP_BPL__ProxyServer::RsaEncryptString_Cpp(const FString &Content, const FString &PublicKeyPEM)
{
    if (Content.IsEmpty() || PublicKeyPEM.IsEmpty())
    {
        return FString();
    }

    // Convert PEM string to BIO
    FTCHARToUTF8 KeyConverter(*PublicKeyPEM);
    BIO *KeyBio = BIO_new_mem_buf((void *)KeyConverter.Get(), KeyConverter.Length());
    if (!KeyBio)
    {
        return FString();
    }

    // Read Public Key
    RSA *RsaKey = PEM_read_bio_RSA_PUBKEY(KeyBio, NULL, NULL, NULL);
    if (!RsaKey)
    {
        // Try reading as RSAPublicKey (PKCS#1) if PUBKEY (SubjectPublicKeyInfo) fails
        BIO_reset(KeyBio);
        RsaKey = PEM_read_bio_RSAPublicKey(KeyBio, NULL, NULL, NULL);
    }

    BIO_free(KeyBio);

    if (!RsaKey)
    {
        return FString();
    }

    // Calculate max data size
    int32 RsaSize = RSA_size(RsaKey);
    // OAEP padding overhead is 41 bytes (approx)
    int32 MaxDataSize = RsaSize - 42;

    FTCHARToUTF8 DataConverter(*Content);
    const uint8 *DataBytes = (const uint8 *)DataConverter.Get();
    int32 DataLength = DataConverter.Length();

    if (DataLength > MaxDataSize)
    {
        RSA_free(RsaKey);
        // Data too large for RSA key
        return FString();
    }

    TArray<uint8> EncryptedData;
    EncryptedData.SetNumUninitialized(RsaSize);

    int32 EncryptedLength = RSA_public_encrypt(DataLength, DataBytes, EncryptedData.GetData(), RsaKey, RSA_PKCS1_OAEP_PADDING);

    RSA_free(RsaKey);

    if (EncryptedLength == -1)
    {
        return FString();
    }

    // Encode to Base64
    return FBase64::Encode(EncryptedData.GetData(), EncryptedLength);
}

FString UCPP_BPL__ProxyServer::RsaDecryptString(const FString &EncryptedBase64, const FString &PrivateKeyPEM)
{
    return RsaDecryptString_Cpp(EncryptedBase64, PrivateKeyPEM);
}

FString UCPP_BPL__ProxyServer::RsaDecryptString_Cpp(const FString &EncryptedBase64, const FString &PrivateKeyPEM)
{
    if (EncryptedBase64.IsEmpty() || PrivateKeyPEM.IsEmpty())
    {
        return FString();
    }

    TArray<uint8> EncryptedData;
    if (!FBase64::Decode(EncryptedBase64, EncryptedData))
    {
        return FString();
    }

    // Convert PEM string to BIO
    FTCHARToUTF8 KeyConverter(*PrivateKeyPEM);
    BIO *KeyBio = BIO_new_mem_buf((void *)KeyConverter.Get(), KeyConverter.Length());
    if (!KeyBio)
    {
        return FString();
    }

    // Read Private Key
    RSA *RsaKey = PEM_read_bio_RSAPrivateKey(KeyBio, NULL, NULL, NULL);
    BIO_free(KeyBio);

    if (!RsaKey)
    {
        return FString();
    }

    int32 RsaSize = RSA_size(RsaKey);
    TArray<uint8> DecryptedData;
    DecryptedData.SetNumUninitialized(RsaSize);

    int32 DecryptedLength = RSA_private_decrypt(EncryptedData.Num(), EncryptedData.GetData(), DecryptedData.GetData(), RsaKey, RSA_PKCS1_OAEP_PADDING);

    RSA_free(RsaKey);

    if (DecryptedLength == -1)
    {
        return FString();
    }

    // Convert result to FString (UTF8)
    DecryptedData.Add(0); // Null terminate
    return FString(UTF8_TO_TCHAR((const ANSICHAR *)DecryptedData.GetData()));
}

void UCPP_BPL__ProxyServer::GenerateRsaKeyPair(int32 KeySizeInBits, FString &OutPublicKeyPEM, FString &OutPrivateKeyPEM)
{
    GenerateRsaKeyPair_Cpp(KeySizeInBits, OutPublicKeyPEM, OutPrivateKeyPEM);
}

void UCPP_BPL__ProxyServer::GenerateRsaKeyPair_Cpp(int32 KeySizeInBits, FString &OutPublicKeyPEM, FString &OutPrivateKeyPEM)
{
    OutPublicKeyPEM.Empty();
    OutPrivateKeyPEM.Empty();

    if (KeySizeInBits < 512)
    {
        KeySizeInBits = 2048;
    }

    BIGNUM *e = BN_new();
    RSA *rsa = RSA_new();

    if (!e || !rsa)
    {
        if (e)
            BN_free(e);
        if (rsa)
            RSA_free(rsa);
        return;
    }

    BN_set_word(e, RSA_F4); // 65537

    if (RSA_generate_key_ex(rsa, KeySizeInBits, e, NULL) != 1)
    {
        BN_free(e);
        RSA_free(rsa);
        return;
    }

    // Write Private Key
    BIO *bp_private = BIO_new(BIO_s_mem());
    if (PEM_write_bio_RSAPrivateKey(bp_private, rsa, NULL, NULL, 0, NULL, NULL) == 1)
    {
        char *data = NULL;
        long len = BIO_get_mem_data(bp_private, &data);
        if (data && len > 0)
        {
            OutPrivateKeyPEM = FString(FUTF8ToTCHAR((const char *)data, len));
        }
    }
    BIO_free(bp_private);

    // Write Public Key
    BIO *bp_public = BIO_new(BIO_s_mem());
    if (PEM_write_bio_RSA_PUBKEY(bp_public, rsa) == 1)
    {
        char *data = NULL;
        long len = BIO_get_mem_data(bp_public, &data);
        if (data && len > 0)
        {
            OutPublicKeyPEM = FString(FUTF8ToTCHAR((const char *)data, len));
        }
    }
    BIO_free(bp_public);

    BN_free(e);
    RSA_free(rsa);
}

FString UCPP_BPL__ProxyServer::HmacSha256String(const FString &Data, const FString &Key)
{
    // Convert inputs to UTF-8
    FTCHARToUTF8 DataConverter(*Data);
    const uint8 *DataBytes = (const uint8 *)DataConverter.Get();
    int32 DataLen = DataConverter.Length();

    FTCHARToUTF8 KeyConverter(*Key);
    const uint8 *KeyBytes = (const uint8 *)KeyConverter.Get();
    int32 KeyLen = KeyConverter.Length();

    // SHA256 Block Size = 64 bytes
    const int32 BlockSize = 64;
    uint8 KeyPad[BlockSize];
    FMemory::Memzero(KeyPad, BlockSize);

    if (KeyLen > BlockSize)
    {
        // If key is longer than block size, hash it
        uint8 KeyHash[32];
        sha256_compute(KeyBytes, KeyLen, KeyHash);
        FMemory::Memcpy(KeyPad, KeyHash, 32);
    }
    else
    {
        FMemory::Memcpy(KeyPad, KeyBytes, KeyLen);
    }

    // Prepare ipad and opad
    uint8 IPad[BlockSize];
    uint8 OPad[BlockSize];

    for (int32 i = 0; i < BlockSize; ++i)
    {
        IPad[i] = KeyPad[i] ^ 0x36;
        OPad[i] = KeyPad[i] ^ 0x5c;
    }

    // Inner Hash: SHA256(IPad || Data)
    // We need to concatenate IPad and Data
    int32 InnerDataLen = BlockSize + DataLen;
    uint8 *InnerData = (uint8 *)FMemory::Malloc(InnerDataLen);
    if (!InnerData)
        return FString();

    FMemory::Memcpy(InnerData, IPad, BlockSize);
    FMemory::Memcpy(InnerData + BlockSize, DataBytes, DataLen);

    uint8 InnerHash[32];
    sha256_compute(InnerData, InnerDataLen, InnerHash);
    FMemory::Free(InnerData);

    // Outer Hash: SHA256(OPad || InnerHash)
    int32 OuterDataLen = BlockSize + 32;
    uint8 *OuterData = (uint8 *)FMemory::Malloc(OuterDataLen);
    if (!OuterData)
        return FString();

    FMemory::Memcpy(OuterData, OPad, BlockSize);
    FMemory::Memcpy(OuterData + BlockSize, InnerHash, 32);

    uint8 OuterHash[32];
    sha256_compute(OuterData, OuterDataLen, OuterHash);
    FMemory::Free(OuterData);

    // Convert to hex string
    FString Out;
    Out.Reserve(64);
    for (int i = 0; i < 32; ++i)
    {
        Out += FString::Printf(TEXT("%02x"), OuterHash[i]);
    }
    return Out;
}