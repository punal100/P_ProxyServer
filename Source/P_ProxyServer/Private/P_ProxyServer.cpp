/*
 * @Author: Punal Manalan
 * @Description: Proxy Server Plugin.
 * @Date: 22/11/2025
 */

#include "P_ProxyServer.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogPProxyServer, Log, All);

// Text localization namespace for this module
#define LOCTEXT_NAMESPACE "FP_ProxyServerModule"

/**
 * Module startup - called after module is loaded into memory
 * Initialize any global resources or register systems here
 */
void FP_ProxyServer::StartupModule()
{
    UE_LOG(LogPProxyServer, Display, TEXT("P_ProxyServer: StartupModule"));
}

/**
 * Module shutdown - called before module is unloaded
 * Clean up any global resources or unregister systems here
 */
void FP_ProxyServer::ShutdownModule()
{
    UE_LOG(LogPProxyServer, Display, TEXT("P_ProxyServer: ShutdownModule"));
}

// Undefine the localization namespace to avoid conflicts
#undef LOCTEXT_NAMESPACE

// Register this module with Unreal's module system
IMPLEMENT_MODULE(FP_ProxyServer, P_ProxyServer)