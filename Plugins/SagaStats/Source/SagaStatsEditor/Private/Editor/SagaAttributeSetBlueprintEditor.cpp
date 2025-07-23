/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Editor/SagaAttributeSetBlueprintEditor.h"

#include "AttributeSet.h"
#include "EdGraphSchema_K2.h"
#include "SagaStatsDelegates.h"
#include "SSEditorLog.h"
#include "SagaAttributeSetBlueprint.h"
#include "Details/Slate/SSNewAttributeViewModel.h"
#include "Details/Slate/SSSNewAttributeVariableWidget.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "SSBlueprintEditor"

FSagaAttributeSetBlueprintEditor::FSagaAttributeSetBlueprintEditor()
{
}

FSagaAttributeSetBlueprintEditor::~FSagaAttributeSetBlueprintEditor()
{
	if (AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow.Reset();
	}
}

void FSagaAttributeSetBlueprintEditor::InitAttributeSetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, const bool bShouldOpenInDefaultsMode)
{
	SS_EDITOR_LOG(Verbose, TEXT("FSagaBlueprintEditor::InitAttributeSetEditor"))
	InitBlueprintEditor(Mode, InitToolkitHost, InBlueprints, bShouldOpenInDefaultsMode);

	const TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	ToolbarExtender->AddToolBarExtension(
		TEXT("Settings"),
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FSagaAttributeSetBlueprintEditor::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);
	RegenerateMenusAndToolbars();

	if (InBlueprints.IsValidIndex(0))
	{
		if (USagaAttributeSetBlueprint* Blueprint = Cast<USagaAttributeSetBlueprint>(InBlueprints[0]))
		{
			Blueprint->RegisterDelegates();
		}
	}
}

void FSagaAttributeSetBlueprintEditor::Compile()
{
	const UBlueprint* Blueprint = GetBlueprintObj();
	
	SS_EDITOR_LOG(VeryVerbose, TEXT("FSagaBlueprintEditor::Compile - PreCompile for %s"), *GetNameSafe(Blueprint))
	if (Blueprint)
	{
		if (const UPackage* Package = Blueprint->GetPackage())
		{
			FSagaStatsDelegates::OnPreCompile.Broadcast(Package->GetFName());
		}
	}
	
	FBlueprintEditor::Compile();
	SS_EDITOR_LOG(VeryVerbose, TEXT("FSagaBlueprintEditor::Compile - PostCompile for %s"), *GetNameSafe(Blueprint))

	// Bring toolkit back to front, cause USagaEditorSubsystem will close any GE referencers and re-open
	// And the re-open will always focus the last Gameplay Effect BP, this focus window will happen after and make sure we don't loose focus
	// when we click the Compile button (but won't handle compile "in background" when hitting Play and some BP are automatically compiled by the editor)
	FocusWindow();
}

void FSagaAttributeSetBlueprintEditor::InitToolMenuContext(FToolMenuContext& MenuContext)
{
	FBlueprintEditor::InitToolMenuContext(MenuContext);
}

FName FSagaAttributeSetBlueprintEditor::GetToolkitFName() const
{
	return FName("SSBlueprintEditor");
}

FText FSagaAttributeSetBlueprintEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AttributeSetEditorAppLabel", "Gameplay Attributes Editor");
}

FText FSagaAttributeSetBlueprintEditor::GetToolkitToolTipText() const
{
	const UObject* EditingObject = GetEditingObject();

	check(EditingObject != nullptr);

	return GetToolTipTextForObject(EditingObject);
}

FString FSagaAttributeSetBlueprintEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("BlueprintAttributeSetEditor");
}

FLinearColor FSagaAttributeSetBlueprintEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FSagaAttributeSetBlueprintEditor::GetDocumentationLink() const
{
	return FBlueprintEditor::GetDocumentationLink(); // todo: point this at the correct documentation
}

TWeakObjectPtr<UObject> FSagaAttributeSetBlueprintEditor::GetLastPinSubCategoryObject() const
{
	return LastPinSubCategoryObject;
}

void FSagaAttributeSetBlueprintEditor::SetLastPinSubCategoryObject(const TWeakObjectPtr<UObject>& InLastPinSubCategoryObject)
{
	LastPinSubCategoryObject = InLastPinSubCategoryObject;
}

bool FSagaAttributeSetBlueprintEditor::GetLastReplicatedState() const
{
	return bLastReplicatedState;
}

void FSagaAttributeSetBlueprintEditor::SetLastReplicatedState(const bool bInLastReplicatedState)
{
	bLastReplicatedState = bInLastReplicatedState;
}

FString FSagaAttributeSetBlueprintEditor::GetLastUsedVariableName() const
{
	return LastUsedVariableName;
}

void FSagaAttributeSetBlueprintEditor::SetLastUsedVariableName(const FString& InLastUsedVariableName)
{
	LastUsedVariableName = InLastUsedVariableName;
}

