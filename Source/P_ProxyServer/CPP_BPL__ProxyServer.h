/*
 * @Author: Punal Manalan
 * @Description: Proxy Server Plugin.
 * @Date: 22/11/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "CPP_STRUCT__ProxyServer.h"
#include "JsonObjectConverter.h"

#include "CPP_BPL__ProxyServer.generated.h"

/**
 *
 */
UCLASS()
class P_PROXYSERVER_API UCPP_BPL__ProxyServer : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Helper functions
	UFUNCTION(BlueprintCallable, Category = "Punal|ProxyServer|Math|LineSegment")
	static FVector GetClosestPointOnLineSegment(
		const FVector &Point,
		const FVector &LineStart,
		const FVector &LineEnd);

	// JSON (de)serialization for FSessionJoinToken
	UFUNCTION(BlueprintCallable, Category = "Punal|ProxyServer|Serialization")
	static bool SessionJoinToken_FromJson(const FString &JsonString, FSessionJoinToken &OutToken);

	UFUNCTION(BlueprintCallable, Category = "Punal|ProxyServer|Serialization")
	static bool SessionJoinToken_ToJson(const FSessionJoinToken &Token, FString &OutJsonString, bool bPrettyPrint = false);

	// Crypto: SHA256 hash of an FString, returned as lowercase hex string
	UFUNCTION(BlueprintCallable, Category = "Punal|ProxyServer|Crypto")
	static FString Sha256String(const FString &Input);

	// Crypto: RSA Encrypt string using Public Key (PEM format). Returns Base64 encoded encrypted data.
	UFUNCTION(BlueprintCallable, Category = "Punal|ProxyServer|Crypto")
	static FString RsaEncryptString(const FString &Content, const FString &PublicKeyPEM);

	// Crypto: RSA Decrypt Base64 string using Private Key (PEM format). Returns decrypted string.
	UFUNCTION(BlueprintCallable, Category = "Punal|ProxyServer|Crypto")
	static FString RsaDecryptString(const FString &EncryptedBase64, const FString &PrivateKeyPEM);

	// Crypto: Generate a new RSA Public/Private Key pair (PEM format).
	// KeySizeInBits: typically 2048 or 4096.
	UFUNCTION(BlueprintCallable, Category = "Punal|ProxyServer|Crypto")
	static void GenerateRsaKeyPair(int32 KeySizeInBits, FString &OutPublicKeyPEM, FString &OutPrivateKeyPEM);

	// Crypto: Generate HMAC-SHA256 signature of Data using Key. Returns lowercase hex string.
	UFUNCTION(BlueprintCallable, Category = "Punal|ProxyServer|Crypto")
	static FString HmacSha256String(const FString &Data, const FString &Key);

	// --- C++ Variants (Non-UFunction) ---

	static FString RsaEncryptString_Cpp(const FString &Content, const FString &PublicKeyPEM);
	static FString RsaDecryptString_Cpp(const FString &EncryptedBase64, const FString &PrivateKeyPEM);
	static void GenerateRsaKeyPair_Cpp(int32 KeySizeInBits, FString &OutPublicKeyPEM, FString &OutPrivateKeyPEM);
};
