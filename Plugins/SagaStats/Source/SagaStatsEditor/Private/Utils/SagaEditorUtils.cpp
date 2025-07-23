/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/

#include "Utils/SagaEditorUtils.h"

#include "AttributeSet.h"
#include "DetailLayoutBuilder.h"
#include "Editor.h"
#include "Editor/SagaAttributeSetBlueprintEditor.h"
#include "Misc/EngineVersionComparison.h"
#include "Subsystems/AssetEditorSubsystem.h"

UAttributeSet* SagaEditorUtils::GetAttributeBeingCustomized(const IDetailLayoutBuilder& InDetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	InDetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	TArray<UAttributeSet*> AttributeSetsBeingCustomized;
	for (TWeakObjectPtr<UObject> ObjectBeingCustomized : ObjectsBeingCustomized)
	{
		if (UAttributeSet* AttributeSet = Cast<UAttributeSet>(ObjectBeingCustomized.Get()))
		{
			AttributeSetsBeingCustomized.Add(AttributeSet);
		}
	}

	return AttributeSetsBeingCustomized.IsValidIndex(0) ? AttributeSetsBeingCustomized[0] : nullptr;
}

UBlueprint* SagaEditorUtils::GetBlueprintFromClass(const UClass* InClass)
{
	if (!InClass)
	{
		return nullptr;
	}

	return Cast<UBlueprint>(InClass->ClassGeneratedBy);
}

TWeakPtr<FSagaAttributeSetBlueprintEditor> SagaEditorUtils::FindBlueprintEditorForAsset(UObject* InObject)
{
	if (!GEditor || !IsValid(InObject))
	{
		return nullptr;
	}

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!IsValid(AssetEditorSubsystem))
	{
		return nullptr;
	}

	IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(InObject, false);
	if (!EditorInstance)
	{
		return nullptr;
	}

	FSagaAttributeSetBlueprintEditor* BlueprintEditor = StaticCast<FSagaAttributeSetBlueprintEditor*>(EditorInstance);
	if (!BlueprintEditor)
	{
		return nullptr;
	}
#if UE_VERSION_NEWER_THAN(5, 1, -1)
	return StaticCastWeakPtr<FSagaAttributeSetBlueprintEditor>(BlueprintEditor->AsWeak());
#else
	TWeakPtr<FSagaBlueprintEditor> WeakPtr(StaticCastSharedRef<FSagaBlueprintEditor>(BlueprintEditor->AsShared()));
	return WeakPtr;
#endif
}
