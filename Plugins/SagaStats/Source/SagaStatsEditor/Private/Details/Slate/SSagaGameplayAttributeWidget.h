/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakFieldPtr.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SComboButton.h"

class SSagaGameplayAttributeWidget : public SCompoundWidget
{
public:

	//Feature Begin Attribute In subclass of AttributeSet
	/** Called when a tag status is changed */
	DECLARE_DELEGATE_TwoParams(FOnAttributeChanged, FProperty*, UClass*)
	//Feature End

	SLATE_BEGIN_ARGS(SSagaGameplayAttributeWidget)
	: _DefaultProperty(nullptr)
	//Feature Begin Attribute In subclass of AttributeSet
	, _DefaultAttributeOwnerClass(nullptr)
	//Feature End
	, _FilterClass(nullptr)
	{}
	SLATE_ARGUMENT(FString, FilterMetaData)
	SLATE_ARGUMENT(FProperty*, DefaultProperty)
	//Feature Begin Attribute In subclass of AttributeSet
	SLATE_ARGUMENT(UClass*, DefaultAttributeOwnerClass)
	//Feature End
	SLATE_ARGUMENT(const UClass*, FilterClass)
	SLATE_ARGUMENT(bool, ShowOnlyOwnedAttributes)
	SLATE_EVENT(FOnAttributeChanged, OnAttributeChanged) // Called when a tag status changes
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FProperty* GetSelectedProperty() const { return SelectedPropertyPtr.Get(); }
	void SetSelectedProperty(FProperty* InSelectedProperty) { SelectedPropertyPtr = InSelectedProperty; }

private:

	TSharedRef<SWidget> GenerateAttributePicker();

	FText GetSelectedValueAsString() const;

	//Feature Begin Attribute In subclass of AttributeSet
	/** Handles updates when the selected attribute changes */
	void OnAttributePicked(FProperty* InProperty, UClass* InAttributeOwnerClass);
	//Feature End

	/** Delegate to call when the selected attribute changes */
	FOnAttributeChanged OnAttributeChanged;

	/** The search string being used to filter the attributes */
	FString FilterMetaData;

	/** The currently selected attribute */
	TWeakFieldPtr<FProperty> SelectedPropertyPtr;

	//Feature Begin Attribute In subclass of AttributeSet
	/*The owner of the currently selected attribute*/
	TWeakObjectPtr<UClass> SelectedAttributeOwnerClassPtr;
	//Feature End

	/** Used to display an attribute picker. */
	TSharedPtr<SComboButton> ComboButton;
	
	/** Attribute set Class we want to see attributes of */
	TWeakObjectPtr<const UClass> FilterClass;
	
	/** Whether to filter the attribute list to only attributes owned by the containing class */
	bool bShowOnlyOwnedAttributes = false;
};
