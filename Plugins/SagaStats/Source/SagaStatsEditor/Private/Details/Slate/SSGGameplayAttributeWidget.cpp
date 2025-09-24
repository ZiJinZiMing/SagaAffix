/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "Details/Slate/SSGGameplayAttributeWidget.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Editor.h"
#include "SGEditorLog.h"
#include "SlateOptMacros.h"
#include "SagaStatsEditor.h"
#include "SGUtils.h"
#include "AttributeSet/SGAttributeSetBlueprint.h"
#include "Misc/TextFilter.h"
#include "UObject/PropertyAccessUtil.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"

#define LOCTEXT_NAMESPACE "K2Node"


DECLARE_DELEGATE_TwoParams(FOnAttributePicked, FProperty*, UClass*);

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

struct FSGGameplayAttributeViewerNode
{
	//Feature Begin Attribute In subclass of AttributeSet
	FSGGameplayAttributeViewerNode(FProperty* InAttribute, const FString& InAttributeName, UClass* InAttributeOwnerClass)
	{
		Attribute = InAttribute;
		AttributeName = MakeShareable(new FString(InAttributeName));
		AttributeOwnerClass = InAttributeOwnerClass;
	}


	/** The displayed name for this node. */
	TSharedPtr<FString> AttributeName;

	TWeakFieldPtr<FProperty> Attribute;
	
	TWeakObjectPtr<UClass> AttributeOwnerClass;
	//Feature End
};

/** The item used for visualizing the attribute in the list. */
class SSGGameplayAttributeItem : public SComboRow<TSharedPtr<FSGGameplayAttributeViewerNode>>
{
public:
	SLATE_BEGIN_ARGS(SSGGameplayAttributeItem)
			: _TextColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
		{
		}

		/** The text this item should highlight, if any. */
		SLATE_ARGUMENT(FText, HighlightText)
		/** The color text this item will use. */
		SLATE_ARGUMENT(FSlateColor, TextColor)
		/** The node this item is associated with. */
		SLATE_ARGUMENT(TSharedPtr<FSGGameplayAttributeViewerNode>, AssociatedNode)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		AssociatedNode = InArgs._AssociatedNode;

		ChildSlot
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 3.0f, 6.0f, 3.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(*AssociatedNode->AttributeName.Get()))
				.HighlightText(InArgs._HighlightText)
				.ColorAndOpacity(this, &SSGGameplayAttributeItem::GetTextColor)
				.IsEnabled(true)
			]
		];

		TextColor = InArgs._TextColor;

		ConstructInternal(
			STableRow::FArguments()
			.ShowSelection(true),
			InOwnerTableView
		);
	}

	/** Returns the text color for the item based on if it is selected or not. */
	FSlateColor GetTextColor() const
	{
		const TSharedPtr<ITypedTableView<TSharedPtr<FSGGameplayAttributeViewerNode>>> OwnerWidget = OwnerTablePtr.Pin();
		const TSharedPtr<FSGGameplayAttributeViewerNode>* MyItem = OwnerWidget->Private_ItemFromWidget(this);
		const bool bIsSelected = OwnerWidget->Private_IsItemSelected(*MyItem);

		if (bIsSelected)
		{
			return FSlateColor::UseForeground();
		}

		return TextColor;
	}

private:
	/** The text color for this item. */
	FSlateColor TextColor;

	/** The Attribute Viewer Node this item is associated with. */
	TSharedPtr<FSGGameplayAttributeViewerNode> AssociatedNode;
};

class SSGGameplayAttributeListWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSGGameplayAttributeListWidget)
		: _FilterClass(nullptr)
		{}

		SLATE_ARGUMENT(FString, FilterMetaData)
		SLATE_ARGUMENT(const UClass*, FilterClass)
		SLATE_ARGUMENT(bool, ShowOnlyOwnedAttributes)
		SLATE_ARGUMENT(FOnAttributePicked, OnAttributePickedDelegate)

	SLATE_END_ARGS()

	/**
	* Construct the widget
	*
	* @param	InArgs			A declaration from which to construct the widget
	*/
	void Construct(const FArguments& InArgs);

	virtual ~SSGGameplayAttributeListWidget() override;

