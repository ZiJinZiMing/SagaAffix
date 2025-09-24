/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SGBlueprintLibrary.generated.h"

struct FGameplayEffectCustomExecutionOutput;
struct FGameplayEffectCustomExecutionParameters;
struct FSGAttributeSetExecutionData;
struct FGameplayAttribute;
class UAbilitySystemComponent;


/** Blueprint library for attribute sets */
UCLASS()
class SAGASTATS_API USGBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Returns an FString representation of a FSGAttributeSetExecutionData structure for debugging purposes.
	 *
	 * The separator used to join the data structure values is a line break (new line).
	 *
	 * @param ExecutionData	The data to get the debug string from.
	 */
	UFUNCTION(BlueprintPure, Category = "Attributes")
	static FString GetDebugStringFromExecutionData(const FSGAttributeSetExecutionData& ExecutionData);

	/**
	 * Returns an FString representation of a FSGAttributeSetExecutionData structure for debugging purposes.
	 *
	 * @param ExecutionData	The data to get the debug string from.
	 * @param Separator String separator to use when joining the structure values (Default: "\n" - new line)
	 */
	UFUNCTION(BlueprintPure, Category = "Attributes")
	static FString GetDebugStringFromExecutionDataSeparator(const FSGAttributeSetExecutionData& ExecutionData, const FString& Separator = TEXT(", "));

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

	
	/**
	 * 从 GameplayEffectContextHandle 中获取 InstigatorAbilitySystemComponent
	 * Gets the InstigatorAbilitySystemComponent from GameplayEffectContextHandle
	 *
	 * @param EffectContextHandle	要从中获取组件的上下文句柄 / The context handle to get the component from
	 * @return 发起者的能力系统组件，如果无效则返回 nullptr / The instigator's ability system component, or nullptr if invalid
	 */
	UFUNCTION(BlueprintPure, Category = "Attributes")
	static UAbilitySystemComponent* GetInstigatorAbilitySystemComponentFromGEContext(FGameplayEffectContextHandle EffectContextHandle);



	// ----------------------------------------------------------------------------------------------------------------
	//	Calculation
	// ----------------------------------------------------------------------------------------------------------------
	UFUNCTION(BlueprintPure, Category = "Calculation")
	static const FGameplayEffectSpec& GetOwningSpec(const FGameplayEffectCustomExecutionParameters& InExecutionParams);

	UFUNCTION(BlueprintPure, Category = "Calculation")
	static FGameplayEffectContextHandle GetEffectContext(const FGameplayEffectCustomExecutionParameters& InExecutionParams);
	
	UFUNCTION(BlueprintPure, Category = "Calculation")
	static const FGameplayTagContainer& GetSourceTags(const FGameplayEffectCustomExecutionParameters& InExecutionParams);
	
	UFUNCTION(BlueprintPure, Category = "Calculation")
	static const FGameplayTagContainer& GetTargetTags(const FGameplayEffectCustomExecutionParameters& InExecutionParams);
	
	UFUNCTION(BlueprintPure, Category = "Calculation")
	static bool AttemptCalculateCapturedAttributeMagnitude(UPARAM(ref) const FGameplayEffectCustomExecutionParameters& InExecutionParams, UPARAM(ref) const TArray<FGameplayEffectAttributeCaptureDefinition>& InRelevantAttributesToCapture, const FGameplayAttribute InAttribute, float& OutMagnitude);
	
	UFUNCTION(BlueprintCallable, Category = "Calculation")
	static bool AttemptCalculateCapturedAttributeMagnitudeWithBase(UPARAM(ref) const FGameplayEffectCustomExecutionParameters& InExecutionParams, UPARAM(ref) const TArray<FGameplayEffectAttributeCaptureDefinition>& InRelevantAttributesToCapture, const FGameplayAttribute InAttribute, const float InBaseValue, float& OutMagnitude);
	
	UFUNCTION(BlueprintCallable, Category = "Calculation")
	static const FGameplayEffectCustomExecutionOutput& AddOutputModifier(UPARAM(ref) FGameplayEffectCustomExecutionOutput& InExecutionOutput, const FGameplayAttribute InAttribute, const EGameplayModOp::Type InModOp, const float InMagnitude);

	UFUNCTION(BlueprintPure, Category = "Calculation")
	static bool AttemptCalculateTransientAggregatorMagnitude(const FGameplayEffectCustomExecutionParameters& ExecutionParameters, FGameplayTag InAggregatorIdentifier, float& OutMagnitude);


	
};
