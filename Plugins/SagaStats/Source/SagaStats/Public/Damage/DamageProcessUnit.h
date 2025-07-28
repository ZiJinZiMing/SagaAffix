// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "DamageProcessUnit.generated.h"

struct FGameplayEffectContextHandle;
class UAffixableDamageFlow;
class UDamageProcessFlow;
/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew)
class SAGASTATS_API UDamageProcessUnit : public UObject
{
	GENERATED_BODY()

public:
	/**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	FGameplayTag DamageFlowTag;

	/**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	int Priority;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void Execute(const UDamageProcessFlow* ProcessFlow, UAffixableDamageFlow* DamageFlow, FGameplayEffectContextHandle Handle); 
};
