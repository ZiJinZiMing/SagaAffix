// Fill out your copyright notice in the Description page of Project Settings.


#include "Damage/AffixableDamageFlow.h"

#include "SagaGameplayEffectContext.h"
#include "Damage/DamageProcessFlow.h"

UAffixableDamageFlow::UAffixableDamageFlow(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	
}

void UAffixableDamageFlow::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	FGameplayEffectContextHandle GEContextHandle = MakeEffectContext(Handle, ActorInfo);
	if (FSagaGameplayEffectContext* SagaGEContext = static_cast<FSagaGameplayEffectContext*>(GEContextHandle.Get()))
	{
		SagaGEContext->GetTargetData_Mutable() = TriggerEventData->TargetData;
		
		DamageProcess(GEContextHandle);

		DamageReaction(GEContextHandle);
	}
	else
	{
		// 记录错误：无法获取到 SagaGameplayEffectContext / Log error: Failed to get SagaGameplayEffectContext
		UE_LOG(LogTemp, Error, TEXT("UAffixableDamageFlow::ActivateAbility - Failed to cast GEContextHandle to FSagaGameplayEffectContext"));
	}

}

void UAffixableDamageFlow::DamageReaction_Implementation(FGameplayEffectContextHandle ContextHandle)
{
	
}

void UAffixableDamageFlow::DamageProcess_Implementation(FGameplayEffectContextHandle ContextHandle)
{
	for (TObjectPtr<UDamageProcessFlow> DamageFlow : DamagePreset->DamageFlows)
	{
		if (DamageFlow)
		{
			if (DamageFlow->DamageFlowTag.IsValid() && DamageEffects.Contains(DamageFlow->DamageFlowTag))
			{
				DamageFlow->Execute(this, DamageEffects[DamageFlow->DamageFlowTag], ContextHandle);
			}
		}
	}
}
