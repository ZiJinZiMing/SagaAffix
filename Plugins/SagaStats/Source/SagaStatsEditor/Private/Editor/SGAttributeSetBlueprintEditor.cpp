/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "Editor/SGAttributeSetBlueprintEditor.h"

#include "AttributeSet.h"
#include "EdGraphSchema_K2.h"
#include "SGDelegates.h"
#include "SGEditorLog.h"
#include "AttributeSet/SGAttributeSetBlueprint.h"
#include "Details/Slate/SGNewAttributeViewModel.h"
#include "Details/Slate/SSGNewAttributeVariableWidget.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "SGAttributeSetBlueprintEditor"

FSGAttributeSetBlueprintEditor::FSGAttributeSetBlueprintEditor()
{
}

FSGAttributeSetBlueprintEditor::~FSGAttributeSetBlueprintEditor()
{
	if (AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow.Reset();
	}
}

void FSGAttributeSetBlueprintEditor::InitAttributeSetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, const bool bShouldOpenInDefaultsMode)
{
	InitBlueprintEditor(Mode, InitToolkitHost, InBlueprints, bShouldOpenInDefaultsMode);

	const TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	ToolbarExtender->AddToolBarExtension(
		TEXT("Settings"),
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FSGAttributeSetBlueprintEditor::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);
	RegenerateMenusAndToolbars();

	if (InBlueprints.IsValidIndex(0))
	{
		if (USGAttributeSetBlueprint* Blueprint = Cast<USGAttributeSetBlueprint>(InBlueprints[0]))
		{
			Blueprint->RegisterDelegates();
		}
	}
}

void FSGAttributeSetBlueprintEditor::Compile()
{
	const UBlueprint* Blueprint = GetBlueprintObj();
	
	SG_EDITOR_NS_LOG(VeryVerbose, TEXT("PreCompile for %s"), *GetNameSafe(Blueprint))
	if (Blueprint)
	{
		if (const UPackage* Package = Blueprint->GetPackage())
		{
			FSGDelegates::OnPreCompile.Broadcast(Package->GetFName());
		}
	}
	
	FBlueprintEditor::Compile();
	SG_EDITOR_NS_LOG(VeryVerbose, TEXT("PostCompile for %s"), *GetNameSafe(Blueprint))

	// Bring toolkit back to front, cause USGEditorSubsystem will close any GE referencers and re-open
	// And the re-open will always focus the last Gameplay Effect BP, this focus window will happen after and make sure we don't loose focus
	// when we click the Compile button (but won't handle compile "in background" when hitting Play and some BP are automatically compiled by the editor)
	FocusWindow();
}

void FSGAttributeSetBlueprintEditor::InitToolMenuContext(FToolMenuContext& MenuContext)
{
	FBlueprintEditor::InitToolMenuContext(MenuContext);
}

FName FSGAttributeSetBlueprintEditor::GetToolkitFName() const
{
	return FName("SGAttributeSetBlueprintEditor");
}

FText FSGAttributeSetBlueprintEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AttributeSetEditorAppLabel", "Saga Gameplay Attributes Editor");
}

FText FSGAttributeSetBlueprintEditor::GetToolkitToolTipText() const
{
	const UObject* EditingObject = GetEditingObject();

	check(EditingObject != nullptr);

	return GetToolTipTextForObject(EditingObject);
}

FString FSGAttributeSetBlueprintEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("SGAttributeSetBlueprintEditor");
}

FLinearColor FSGAttributeSetBlueprintEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FSGAttributeSetBlueprintEditor::GetDocumentationLink() const
{
	return FBlueprintEditor::GetDocumentationLink(); // todo: point this at the correct documentation
}

TWeakObjectPtr<UObject> FSGAttributeSetBlueprintEditor::GetLastPinSubCategoryObject() const
{
	return LastPinSubCategoryObject;
}

void FSGAttributeSetBlueprintEditor::SetLastPinSubCategoryObject(const TWeakObjectPtr<UObject>& InLastPinSubCategoryObject)
{
	LastPinSubCategoryObject = InLastPinSubCategoryObject;
}

bool FSGAttributeSetBlueprintEditor::GetLastReplicatedState() const
{
	return bLastReplicatedState;
}

void FSGAttributeSetBlueprintEditor::SetLastReplicatedState(const bool bInLastReplicatedState)
{
	bLastReplicatedState = bInLastReplicatedState;
}

FString FSGAttributeSetBlueprintEditor::GetLastUsedVariableName() const
{
	return LastUsedVariableName;
}

void FSGAttributeSetBlueprintEditor::SetLastUsedVariableName(const FString& InLastUsedVariableName)
{
	LastUsedVariableName = InLastUsedVariableName;
}

