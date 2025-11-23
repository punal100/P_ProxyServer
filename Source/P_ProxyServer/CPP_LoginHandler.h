/*
 * @Author: Punal Manalan
 * @Description: Proxy Server Plugin.
 * @Date: 22/11/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/OnlineReplStructs.h"

#include "CPP_LoginHandler.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnSentAPIResponse, const FString &, Sent_Payload, const FString &, Response_Payload, const FString &, Error);

UINTERFACE(MinimalAPI, BlueprintType)
class UCPP_LoginHandler : public UInterface
{
    GENERATED_BODY()
};

class P_PROXYSERVER_API ICPP_LoginHandler
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Login Logic")
    bool InitializeLoginHandler();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Login Logic")
    bool LoadGlobalKeyFromFile(const FString &RelativePath, FString &OutKeyContent);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Login Logic")
    bool SendAPIRequestToBackendServer(const FString &Send_Payload, const FOnSentAPIResponse &ResponseDelegate);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Login Logic")
    bool HandleAPIResponseFromBackendServer(const FString &Sent_Payload, const FString &Response_Payload, FString &Error);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Login Logic")
    bool HandleAPIFromBackendServer(const FString &Received_Payload);

    // Corresponds to GameMode::PreLogin
    // Returns TRUE if login is allowed. If FALSE, sets OutErrorMessage.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Login Logic")
    bool ValidatePlayerLogin(const FString &Options, const FString &Address, const FUniqueNetIdRepl &UniqueId, FString &OutErrorMessage);

    // Corresponds to GameMode::PostLogin
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Login Logic")
    void OnPlayerPostLogin(APlayerController *NewPlayer);
};
