// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageFlowPreset.h"
#include "Abilities/GameplayAbility.h"

#include "AffixableDamageFlow.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class SAGASTATS_API UAffixableDamageFlow : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAffixableDamageFlow(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	/**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	TObjectPtr<UDamageFlowPreset> DamagePreset;

protected:
	
	/*
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	*/

	/**/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void DamageProcess(FGameplayEffectContextHandle ContextHandle, const TMap<FGameplayTag, TSubclassOf<UGameplayEffect>>& DamageEffects);

	/*
	/*#1#
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void DamageReaction(FGameplayEffectContextHandle ContextHandle);

	/*#1#
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void GetEffects(FGameplayEffectContextHandle ContextHandle, TMap<FGameplayTag, TSubclassOf<UGameplayEffect>>& OutDamageEffects);

	/*#1#
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void MakeDamageGEContext(const FGameplayEventData& TriggerEventData,FGameplayEffectContextHandle& OutContextHandle);
	*/

	
};
