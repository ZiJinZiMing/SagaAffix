/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#include "Details/SGGameplayAttributeDetails.h"

#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "SGDelegates.h"
#include "SGEditorLog.h"
#include "IPropertyUtilities.h"
#include "Details/Slate/SSGGameplayAttributeWidget.h"

FSGGameplayAttributeDetails::~FSGGameplayAttributeDetails()
{
	FSGDelegates::OnRequestDetailsRefresh.RemoveAll(this);
}

TSharedRef<IPropertyTypeCustomization> FSGGameplayAttributeDetails::MakeInstance()
{
	return MakeShared<FSGGameplayAttributeDetails>();
}

// ReSharper disable once CppParameterMayBeConst
void FSGGameplayAttributeDetails::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	SG_EDITOR_LOG(Verbose, TEXT("FSGGameplayAttributeDetails::CustomizeHeader ..."))

	const TSharedPtr<IPropertyUtilities> Utilities = StructCustomizationUtils.GetPropertyUtilities();
	FSGDelegates::OnRequestDetailsRefresh.AddSP(this, &FSGGameplayAttributeDetails::HandleRequestRefresh, Utilities);

	// Can't use GET_MEMBER_NAME_CHECKED for those two props since they're private and requires adding this class as a friend class to FGameplayAttribute
	//
	// MyProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGameplayAttribute, Attribute));
	// OwnerProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGameplayAttribute, AttributeOwner));

	MyProperty = StructPropertyHandle->GetChildHandle(FName(TEXT("Attribute")));
	OwnerProperty = StructPropertyHandle->GetChildHandle(FName(TEXT("AttributeOwner")));
	NameProperty = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FGameplayAttribute, AttributeName));
	
	//Feature Begin Attribute In subclass of AttributeSet
	OwnerClassProperty = StructPropertyHandle->GetChildHandle(FName(TEXT("AttributeOwnerClass")));
	//Feature End

	const FString& FilterMetaStr = StructPropertyHandle->GetProperty()->GetMetaData(TEXT("FilterMetaTag"));
	const bool bShowOnlyOwnedAttributes = StructPropertyHandle->GetProperty()->HasMetaData(TEXT("ShowOnlyOwnedAttributes"));
	const UClass* OuterBaseClass = StructPropertyHandle->GetOuterBaseClass();
	SG_EDITOR_LOG(
		VeryVerbose,
		TEXT("FSGGameplayAttributeDetails::CustomizeHeader - OuterBaseClass: %s (bShowOnlyOwnedAttributed: %s)"),
		*GetNameSafe(OuterBaseClass),
		*LexToString(bShowOnlyOwnedAttributes)
	)

	FProperty* PropertyValue = nullptr;
	if (MyProperty.IsValid())
	{
		FProperty* ObjPtr = nullptr;
		MyProperty->GetValue(ObjPtr);
		PropertyValue = ObjPtr;
	}

	//Feature Begin Attribute In subclass of AttributeSet
	UClass* AttributeOwnerValue = nullptr;
	if (OwnerClassProperty.IsValid())
	{
		UObject* ObjPtr = nullptr;
		OwnerClassProperty->GetValue(ObjPtr);
		AttributeOwnerValue = Cast<UClass>(ObjPtr);
	}
	//Feature End

	AttributeWidget = SNew(SSGGameplayAttributeWidget)
		.OnAttributeChanged(this, &FSGGameplayAttributeDetails::OnAttributeChanged)
		.DefaultProperty(PropertyValue)
		//Feature Begin Attribute In subclass of AttributeSet
		.DefaultAttributeOwnerClass(AttributeOwnerValue)
		//Feature End
		.FilterMetaData(FilterMetaStr)
		.ShowOnlyOwnedAttributes(bShowOnlyOwnedAttributes)
		.FilterClass(bShowOnlyOwnedAttributes ? OuterBaseClass : nullptr);

	HeaderRow.
	NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
		.MinDesiredWidth(500)
		.MaxDesiredWidth(4096)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.Padding(0.f, 0.f, 2.f, 0.f)
			[
				AttributeWidget.ToSharedRef()
			]
		];
}

void FSGGameplayAttributeDetails::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

//Feature Begin Attribute In subclass of AttributeSet
void FSGGameplayAttributeDetails::OnAttributeChanged(FProperty* SelectedAttribute, UClass* InAttributeOwnerClass) const
{
	if (MyProperty.IsValid())
	{
		MyProperty->SetValue(SelectedAttribute);

		//Feature Begin Attribute In subclass of AttributeSet
		OwnerClassProperty->SetValue(InAttributeOwnerClass);
		//Feature End

		// When we set the attribute we should also set the owner and name info
		if (OwnerProperty.IsValid())
		{
			OwnerProperty->SetValue(SelectedAttribute ? SelectedAttribute->GetOwnerStruct() : nullptr);
		}

		if (NameProperty.IsValid())
		{
			FString AttributeName = TEXT("None");
			if (SelectedAttribute)
			{
				SelectedAttribute->GetName(AttributeName);
			}
			NameProperty->SetValue(AttributeName);
		}
	}
}
//Feature End


// ReSharper disable once CppMemberFunctionMayBeStatic
void FSGGameplayAttributeDetails::HandleRequestRefresh(const TSharedPtr<IPropertyUtilities> InPropertyUtilities)
{
	SG_EDITOR_LOG(VeryVerbose, TEXT("FSGGameplayAttributeDetails::HandleRequestRefresh ..."))
	if (InPropertyUtilities.IsValid())
	{
		InPropertyUtilities->ForceRefresh();
	}
}
