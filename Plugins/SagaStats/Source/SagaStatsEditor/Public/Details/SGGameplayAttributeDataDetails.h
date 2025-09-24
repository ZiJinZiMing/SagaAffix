/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "ISGGameplayAttributeDataDetailsBase.h"

class FSGGameplayAttributeDataDetailsRow;
class IPropertyHandle;
class IPropertyTypeCustomization;

/**
 * Details customization for FGameplayAttributeData.
 *
 * And ability to view / set BaseValue and CurrentValue (as DefaultValue)
 */
class FSGGameplayAttributeDataDetails : public ISGGameplayAttributeDataDetailsBase
{
public:
	FSGGameplayAttributeDataDetails();
	virtual ~FSGGameplayAttributeDataDetails() override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin ISGGameplayAttributeDataDetailsBase interface
	virtual void InitializeFromStructHandle(const TSharedRef<IPropertyHandle>& InStructPropertyHandle, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual FGameplayAttributeData* GetGameplayAttributeData() const override;
	//~ End ISGGameplayAttributeDataDetailsBase interface

	//~ Begin IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization interface

private:
	/** Keep track of the FGameplayAttributeData struct we are editing */
	TSharedPtr<FGameplayAttributeData*> AttributeDataBeingCustomized;

	TSharedPtr<FSGGameplayAttributeDataDetailsRow> BaseValueRow;
	
	void HandleBaseValueChanged(float InValue) const;
};
