/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "UObject/WeakFieldPtr.h"

class FSGAttributeSetBlueprintEditor;
class UAttributeSet;
struct FGameplayAttributeData;

/**
 * Details customization for FGameplayAttributeData / FSGClampedGameplayAttributeData (Base Class).
 *
 * And ability to view / set BaseValue and CurrentValue (as DefaultValue)
 *
 * This is the default Base class shared by both FGameplayAttributeData and FSGClampedGameplayAttributeData details customization classes.
 */
class ISGGameplayAttributeDataDetailsBase : public IPropertyTypeCustomization
{
public:
	ISGGameplayAttributeDataDetailsBase();
	virtual ~ISGGameplayAttributeDataDetailsBase() override;

	/** Setup - mainly to register SP delegates as they need to happen after construction */
	virtual void Initialize();
	virtual void InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils);
	virtual FGameplayAttributeData* GetGameplayAttributeData() const = 0;

	//~ Begin IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override
	{
	}

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override
	{
	}

	//~ End IPropertyTypeCustomization interface

	TWeakFieldPtr<FProperty> GetPropertyBeingCustomized() const;
	TWeakObjectPtr<UBlueprint> GetBlueprintBeingCustomized() const;
	TWeakObjectPtr<UAttributeSet> GetAttributeSetBeingCustomized() const;

protected:
	/** Keep track of the property we are editing */
	TWeakFieldPtr<FProperty> PropertyBeingCustomized;

	/** The blueprint we are editing */
	TWeakObjectPtr<UBlueprint> BlueprintBeingCustomized;

	/** The UObject we are editing */
	TWeakObjectPtr<UAttributeSet> AttributeSetBeingCustomized;

	/** True if the slider is being used to change the value of the property */
	bool bIsUsingSlider = false;

	/** When using the slider, what was the last committed value */
	float LastSliderCommittedValue = 0.f;

	/** A weak reference to the property utilities used by this type customization */
	TWeakPtr<IPropertyUtilities> PropertyUtilities;

	void HandleSettingsChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent);

	TOptional<float> OnGetBaseValue() const;
	void OnBeginSliderMovement();
	void OnEndSliderMovement(float InValue);
	void OnValueCommitted(float InNewValue, ETextCommit::Type InCommitInfo);
	void SetValueWithTransaction(float InNewValue) const;

	static bool IsCompactView();

	FText GetHeaderBaseValueText() const;
};
