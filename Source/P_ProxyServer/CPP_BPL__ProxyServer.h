/*
 * @Author: Punal Manalan
 * @Description: Proxy Server Plugin.
 * @Date: 22/11/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"

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
};
