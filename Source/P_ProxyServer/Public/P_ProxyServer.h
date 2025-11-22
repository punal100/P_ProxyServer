/*
 * @Author: Punal Manalan
 * @Description: Proxy Server Plugin.
 * @Date: 22/11/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Main module class for ProxyServer Plugin
 * Handles module lifecycle (startup/shutdown) and initialization
 */
class FP_ProxyServer : public IModuleInterface
{
public:
    /** Called when module is loaded into memory */
    virtual void StartupModule() override;

    /** Called when module is unloaded from memory */
    virtual void ShutdownModule() override;
};
