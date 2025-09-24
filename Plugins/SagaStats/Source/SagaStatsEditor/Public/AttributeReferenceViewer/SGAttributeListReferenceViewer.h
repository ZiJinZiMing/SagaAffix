/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Misc/TextFilter.h"
#include "UObject/WeakFieldPtr.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

struct FGameplayAttribute;
class SBorder;
class SSearchBox;

struct FSGAttributeListReferenceViewerNode
{
	FSGAttributeListReferenceViewerNode(FProperty* InAttribute, const FString& InAttributeName, UClass* InAttributeOwnerClass)
	{
		Attribute = InAttribute;
		AttributeName = MakeShareable(new FString(InAttributeName));
		AttributeOwnerClass = InAttributeOwnerClass;
	}

	/** The displayed name for this node. */
	TSharedPtr<FString> AttributeName;

	TWeakFieldPtr<FProperty> Attribute;

	TWeakObjectPtr<UClass> AttributeOwnerClass;
	
};

/** Widget allowing user to list Gameplay Attributes and open up the reference viewer for them */
class SAGASTATSEDITOR_API SSGAttributeListReferenceViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSGAttributeListReferenceViewer)
		: _Padding(FMargin(2.0f))
	{}

		// Padding inside the picker widget
		SLATE_ARGUMENT(FMargin, Padding)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	/** Updates the tag list when the filter text changes */
	void OnFilterTextChanged(const FText& InFilterText);
	
	/** Gets the widget to focus once the menu opens. */
	TSharedPtr<SWidget> GetWidgetToFocusOnOpen();
	
private:
	typedef TTextFilter<const FGameplayAttribute&> FAttributeTextFilter;
	
	/** Allows for the user to find a specific gameplay attribute in the list */
	TSharedPtr<SSearchBox> SearchAttributeBox;

	/** Filters needed for filtering the attributes */
	TSharedPtr<FAttributeTextFilter> AttributeTextFilter;
	
	/** Container widget holding the attribute list */
	TSharedPtr<SBorder> AttributesContainerWidget;
	
	/** Holds the Slate List widget that holds the attributes for the Attribute Viewer. */
	TSharedPtr<SListView<TSharedPtr<FSGAttributeListReferenceViewerNode>>> AttributeList;
	
	/** Array of items that can be selected in the dropdown menu */
	TArray<TSharedPtr<FSGAttributeListReferenceViewerNode>> PropertyOptions;
	
	/** Updates the list of items in the dropdown menu */
	void UpdatePropertyOptions();
	
	/** Creates the row widget when called by Slate when an item appears on the list. */
	TSharedRef<ITableRow> OnGenerateRowForAttributeViewer(TSharedPtr<FSGAttributeListReferenceViewerNode> InItem, const TSharedRef<STableViewBase>& InOwnerTable) const;
	
	/** Called when an item is selected from the list. */
	void OnAttributeSelectionChanged(TSharedPtr<FSGAttributeListReferenceViewerNode> InItem, ESelectInfo::Type SelectInfo) const;
};
