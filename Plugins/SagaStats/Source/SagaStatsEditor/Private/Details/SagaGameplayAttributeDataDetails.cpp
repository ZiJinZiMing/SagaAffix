/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Details/SagaGameplayAttributeDataDetails.h"

#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "SSEditorLog.h"
#include "Details/SSGameplayAttributeDataDetailsRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SagaGameplayAttributeDataDetails"

FSagaGameplayAttributeDataDetails::FSagaGameplayAttributeDataDetails()
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("Construct FSagaGameplayAttributeDataDetails ..."))
}

FSagaGameplayAttributeDataDetails::~FSagaGameplayAttributeDataDetails()
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("Destroyed FSagaGameplayAttributeDataDetails ..."))
	AttributeDataBeingCustomized.Reset();
}

TSharedRef<IPropertyTypeCustomization> FSagaGameplayAttributeDataDetails::MakeInstance()
{
	TSharedRef<FSagaGameplayAttributeDataDetails> Details = MakeShared<FSagaGameplayAttributeDataDetails>();
	Details->Initialize();
	return Details;
}

void FSagaGameplayAttributeDataDetails::InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	ISagaGameplayAttributeDataDetailsBase::InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

	if (FGameplayAttribute::IsGameplayAttributeDataProperty(PropertyBeingCustomized.Get()))
	{
		const FStructProperty* StructProperty = CastField<FStructProperty>(PropertyBeingCustomized.Get());
		check(StructProperty);

		if (FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(AttributeSetBeingCustomized.Get()))
		{
			AttributeDataBeingCustomized = MakeShared<FGameplayAttributeData*>(DataPtr);
		}
	}
}

FGameplayAttributeData* FSagaGameplayAttributeDataDetails::GetGameplayAttributeData() const
{
	if (!AttributeDataBeingCustomized.IsValid())
	{
		return nullptr;
	}

	return *AttributeDataBeingCustomized.Get();
}

void FSagaGameplayAttributeDataDetails::CustomizeHeader(const TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("FSagaGameplayAttributeDataDetails::CustomizeHeader ..."))
	InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

	// Check if we want to display details in compact mode
	if (IsCompactView())
	{
		SS_EDITOR_LOG(VeryVerbose, TEXT("FSagaGameplayAttributeDataDetails::CustomizeHeader bUseCompactView: true"))
		// Not adding anything to the row will make the header to not display
		return;
	}

	SS_EDITOR_LOG(VeryVerbose, TEXT("FSagaGameplayAttributeDataDetails::CustomizeHeader bUseCompactView: false"))

	const FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	check(AttributeData);

	InHeaderRow.
		NameContent()
		[
			InStructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(500)
		.MaxDesiredWidth(4096)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			  .HAlign(HAlign_Fill)
			  .Padding(0.f, 0.f, 2.f, 0.f)
			[
				SNew(STextBlock)
				.Text(this, &FSagaGameplayAttributeDataDetails::GetHeaderBaseValueText)
			]
		];
}

void FSagaGameplayAttributeDataDetails::CustomizeChildren(const TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("FSagaGameplayAttributeDataDetails::CustomizeChildren ..."))

	const FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	if (!AttributeData)
	{
		return;
	}

	SS_EDITOR_LOG(VeryVerbose, TEXT("AttributeDataBeingCustomized -> %s: %f"), *GetNameSafe(PropertyBeingCustomized.Get()), AttributeData->GetBaseValue())
	LastSliderCommittedValue = AttributeData->GetBaseValue();

	const FText NameText = IsCompactView() && PropertyBeingCustomized.IsValid() ? PropertyBeingCustomized->GetDisplayNameText() : LOCTEXT("BaseValueLabel", "Base Value");
	BaseValueRow = MakeShared<FSSGameplayAttributeDataDetailsRow>(SharedThis(this), NameText, AttributeData->GetBaseValue());
	BaseValueRow->OnValueChanged().AddSP(this, &FSagaGameplayAttributeDataDetails::HandleBaseValueChanged);
	BaseValueRow->CustomizeChildren(InStructPropertyHandle, InStructBuilder, InStructCustomizationUtils);
}

void FSagaGameplayAttributeDataDetails::HandleBaseValueChanged(const float InValue) const
{
	FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	check(AttributeData);
	
	// Set the new value.
	AttributeData->SetBaseValue(InValue);
	AttributeData->SetCurrentValue(InValue);
}

#undef LOCTEXT_NAMESPACE
