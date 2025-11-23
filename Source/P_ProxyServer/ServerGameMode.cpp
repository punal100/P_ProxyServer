#include "ServerGameMode.h"
#include "CPP_LoginManagerSubsystem.h"
#include "Engine/World.h"
#include "OnlineSubsystemTypes.h"

void AServerGameMode::PreLogin(const FString &Options, const FString &Address, const FUniqueNetIdRepl &UniqueId, FString &ErrorMessage)
{
    UCPP_LoginManagerSubsystem *LoginSys = GetWorld() ? GetWorld()->GetSubsystem<UCPP_LoginManagerSubsystem>() : nullptr;

    if (LoginSys && LoginSys->Implements<UCPP_LoginHandler>())
    {
        FString SubsystemError;
        bool bAllowed = ICPP_LoginHandler::Execute_ValidatePlayerLogin(LoginSys, Options, Address, UniqueId, SubsystemError);
        if (!bAllowed)
        {
            ErrorMessage = SubsystemError;
            return; // reject
        }
    }

    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void AServerGameMode::PostLogin(APlayerController *NewPlayer)
{
    Super::PostLogin(NewPlayer);

    UCPP_LoginManagerSubsystem *LoginSys = GetWorld() ? GetWorld()->GetSubsystem<UCPP_LoginManagerSubsystem>() : nullptr;
    if (LoginSys && LoginSys->Implements<UCPP_LoginHandler>())
    {
        ICPP_LoginHandler::Execute_OnPlayerPostLogin(LoginSys, NewPlayer);
    }
}
