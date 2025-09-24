/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "AssetTypes/SGAssetTypeActions_AttributeSet.h"

#include "AssetTypes/SGBlueprintFactory.h"
#include "Editor/SGAttributeSetBlueprintEditor.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "SGAssetTypeActions_AttributeSet"

FSGAssetTypeActions_AttributeSet::FSGAssetTypeActions_AttributeSet(const EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

void FSGAssetTypeActions_AttributeSet::OpenAssetEditor(const TArray<UObject*>& InObjects, const TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(*ObjIt))
		{
			const TSharedRef<FSGAttributeSetBlueprintEditor> NewEditor(new FSGAttributeSetBlueprintEditor());

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

const TArray<FText>& FSGAssetTypeActions_AttributeSet::GetSubMenus() const
{
	static const TArray SubMenus
	{
		LOCTEXT("AssetTypeActions_AttributesSubMenu", "SagaStats")
	};

	return SubMenus;
}

UFactory* FSGAssetTypeActions_AttributeSet::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	check(InBlueprint && InBlueprint->IsA(USGAttributeSetBlueprint::StaticClass()));
	USGBlueprintFactory* Factory = NewObject<USGBlueprintFactory>();
	Factory->ParentClass = TSubclassOf<USGAttributeSet>(*InBlueprint->GeneratedClass);
	return Factory;
}

#undef LOCTEXT_NAMESPACE
