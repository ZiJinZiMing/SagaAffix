/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SagaBlueprintLibrary.generated.h"

struct FSagaAttributeSetExecutionData;
struct FGameplayAttribute;


/** Blueprint library for blueprint attribute sets */
UCLASS()
class SAGASTATS_API USagaBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Returns an FString representation of a FSagaAttributeSetExecutionData structure for debugging purposes.
	 *
	 * The separator used to join the data structure values is a line break (new line).
	 *
	 * @param ExecutionData	The data to get the debug string from.
	 */
	UFUNCTION(BlueprintPure, Category = "Attributes")
	static FString GetDebugStringFromExecutionData(const FSagaAttributeSetExecutionData& ExecutionData);

	/**
	 * Returns an FString representation of a FSagaAttributeSetExecutionData structure for debugging purposes.
	 *
	 * @param ExecutionData	The data to get the debug string from.
	 * @param Separator String separator to use when joining the structure values (Default: "\n" - new line)
	 */
	UFUNCTION(BlueprintPure, Category = "Attributes")
	static FString GetDebugStringFromExecutionDataSeparator(const FSagaAttributeSetExecutionData& ExecutionData, const FString& Separator = TEXT(", "));

	/** Returns the Attribute name */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	static FString GetDebugStringFromAttribute(const FGameplayAttribute& Attribute);

	/** Simple equality operator for gameplay attributes and string (for K2 Switch Node) */
	UFUNCTION(BlueprintPure, Category = "Attributes | PinOptions", meta = (BlueprintInternalUseOnly = "true"))
	static bool NotEqual_GameplayAttributeGameplayAttribute(FGameplayAttribute A, FString B);

	/** Returns the Attribute name as an FText */
	UFUNCTION(BlueprintPure, Category = "Attributes")
	static FText GetAttributeDisplayNameText(const FGameplayAttribute& InAttribute);

	UFUNCTION(BlueprintPure, Category = "Attributes")
	static bool GetAttributeByPropertyName(const TSubclassOf<UAttributeSet>& AttributeSetClass, const FName& PropertyName, FGameplayAttribute& OutAttribute);
};
