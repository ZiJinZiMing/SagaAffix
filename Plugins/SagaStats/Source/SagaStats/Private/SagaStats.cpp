/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#include "SagaStats.h"

#include "SGAbilitySystemComponent.h"
#include "GameFramework/HUD.h"

#define LOCTEXT_NAMESPACE "FSagaStatsModule"

void FSagaStatsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	if (!IsRunningDedicatedServer())
	{
		AHUD::OnShowDebugInfo.AddStatic(&USGAbilitySystemComponent::OnShowMeterDebugInfo);
	}
}

void FSagaStatsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSagaStatsModule, SagaStats)