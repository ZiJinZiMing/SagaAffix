/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SSGAttributeListReferenceViewer;
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
    void RegisterConsoleCommands();
    void UnregisterConsoleCommands();
    void ExecuteShowGameplayAttributeReferencesWindow(const TArray<FString>& InArgs);

private:
	TArray<IConsoleCommand*> ConsoleCommands;

	
    /** Pin factory for gameplay abilities; Cached so it can be unregistered */
    TSharedPtr<FGraphPanelPinFactory> GameplayAbilitiesGraphPanelPinFactory;
	TSharedPtr<SSGAttributeListReferenceViewer> AttributeListReferenceViewerWidget;
	TSharedPtr<SWindow> AttributeListReferenceViewerWindow;

    /** All created asset type actions. Cached here so that we can unregister it during shutdown. */
    TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
};
