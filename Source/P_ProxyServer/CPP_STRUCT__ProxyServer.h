/*
 * @Author: Punal Manalan
 * @Description: Proxy Server Plugin.
 * @Date: 22/11/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "CPP_STRUCT__ProxyServer.generated.h"

// Punal Manalan, NOTE: This is Received from the Server Backend when a Player Requests to Join the Server
// Session Token Struct
USTRUCT(BlueprintType)
struct P_PROXYSERVER_API FSessionJoinToken
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Punal|Token")
    FString playerID = "";

    UPROPERTY(BlueprintReadWrite, Category = "Punal|Token")
    int64 timeStamp = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Punal|Token")
    FString sessionSecret = "";
};

// Pair of Player ID and Encrypted Token Struct
USTRUCT(BlueprintType)
struct P_PROXYSERVER_API FSessionJoinTokenEncrypted
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Punal|Token")
    FString playerID;

    /* Punal Manalan, NOTE: HMAC_SHA256( JSON of FSessionJoinToken )
     *  Signature is Compared with the Signature the FSessionJoinToken
     */
    UPROPERTY(BlueprintReadWrite, Category = "Punal|Token")
    FString signature = "";

    /* Punal Manalan, NOTE: The actual Encrypted Session Secret.
     * this is the Encrypted version of FSessionJoinToken JSON
     * that is Received from the Player After After Pre Login(Mainly after Post Login, with Timeout).
     */
    UPROPERTY(BlueprintReadWrite, Category = "Punal|Token")
    FString sessionJoinTokenEncryptedBASE64;
};

// Player Data Struct
USTRUCT(BlueprintType)
struct P_PROXYSERVER_API FPlayerData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Punal|Player")
    FString playerID;

    UPROPERTY(BlueprintReadWrite, Category = "Punal|Player")
    FSessionJoinToken sessionJoinTokenFromServer;

    UPROPERTY(BlueprintReadWrite, Category = "Punal|Player")
    FSessionJoinTokenEncrypted sessionJoinTokenEncryptedFromPlayer;

    UPROPERTY(BlueprintReadWrite, Category = "Punal|Player")
    bool bIsTokenSignatureValid = false;

    UPROPERTY(BlueprintReadWrite, Category = "Punal|Player")
    bool bIsTokenSecretValid = false;

    UPROPERTY(BlueprintReadWrite, Category = "Punal|Player")
    TArray<FString> roles;
};