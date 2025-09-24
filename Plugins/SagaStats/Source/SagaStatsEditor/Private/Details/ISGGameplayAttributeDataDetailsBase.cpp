/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "Details/ISGGameplayAttributeDataDetailsBase.h"

#include "AttributeSet.h"
#include "SGEditorLog.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Utils/SGEditorUtils.h"

#define LOCTEXT_NAMESPACE "SagaGameplayAttributeDataDetails"

ISGGameplayAttributeDataDetailsBase::ISGGameplayAttributeDataDetailsBase()
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("Construct ISGGameplayAttributeDataDetailsBase ..."))
}

ISGGameplayAttributeDataDetailsBase::~ISGGameplayAttributeDataDetailsBase()
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("Destroyed ISGGameplayAttributeDataDetailsBase ..."))
	PropertyBeingCustomized.Reset();
	BlueprintBeingCustomized.Reset();
	AttributeSetBeingCustomized.Reset();

}

void ISGGameplayAttributeDataDetailsBase::Initialize()
{
}

void ISGGameplayAttributeDataDetailsBase::InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	PropertyUtilities = InStructCustomizationUtils.GetPropertyUtilities();

	PropertyBeingCustomized = InStructPropertyHandle->GetProperty();
	check(PropertyBeingCustomized.IsValid());

	const UClass* OwnerClass = PropertyBeingCustomized->Owner.GetOwnerClass();
	check(OwnerClass);
	
	const UClass* OuterBaseClass = InStructPropertyHandle->GetOuterBaseClass();
	check(OuterBaseClass);

	AttributeSetBeingCustomized = Cast<UAttributeSet>(OuterBaseClass->GetDefaultObject());
	
	BlueprintBeingCustomized = SGEditorUtils::GetBlueprintFromClass(OuterBaseClass);
}

TWeakFieldPtr<FProperty> ISGGameplayAttributeDataDetailsBase::GetPropertyBeingCustomized() const
{
	return PropertyBeingCustomized;
}

TWeakObjectPtr<UBlueprint> ISGGameplayAttributeDataDetailsBase::GetBlueprintBeingCustomized() const
{
	return BlueprintBeingCustomized;
}

TWeakObjectPtr<UAttributeSet> ISGGameplayAttributeDataDetailsBase::GetAttributeSetBeingCustomized() const
{
	return AttributeSetBeingCustomized;
}

// ReSharper disable once CppMemberFunctionMayBeConst
// ReSharper disable CppParameterMayBeConstPtrOrRef
void ISGGameplayAttributeDataDetailsBase::HandleSettingsChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent)
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("ISGGameplayAttributeDataDetailsBase::HandleSettingsChanged - InObject: %s, Property: %s"), *GetNameSafe(InObject), *GetNameSafe(InPropertyChangedEvent.Property))

	if (const TSharedPtr<IPropertyUtilities> Utilities = PropertyUtilities.Pin())
	{
		SG_EDITOR_LOG(VeryVerbose, TEXT("ISGGameplayAttributeDataDetailsBase::HandleSettingsChanged - ForceRefresh InObject: %s, Property: %s"), *GetNameSafe(InObject), *GetNameSafe(InPropertyChangedEvent.Property))
		Utilities->ForceRefresh();
	}
}

TOptional<float> ISGGameplayAttributeDataDetailsBase::OnGetBaseValue() const
{
	return LastSliderCommittedValue;
}

void ISGGameplayAttributeDataDetailsBase::OnBeginSliderMovement()
{
	bIsUsingSlider = true;
}

void ISGGameplayAttributeDataDetailsBase::OnEndSliderMovement(const float InValue)
{
	bIsUsingSlider = false;
	SG_EDITOR_LOG(VeryVerbose, TEXT("End slider with %f (LastSliderCommittedValue: %f)"), InValue, LastSliderCommittedValue)
	SetValueWithTransaction(InValue);
}

// ReSharper disable once CppParameterNeverUsed
void ISGGameplayAttributeDataDetailsBase::OnValueCommitted(const float InNewValue, ETextCommit::Type InCommitInfo)
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("OnValueCommitted: %f"), InNewValue)
	if (!bIsUsingSlider)
	{
		SetValueWithTransaction(InNewValue);
	}

	LastSliderCommittedValue = InNewValue;
}

void ISGGameplayAttributeDataDetailsBase::SetValueWithTransaction(const float InNewValue) const
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

bool ISGGameplayAttributeDataDetailsBase::IsCompactView()
{
	return false;
}

FText ISGGameplayAttributeDataDetailsBase::GetHeaderBaseValueText() const
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
