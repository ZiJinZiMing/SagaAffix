// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "SagaDPU.generated.h"


struct FGameplayEffectContextHandle;
/**
 * 伤害计算单元 / Damage Processing Unit
 * Base class for damage processing units in the DAG system
 */
UCLASS(Abstract)
class SAGASTATS_API USagaDPU : public UObject
{
	GENERATED_BODY()


protected:
	/**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RequiredTokens;

	/**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer ProducedTokens;

	// 优先级 - Priority (用于同层节点排序)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Priority;
	
public:
	int GetPriority() const { return Priority; }
	
	/**
	 * 获取此DPU需要的Token类型 / Get required token types for this DPU
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Saga|DamageProcess")
	void GetRequiredTokens(FGameplayTagContainer& OutRequiredTokens) const;

	/**
	 * 获取此DPU产生的Token类型 / Get produced token types from this DPU
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Saga|DamageProcess")
	void GetProducedTokens(FGameplayTagContainer& OutProducedTokens) const;

	

	/**
	 * 执行DPU处理 / Execute DPU processing
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Saga|DamageProcess")
	void Execute(FGameplayEffectContextHandle ContextHandle);
	
	UFUNCTION(BlueprintNativeEvent,Category = "Saga|DamageProcess")
	FString GetDisplayName() const;

	/*是否满足当前DamageProcess条件*/
	UFUNCTION(BlueprintNativeEvent, Category = "Saga|DamageProcess")
	bool SatisfiedDamageProcess(FGameplayEffectContextHandle ContextHandle) const;
		
};
