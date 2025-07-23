/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Details/SSGameplayAttributeDataDetailsRow.h"

#include "AttributeSet.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "SSEditorLog.h"
#include "IDetailChildrenBuilder.h"
#include "Details/ISagaGameplayAttributeDataDetailsBase.h"
#include "ScopedTransaction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "SagaGameplayAttributeDataDetails"

FSSGameplayAttributeDataDetailsRow::FSSGameplayAttributeDataDetailsRow(const TWeakPtr<ISagaGameplayAttributeDataDetailsBase>& InDetailsOwner, const FText& InRowNameText, const float InInitialValue)
	: LastSliderCommittedValue(InInitialValue)
	, DetailsOwnerPtr(InDetailsOwner)
	, RowNameText(InRowNameText)
{
}

void FSSGameplayAttributeDataDetailsRow::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	FDetailWidgetRow& Row = InStructBuilder.AddCustomRow(RowNameText);
	Row.NameContent()
	[
		SNew(STextBlock)
		.Text(RowNameText)
		.Font(IDetailLayoutBuilder::GetDetailFont())
	];

	Row.ValueContent()
	[
		SNew(SNumericEntryBox<float>)
		.Value(this, &FSSGameplayAttributeDataDetailsRow::OnGetValue)
		// Because we have no lower or upper limit in MaxSliderValue, we need to "unspecify" the max value here,
		// otherwise the spinner has a limited range, with TNumericLimits<NumericType>::Max() as the MaxValue
		// and the spinning increment is huge
		.MinValue(TOptional<float>())
		.MaxValue(TOptional<float>())
		.MinSliderValue(TOptional<float>())
		.MaxSliderValue(TOptional<float>())
		.OnBeginSliderMovement(this, &FSSGameplayAttributeDataDetailsRow::OnBeginSliderMovement)
		.OnEndSliderMovement(this, &FSSGameplayAttributeDataDetailsRow::OnEndSliderMovement)
		.OnValueCommitted(this, &FSSGameplayAttributeDataDetailsRow::OnValueCommitted)
		.OnValueChanged(this, &FSSGameplayAttributeDataDetailsRow::OnValueCommitted, ETextCommit::Default)
		.AllowSpin(true)
	];
}

TOptional<float> FSSGameplayAttributeDataDetailsRow::OnGetValue() const
{
	return LastSliderCommittedValue;
}

void FSSGameplayAttributeDataDetailsRow::OnBeginSliderMovement()
{
	bIsUsingSlider = true;
}

void FSSGameplayAttributeDataDetailsRow::OnEndSliderMovement(const float InValue)
{
	bIsUsingSlider = false;
	SS_EDITOR_LOG(VeryVerbose, TEXT("End slider with %f (LastSliderCommittedValue: %f)"), InValue, LastSliderCommittedValue)
	SetValueWithTransaction(InValue);
}

// ReSharper disable once CppParameterNeverUsed
void FSSGameplayAttributeDataDetailsRow::OnValueCommitted(const float InNewValue, ETextCommit::Type InCommitInfo)
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("OnValueCommitted: %f"), InNewValue)
	if (!bIsUsingSlider)
	{
		SetValueWithTransaction(InNewValue);
	}

	LastSliderCommittedValue = InNewValue;
}

void FSSGameplayAttributeDataDetailsRow::SetValueWithTransaction(const float InNewValue) const
{
	const TSharedPtr<ISagaGameplayAttributeDataDetailsBase> DetailsOwner = DetailsOwnerPtr.Pin();
	check(DetailsOwner.IsValid());

	const FGameplayAttributeData* AttributeData = DetailsOwner->GetGameplayAttributeData();
	check(AttributeData);
	check(DetailsOwner->GetAttributeSetBeingCustomized().IsValid());

	FScopedTransaction Transaction(FText::Format(LOCTEXT("SetRowValueProperty", "Set Attribute Data {0}"), RowNameText));

	DetailsOwner->GetAttributeSetBeingCustomized()->SetFlags(RF_Transactional);
	DetailsOwner->GetAttributeSetBeingCustomized()->Modify();

	// Set the new value.
	OnValueChangedDelegate.Broadcast(InNewValue);

	if (DetailsOwner->GetBlueprintBeingCustomized().IsValid())
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(DetailsOwner->GetBlueprintBeingCustomized().Get());
	}
}

#undef LOCTEXT_NAMESPACE