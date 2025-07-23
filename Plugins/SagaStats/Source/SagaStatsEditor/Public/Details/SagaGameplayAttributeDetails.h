/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class SSagaGameplayAttributeWidget;

/**
 * This is the main customization class for FGameplayAttribute properties
 *
 * Its almost identical to the engine customization for FGameplayAttribute, its main purpose is to make use of
 * SSagaGameplayAttributeWidget to allow the display of FGameplayAttribute FProperties in Attribute picker dropdown.
 */
class FSagaGameplayAttributeDetails : public IPropertyTypeCustomization
{
public:
	virtual ~FSagaGameplayAttributeDetails() override;

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	/** The attribute property */
	TSharedPtr<IPropertyHandle> MyProperty;

	/** The owner property */
	TSharedPtr<IPropertyHandle> OwnerProperty;

	/** The name property */
	TSharedPtr<IPropertyHandle> NameProperty;

	//Feature Begin Attribute In subclass of AttributeSet
	/** The property owner class */
	TSharedPtr<IPropertyHandle> OwnerClassProperty;
	//Feature End

	
	/** Slate Attribute Widget */
	TSharedPtr<SSagaGameplayAttributeWidget> AttributeWidget;

	//Feature Begin Attribute In subclass of AttributeSet
	void OnAttributeChanged(FProperty* SelectedAttribute, UClass* InAttributeOwnerClass) const;
	//Feature End
	
	void HandleRequestRefresh(const TSharedPtr<IPropertyUtilities> InPropertyUtilities);
};
