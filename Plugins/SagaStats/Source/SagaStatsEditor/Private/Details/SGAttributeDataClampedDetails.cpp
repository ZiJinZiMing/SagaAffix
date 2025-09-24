/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "Details/SGAttributeDataClampedDetails.h"

#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "SGEditorLog.h"
#include "Details/SGGameplayAttributeDataDetailsRow.h"
#include "IDetailChildrenBuilder.h"
#include "AttributeSet/SGAttributeSet.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SagaGameplayAttributeDataDetails"

FSGAttributeDataClampedDetails::FSGAttributeDataClampedDetails()
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("Construct FSGAttributeDataClampedDetails ..."))
}

FSGAttributeDataClampedDetails::~FSGAttributeDataClampedDetails()
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("Destroyed FSGAttributeDataClampedDetails ..."))
	// if (MinValueRow.IsValid())
	// {
	// 	MinValueRow->OnValueChanged().RemoveAll(this);
	// 	MinValueRow.Reset();
	// }
	//
	// if (MaxValueRow.IsValid())
	// {
	// 	MaxValueRow->OnValueChanged().RemoveAll(this);
	// 	MaxValueRow.Reset();
	// }
	
	AttributeDataBeingCustomized.Reset();
}

TSharedRef<IPropertyTypeCustomization> FSGAttributeDataClampedDetails::MakeInstance()
{
	TSharedRef<FSGAttributeDataClampedDetails> Details = MakeShared<FSGAttributeDataClampedDetails>();
	Details->Initialize();
	return Details;
}

void FSGAttributeDataClampedDetails::InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	ISGGameplayAttributeDataDetailsBase::InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

	if (!FGameplayAttribute::IsGameplayAttributeDataProperty(PropertyBeingCustomized.Get()))
	{
		return;
	}

	const FStructProperty* StructProperty = CastField<FStructProperty>(PropertyBeingCustomized.Get());
	check(StructProperty);

	if (FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(AttributeSetBeingCustomized.Get()))
	{
		const UStruct* Struct = StructProperty->Struct;
		if (Struct && Struct->IsChildOf(FSGClampedGameplayAttributeData::StaticStruct()))
		{
			AttributeDataBeingCustomized = MakeShared<FSGClampedGameplayAttributeData*>(static_cast<FSGClampedGameplayAttributeData*>(DataPtr));
			if (AttributeDataBeingCustomized.IsValid())
			{
				const FSGClampedGameplayAttributeData* Clamped = *AttributeDataBeingCustomized.Get();
				check(Clamped);
				SG_EDITOR_LOG(VeryVerbose, TEXT("\t Clamped DataPtr -> %s: MinValue: %s, MaxValue: %s"), *GetNameSafe(PropertyBeingCustomized.Get()), *Clamped->MinValue.ToString(), *Clamped->MaxValue.ToString())
			}
		}
	}
}

FGameplayAttributeData* FSGAttributeDataClampedDetails::GetGameplayAttributeData() const
{
	return GetGameplayClampedAttributeData();
}

void FSGAttributeDataClampedDetails::CustomizeHeader(const TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("FSGAttributeDataClampedDetails::CustomizeHeader ..."))
	InitializeFromStructHandle(InStructPropertyHandle, InStructCustomizationUtils);

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
				.Text(this, &FSGAttributeDataClampedDetails::GetHeaderBaseValueText)
			]
		];
}

void FSGAttributeDataClampedDetails::CustomizeChildren(const TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("FSGAttributeDataClampedDetails::CustomizeChildren ..."))

	const FSGClampedGameplayAttributeData* AttributeData = GetGameplayClampedAttributeData();
	if (!AttributeData)
	{
		return;
	}

	SG_EDITOR_LOG(VeryVerbose, TEXT("AttributeDataBeingCustomized -> %s: %f"), *GetNameSafe(PropertyBeingCustomized.Get()), AttributeData->GetBaseValue())

	BaseValueRow = MakeShared<FSGGameplayAttributeDataDetailsRow>(SharedThis(this), LOCTEXT("BaseValueLabel", "Base Value"), AttributeData->GetBaseValue());
	BaseValueRow->OnValueChanged().AddSP(this, &FSGAttributeDataClampedDetails::HandleBaseValueChanged);
	BaseValueRow->CustomizeChildren(InStructPropertyHandle, InStructBuilder, InStructCustomizationUtils);

	const TSharedPtr<IPropertyHandle> MinValueHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSGClampedGameplayAttributeData, MinValue));
	if (MinValueHandle.IsValid())
	{
		IDetailPropertyRow& Row = InStructBuilder.AddProperty(MinValueHandle.ToSharedRef());
		Row.ShouldAutoExpand(true);
	}
	
	const TSharedPtr<IPropertyHandle> MaxValueHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSGClampedGameplayAttributeData, MaxValue));
	if (MaxValueHandle.IsValid())
	{
		IDetailPropertyRow& Row = InStructBuilder.AddProperty(MaxValueHandle.ToSharedRef());
		Row.ShouldAutoExpand(true);
	}
}

FSGClampedGameplayAttributeData* FSGAttributeDataClampedDetails::GetGameplayClampedAttributeData() const
{
	if (!AttributeDataBeingCustomized.IsValid())
	{
		return nullptr;
	}

	return *AttributeDataBeingCustomized.Get();
}

void FSGAttributeDataClampedDetails::HandleBaseValueChanged(const float InValue) const
{
	FSGClampedGameplayAttributeData* ClampedAttributeData = GetGameplayClampedAttributeData();
	check(ClampedAttributeData);
	
	// Set the new value.
	ClampedAttributeData->SetBaseValue(InValue);
	ClampedAttributeData->SetCurrentValue(InValue);
}

#undef LOCTEXT_NAMESPACE
