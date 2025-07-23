/******************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats is a status system that supports fully blueprintable attribute definitions and value calculations.
******************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SSExecutionCalculationBlueprintLibrary.generated.h"

struct FGameplayAttribute;
struct FGameplayEffectAttributeCaptureDefinition;
struct FGameplayEffectContextHandle;
struct FGameplayEffectCustomExecutionOutput;
struct FGameplayEffectCustomExecutionParameters;
struct FGameplayEffectSpec;
struct FGameplayTagContainer;

/**
 * Blueprint library for blueprint attribute sets.
 *
 * Includes Gameplay Effect Execution Calculation helpers for Blueprint implementation of Exec Calc classes.
 */
UCLASS()
class SAGASTATS_API USSExecutionCalculationBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
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