private:
	typedef TTextFilter<const FGameplayAttribute&> FAttributeTextFilter;

	/** Called by Slate when the filter box changes text. */
	void OnFilterTextChanged(const FText& InFilterText);

	/** Creates the row widget when called by Slate when an item appears on the list. */
	TSharedRef<ITableRow> OnGenerateRowForAttributeViewer(TSharedPtr<FSGGameplayAttributeViewerNode> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	/** Called by Slate when an item is selected from the tree/list. */
	void OnAttributeSelectionChanged(TSharedPtr<FSGGameplayAttributeViewerNode> Item, ESelectInfo::Type SelectInfo) const;

	/** Updates the list of items in the dropdown menu */
	TSharedPtr<FSGGameplayAttributeViewerNode> UpdatePropertyOptions();

	/** Delegate to be called when an attribute is picked from the list */
	FOnAttributePicked OnAttributePicked;

	/** The search box */
	TSharedPtr<SSearchBox> SearchBoxPtr;

	/** Holds the Slate List widget which holds the attributes for the Attribute Viewer. */
	TSharedPtr<SListView<TSharedPtr<FSGGameplayAttributeViewerNode>>> AttributeList;

	/** Array of items that can be selected in the dropdown menu */
	TArray<TSharedPtr<FSGGameplayAttributeViewerNode>> PropertyOptions;

	/** Filters needed for filtering the assets */
	TSharedPtr<FAttributeTextFilter> AttributeTextFilter;

	/** Filter for meta data */
	FString FilterMetaData;
	
	/** Attribute set Class we want to see attributes of */
	TWeakObjectPtr<const UClass> FilterClass;
	
	/** Whether to filter the attribute list to only attributes owned by the containing class */
	bool bShowOnlyOwnedAttributes = false;
};

SSGGameplayAttributeListWidget::~SSGGameplayAttributeListWidget()
{
	if (OnAttributePicked.IsBound())
	{
		OnAttributePicked.Unbind();
	}
}

void SSGGameplayAttributeListWidget::Construct(const FArguments& InArgs)
{
	struct FLocal
	{
		//Feature Begin Attribute In subclass of AttributeSet
		static void AttributeToStringArray(const FGameplayAttribute& InAttribute, OUT TArray<FString>& StringArray)
		{
			
			// UClass* Class = Property.GetOwnerClass();
			// if ((Class->IsChildOf(UAttributeSet::StaticClass()) && !Class->ClassGeneratedBy) ||
			// 	(Class->IsChildOf(UAbilitySystemComponent::StaticClass()) && !Class->ClassGeneratedBy))
			// {
			// 	StringArray.Add(FString::Printf(TEXT("%s.%s"), *Class->GetName(), *Property.GetName()));
			// }

			// SS Changed ...
			
			const UClass* Class = InAttribute.GetAttributeSetClass();
			if (Class->IsChildOf(UAttributeSet::StaticClass()) ||
				(Class->IsChildOf(UAbilitySystemComponent::StaticClass()) && !Class->ClassGeneratedBy))
			{
				StringArray.Add(FString::Printf(TEXT("%s.%s"), *FSGUtils::GetAttributeClassName(Class), *InAttribute.GetName()));
			}
		}
		//Feature End
	};

	FilterMetaData = InArgs._FilterMetaData;
	OnAttributePicked = InArgs._OnAttributePickedDelegate;
	FilterClass = InArgs._FilterClass;
	//Feature Begin Attribute In subclass of AttributeSet
	bShowOnlyOwnedAttributes = InArgs._ShowOnlyOwnedAttributes;
	//Feature End

	// Setup text filtering
	AttributeTextFilter = MakeShared<FAttributeTextFilter>(FAttributeTextFilter::FItemToStringArray::CreateStatic(&FLocal::AttributeToStringArray));

	// Preload to ensure BP Attributes are loaded in memory so that they can be listed here
	// ISSEditorModule::Get().PreloadAssetsByClass(USGAttributeSetBlueprint::StaticClass());
	FSagaStatsEditorModule::Get().PreloadAssetsByClass(USGAttributeSetBlueprint::StaticClass());
	
	UpdatePropertyOptions();
	
	TSharedPtr<SWidget> ClassViewerContent;

	SAssignNew(ClassViewerContent, SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(SearchBoxPtr, SSearchBox)
			.HintText(NSLOCTEXT("Abilities", "SearchBoxHint", "Search Attributes (BP)"))
			.OnTextChanged(this, &SSGGameplayAttributeListWidget::OnFilterTextChanged)
			.DelayChangeNotificationsWhileTyping(true)
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSeparator)
			.Visibility(EVisibility::Collapsed)
		]

		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(AttributeList, SListView<TSharedPtr<FSGGameplayAttributeViewerNode>>)
			.Visibility(EVisibility::Visible)
			.SelectionMode(ESelectionMode::Single)
			.ListItemsSource(&PropertyOptions)

 			// Generates the actual widget for a tree item
			.OnGenerateRow(this, &SSGGameplayAttributeListWidget::OnGenerateRowForAttributeViewer)

 			// Find out when the user selects something in the tree
			.OnSelectionChanged(this, &SSGGameplayAttributeListWidget::OnAttributeSelectionChanged)
		];


	ChildSlot
	[
		ClassViewerContent.ToSharedRef()
	];

	// Ensure we gain focus on search box when opening the combo box
	if (GEditor)
	{
		TWeakPtr<SSGGameplayAttributeListWidget> LocalWeakThis = SharedThis(this);
		GEditor->GetTimerManager()->SetTimerForNextTick([LocalWeakThis]
		{
			if (const TSharedPtr<SSGGameplayAttributeListWidget> StrongThis = LocalWeakThis.Pin(); StrongThis.IsValid())
			{
				FSlateApplication::Get().SetKeyboardFocus(StrongThis->SearchBoxPtr);
				FSlateApplication::Get().SetUserFocus(0, StrongThis->SearchBoxPtr);
			}
		});
	}
}

