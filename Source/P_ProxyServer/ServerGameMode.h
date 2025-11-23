// Sample Server GameMode that routes login/post-login to the subsystem
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

// Forward declare types to avoid pulling in OnlineSubsystem headers here.
struct FUniqueNetIdRepl;
class APlayerController;
#include "ServerGameMode.generated.h"

UCLASS()
class P_PROXYSERVER_API AServerGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void PreLogin(const FString &Options, const FString &Address, const FUniqueNetIdRepl &UniqueId, FString &ErrorMessage) override;
    virtual void PostLogin(APlayerController *NewPlayer) override;
};
