/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "ISagaGameplayAttributeDataDetailsBase.h"

class FSSGameplayAttributeDataDetailsRow;
class IPropertyHandle;
class IPropertyTypeCustomization;

/**
 * Details customization for FGameplayAttributeData.
 *
 * And ability to view / set BaseValue and CurrentValue (as DefaultValue)
 */
class FSagaGameplayAttributeDataDetails : public ISagaGameplayAttributeDataDetailsBase
{
public:
	FSagaGameplayAttributeDataDetails();
	virtual ~FSagaGameplayAttributeDataDetails() override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin ISagaGameplayAttributeDataDetailsBase interface
	virtual void InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual FGameplayAttributeData* GetGameplayAttributeData() const override;
	//~ End ISagaGameplayAttributeDataDetailsBase interface

	//~ Begin IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization interface

private:
	/** Keep track of the FGameplayAttributeData struct we are editing */
	TSharedPtr<FGameplayAttributeData*> AttributeDataBeingCustomized;

	TSharedPtr<FSSGameplayAttributeDataDetailsRow> BaseValueRow;
	
	void HandleBaseValueChanged(float InValue) const;
};
