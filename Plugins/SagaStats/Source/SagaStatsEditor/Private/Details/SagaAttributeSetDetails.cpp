/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Details/SagaAttributeSetDetails.h"

#include "AttributeSet.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "EdGraphSchema_K2.h"
#include "SSEditorLog.h"
#include "Details/Slate/SSSNewAttributeVariableWidget.h"
#include "Details/Slate/SSagaPositiveActionButton.h"
#include "Editor/SagaAttributeSetBlueprintEditor.h"
#include "Engine/Blueprint.h"
#include "Slate/SSNewAttributeViewModel.h"
#include "Styling/SSAppStyle.h"
#include "Utils/SagaEditorUtils.h"

#define LOCTEXT_NAMESPACE "FSagaAttributeSetDetails"

TSharedRef<IDetailCustomization> FSagaAttributeSetDetails::MakeInstance()
{
	return MakeShared<FSagaAttributeSetDetails>();
}

void FSagaAttributeSetDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailLayout)
{
	AttributeBeingCustomized = SagaEditorUtils::GetAttributeBeingCustomized(InDetailLayout);
	if (!AttributeBeingCustomized.IsValid())
	{
		return;
	}

	BlueprintBeingCustomized = SagaEditorUtils::GetBlueprintFromClass(AttributeBeingCustomized->GetClass());
	if (!BlueprintBeingCustomized.IsValid())
	{
		return;
	}

	BlueprintEditorPtr = SagaEditorUtils::FindBlueprintEditorForAsset(BlueprintBeingCustomized.Get());
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
		SNew(SSagaPositiveActionButton)
		.ToolTipText(LOCTEXT("NewInputPoseTooltip", "Create a new Attribute"))
		.Icon(FSSAppStyle::GetBrush("Icons.Plus"))
		.Text(LOCTEXT("AddNewLabel", "Add Attribute"))
		.OnGetMenuContent(this, &FSagaAttributeSetDetails::CreateNewAttributeVariableWidget)
	];
	Category.HeaderContent(HeaderContentWidget);
}

TSharedRef<SWidget> FSagaAttributeSetDetails::CreateNewAttributeVariableWidget()
{
	TSharedRef<FSSNewAttributeViewModel> ViewModel = MakeShared<FSSNewAttributeViewModel>();
	ViewModel->SetCustomizedBlueprint(BlueprintBeingCustomized);

	FEdGraphPinType PinType;
	PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	PinType.PinSubCategory = NAME_None;
	PinType.PinSubCategoryObject = FGameplayAttributeData::StaticStruct();

	// Init from last saved variable states
	if (const TSharedPtr<FSagaAttributeSetBlueprintEditor> BlueprintEditor = BlueprintEditorPtr.Pin()) 
	{
		ViewModel->SetVariableName(BlueprintEditor->GetLastUsedVariableName());
		ViewModel->SetbIsReplicated(BlueprintEditor->GetLastReplicatedState());
		if (BlueprintEditor->GetLastPinSubCategoryObject().IsValid())
		{
			PinType.PinSubCategoryObject = BlueprintEditor->GetLastPinSubCategoryObject();
		}
	}
	ViewModel->SetPinType(PinType);

	return SNew(SSSNewAttributeVariableWidget, ViewModel)
		.OnCancel_Static(&FSagaAttributeSetDetails::HandleAttributeWindowCancel)
		.OnFinish(this, &FSagaAttributeSetDetails::HandleAttributeWindowFinish);
}

void FSagaAttributeSetDetails::HandleAttributeWindowCancel(const TSharedPtr<FSSNewAttributeViewModel>& InViewModel)
{
	check(InViewModel.IsValid());
	SS_EDITOR_LOG(Verbose, TEXT("Cancel -> %s"), *InViewModel->ToString())
}

void FSagaAttributeSetDetails::HandleAttributeWindowFinish(const TSharedPtr<FSSNewAttributeViewModel>& InViewModel) const
{
	check(InViewModel.IsValid());
	SS_EDITOR_LOG(Verbose, TEXT("Finish -> %s"), *InViewModel->ToString())

	if (!BlueprintBeingCustomized.IsValid())
	{
		SS_EDITOR_LOG(Error, TEXT("Failed to add new variable to blueprint because BlueprintBeingCustomized is null"))
		return;
	}

	if (const TSharedPtr<FSagaAttributeSetBlueprintEditor> BlueprintEditor = BlueprintEditorPtr.Pin()) 
	{
		BlueprintEditor->SetLastPinSubCategoryObject(InViewModel->GetPinType().PinSubCategoryObject);
		BlueprintEditor->SetLastReplicatedState(InViewModel->GetbIsReplicated());
	}

	SSSNewAttributeVariableWidget::AddMemberVariable(
		BlueprintBeingCustomized.Get(),
		InViewModel->GetVariableName(),
		InViewModel->GetPinType(),
		InViewModel->GetDescription(),
		InViewModel->GetbIsReplicated()
	);
}

#undef LOCTEXT_NAMESPACE
