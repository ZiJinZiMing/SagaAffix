/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#include "SagaStatsEditor.h"
#include "AssetToolsModule.h"
#include "SGEditorLog.h"
#include "PropertyEditorModule.h"
#include "SGEditorSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetTypes/SGAssetTypeActions_AttributeSet.h"
#include "AttributeReferenceViewer/SGAttributeListReferenceViewer.h"
#include "Details/SGAttributeSetDetails.h"
#include "Details/SGAttributeDataClampedDetails.h"
#include "Details/SGGameplayAttributeDataDetails.h"
#include "Details/SGGameplayAttributeDetails.h"
#include "Editor/SGGraphPanelPinFactory.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/EngineVersionComparison.h"
#include "ReferencerHandlers/FSGSwitchNodeReferencerHandler.h"
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
	SG_EDITOR_LOG(Verbose, TEXT("FSagaStatsEditorModule::StartupModule"))
	
	RegisterConsoleCommands();

	// Every other logic that would have happened in here is delayed to OnPostEngineInit
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FSagaStatsEditorModule::OnPostEngineInit);

	// Register factories for pins and nodes
	//
	// That is for K2 nodes with FGameplayAttribute pins, like GetFloatAttributeBase from ASC
	GameplayAbilitiesGraphPanelPinFactory = MakeShared<FSGGraphPanelPinFactory>();
	FEdGraphUtilities::RegisterVisualPinFactory(GameplayAbilitiesGraphPanelPinFactory);
}

void FSagaStatsEditorModule::ShutdownModule()
{
	SG_EDITOR_LOG(Verbose, TEXT("FSagaStatsEditorModule::ShutdownModule"))

	FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	
	UnregisterConsoleCommands();

	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded(TEXT("PropertyEditor")))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("GameplayAttribute"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("GameplayAttributeData"));
		PropertyModule.UnregisterCustomPropertyTypeLayout(TEXT("SGClampedGameplayAttributeData"));

		PropertyModule.UnregisterCustomClassLayout(TEXT("SGAttributeSet"));
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
		USGEditorSubsystem::Get().UnregisterReferencerHandler(TEXT("K2Node_SwitchGameplayAttribute"));
	}
}

void FSagaStatsEditorModule::PreloadAssetsByClass(UClass* InClass) const
{
	SG_EDITOR_NS_LOG(Verbose, TEXT("Preloading assets with class %s"), *GetNameSafe(InClass))
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

	SG_EDITOR_NS_LOG(Verbose, TEXT("Preloading %d assets with class %s"), Assets.Num(), *GetNameSafe(InClass))
	for (const FAssetData& Asset : Assets)
	{
		SG_EDITOR_NS_LOG(Verbose, TEXT("\nPreload asset PackageName: %s"), *Asset.PackageName.ToString())
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
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("GameplayAttribute"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSGGameplayAttributeDetails::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("GameplayAttributeData"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSGGameplayAttributeDataDetails::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout(TEXT("SGClampedGameplayAttributeData"), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSGAttributeDataClampedDetails::MakeInstance));
		
		PropertyModule.RegisterCustomClassLayout(TEXT("SGAttributeSet"), FOnGetDetailCustomizationInstance::CreateStatic(&FSGAttributeSetDetails::MakeInstance));

		// Asset Types
		{
			IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();

			constexpr EAssetTypeCategories::Type AssetCategory = EAssetTypeCategories::Gameplay;
			SG_EDITOR_NS_LOG(Verbose, TEXT("FSGAssetTypeActions_AttributeSet"))
			RegisterAssetTypeAction(AssetTools, MakeShared<FSGAssetTypeActions_AttributeSet>(AssetCategory));
		}
	}

	if (GEditor)
	{
		USGEditorSubsystem::Get().RegisterReferencerHandler(TEXT("K2Node_SwitchGameplayAttribute"), FSGSwitchNodeReferencerHandler::Create());
	}
}

void FSagaStatsEditorModule::RegisterAssetTypeAction(class IAssetTools& AssetTools,
	TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}


void FSagaStatsEditorModule::RegisterConsoleCommands()
{
	
	/* todo:子属性引用存在Bug
	ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Attribute.ReferenceList"),
		TEXT("Display Attribute references dialog"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FSagaStatsEditorModule::ExecuteShowGameplayAttributeReferencesWindow),
		ECVF_Default
	));
	*/
	
}

void FSagaStatsEditorModule::UnregisterConsoleCommands()
{
	for (IConsoleCommand* ConsoleCommand : ConsoleCommands)
	{
		IConsoleManager::Get().UnregisterConsoleObject(ConsoleCommand);
	}

	ConsoleCommands.Empty();
}


void FSagaStatsEditorModule::ExecuteShowGameplayAttributeReferencesWindow(const TArray<FString>& InArgs)
{
	const FString Argv = FString::Join(InArgs, TEXT(" "));

	SG_EDITOR_NS_LOG(Verbose, TEXT("Start command - Search for %s"), *Argv)

	const FVector2D WindowSize(800, 800);

	AttributeListReferenceViewerWidget = SNew(SSGAttributeListReferenceViewer);
	const FText Title = LOCTEXT("AttributeListReferenceViewer_Title", "Attribute Reference Viewer");

	AttributeListReferenceViewerWindow = SNew(SWindow)
		.Title(Title)
		.HasCloseButton(true)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.ClientSize(WindowSize)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.FillHeight(1)
				[
					AttributeListReferenceViewerWidget.ToSharedRef()
				]
			]
		];

	// NOTE: FGlobalTabmanager::Get()-> is actually dereferencing a SharedReference, not a SharedPtr, so it cannot be null.
	if (FGlobalTabmanager::Get()->GetRootWindow().IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(AttributeListReferenceViewerWindow.ToSharedRef(), FGlobalTabmanager::Get()->GetRootWindow().ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(AttributeListReferenceViewerWindow.ToSharedRef());
	}

	check(AttributeListReferenceViewerWidget.IsValid());

	// Set focus to the search box on creation
	FSlateApplication::Get().SetKeyboardFocus(AttributeListReferenceViewerWidget->GetWidgetToFocusOnOpen());
	FSlateApplication::Get().SetUserFocus(0, AttributeListReferenceViewerWidget->GetWidgetToFocusOnOpen());
}


#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSagaStatsEditorModule, SagaStatsEditor)