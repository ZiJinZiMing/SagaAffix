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
	
	/**
	 * 检查给定的标签容器是否满足指定的标签需求 / Check if the given tag container meets the specified tag requirements
	 * @param TagContainer 要检查的标签容器 / Tag container to check
	 * @param Requirements 标签需求 / Tag requirements
	 * @return 如果满足需求则返回true / Returns true if requirements are met
	 */
	UFUNCTION(BlueprintPure, Category = "Calculation|Tags")
	static bool RequirementsMet(const FGameplayTagContainer& TagContainer, const FGameplayTagRequirements& Requirements);
	
	/**
	 * 打印FGameplayEffectCustomExecutionOutput中的修改器信息 / Print modifier information from FGameplayEffectCustomExecutionOutput
	 * @param ExecutionOutput 要打印的执行输出 / Execution output to print
	 */
	UFUNCTION(BlueprintCallable, Category = "Calculation|Debug")
	static void PrintExecutionOutputModifiers(const FGameplayEffectCustomExecutionOutput& ExecutionOutput);

};
