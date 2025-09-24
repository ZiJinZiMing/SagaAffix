/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "Details/SGAttributeSetDetails.h"

#include "AttributeSet.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "EdGraphSchema_K2.h"
#include "SGEditorLog.h"
#include "Details/Slate/SSGNewAttributeVariableWidget.h"
#include "Details/Slate/SSGPositiveActionButton.h"
#include "Editor/SGAttributeSetBlueprintEditor.h"
#include "Engine/Blueprint.h"
#include "Slate/SGNewAttributeViewModel.h"
#include "Styling/SGAppStyle.h"
#include "Utils/SGEditorUtils.h"

#define LOCTEXT_NAMESPACE "FSGAttributeSetDetails"

TSharedRef<IDetailCustomization> FSGAttributeSetDetails::MakeInstance()
{
	return MakeShared<FSGAttributeSetDetails>();
}

void FSGAttributeSetDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailLayout)
{
	AttributeBeingCustomized = SGEditorUtils::GetAttributeBeingCustomized(InDetailLayout);
	if (!AttributeBeingCustomized.IsValid())
	{
		return;
	}

	BlueprintBeingCustomized = SGEditorUtils::GetBlueprintFromClass(AttributeBeingCustomized->GetClass());
	if (!BlueprintBeingCustomized.IsValid())
	{
		return;
	}

	BlueprintEditorPtr = SGEditorUtils::FindBlueprintEditorForAsset(BlueprintBeingCustomized.Get());
	if (!BlueprintEditorPtr.IsValid())
	{
		return;
	}

	IDetailCategoryBuilder& Category = InDetailLayout.EditCategory("Default", LOCTEXT("DefaultsCategory", "Variables"));
	Category.RestoreExpansionState(false);

	const TSharedRef<SHorizontalBox> HeaderContentWidget = SNew(SHorizontalBox);
	HeaderContentWidget->AddSlot()
	[
		SNew(SHorizontalBox)
	];

	HeaderContentWidget->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	[
		SNew(SSGPositiveActionButton)
		.ToolTipText(LOCTEXT("NewInputPoseTooltip", "Create a new Attribute"))
		.Icon(FSGAppStyle::GetBrush("Icons.Plus"))
		.Text(LOCTEXT("AddNewLabel", "Add Attribute"))
		.OnGetMenuContent(this, &FSGAttributeSetDetails::CreateNewAttributeVariableWidget)
	];
	Category.HeaderContent(HeaderContentWidget);
}

TSharedRef<SWidget> FSGAttributeSetDetails::CreateNewAttributeVariableWidget()
{
	TSharedRef<FSGNewAttributeViewModel> ViewModel = MakeShared<FSGNewAttributeViewModel>();
	ViewModel->SetCustomizedBlueprint(BlueprintBeingCustomized);

	FEdGraphPinType PinType;
	PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	PinType.PinSubCategory = NAME_None;
	PinType.PinSubCategoryObject = FGameplayAttributeData::StaticStruct();

	// Init from last saved variable states
	if (const TSharedPtr<FSGAttributeSetBlueprintEditor> BlueprintEditor = BlueprintEditorPtr.Pin()) 
	{
		ViewModel->SetVariableName(BlueprintEditor->GetLastUsedVariableName());
		ViewModel->SetbIsReplicated(BlueprintEditor->GetLastReplicatedState());
		if (BlueprintEditor->GetLastPinSubCategoryObject().IsValid())
		{
			PinType.PinSubCategoryObject = BlueprintEditor->GetLastPinSubCategoryObject();
		}
	}
	ViewModel->SetPinType(PinType);

	return SNew(SSGNewAttributeVariableWidget, ViewModel)
		.OnCancel_Static(&FSGAttributeSetDetails::HandleAttributeWindowCancel)
		.OnFinish(this, &FSGAttributeSetDetails::HandleAttributeWindowFinish);
}

void FSGAttributeSetDetails::HandleAttributeWindowCancel(const TSharedPtr<FSGNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());
	SG_EDITOR_LOG(Verbose, TEXT("Cancel -> %s"), *InViewModel->ToString())
}

void FSGAttributeSetDetails::HandleAttributeWindowFinish(const TSharedPtr<FSGNewAttributeViewModel>& InViewModel) const
{
	check(InViewModel.IsValid());
	SG_EDITOR_LOG(Verbose, TEXT("Finish -> %s"), *InViewModel->ToString())

	if (!BlueprintBeingCustomized.IsValid())
	{
		SG_EDITOR_LOG(Error, TEXT("Failed to add new variable to blueprint because BlueprintBeingCustomized is null"))
		return;
	}

	if (const TSharedPtr<FSGAttributeSetBlueprintEditor> BlueprintEditor = BlueprintEditorPtr.Pin()) 
	{
		BlueprintEditor->SetLastPinSubCategoryObject(InViewModel->GetPinType().PinSubCategoryObject);
		BlueprintEditor->SetLastReplicatedState(InViewModel->GetbIsReplicated());
	}

	SSGNewAttributeVariableWidget::AddMemberVariable(
		BlueprintBeingCustomized.Get(),
		InViewModel->GetVariableName(),
		InViewModel->GetPinType(),
		InViewModel->GetDescription(),
		InViewModel->GetbIsReplicated()
	);
}

#undef LOCTEXT_NAMESPACE
