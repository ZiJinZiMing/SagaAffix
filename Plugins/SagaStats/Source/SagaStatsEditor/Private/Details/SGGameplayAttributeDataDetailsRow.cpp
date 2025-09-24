/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "Details/SGGameplayAttributeDataDetailsRow.h"

#include "AttributeSet.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "SGEditorLog.h"
#include "IDetailChildrenBuilder.h"
#include "Details/ISGGameplayAttributeDataDetailsBase.h"
#include "ScopedTransaction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "SagaGameplayAttributeDataDetails"

FSGGameplayAttributeDataDetailsRow::FSGGameplayAttributeDataDetailsRow(const TWeakPtr<ISGGameplayAttributeDataDetailsBase>& InDetailsOwner, const FText& InRowNameText, const float InInitialValue)
	: LastSliderCommittedValue(InInitialValue)
	, DetailsOwnerPtr(InDetailsOwner)
	, RowNameText(InRowNameText)
{
}

void FSGGameplayAttributeDataDetailsRow::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
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
		.Value(this, &FSGGameplayAttributeDataDetailsRow::OnGetValue)
		// Because we have no lower or upper limit in MaxSliderValue, we need to "unspecify" the max value here,
		// otherwise the spinner has a limited range, with TNumericLimits<NumericType>::Max() as the MaxValue
		// and the spinning increment is huge
		.MinValue(TOptional<float>())
		.MaxValue(TOptional<float>())
		.MinSliderValue(TOptional<float>())
		.MaxSliderValue(TOptional<float>())
		.OnBeginSliderMovement(this, &FSGGameplayAttributeDataDetailsRow::OnBeginSliderMovement)
		.OnEndSliderMovement(this, &FSGGameplayAttributeDataDetailsRow::OnEndSliderMovement)
		.OnValueCommitted(this, &FSGGameplayAttributeDataDetailsRow::OnValueCommitted)
		.OnValueChanged(this, &FSGGameplayAttributeDataDetailsRow::OnValueCommitted, ETextCommit::Default)
		.AllowSpin(true)
	];
}

TOptional<float> FSGGameplayAttributeDataDetailsRow::OnGetValue() const
{
	return LastSliderCommittedValue;
}

void FSGGameplayAttributeDataDetailsRow::OnBeginSliderMovement()
{
	bIsUsingSlider = true;
}

void FSGGameplayAttributeDataDetailsRow::OnEndSliderMovement(const float InValue)
{
	bIsUsingSlider = false;
	SG_EDITOR_LOG(VeryVerbose, TEXT("End slider with %f (LastSliderCommittedValue: %f)"), InValue, LastSliderCommittedValue)
	SetValueWithTransaction(InValue);
}

// ReSharper disable once CppParameterNeverUsed
void FSGGameplayAttributeDataDetailsRow::OnValueCommitted(const float InNewValue, ETextCommit::Type InCommitInfo)
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("OnValueCommitted: %f"), InNewValue)
	if (!bIsUsingSlider)
	{
		SetValueWithTransaction(InNewValue);
	}

	LastSliderCommittedValue = InNewValue;
}

void FSGGameplayAttributeDataDetailsRow::SetValueWithTransaction(const float InNewValue) const
{
	const TSharedPtr<ISGGameplayAttributeDataDetailsBase> DetailsOwner = DetailsOwnerPtr.Pin();
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