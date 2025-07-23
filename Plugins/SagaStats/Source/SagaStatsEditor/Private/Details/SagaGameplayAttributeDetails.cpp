/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#include "Details/SagaGameplayAttributeDetails.h"

#include "AttributeSet.h"
#include "DetailWidgetRow.h"
#include "SagaStatsDelegates.h"
#include "SSEditorLog.h"
#include "IPropertyUtilities.h"
#include "Details/Slate/SSagaGameplayAttributeWidget.h"

FSagaGameplayAttributeDetails::~FSagaGameplayAttributeDetails()
{
	FSagaStatsDelegates::OnRequestDetailsRefresh.RemoveAll(this);
}

TSharedRef<IPropertyTypeCustomization> FSagaGameplayAttributeDetails::MakeInstance()
{
	return MakeShared<FSagaGameplayAttributeDetails>();
}

// ReSharper disable once CppParameterMayBeConst
void FSagaGameplayAttributeDetails::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	SS_EDITOR_LOG(Verbose, TEXT("FSagaGameplayAttributeDetails::CustomizeHeader ..."))

	const TSharedPtr<IPropertyUtilities> Utilities = StructCustomizationUtils.GetPropertyUtilities();
	FSagaStatsDelegates::OnRequestDetailsRefresh.AddSP(this, &FSagaGameplayAttributeDetails::HandleRequestRefresh, Utilities);

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
	SS_EDITOR_LOG(
		VeryVerbose,
		TEXT("FSagaGameplayAttributeDetails::CustomizeHeader - OuterBaseClass: %s (bShowOnlyOwnedAttributed: %s)"),
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

	AttributeWidget = SNew(SSagaGameplayAttributeWidget)
		.OnAttributeChanged(this, &FSagaGameplayAttributeDetails::OnAttributeChanged)
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

void FSagaGameplayAttributeDetails::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

//Feature Begin Attribute In subclass of AttributeSet
void FSagaGameplayAttributeDetails::OnAttributeChanged(FProperty* SelectedAttribute, UClass* InAttributeOwnerClass) const
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
void FSagaGameplayAttributeDetails::HandleRequestRefresh(const TSharedPtr<IPropertyUtilities> InPropertyUtilities)
{
	SS_EDITOR_LOG(VeryVerbose, TEXT("FSagaGameplayAttributeDetails::HandleRequestRefresh ..."))
	if (InPropertyUtilities.IsValid())
	{
		InPropertyUtilities->ForceRefresh();
	}
}