void FSGAttributeSetBlueprintEditor::FillToolbar(FToolBarBuilder& InToolbarBuilder)
{
	InToolbarBuilder.BeginSection(TEXT("SagaAttributes"));
	{
		
		InToolbarBuilder.AddComboButton(
			FUIAction(),
			FOnGetContent::CreateSP(this, &FSGAttributeSetBlueprintEditor::GenerateToolbarMenu),
			LOCTEXT("ToolbarAddLabel", "Add Attribute"),
			LOCTEXT("ToolbarAddToolTip", "Create a new Attribute"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus"),
			false
		);
		
		/* TODO：DataTable功能和生成C++功能
		InToolbarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateSP(this, &FSGAttributeSetBlueprintEditor::CreateDataTableWindow)),
			NAME_None,
			LOCTEXT("ToolbarGenerateDataTableLabel", "Create DataTable"),
			LOCTEXT("ToolbarGenerateDataTableTooltip", "Automatically generate a DataTable from this Blueprint Attributes properties"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.DataTable")
		);

		InToolbarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateSP(this, &FSGAttributeSetBlueprintEditor::CreateAttributeWizard)),
			NAME_None,
			LOCTEXT("ToolbarGenerateCPPLabel", "Generate Equivalent C++"),
			LOCTEXT(
				"ToolbarGenerateCPPTooltip",
				"Provides a preview of what this class could look like in C++, "
				"and the ability to generate C++ header / source files from this Blueprint."
			),
			FSlateIcon(FSSEditorStyle::Get().GetStyleSetName(), "Icons.HeaderView")
		);
		*/
		
	}
	InToolbarBuilder.EndSection();
}

TSharedRef<SWidget> FSGAttributeSetBlueprintEditor::GenerateToolbarMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	TSharedRef<FSGNewAttributeViewModel> ViewModel = MakeShared<FSGNewAttributeViewModel>();
	ViewModel->SetCustomizedBlueprint(GetBlueprintObj());
	ViewModel->SetVariableName(LastUsedVariableName);
	ViewModel->SetbIsReplicated(bLastReplicatedState);

	FEdGraphPinType PinType;
	PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	PinType.PinSubCategory = NAME_None;
	PinType.PinSubCategoryObject = LastPinSubCategoryObject.IsValid() ? LastPinSubCategoryObject.Get() : FGameplayAttributeData::StaticStruct();
	ViewModel->SetPinType(PinType);

	TSharedRef<SSGNewAttributeVariableWidget> Widget = SNew(SSGNewAttributeVariableWidget, ViewModel)
		.OnCancel_Static(&FSGAttributeSetBlueprintEditor::HandleAttributeWindowCancel)
		.OnFinish(this, &FSGAttributeSetBlueprintEditor::HandleAttributeWindowFinish);

	MenuBuilder.AddWidget(Widget, FText::GetEmpty());
	return MenuBuilder.MakeWidget();
}

void FSGAttributeSetBlueprintEditor::HandleAttributeWindowCancel(const TSharedPtr<FSGNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());
}

void FSGAttributeSetBlueprintEditor::HandleAttributeWindowFinish(const TSharedPtr<FSGNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());

	LastPinSubCategoryObject = InViewModel->GetPinType().PinSubCategoryObject;
	bLastReplicatedState = InViewModel->GetbIsReplicated();
	LastUsedVariableName = InViewModel->GetVariableName();

	SSGNewAttributeVariableWidget::AddMemberVariable(
		GetBlueprintObj(),
		InViewModel->GetVariableName(),
		InViewModel->GetPinType(),
		InViewModel->GetDescription(),
		InViewModel->GetbIsReplicated()
	);
}
/*

void FSGAttributeSetBlueprintEditor::CreateAttributeWizard()
{
	const FSSAttributeWindowArgs WindowArgs;
	const FAssetData AssetData(GetBlueprintObj());
	if (!AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow = ISSScaffoldModule::Get().CreateAttributeWizard(AssetData, WindowArgs);
		AttributeWizardWindow->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &FSGAttributeSetBlueprintEditor::HandleAttributeWizardClosed));
	}
	else
	{
		AttributeWizardWindow->BringToFront();
	}
}

// ReSharper disable once CppParameterNeverUsed
void FSGAttributeSetBlueprintEditor::HandleAttributeWizardClosed(const TSharedRef<SWindow>& InWindow)
{
	if (AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow.Reset();
	}
}


void FSGAttributeSetBlueprintEditor::CreateDataTableWindow()
{
	if (!DataTableWindow.IsValid())
	{
		const FSSDataTableWindowArgs WindowArgs;
		DataTableWindow = ISSEditorModule::Get().CreateDataTableWindow(GetBlueprintObj(), WindowArgs);
		DataTableWindow->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &FSGAttributeSetBlueprintEditor::HandleDataTableWindowClosed));
	}
	else
	{
		DataTableWindow->BringToFront();
	}
}

// ReSharper disable once CppParameterNeverUsed
void FSGAttributeSetBlueprintEditor::HandleDataTableWindowClosed(const TSharedRef<SWindow>& InWindow)
{
	if (DataTableWindow.IsValid())
	{
		DataTableWindow.Reset();
	}
}
*/

#undef LOCTEXT_NAMESPACE