void FSagaAttributeSetBlueprintEditor::FillToolbar(FToolBarBuilder& InToolbarBuilder)
{
	InToolbarBuilder.BeginSection(TEXT("BlueprintAttributes"));
	{
		
		InToolbarBuilder.AddComboButton(
			FUIAction(),
			FOnGetContent::CreateSP(this, &FSagaAttributeSetBlueprintEditor::GenerateToolbarMenu),
			LOCTEXT("ToolbarAddLabel", "Add Attribute"),
			LOCTEXT("ToolbarAddToolTip", "Create a new Attribute"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus"),
			false
		);
		
		/* TODO：DataTable功能和生成C++功能
		InToolbarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateSP(this, &FSagaBlueprintEditor::CreateDataTableWindow)),
			NAME_None,
			LOCTEXT("ToolbarGenerateDataTableLabel", "Create DataTable"),
			LOCTEXT("ToolbarGenerateDataTableTooltip", "Automatically generate a DataTable from this Blueprint Attributes properties"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.DataTable")
		);

		InToolbarBuilder.AddToolBarButton(
			FUIAction(FExecuteAction::CreateSP(this, &FSagaBlueprintEditor::CreateAttributeWizard)),
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

TSharedRef<SWidget> FSagaAttributeSetBlueprintEditor::GenerateToolbarMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	TSharedRef<FSSNewAttributeViewModel> ViewModel = MakeShared<FSSNewAttributeViewModel>();
	ViewModel->SetCustomizedBlueprint(GetBlueprintObj());
	ViewModel->SetVariableName(LastUsedVariableName);
	ViewModel->SetbIsReplicated(bLastReplicatedState);

	FEdGraphPinType PinType;
	PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	PinType.PinSubCategory = NAME_None;
	PinType.PinSubCategoryObject = LastPinSubCategoryObject.IsValid() ? LastPinSubCategoryObject.Get() : FGameplayAttributeData::StaticStruct();
	ViewModel->SetPinType(PinType);

	TSharedRef<SSSNewAttributeVariableWidget> Widget = SNew(SSSNewAttributeVariableWidget, ViewModel)
		.OnCancel_Static(&FSagaAttributeSetBlueprintEditor::HandleAttributeWindowCancel)
		.OnFinish(this, &FSagaAttributeSetBlueprintEditor::HandleAttributeWindowFinish);

	MenuBuilder.AddWidget(Widget, FText::GetEmpty());
	return MenuBuilder.MakeWidget();
}

void FSagaAttributeSetBlueprintEditor::HandleAttributeWindowCancel(const TSharedPtr<FSSNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());
}

void FSagaAttributeSetBlueprintEditor::HandleAttributeWindowFinish(const TSharedPtr<FSSNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());

	LastPinSubCategoryObject = InViewModel->GetPinType().PinSubCategoryObject;
	bLastReplicatedState = InViewModel->GetbIsReplicated();
	LastUsedVariableName = InViewModel->GetVariableName();

	SSSNewAttributeVariableWidget::AddMemberVariable(
		GetBlueprintObj(),
		InViewModel->GetVariableName(),
		InViewModel->GetPinType(),
		InViewModel->GetDescription(),
		InViewModel->GetbIsReplicated()
	);
}
/*

void FSagaBlueprintEditor::CreateAttributeWizard()
{
	const FSSAttributeWindowArgs WindowArgs;
	const FAssetData AssetData(GetBlueprintObj());
	if (!AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow = ISSScaffoldModule::Get().CreateAttributeWizard(AssetData, WindowArgs);
		AttributeWizardWindow->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &FSagaBlueprintEditor::HandleAttributeWizardClosed));
	}
	else
	{
		AttributeWizardWindow->BringToFront();
	}
}

// ReSharper disable once CppParameterNeverUsed
void FSagaBlueprintEditor::HandleAttributeWizardClosed(const TSharedRef<SWindow>& InWindow)
{
	if (AttributeWizardWindow.IsValid())
	{
		AttributeWizardWindow.Reset();
	}
}


void FSagaBlueprintEditor::CreateDataTableWindow()
{
	if (!DataTableWindow.IsValid())
	{
		const FSSDataTableWindowArgs WindowArgs;
		DataTableWindow = ISSEditorModule::Get().CreateDataTableWindow(GetBlueprintObj(), WindowArgs);
		DataTableWindow->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &FSagaBlueprintEditor::HandleDataTableWindowClosed));
	}
	else
	{
		DataTableWindow->BringToFront();
	}
}

// ReSharper disable once CppParameterNeverUsed
void FSagaBlueprintEditor::HandleDataTableWindowClosed(const TSharedRef<SWindow>& InWindow)
{
	if (DataTableWindow.IsValid())
	{
		DataTableWindow.Reset();
	}
}
*/

#undef LOCTEXT_NAMESPACE
