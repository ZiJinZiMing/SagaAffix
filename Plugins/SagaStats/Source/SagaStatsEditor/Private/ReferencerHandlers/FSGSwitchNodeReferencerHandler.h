/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"
#include "ReferencerHandlers/ISGAttributeReferencerHandler.h"
#include "UObject/WeakFieldPtr.h"

#if UE_VERSION_NEWER_THAN(5, 2, -1)
#include "AssetRegistry/AssetIdentifier.h"
#else
#include "AssetRegistry/AssetData.h"
#endif

/**
 * Handlers for attribute renames on UK2Node_SwitchGameplayAttribute nodes.
 */
class SAGASTATSEDITOR_API FSGSwitchNodeReferencerHandler : public ISGAttributeReferencerHandler
{
public:
	/** Data holder for a Pin Attribute property that needs an update */
	struct FPinAttributeToReplace
	{
		int32 Index = 0;
		TWeakFieldPtr<FProperty> Property = nullptr;

		FPinAttributeToReplace(const int32 Index, FProperty* Property)
			: Index(Index)
			, Property(Property)
		{
		}
	};
	
	/** Data holder for cache map, representing a reference to a FGameplayAttribute property in a K2SwitchNode PinAttributes */
	struct FAttributeReference
	{
		FString PackageNameOwner;
		FString AttributeName;
		int32 Index = -1;

		FAttributeReference() = default;
	};
	
	static TSharedPtr<ISGAttributeReferencerHandler> Create();
	virtual ~FSGSwitchNodeReferencerHandler() override;

	//~ Begin ISGAttributeReferencerHandler
	virtual void OnPreCompile(const FString& InPackageName) override;
	virtual void OnPostCompile(const FString& InPackageName) override;
	virtual bool HandlePreCompile(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload) override;
	virtual bool HandleAttributeRename(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages) override;
	virtual bool HandleAttributeRemoved(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages) override;
	//~ End ISGAttributeReferencerHandler
protected:
	TMap<FAssetIdentifier, TArray<FAttributeReference>> PinAttributesCacheMap;
	
	bool GetCachedAttributeForIndex(const FAssetIdentifier& InAssetIdentifier, int32 InIndex, FAttributeReference& OutAttributeReference);
};
