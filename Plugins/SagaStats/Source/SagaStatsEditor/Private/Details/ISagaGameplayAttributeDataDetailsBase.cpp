/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "Details/ISagaGameplayAttributeDataDetailsBase.h"

#include "AttributeSet.h"
#include "SSEditorLog.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Utils/SagaEditorUtils.h"

#define LOCTEXT_NAMESPACE "SagaGameplayAttributeDataDetails"

ISagaGameplayAttributeDataDetailsBase::ISagaGameplayAttributeDataDetailsBase()
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("Construct ISagaGameplayAttributeDataDetailsBase ..."))
}

ISagaGameplayAttributeDataDetailsBase::~ISagaGameplayAttributeDataDetailsBase()
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("Destroyed ISagaGameplayAttributeDataDetailsBase ..."))
	PropertyBeingCustomized.Reset();
	BlueprintBeingCustomized.Reset();
	AttributeSetBeingCustomized.Reset();

}

void ISagaGameplayAttributeDataDetailsBase::Initialize()
{
}

void ISagaGameplayAttributeDataDetailsBase::InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	PropertyUtilities = InStructCustomizationUtils.GetPropertyUtilities();

	PropertyBeingCustomized = InStructPropertyHandle->GetProperty();
	check(PropertyBeingCustomized.IsValid());

	const UClass* OwnerClass = PropertyBeingCustomized->Owner.GetOwnerClass();
	check(OwnerClass);
	
	const UClass* OuterBaseClass = InStructPropertyHandle->GetOuterBaseClass();
	check(OuterBaseClass);

	AttributeSetBeingCustomized = Cast<UAttributeSet>(OuterBaseClass->GetDefaultObject());
	
	BlueprintBeingCustomized = SagaEditorUtils::GetBlueprintFromClass(OuterBaseClass);
}

TWeakFieldPtr<FProperty> ISagaGameplayAttributeDataDetailsBase::GetPropertyBeingCustomized() const
{
	return PropertyBeingCustomized;
}

TWeakObjectPtr<UBlueprint> ISagaGameplayAttributeDataDetailsBase::GetBlueprintBeingCustomized() const
{
	return BlueprintBeingCustomized;
}

TWeakObjectPtr<UAttributeSet> ISagaGameplayAttributeDataDetailsBase::GetAttributeSetBeingCustomized() const
{
	return AttributeSetBeingCustomized;
}

// ReSharper disable once CppMemberFunctionMayBeConst
// ReSharper disable CppParameterMayBeConstPtrOrRef
void ISagaGameplayAttributeDataDetailsBase::HandleSettingsChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent)
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("ISagaGameplayAttributeDataDetailsBase::HandleSettingsChanged - InObject: %s, Property: %s"), *GetNameSafe(InObject), *GetNameSafe(InPropertyChangedEvent.Property))

	if (const TSharedPtr<IPropertyUtilities> Utilities = PropertyUtilities.Pin())
	{
		SS_EDITOR_LOG(VeryVerbose, TEXT("ISagaGameplayAttributeDataDetailsBase::HandleSettingsChanged - ForceRefresh InObject: %s, Property: %s"), *GetNameSafe(InObject), *GetNameSafe(InPropertyChangedEvent.Property))
		Utilities->ForceRefresh();
	}
}

TOptional<float> ISagaGameplayAttributeDataDetailsBase::OnGetBaseValue() const
{
	return LastSliderCommittedValue;
}

void ISagaGameplayAttributeDataDetailsBase::OnBeginSliderMovement()
{
	bIsUsingSlider = true;
}

void ISagaGameplayAttributeDataDetailsBase::OnEndSliderMovement(const float InValue)
{
	bIsUsingSlider = false;
	SS_EDITOR_LOG(VeryVerbose, TEXT("End slider with %f (LastSliderCommittedValue: %f)"), InValue, LastSliderCommittedValue)
	SetValueWithTransaction(InValue);
}

// ReSharper disable once CppParameterNeverUsed
void ISagaGameplayAttributeDataDetailsBase::OnValueCommitted(const float InNewValue, ETextCommit::Type InCommitInfo)
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("OnValueCommitted: %f"), InNewValue)
	if (!bIsUsingSlider)
	{
		SetValueWithTransaction(InNewValue);
	}

	LastSliderCommittedValue = InNewValue;
}

void ISagaGameplayAttributeDataDetailsBase::SetValueWithTransaction(const float InNewValue) const
{
	FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	check(AttributeData);
	check(AttributeSetBeingCustomized.IsValid());
	check(BlueprintBeingCustomized.IsValid());

	FScopedTransaction Transaction(LOCTEXT("SetBaseValueProperty", "Set Attribute Data Base Value"));

	AttributeSetBeingCustomized->SetFlags(RF_Transactional);
	AttributeSetBeingCustomized->Modify();

	// Set the new value.
	AttributeData->SetBaseValue(InNewValue);
	AttributeData->SetCurrentValue(InNewValue);

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BlueprintBeingCustomized.Get());
}

bool ISagaGameplayAttributeDataDetailsBase::IsCompactView()
{
	return false;
}

FText ISagaGameplayAttributeDataDetailsBase::GetHeaderBaseValueText() const
{
	const FGameplayAttributeData* AttributeData = GetGameplayAttributeData();
	check(AttributeData);
	
	return FText::Format(
		FText::FromString(TEXT("Base:{0}, Current:{1}")),
		FText::FromString(FString::Printf(TEXT("%.2f"), AttributeData->GetBaseValue())),
		FText::FromString(FString::Printf(TEXT("%.2f"), AttributeData->GetCurrentValue()))
	);
}

#undef LOCTEXT_NAMESPACE
