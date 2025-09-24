/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/UObjectBaseUtility.h"
#include "UObject/WeakObjectPtr.h"

class UBlueprint;
class FTokenizedMessage;
struct FAssetIdentifier;

struct SAGASTATSEDITOR_API FSGAttributeReferencerPayload
{
	/** UObject class default object container of referencer */
	TWeakObjectPtr<UObject> DefaultObject;

	/** UObject class default object container of referencer */
	TWeakObjectPtr<UBlueprint> ReferencerBlueprint;
	
	/** Original package name of owner AttributeSet (from which an attribute got renamed) */
	FString PackageName;

	/** Value of previous attribute name */
	FString OldPropertyName;
	
	/** Value of new attribute name */
	FString NewPropertyName;

	/** Value of a removed attribute name, will only be filled when a variable is removed (not renamed) */
	FString RemovedPropertyName;
	
	FSGAttributeReferencerPayload() = default;

	FString ToString() const
	{
		return FString::Printf(
			TEXT("DefaultObject: %s, PackageName: %s, OldPropertyName: %s, NewPropertyName: %s"),
			*GetNameSafe(DefaultObject.Get()),
			*PackageName,
			*OldPropertyName,
			*NewPropertyName
		);
	}
};

/**
 * Interface for custom referencer handler for a given UObject CDO
 */
class SAGASTATSEDITOR_API ISGAttributeReferencerHandler : public TSharedFromThis<ISGAttributeReferencerHandler>
{
public:
	virtual ~ISGAttributeReferencerHandler() = default;

	virtual void OnPreCompile(const FString& InPackageName) = 0;
	virtual void OnPostCompile(const FString& InPackageName) = 0;
	
	virtual bool HandlePreCompile(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload) = 0;
	virtual bool HandleAttributeRename(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages) = 0;
	virtual bool HandleAttributeRemoved(const FAssetIdentifier& InAssetIdentifier, const FSGAttributeReferencerPayload& InPayload, TArray<TSharedRef<FTokenizedMessage>>& OutMessages) = 0;
};
