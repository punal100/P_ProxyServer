/*
 * @Author: Punal Manalan
 * @Description: Proxy Server Plugin.
 * @Date: 22/11/2025
 */

#include "CPP_LoginManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

bool UCPP_LoginManagerSubsystem::ShouldCreateSubsystem(UObject *Outer) const
{
    // ShouldCreateSubsystem is const; call the non-const implementation via const_cast
    const_cast<UCPP_LoginManagerSubsystem *>(this)->InitializeLoginHandler_Implementation();
    return true;
}

void UCPP_LoginManagerSubsystem::GenerateNewRSAKeyPair(int32 KeySizeInBits, bool bIsGlobalKey)
{
    if (bIsGlobalKey)
    {
        UCPP_BPL__ProxyServer::GenerateRsaKeyPair(KeySizeInBits, Server_Global_PublicKeyPEM, Server_Global_PrivateKeyPEM);
    }
    else
    {
        UCPP_BPL__ProxyServer::GenerateRsaKeyPair(KeySizeInBits, Server_Local_PublicKeyPEM, Server_Local_PrivateKeyPEM);
    }
}

bool UCPP_LoginManagerSubsystem::InitializeLoginHandler_Implementation()
{
    // Load Global Keys from Config-specified files
    if (EnableGloablEncryptionValidation)
    {
        // 1. Load Private Key
        if (!LoadGlobalKeyFromFile_Implementation(GlobalPrivateKeyFilename, Server_Global_PrivateKeyPEM))
        {
            UE_LOG(LogTemp, Error, TEXT("CRITICAL: Failed to load Global Private Key from file: %s"), *GlobalPrivateKeyFilename);
            return false;
        }

        // 2. Load Public Key
        if (!LoadGlobalKeyFromFile_Implementation(GlobalPublicKeyFilename, Server_Global_PublicKeyPEM))
        {
            UE_LOG(LogTemp, Error, TEXT("CRITICAL: Failed to load Global Public Key from file: %s"), *GlobalPublicKeyFilename);
            return false;
        }
    }

    if (EnableLocalEncryptionValidation)
    {
        // Generate a new Local Key Pair at Runtime
        // Note: GenerateNewRSAKeyPair is a void function that populates member vars.
        GenerateNewRSAKeyPair(2048, false);
    }

    return true;
}

bool UCPP_LoginManagerSubsystem::LoadGlobalKeyFromFile_Implementation(const FString &RelativePath, FString &OutKeyContent)
{
    // 1. Get the Plugin pointer
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("P_ProxyServer"));

    if (!Plugin.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find Plugin 'P_ProxyServer'"));
        return false;
    }

    // 2. Get the Plugin's Content Directory
    // This returns Path Such as: ".../Plugins/P_ProxyServer/Content/"
    FString PluginContentDir = Plugin->GetContentDir();

    // 3. Combine with your relative path ("Secrets/GlobalKey.pem")
    FString FullPath = FPaths::Combine(PluginContentDir, RelativePath);

    // 4. Load
    if (!FFileHelper::LoadFileToString(OutKeyContent, *FullPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load key at: %s"), *FullPath);
        return false;
    }

    return true;
}

