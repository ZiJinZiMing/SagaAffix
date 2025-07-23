/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Types/SlateEnums.h"

class IDetailChildrenBuilder;
class ISagaGameplayAttributeDataDetailsBase;
class IPropertyHandle;
class IPropertyTypeCustomizationUtils;

/**
 * Details row class, used by IPropertyTypeCustomization implementations.
 *
 * Wraps and implements the logic of dealing with a single row property, for either BaseValue, MinValue, MaxValue, etc.
 *
 * A float property in a given FGameplayAttributeData struct or child struct.
 */
class FSSGameplayAttributeDataDetailsRow : public TSharedFromThis<FSSGameplayAttributeDataDetailsRow>
{
public:
	explicit FSSGameplayAttributeDataDetailsRow(const TWeakPtr<ISagaGameplayAttributeDataDetailsBase>& InDetailsOwner, const FText& InRowNameText, float InInitialValue);
	virtual ~FSSGameplayAttributeDataDetailsRow() = default;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnValueChanged, float)
	FOnValueChanged& OnValueChanged() { return OnValueChangedDelegate; }

private:
	/** True if the slider is being used to change the value of the property */
	bool bIsUsingSlider = false;

	/** When using the slider, what was the last committed value (min value) */
	float LastSliderCommittedValue = 0.f;

	TWeakPtr<ISagaGameplayAttributeDataDetailsBase> DetailsOwnerPtr;
	FText RowNameText;

	FOnValueChanged OnValueChangedDelegate;

	TOptional<float> OnGetValue() const;
	void OnBeginSliderMovement();
	void OnEndSliderMovement(float InValue);
	void OnValueCommitted(float InNewValue, ETextCommit::Type InCommitInfo);
	void SetValueWithTransaction(float InNewValue) const;
};