TSharedRef<ITableRow> SSGGameplayAttributeListWidget::OnGenerateRowForAttributeViewer(const TSharedPtr<FSGGameplayAttributeViewerNode> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
	TSharedRef<SSGGameplayAttributeItem> ReturnRow = SNew(SSGGameplayAttributeItem, OwnerTable)
		.HighlightText(SearchBoxPtr->GetText())
		.TextColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.f))
		.AssociatedNode(Item);

	return ReturnRow;
}

TSharedPtr<FSGGameplayAttributeViewerNode> SSGGameplayAttributeListWidget::UpdatePropertyOptions()
{
	PropertyOptions.Empty();
	TSharedPtr<FSGGameplayAttributeViewerNode> InitiallySelected = MakeShared<FSGGameplayAttributeViewerNode>(nullptr, TEXT("None"), nullptr);

	PropertyOptions.Add(InitiallySelected);

	/*
	const USSEditorSettings& Settings = USSEditorSettings::Get();
	*/

	// Use IncludeSuper for iteration here only if bShowOnlyOwnedAttributed is used. To handle the use case of
	// FSGClampedGameplayAttributeData defined in a native class (for instance after wizard generation), whose value
	// are tweaked in the details panel of a child Blueprint
	
	//Feature Begin Attribute In subclass of AttributeSet
	// EFieldIteratorFlags::SuperClassFlags IteratorFlag = EFieldIteratorFlags::ExcludeSuper;
	// const EFieldIteratorFlags::SuperClassFlags IteratorFlag = bShowOnlyOwnedAttributes ? EFieldIteratorFlags::IncludeSuper : EFieldIteratorFlags::ExcludeSuper;
	const EFieldIteratorFlags::SuperClassFlags IteratorFlag = EFieldIteratorFlags::IncludeSuper ;
	//Feature End

	// Gather all UAttribute classes
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;

		// If we have been given a FilterClass, only show attributes of this AttributeSet class
		// (Way it's done right now, is containing details customization checks for ShowOnlyOwnedAttributed metadata on the
		// FGameplayAttribute property, and only passes down outer base class if metadata is present)
		if (FilterClass.IsValid() && !Class->IsChildOf(FilterClass.Get()))
		{
			continue;
		}
		
		if (FSGUtils::IsValidAttributeClass(Class))
		{
			// Allow entire classes to be filtered globally
			if (Class->HasMetaData(TEXT("HideInDetailsView")))
			{
				continue;
			}

			for (TFieldIterator<FProperty> PropertyIt(Class, IteratorFlag); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				if (!FSGUtils::IsValidGameplayAttributePropertyType(Property))
				{
					continue;
				}
				
				FGameplayAttribute Attribute(Property, Class);

				if(!Attribute.IsValid())
				{
					continue;
				}
				
				// if we have a search string and this doesn't match, don't show it
				if (AttributeTextFilter.IsValid() && !AttributeTextFilter->PassesFilter(Attribute))
				{
					continue;
				}

				// don't show attributes that are filtered by meta data
				if (!FilterMetaData.IsEmpty() && Property->HasMetaData(*FilterMetaData))
				{
					continue;
				}

				// Allow properties to be filtered globally (never show up)
				if (Property->HasMetaData(TEXT("HideInDetailsView")))
				{
					continue;
				}

				// Only allow field of expected types
				FString CPPType = Property->GetCPPType();
				if (!FSGUtils::IsValidCPPType(CPPType))
				{
					continue;
				}

				const FString AttributeName = FString::Printf(TEXT("%s.%s"), *FSGUtils::GetAttributeClassName(Class), *Property->GetName());

				/*
				// Allow properties to be filtered globally via Developer Settings (never show up)
				if (USSEditorSettings::IsAttributeFiltered(Settings.FilterAttributesList, AttributeName))
				{
					continue;
				}
				*/

				PropertyOptions.Add(MakeShared<FSGGameplayAttributeViewerNode>(Property, AttributeName, Class));
			}
		}

		// UAbilitySystemComponent can add 'system' attributes
		if (Class->IsChildOf(UAbilitySystemComponent::StaticClass()) && !Class->ClassGeneratedBy)
		{
			for (TFieldIterator<FProperty> PropertyIt(Class, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
			{
				FProperty* Property = *PropertyIt;
				
				if (!FSGUtils::IsValidGameplayAttributePropertyType(Property))
				{
					continue;
				}
				
				// SystemAttributes have to be explicitly tagged
				if (Property->HasMetaData(TEXT("SystemGameplayAttribute")) == false)
				{
					continue;
				}

				FGameplayAttribute Attribute(Property, Class);

				if(!Attribute.IsValid())
				{
					continue;
				}
				
				// if we have a search string and this doesn't match, don't show it
				if (AttributeTextFilter.IsValid() && !AttributeTextFilter->PassesFilter(Attribute))
				{
					continue;
				}

				const FString AttributeName = FString::Printf(TEXT("%s.%s"), *Class->GetName(), *Property->GetName());
				
				/*
				// Allow properties to be filtered globally via Developer Settings (never show up)
				if (USSEditorSettings::IsAttributeFiltered(Settings.FilterAttributesList, AttributeName))
				{
					continue;
				}
				*/

				PropertyOptions.Add(MakeShared<FSGGameplayAttributeViewerNode>(Property, AttributeName, Class));
			}
		}
	}

	return InitiallySelected;
}

void SSGGameplayAttributeListWidget::OnFilterTextChanged(const FText& InFilterText)
{
	AttributeTextFilter->SetRawFilterText(InFilterText);
	SearchBoxPtr->SetError(AttributeTextFilter->GetFilterErrorText());

	UpdatePropertyOptions();
}

// ReSharper disable once CppParameterNeverUsed
void SSGGameplayAttributeListWidget::OnAttributeSelectionChanged(const TSharedPtr<FSGGameplayAttributeViewerNode> Item, ESelectInfo::Type SelectInfo) const
{
	OnAttributePicked.ExecuteIfBound(Item->Attribute.Get(),Item->AttributeOwnerClass.Get());
}

void SSGGameplayAttributeWidget::Construct(const FArguments& InArgs)
{
	FilterMetaData = InArgs._FilterMetaData;
	OnAttributeChanged = InArgs._OnAttributeChanged;
	SelectedPropertyPtr = InArgs._DefaultProperty;
	//Feature Begin Attribute In subclass of AttributeSet
	SelectedAttributeOwnerClassPtr = InArgs._DefaultAttributeOwnerClass;
	//Feature End
	FilterClass = InArgs._FilterClass;
	bShowOnlyOwnedAttributes = InArgs._ShowOnlyOwnedAttributes;
	
	// set up the combo button
	SAssignNew(ComboButton, SComboButton)
		.OnGetMenuContent(this, &SSGGameplayAttributeWidget::GenerateAttributePicker)
		.ContentPadding(FMargin(2.0f, 2.0f))
		.ToolTipText(this, &SSGGameplayAttributeWidget::GetSelectedValueAsString)
		.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &SSGGameplayAttributeWidget::GetSelectedValueAsString)
		];

	ChildSlot
	[
		ComboButton.ToSharedRef()
	];
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void SSGGameplayAttributeWidget::OnAttributePicked(FProperty* InProperty, UClass* InAttributeOwnerClass)
{
	FProperty* Property = InProperty ? PropertyAccessUtil::FindPropertyByName(InProperty->GetFName(), InProperty->GetOwnerStruct()) : nullptr;

	//Feature Begin Attribute In subclass of AttributeSet
	if (OnAttributeChanged.IsBound())
	{
		OnAttributeChanged.Execute(Property ? Property : nullptr, InAttributeOwnerClass);
	}
	//Feature End

	// Update the selected item for displaying
	SelectedPropertyPtr = Property ? Property : nullptr;

	//Feature Begin Attribute In subclass of AttributeSet
	SelectedAttributeOwnerClassPtr = InAttributeOwnerClass;
	//Feature End

	// close the list
	ComboButton->SetIsOpen(false);
}

