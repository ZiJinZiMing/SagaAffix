/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "AssetTypes/SagaAssetTypeActions_AttributeSet.h"

#include "SagaAttributeSet.h"
#include "AssetTypes/SagaBlueprintFactory.h"
#include "Editor/SagaAttributeSetBlueprintEditor.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "SagaAssetTypeActions_AttributeSet"

FSagaAssetTypeActions_AttributeSet::FSagaAssetTypeActions_AttributeSet(const EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

void FSagaAssetTypeActions_AttributeSet::OpenAssetEditor(const TArray<UObject*>& InObjects, const TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(*ObjIt))
		{
			const TSharedRef<FSagaAttributeSetBlueprintEditor> NewEditor(new FSagaAttributeSetBlueprintEditor());

			TArray<UBlueprint*> Blueprints;
			Blueprints.Add(Blueprint);

			NewEditor->InitAttributeSetEditor(Mode, EditWithinLevelEditor, Blueprints, false);
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FailedToLoadAttributeSetBlueprint", "Attribute Set Blueprint could not be loaded because it derives from an invalid class. Check to make sure the parent class for this blueprint hasn't been removed!"));
		}
	}
}

const TArray<FText>& FSagaAssetTypeActions_AttributeSet::GetSubMenus() const
{
	static const TArray SubMenus
	{
		LOCTEXT("AssetTypeActions_SagaAttributesSubMenu", "SagaStats")
	};

	return SubMenus;
}

UFactory* FSagaAssetTypeActions_AttributeSet::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	check(InBlueprint && InBlueprint->IsA(USagaAttributeSetBlueprint::StaticClass()));
	USagaBlueprintFactory* Factory = NewObject<USagaBlueprintFactory>();
	Factory->ParentClass = TSubclassOf<USagaAttributeSet>(*InBlueprint->GeneratedClass);
	return Factory;
}

#undef LOCTEXT_NAMESPACE
