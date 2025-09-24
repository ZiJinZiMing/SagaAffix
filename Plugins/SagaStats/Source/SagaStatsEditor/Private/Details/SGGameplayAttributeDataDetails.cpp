/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "Details/SGGameplayAttributeDataDetails.h"

#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "SGEditorLog.h"
#include "Details/SGGameplayAttributeDataDetailsRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SagaGameplayAttributeDataDetails"

FSGGameplayAttributeDataDetails::FSGGameplayAttributeDataDetails()
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("Construct FSGGameplayAttributeDataDetails ..."))
}

FSGGameplayAttributeDataDetails::~FSGGameplayAttributeDataDetails()
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("Destroyed FSGGameplayAttributeDataDetails ..."))
	AttributeDataBeingCustomized.Reset();
}

TSharedRef<IPropertyTypeCustomization> FSGGameplayAttributeDataDetails::MakeInstance()
{
	TSharedRef<FSGGameplayAttributeDataDetails> Details = MakeShared<FSGGameplayAttributeDataDetails>();
	Details->Initialize();
	return Details;
}

void FSGGameplayAttributeDataDetails::InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	ISGGameplayAttributeDataDetailsBase::InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

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

FGameplayAttributeData* FSGGameplayAttributeDataDetails::GetGameplayAttributeData() const
{
	if (!AttributeDataBeingCustomized.IsValid())
	{
		return nullptr;
	}

	return *AttributeDataBeingCustomized.Get();
}

void FSGGameplayAttributeDataDetails::CustomizeHeader(const TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("FSGGameplayAttributeDataDetails::CustomizeHeader ..."))
	InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

	// Check if we want to display details in compact mode
	if (IsCompactView())
	{
		SG_EDITOR_LOG(VeryVerbose, TEXT("FSGGameplayAttributeDataDetails::CustomizeHeader bUseCompactView: true"))
		// Not adding anything to the row will make the header to not display
		return;
	}

	SG_EDITOR_LOG(VeryVerbose, TEXT("FSGGameplayAttributeDataDetails::CustomizeHeader bUseCompactView: false"))

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
				.Text(this, &FSGGameplayAttributeDataDetails::GetHeaderBaseValueText)
			]
		];
}

void FSGGameplayAttributeDataDetails::CustomizeChildren(const TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("FSGGameplayAttributeDataDetails::CustomizeChildren ..."))

	const FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	if (!AttributeData)
	{
		return;
	}

	SG_EDITOR_LOG(VeryVerbose, TEXT("AttributeDataBeingCustomized -> %s: %f"), *GetNameSafe(PropertyBeingCustomized.Get()), AttributeData->GetBaseValue())
	LastSliderCommittedValue = AttributeData->GetBaseValue();

	const FText NameText = IsCompactView() && PropertyBeingCustomized.IsValid() ? PropertyBeingCustomized->GetDisplayNameText() : LOCTEXT("BaseValueLabel", "Base Value");
	BaseValueRow = MakeShared<FSGGameplayAttributeDataDetailsRow>(SharedThis(this), NameText, AttributeData->GetBaseValue());
	BaseValueRow->OnValueChanged().AddSP(this, &FSGGameplayAttributeDataDetails::HandleBaseValueChanged);
	BaseValueRow->CustomizeChildren(InStructPropertyHandle, InStructBuilder, InStructCustomizationUtils);
}

void FSGGameplayAttributeDataDetails::HandleBaseValueChanged(const float InValue) const
{
	FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	check(AttributeData);
	
	// Set the new value.
	AttributeData->SetBaseValue(InValue);
	AttributeData->SetCurrentValue(InValue);
}

#undef LOCTEXT_NAMESPACE
