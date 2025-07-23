/******************************************************************************
* ProjectName:  SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************/

#include "SagaStatsEditor.h"
#include "AssetToolsModule.h"
#include "SSEditorLog.h"
#include "PropertyEditorModule.h"
#include "SagaEditorSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetTypes/SagaAssetTypeActions_AttributeSet.h"
#include "Details/SagaAttributeSetDetails.h"
#include "Details/SagaAttributeDataClampedDetails.h"
#include "Details/SagaGameplayAttributeDataDetails.h"
#include "Details/SagaGameplayAttributeDetails.h"
#include "Editor/SSGraphPanelPinFactory.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/EngineVersionComparison.h"
#include "ReferencerHandlers/FSSSwitchNodeReferencerHandler.h"
#define LOCTEXT_NAMESPACE "FSagaStatsEditorModule"

void FSagaStatsEditorModule::StartupModule()
{
	// Module is set to kick in in PreLoadingScreen
	// so that it starts right before GameplayAbilitiesEditorModule (which is PreDefault)
	//
	// Mainly to hook in our own GraphPanelPinFactory to return our version of SGraphPing widget, since we can't really unregister
	// the visual pin factory of Gameplay Abilities Editor module from outside modules.
	//
	// Registering ours earlier so that editor considers it before evaluating the default one (we need to expose Attributes added in BP for K2 Nodes)
	//
	SS_EDITOR_LOG(Verbose, TEXT("FSagaStatsEditorModule::StartupModule"))


	// Every other logic that would have happened in here is delayed to OnPostEngineInit
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FSagaStatsEditorModule::OnPostEngineInit);

	// Register factories for pins and nodes
	//
	// That is for K2 nodes with FGameplayAttribute pins, like GetFloatAttributeBase from ASC
	GameplayAbilitiesGraphPanelPinFactory = MakeShared<FSSGraphPanelPinFactory>();
	FEdGraphUtilities::RegisterVisualPinFactory(GameplayAbilitiesGraphPanelPinFactory);
}

void FSagaStatsEditorModule::ShutdownModule()
{
	SS_EDITOR_LOG(Verbose, TEXT("FSagaStatsEditorModule::ShutdownModule"))

	FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	

	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded(TEXT("PropertyEditor")))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("GameplayAttribute"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("GameplayAttributeData"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("SagaClampedGameplayAttributeData"));

		PropertyModule.UnregisterCustomClassLayout(TEXT("SagaAttributeSet"));
	}

	// Unregister asset type actions
	if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetTools")))
	{
		IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
		for (TSharedPtr<IAssetTypeActions>& AssetTypeAction : CreatedAssetTypeActions)
		{
			if (AssetTypeAction.IsValid())
			{
				AssetToolsModule.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
			}
		}
	}
	CreatedAssetTypeActions.Empty();

	// Unregister graph factories
	if (GameplayAbilitiesGraphPanelPinFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(GameplayAbilitiesGraphPanelPinFactory);
		GameplayAbilitiesGraphPanelPinFactory.Reset();
	}

	if (GEditor)
	{
		USagaEditorSubsystem::Get().UnregisterReferencerHandler(TEXT("SagaK2Node_SwitchGameplayAttribute"));
	}
}

void FSagaStatsEditorModule::PreloadAssetsByClass(UClass* InClass) const
{
	SS_EDITOR_LOG(Verbose, TEXT("FSagaStatsEditorModule::PreloadAssetsByClass - Preloading assets with class %s"), *GetNameSafe(InClass))
	if (!InClass)
	{
		return;
	}

	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	TArray<FAssetData> Assets;
#if UE_VERSION_NEWER_THAN(5, 1, -1)
	AssetRegistry.GetAssetsByClass(InClass->GetClassPathName(), Assets, true);
#else
	AssetRegistry.GetAssetsByClass(InClass->GetFName(), Assets, true);
#endif

	SS_EDITOR_LOG(Verbose, TEXT("FSagaStatsEditorModule::PreloadAssetsByClass - Preloading %d assets with class %s"), Assets.Num(), *GetNameSafe(InClass))
	for (const FAssetData& Asset : Assets)
	{
		SS_EDITOR_LOG(Verbose, TEXT("\nFSagaStatsEditorModule::PreloadAssetsByClass Preload asset PackageName: %s"), *Asset.PackageName.ToString())
		if (!Asset.IsAssetLoaded())
		{
			Asset.GetAsset();
		}
	}
}

void FSagaStatsEditorModule::OnPostEngineInit()
{
	// Detail Customizations
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

		// Unregister GAS default customization for Gameplay Attributes and Sets
		PropertyModule.UnregisterCustomClassLayout(TEXT("AttributeSet"));
		PropertyModule.UnregisterCustomPropertyTypeLayout("GameplayAttribute");

		// And register our own customizations
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("GameplayAttribute"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSagaGameplayAttributeDetails::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("GameplayAttributeData"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSagaGameplayAttributeDataDetails::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("SagaClampedGameplayAttributeData"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSagaAttributeDataClampedDetails::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(TEXT("SagaAttributeSet"), FOnGetDetailCustomizationInstance::CreateStatic(&FSagaAttributeSetDetails::MakeInstance));

		// Asset Types
		{
			IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();

			constexpr EAssetTypeCategories::Type AssetCategory = EAssetTypeCategories::Gameplay;
			SS_EDITOR_LOG(Verbose, TEXT("FSagaStatsEditorModule::RegisterAssetTypeAction FSagaAssetTypeActions_AttributeSet"))
			RegisterAssetTypeAction(AssetTools, MakeShared<FSagaAssetTypeActions_AttributeSet>(AssetCategory));
		}
	}

	if (GEditor)
	{
		USagaEditorSubsystem::Get().RegisterReferencerHandler(TEXT("SagaK2Node_SwitchGameplayAttribute"), FSSSwitchNodeReferencerHandler::Create());
	}
}

void FSagaStatsEditorModule::RegisterAssetTypeAction(class IAssetTools& AssetTools,
	TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSagaStatsEditorModule, SagaStatsEditor)