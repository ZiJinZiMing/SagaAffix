/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"

#include "ISGGameplayAttributeDataDetailsBase.h"

class FSGGameplayAttributeDataDetailsRow;
struct FSGClampedGameplayAttributeData;

/**
 * Details customization for FSGClampedGameplayAttributeData.
 *
 * And ability to view / set BaseValue and CurrentValue (as DefaultValue)
 */
class FSGAttributeDataClampedDetails : public ISGGameplayAttributeDataDetailsBase
{
public:
	FSGAttributeDataClampedDetails();
	virtual ~FSGAttributeDataClampedDetails() override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin ISSGameplayAttributeDataClampedDetailsBase interface
	virtual void InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual FGameplayAttributeData* GetGameplayAttributeData() const override;
	//~ End ISSGameplayAttributeDataClampedDetailsBase interface

	//~ Begin IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization interface

	FSGClampedGameplayAttributeData* GetGameplayClampedAttributeData() const;

private:
	/** Keep track of the FSGClampedGameplayAttributeData struct we are editing */
	TSharedPtr<FSGClampedGameplayAttributeData*> AttributeDataBeingCustomized;

	TSharedPtr<FSGGameplayAttributeDataDetailsRow> BaseValueRow;

	void HandleBaseValueChanged(float InValue) const;
};
