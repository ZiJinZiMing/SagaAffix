/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#include "FSSSwitchNodeReferencerHandler.h"

#include "SSEditorLog.h"
#include "Editor/SagaK2Node_SwitchGameplayAttribute.h"
#include "EdGraph/EdGraph.h"
#include "Engine/Blueprint.h"
#include "SagaEditorSubsystem.h"


TSharedPtr<ISSAttributeReferencerHandler> FSSSwitchNodeReferencerHandler::Create()
{
	return MakeShared<FSSSwitchNodeReferencerHandler>();
}

FSSSwitchNodeReferencerHandler::~FSSSwitchNodeReferencerHandler()
{
	PinAttributesCacheMap.Reset();
}

void FSSSwitchNodeReferencerHandler::OnPreCompile(const FString& InPackageName)
{
	PinAttributesCacheMap.Reset();
}

void FSSSwitchNodeReferencerHandler::OnPostCompile(const FString& InPackageName)
{
}

bool FSSSwitchNodeReferencerHandler::HandlePreCompile(const FAssetIdentifier& InAssetIdentifier, const FSSAttributeReferencerPayload& InPayload)
{
	SS_EDITOR_NS_LOG(Verbose, TEXT("InAssetIdentifier: %s, InPayload: %s"), *InAssetIdentifier.ToString(), *InPayload.ToString())

	const USagaK2Node_SwitchGameplayAttribute* Node = Cast<USagaK2Node_SwitchGameplayAttribute>(InPayload.DefaultObject);
	if (!Node)
	{
		return false;
	}

	TArray<FGameplayAttribute> GameplayAttributes = Node->PinAttributes;
	
	TArray<FAttributeReference> AttributesCache;
	AttributesCache.Reserve(Node->PinAttributes.Num());

	int32 CurrentIndex = 0;
	for (const FGameplayAttribute& Attribute : Node->PinAttributes)
	{
		++CurrentIndex;
		
		if (!Attribute.IsValid())
		{
			continue;
		}

		FString FinalValue;
		FGameplayAttribute::StaticStruct()->ExportText(FinalValue, &Attribute, &Attribute, nullptr, PPF_SerializedAsImportText, nullptr);
		if (FinalValue.IsEmpty())
		{
			continue;
		}

		FAttributeReference Reference;
		if (USagaEditorSubsystem::ParseAttributeFromDefaultValue(FinalValue, Reference.PackageNameOwner, Reference.AttributeName))
		{
			Reference.Index = CurrentIndex  - 1;
			AttributesCache.Add(Reference);
		}
	}

	PinAttributesCacheMap.Add(InAssetIdentifier, AttributesCache);
	return true;
}

bool FSSSwitchNodeReferencerHandler::HandleAttributeRename(const FAssetIdentifier& InAssetIdentifier, const FSSAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SS_EDITOR_NS_LOG(Verbose, TEXT("InAssetIdentifier: %s, InPayload: %s"), *InAssetIdentifier.ToString(), *InPayload.ToString())
	
	USagaK2Node_SwitchGameplayAttribute* Node = Cast<USagaK2Node_SwitchGameplayAttribute>(InPayload.DefaultObject);
	if (!Node)
	{
		return false;
	}

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InPayload.PackageName);
	if (!Blueprint)
	{
		SS_EDITOR_NS_LOG(Warning, TEXT("FSSSwitchNodeReferencerHandler::HandleAttributeRename - Failed to update pin attributes because of invalid Blueprint for %s"), *InPayload.PackageName)
		return false;
	}

	int32 CurrentIndex = 0;
	TArray<FPinAttributeToReplace> PinAttributesToReplace;
	for (const FGameplayAttribute& Attribute : Node->PinAttributes)
	{
		FAttributeReference CachedAttribute;
		if (!GetCachedAttributeForIndex(InAssetIdentifier, CurrentIndex, CachedAttribute))
		{
			CurrentIndex++;
			continue;
		}

		if (!Attribute.IsValid() && CachedAttribute.AttributeName == InPayload.OldPropertyName)
		{
			if (!Blueprint->GeneratedClass)
			{
				CurrentIndex++;
				continue;
			}

			if (FProperty* Prop = FindFProperty<FProperty>(Blueprint->GeneratedClass, FName(*InPayload.NewPropertyName)))
			{
				PinAttributesToReplace.Add(FPinAttributeToReplace(CurrentIndex, Prop));
			}
		}

		CurrentIndex++;
	}

	// Now that we have the list of modifiers that needs an update, update modifiers
	for (const FPinAttributeToReplace& ModifierToReplace : PinAttributesToReplace)
	{
		const int32 Index = ModifierToReplace.Index;

		if (!Node->PinAttributes.IsValidIndex(Index))
		{
			continue;
		}

		Node->PinAttributes[Index] = FGameplayAttribute(ModifierToReplace.Property.Get());
	}

	if (!PinAttributesToReplace.IsEmpty())
	{
		Node->ReconstructNode();
		if (Node->GetGraph())
		{
			Node->GetGraph()->NotifyGraphChanged();
		}
	}

	return !PinAttributesToReplace.IsEmpty();
}

bool FSSSwitchNodeReferencerHandler::GetCachedAttributeForIndex(const FAssetIdentifier& InAssetIdentifier, int32 InIndex, FAttributeReference& OutAttributeReference)
{
	if (!PinAttributesCacheMap.Contains(InAssetIdentifier))
	{
		return false;
	}

	TArray<FAttributeReference> Attributes = PinAttributesCacheMap.FindChecked(InAssetIdentifier);
	FAttributeReference* FoundElement = Attributes.FindByPredicate([InIndex](const FAttributeReference& Item)
	{
		return Item.Index == InIndex;
	});

	if (!FoundElement)
	{
		return false;
	}

	OutAttributeReference = MoveTemp(*FoundElement);
	return true;
}

