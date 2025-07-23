/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"
#include "SagaAttributeSetBlueprint.h"

class FSagaAssetTypeActions_AttributeSet : public FAssetTypeActions_Blueprint
{
public:
	explicit FSagaAssetTypeActions_AttributeSet(EAssetTypeCategories::Type InAssetCategory);

	// IAssetTypeActions Implementation
	virtual FText GetName() const override
	{
		return NSLOCTEXT("AssetTypeActions", "SagaAssetTypeActions_AttributeSet", "SagaAttributeSet");
	}

	virtual FColor GetTypeColor() const override
	{
		return FColor(0, 87, 96);
	}
	
	virtual UClass* GetSupportedClass() const override
	{
		return USagaAttributeSetBlueprint::StaticClass();
	}
	
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override
	{
		return AssetCategory;
	}

	virtual const TArray<FText>& GetSubMenus() const override;
	
	// virtual uint32 GetCategories() override { return EAssetTypeCategories::Gameplay; }
	// virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;
	// virtual void PerformAssetDiff(UObject* Asset1, UObject* Asset2, const struct FRevisionInfo& OldRevision, const struct FRevisionInfo& NewRevision) const override;

	// FAssetTypeActions_Blueprint interface
	virtual UFactory* GetFactoryForBlueprintType(UBlueprint* InBlueprint) const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
