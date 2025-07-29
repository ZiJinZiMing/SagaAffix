﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "SagaAffixBase.generated.h"

class UDamageProcessUnit;
/**
 * 
 */
UCLASS(Abstract)
class SAGASTATS_API USagaAffixBase : public UGameplayAbility
{
	GENERATED_BODY()

public:

	USagaAffixBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(Instanced, EditAnywhere, Category = Affix)
	TArray<TObjectPtr<UDamageProcessUnit>> DamageProcessUnits;
	
	UPROPERTY(EditAnywhere, Category = Affix)
	TArray<TSubclassOf<UGameplayEffect>> Effects;
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
};
