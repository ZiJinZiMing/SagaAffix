// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "DamageProcessFlow.generated.h"

class UGameplayEffect;
struct FGameplayEffectContextHandle;
class UAffixableDamageFlow;
/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew)
class SAGASTATS_API UDamageProcessFlow : public UObject
{
	GENERATED_BODY()

public:
	/**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	FGameplayTag DamageFlowTag;

	/**/
	UFUNCTION(BlueprintNativeEvent, Category = "Damage")
	void Execute(UAffixableDamageFlow* DamageFlow, TSubclassOf<UGameplayEffect> DamageEffect, const FGameplayEffectContextHandle& Handle) const;

	/**/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	
	void CalcDamage( UAffixableDamageFlow* DamageFlow, TSubclassOf<UGameplayEffect> DamageEffect, const FGameplayEffectContextHandle& Handle)const;

	/**/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void ApplyDamage( UAffixableDamageFlow* DamageFlow, const FGameplayEffectContextHandle& Handle)const;


	UFUNCTION(BlueprintCallable, Category = "Damage")
	void ApplyDamageProcessUnits( UAffixableDamageFlow* DamageFlow, const FGameplayEffectContextHandle& Handle)const;

};
