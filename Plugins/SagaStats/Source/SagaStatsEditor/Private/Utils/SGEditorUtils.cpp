/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#include "Utils/SGEditorUtils.h"

#include "AttributeSet.h"
#include "DetailLayoutBuilder.h"
#include "Editor.h"
#include "Editor/SGAttributeSetBlueprintEditor.h"
#include "Misc/EngineVersionComparison.h"
#include "Subsystems/AssetEditorSubsystem.h"

UAttributeSet* SGEditorUtils::GetAttributeBeingCustomized(const IDetailLayoutBuilder& InDetailLayout)
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

UBlueprint* SGEditorUtils::GetBlueprintFromClass(const UClass* InClass)
{
	if (!InClass)
	{
		return nullptr;
	}

	return Cast<UBlueprint>(InClass->ClassGeneratedBy);
}

TWeakPtr<FSGAttributeSetBlueprintEditor> SGEditorUtils::FindBlueprintEditorForAsset(UObject* InObject)
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

	FSGAttributeSetBlueprintEditor* BlueprintEditor = StaticCast<FSGAttributeSetBlueprintEditor*>(EditorInstance);
	if (!BlueprintEditor)
	{
		return nullptr;
	}
	return StaticCastWeakPtr<FSGAttributeSetBlueprintEditor>(BlueprintEditor->AsWeak());
}
