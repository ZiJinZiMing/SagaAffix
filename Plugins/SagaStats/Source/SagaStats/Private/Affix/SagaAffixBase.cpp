// Fill out your copyright notice in the Description page of Project Settings.


#include "Affix/SagaAffixBase.h"

#include "SagaAbilitySystemComponent.h"

void USagaAffixBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if(USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo_Ensured()))
	{
		//DamageProcess
		for (TObjectPtr<UDamageProcessUnit> Unit : DamageProcessUnits)
		{
			ASC->GetDamageProcessUnits_Mutable().Add(Unit);
		}

		for (TSubclassOf<UGameplayEffect> EffectClass : Effects)
		{
			//todo: GEContext
			ASC->BP_ApplyGameplayEffectToSelf(EffectClass, 0, MakeEffectContext(Handle, ActorInfo));
		}

	}
	
}

void USagaAffixBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo_Ensured()))
	{
		//DamageProcess
		for (TObjectPtr<UDamageProcessUnit> Unit : DamageProcessUnits)
		{
			ASC->GetDamageProcessUnits_Mutable().Remove(Unit);
		}


		for (TSubclassOf<UGameplayEffect> EffectClass : Effects)
		{
			ASC->RemoveActiveGameplayEffectBySourceEffect(EffectClass, ASC);
		}
		
	}
}