TSharedRef<SWidget> SSGGameplayAttributeWidget::GenerateAttributePicker()
{
	return SNew(SBox)
		.WidthOverride(280)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(500)
			[
				SNew(SSGGameplayAttributeListWidget)
				.OnAttributePickedDelegate(FOnAttributePicked::CreateSP(this, &SSGGameplayAttributeWidget::OnAttributePicked))
				.FilterClass(FilterClass.Get())
				.ShowOnlyOwnedAttributes(bShowOnlyOwnedAttributes)
				.FilterMetaData(FilterMetaData)
			]
		];
}

FText SSGGameplayAttributeWidget::GetSelectedValueAsString() const
{
	FText None = FText::FromString(TEXT("None"));
	
	// SG_EDITOR_LOG(VeryVerbose, TEXT("SSGGameplayAttributeWidget::GetSelectedValueAsString - SelectedProperty: %s"), *GetNameSafe(SelectedProperty))

	if (!SelectedPropertyPtr.IsValid())
	{
		// SG_EDITOR_LOG(Warning, TEXT("Likely invalid property, SelectedProperty nullptr"))
		return None;
	}

	if (!SelectedPropertyPtr->IsValidLowLevel())
	{
		SG_EDITOR_NS_LOG(Warning, TEXT("Likely invalid property, IsValidLowLevel returned false"))
		return None;
	}
	
	// Check for likely dirty property
	if (SelectedPropertyPtr->GetOffset_ForInternal() == 0)
	{
		SG_EDITOR_NS_LOG(Warning, TEXT("Likely invalid property, offset 0"))
		return None;
	}

	//Feature Begin Attribute In subclass of AttributeSet
	const UClass* Class =  SelectedAttributeOwnerClassPtr != nullptr ? SelectedAttributeOwnerClassPtr.Get() : SelectedPropertyPtr->GetOwnerClass();
	//Feature End
	
	const FString PropertyName = SelectedPropertyPtr->GetDisplayNameText().ToString();
	const FString PropertyString = FString::Printf(TEXT("%s.%s"), *FSGUtils::GetAttributeClassName(Class), *PropertyName);
	return FText::FromString(PropertyString);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
