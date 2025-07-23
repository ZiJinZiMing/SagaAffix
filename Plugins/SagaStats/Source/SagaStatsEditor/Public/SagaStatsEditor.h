/******************************************************************************
* ProjectName:  SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IAssetTypeActions;
struct FGraphPanelPinFactory;

class FSagaStatsEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FSagaStatsEditorModule& Get()
    {
        static const FName ModuleName = "SagaStatsEditor";
        return FModuleManager::LoadModuleChecked<FSagaStatsEditorModule>(ModuleName);
    }

    void PreloadAssetsByClass(UClass* InClass) const;

    void OnPostEngineInit();

    void RegisterAssetTypeAction(class IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);
    
private:
    /** Pin factory for gameplay abilities; Cached so it can be unregistered */
    TSharedPtr<FGraphPanelPinFactory> GameplayAbilitiesGraphPanelPinFactory;

    /** All created asset type actions. Cached here so that we can unregister it during shutdown. */
    TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
};