bool UCPP_LoginManagerSubsystem::SendAPIRequestToBackendServer_Implementation(const FString &Send_Payload, const FOnSentAPIResponse &ResponseDelegate)
{
    // Punal Manalan, NOTE: For the Most Part ResponseDelegate will be HandleAPIResponseFromBackendServer Function
    FHttpModule *Http = &FHttpModule::Get();
    if (!Http)
    {
        return false;
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();

    Request->SetURL(Backend_Server_URL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(Send_Payload);

    Request->OnProcessRequestComplete().BindLambda(
        [this, Send_Payload, ResponseDelegate](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
        {
            if (bConnectedSuccessfully && Response.IsValid())
            {
                FString ResponseContent = Response->GetContentAsString();

                /* Punal Manalan, NOTE: No need to Handle the response internally, as the Delegate will do that
                 * as the Passed Delegate would normally be the HandleAPIResponseFromBackendServer Function
                 */
                // HandleAPIResponseFromBackendServer_Implementation(Send_Payload, ResponseContent, TEXT(""));

                // Notify the delegate
                ResponseDelegate.ExecuteIfBound(Send_Payload, ResponseContent, TEXT(""));
            }
            else
            {
                FString ErrorMessage = TEXT("Connection failed");
                if (Response.IsValid())
                {
                    ErrorMessage = FString::Printf(TEXT("Request failed with code: %d"), Response->GetResponseCode());
                }
                ResponseDelegate.ExecuteIfBound(Send_Payload, TEXT(""), ErrorMessage);
            }
        });

    Request->ProcessRequest();
    return true;
}

bool UCPP_LoginManagerSubsystem::HandleAPIResponseFromBackendServer_Implementation(const FString &Sent_Payload, const FString &Response_Payload, FString &Error)
{
    // TODO: parse response and update internal state as required.
    return true;
}

bool UCPP_LoginManagerSubsystem::HandleAPIFromBackendServer_Implementation(const FString &Received_Payload)
{
    // TODO: handle push/API events from backend
    return true;
}
bool UCPP_LoginManagerSubsystem::IsServerLocked() const
{
    return bIsServerLocked;
}

void UCPP_LoginManagerSubsystem::SetServerLocked(bool bLocked)
{
    bIsServerLocked = bLocked;
}

FString UCPP_LoginManagerSubsystem::GetBackendServerURL() const
{
    return Backend_Server_URL;
}

void UCPP_LoginManagerSubsystem::SetBackendServerURL(const FString &URL)
{
    Backend_Server_URL = URL;
}

TArray<FString> UCPP_LoginManagerSubsystem::GetAllowedRoleList() const
{
    return AllowedRole_List;
}

void UCPP_LoginManagerSubsystem::SetAllowedRoleList(const TArray<FString> &NewList)
{
    AllowedRole_List = NewList;
}

TArray<FString> UCPP_LoginManagerSubsystem::GetBannedRoleList() const
{
    return RestrictedRole_List;
}

void UCPP_LoginManagerSubsystem::SetBannedRoleList(const TArray<FString> &NewList)
{
    RestrictedRole_List = NewList;
}

TMap<FString, FPlayerData> UCPP_LoginManagerSubsystem::GetPlayerID_SessionData_Map() const
{
    return PlayerID_SessionData_Map;
}

void UCPP_LoginManagerSubsystem::SetPlayerID_SessionData_Map(const TMap<FString, FPlayerData> &NewMap)
{
    PlayerID_SessionData_Map = NewMap;
}

FString UCPP_LoginManagerSubsystem::GetServer_Global_PrivateKeyPEM() const
{
    return Server_Global_PrivateKeyPEM;
}

void UCPP_LoginManagerSubsystem::SetServer_Global_PrivateKeyPEM(const FString &NewPrivateKeyPEM)
{
    Server_Global_PrivateKeyPEM = NewPrivateKeyPEM;
}

FString UCPP_LoginManagerSubsystem::GetServer_Global_PublicKeyPEM() const
{
    return Server_Global_PublicKeyPEM;
}

void UCPP_LoginManagerSubsystem::SetServer_Global_PublicKeyPEM(const FString &NewPublicKeyPEM)
{
    Server_Global_PublicKeyPEM = NewPublicKeyPEM;
}

bool UCPP_LoginManagerSubsystem::ValidatePlayerLogin_Implementation(const FString &Options, const FString &Address, const FUniqueNetIdRepl &UniqueId, FString &OutErrorMessage)
{
    // Check if server is "locked"
    if (bIsServerLocked)
    {
        OutErrorMessage = TEXT("Server is currently locked by the Admin.");
        return false; // REJECT
    }

    // Check if the player is assigned a role that is Valid or Banned
    bool HasValidRole = false;
    bool HasRestrictedRole = false;
    const FString PlayerId = UniqueId.ToString();
    for (const TPair<FString, FPlayerData> &Pair : PlayerID_SessionData_Map)
    {
        const FString &MappedId = Pair.Key;
        if (MappedId == PlayerId)
        {
            for (const FString &Role : Pair.Value.roles)
            {
                if (RestrictedRole_List.Contains(Role))
                {
                    OutErrorMessage = TEXT("You are restricted from this server.");
                    // HasRestrictedRole = true;
                    return false; // REJECT
                }
                if (!HasValidRole)
                {
                    if (AllowedRole_List.Contains(Role))
                    {
                        HasValidRole = true; // found a valid role
                    }
                }
            }
            break; // found mapping, stop searching
        }
    }

    // If no valid role found, reject
    if (!HasValidRole)
    {
        OutErrorMessage = TEXT("This server is not configured to accept the Specific roles of the Player.");
        return false; // REJECT
    }

    return true; // ALLOW
}

void UCPP_LoginManagerSubsystem::OnPlayerPostLogin_Implementation(APlayerController *NewPlayer)
{
    if (!NewPlayer)
        return;
    UE_LOG(LogTemp, Warning, TEXT("Subsystem handling new player: %s"), *NewPlayer->GetName());
}
