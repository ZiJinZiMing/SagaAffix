/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"

#include "ISagaGameplayAttributeDataDetailsBase.h"

class FSSGameplayAttributeDataDetailsRow;
struct FSagaClampedGameplayAttributeData;

/**
 * Details customization for FSagaClampedGameplayAttributeData.
 *
 * And ability to view / set BaseValue and CurrentValue (as DefaultValue)
 */
class FSagaAttributeDataClampedDetails : public ISagaGameplayAttributeDataDetailsBase
{
public:
	FSagaAttributeDataClampedDetails();
	virtual ~FSagaAttributeDataClampedDetails() override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin ISSGameplayAttributeDataClampedDetailsBase interface
	virtual void InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual FGameplayAttributeData* GetGameplayAttributeData() const override;
	//~ End ISSGameplayAttributeDataClampedDetailsBase interface

	//~ Begin IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization interface

	FSagaClampedGameplayAttributeData* GetGameplayClampedAttributeData() const;

private:
	/** Keep track of the FSagaClampedGameplayAttributeData struct we are editing */
	TSharedPtr<FSagaClampedGameplayAttributeData*> AttributeDataBeingCustomized;

	TSharedPtr<FSSGameplayAttributeDataDetailsRow> BaseValueRow;

	void HandleBaseValueChanged(float InValue) const;
};
