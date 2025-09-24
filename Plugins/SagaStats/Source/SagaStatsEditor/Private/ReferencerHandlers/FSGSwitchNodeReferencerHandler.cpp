/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#include "FSGSwitchNodeReferencerHandler.h"

#include "SGEditorLog.h"
#include "EdGraph/EdGraph.h"
#include "Engine/Blueprint.h"
#include "SGEditorSubsystem.h"
#include "GraphNode/K2Node_SwitchGameplayAttribute.h"


TSharedPtr<ISGAttributeReferencerHandler> FSGSwitchNodeReferencerHandler::Create()
{
	return MakeShared<FSGSwitchNodeReferencerHandler>();
}

FSGSwitchNodeReferencerHandler::~FSGSwitchNodeReferencerHandler()
{
	PinAttributesCacheMap.Reset();
}

void FSGSwitchNodeReferencerHandler::OnPreCompile(const FString& InPackageName)
{
	PinAttributesCacheMap.Reset();
}

void FSGSwitchNodeReferencerHandler::OnPostCompile(const FString& InPackageName)
{
}

bool FSGSwitchNodeReferencerHandler::HandlePreCompile(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload)
{
	SG_EDITOR_NS_LOG(Verbose, TEXT("InAssetIdentifier: %s, InPayload: %s"), *InAssetIdentifier.ToString(), *InPayload.ToString())

	const UK2Node_SwitchGameplayAttribute* Node = Cast<UK2Node_SwitchGameplayAttribute>(InPayload.DefaultObject);
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

		const FString AttributeName = Attribute.GetName();
		const FProperty* Property = Attribute.GetUProperty();
		const FString PropertyPathName = Property ? Property->GetPathName() : TEXT("");

		FAttributeReference Reference;
		const FString FinalValue = FString::Printf(TEXT("(AttributeName=\"%s\",Attribute=%s)"), *AttributeName, *PropertyPathName);
		if (USGEditorSubsystem::ParseAttributeFromDefaultValue(FinalValue, Reference.PackageNameOwner, Reference.AttributeName))
		{
			Reference.Index = CurrentIndex  - 1;
			AttributesCache.Add(Reference);
		}
	}

	PinAttributesCacheMap.Add(InAssetIdentifier, AttributesCache);
	return true;
}

bool FSGSwitchNodeReferencerHandler::HandleAttributeRename(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SG_EDITOR_NS_LOG(Verbose, TEXT("InAssetIdentifier: %s, InPayload: %s"), *InAssetIdentifier.ToString(), *InPayload.ToString())
	
	UK2Node_SwitchGameplayAttribute* Node = Cast<UK2Node_SwitchGameplayAttribute>(InPayload.DefaultObject);
	if (!Node)
	{
		return false;
	}

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InPayload.PackageName);
	if (!Blueprint)
	{
		SG_EDITOR_NS_LOG(Warning, TEXT("FSGSwitchNodeReferencerHandler::HandleAttributeRename - Failed to update pin attributes because of invalid Blueprint for %s"), *InPayload.PackageName)
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

		// In 5.5, a renamed or removed attribute from a GBA Blueprint that is referenced there will be returned as valid,
		// whereas up to 5.4 Attribute.IsValid() was returning false as we used to expect
		
		// if (!Attribute.IsValid() && CachedAttribute.AttributeName == InPayload.OldPropertyName)
		if (CachedAttribute.AttributeName == InPayload.OldPropertyName)
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


bool FSGSwitchNodeReferencerHandler::HandleAttributeRemoved(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages)
{
	SG_EDITOR_NS_LOG(Verbose, TEXT("InAssetIdentifier: %s, InPayload: %s"), *InAssetIdentifier.ToString(), *InPayload.ToString())
	
	UK2Node_SwitchGameplayAttribute* Node = Cast<UK2Node_SwitchGameplayAttribute>(InPayload.DefaultObject);
	if (!Node)
	{
		return false;
	}

	const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *InPayload.PackageName);
	if (!Blueprint)
	{
		SG_EDITOR_NS_LOG(Warning, TEXT("Failed to update pin attributes because of invalid Blueprint for %s"), *InPayload.PackageName)
		return false;
	}

	int32 CurrentIndex = 0;
	
	// List of pin attributes pending a reset - e.g. set to back to None and not the attribute that was removed in GBA Blueprint
	TArray<FPinAttributeToReplace> PinAttributesToReset;
	for (const FGameplayAttribute& Attribute : Node->PinAttributes)
	{
		FAttributeReference CachedAttribute;
		if (!GetCachedAttributeForIndex(InAssetIdentifier, CurrentIndex, CachedAttribute))
		{
			CurrentIndex++;
			continue;
		}

		if (CachedAttribute.AttributeName == InPayload.RemovedPropertyName)
		{
			PinAttributesToReset.Add(FPinAttributeToReplace(CurrentIndex, nullptr));
		}

		CurrentIndex++;
	}

	// Now that we have the list of modifiers that needs an update, update pins
	for (const FPinAttributeToReplace& ModifierToReplace : PinAttributesToReset)
	{
		const int32 Index = ModifierToReplace.Index;

		if (!Node->PinAttributes.IsValidIndex(Index))
		{
			continue;
		}

		Node->PinAttributes[Index] = FGameplayAttribute();
	}

	if (!PinAttributesToReset.IsEmpty())
	{
		Node->ReconstructNode();
		if (Node->GetGraph())
		{
			Node->GetGraph()->NotifyGraphChanged();
		}
	}

	return !PinAttributesToReset.IsEmpty();
}

bool FSGSwitchNodeReferencerHandler::GetCachedAttributeForIndex(const FAssetIdentifier& InAssetIdentifier, int32 InIndex, FAttributeReference& OutAttributeReference)
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

