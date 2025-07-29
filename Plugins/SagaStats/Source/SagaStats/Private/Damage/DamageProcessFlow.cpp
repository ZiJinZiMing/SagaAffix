// Fill out your copyright notice in the Description page of Project Settings.


#include "Damage/DamageProcessFlow.h"

#include "GameplayEffectTypes.h"
#include "SagaAbilitySystemComponent.h"
#include "Damage/AffixableDamageFlow.h"
#include "Damage/DamageProcessUnit.h"
#include "Abilities/GameplayAbility.h"


void UDamageProcessFlow::Execute_Implementation(UAffixableDamageFlow* DamageFlow, TSubclassOf<UGameplayEffect> DamageEffect,const FGameplayEffectContextHandle& Handle)const
{
	CalcDamage(DamageFlow, DamageEffect, Handle);

	//DamageProcessor
	ApplyDamageProcessUnits(DamageFlow,Handle);

	//Apply to attributeset
	ApplyDamage(DamageFlow,  Handle);
	
}

void UDamageProcessFlow::ApplyDamageProcessUnits(UAffixableDamageFlow* DamageFlow,
	const FGameplayEffectContextHandle& Handle) const
{
	//DamageProcessor
	if (USagaAbilitySystemComponent* ASC = Cast<USagaAbilitySystemComponent>(DamageFlow->GetAbilitySystemComponentFromActorInfo_Ensured()))
	{
		//Filter by DamageFlowTag
		TArray<TObjectPtr<UDamageProcessUnit>> Units = ASC->GetDamageProcessUnits().FilterByPredicate([this](TObjectPtr<UDamageProcessUnit> Unit)
		{
			return Unit->DamageFlowTag == DamageFlowTag;
		});

		//Sort by Priority
		Units.Sort([](const TObjectPtr<UDamageProcessUnit> A, const TObjectPtr<UDamageProcessUnit> B) { return A->Priority > B->Priority; });

		//Execute
		for (TObjectPtr<UDamageProcessUnit> Unit : Units)
		{
			Unit->Execute(this, DamageFlow, Handle);
		}
	}

}

void UDamageProcessFlow::CalcDamage_Implementation(UAffixableDamageFlow* DamageFlow, TSubclassOf<UGameplayEffect> DamageEffect, const FGameplayEffectContextHandle& Handle)const
{
	UAbilitySystemComponent* ASC = DamageFlow->GetAbilitySystemComponentFromActorInfo_Ensured();
	FGameplayEffectSpecHandle GESpec = ASC->MakeOutgoingSpec(DamageEffect, 0,Handle);
	ASC->BP_ApplyGameplayEffectSpecToSelf(GESpec);
}

void UDamageProcessFlow::ApplyDamage_Implementation(UAffixableDamageFlow* DamageFlow, 
	const FGameplayEffectContextHandle& Handle)const
{
	
}
