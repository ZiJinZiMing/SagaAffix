// Fill out your copyright notice in the Description page of Project Settings.


#include "SagaBlueprintFunctionLibrary.h"
#include "SagaGameplayEffectContext.h"
#include "Abilities/GameplayAbility.h"


bool USagaBlueprintFunctionLibrary::GetTargetDataHandleFromGEContext(FGameplayEffectContextHandle EffectContextHandle,FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	if (FSagaGameplayEffectContext* GEContext = static_cast<FSagaGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		OutTargetDataHandle = GEContext->GetTargetData_Mutable();
		return true;
	}
	return false;
}

bool USagaBlueprintFunctionLibrary::GEContextAppendTargetData(FGameplayEffectContextHandle EffectContextHandle,
	const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if (FSagaGameplayEffectContext* GEContext = static_cast<FSagaGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		GEContext->GetTargetData_Mutable().Append(TargetDataHandle);
		return true;
	}
	return false;
}

FGameplayEffectContextHandle USagaBlueprintFunctionLibrary::AbilityMakeEffectContext(const UGameplayAbility* Ability)
{
	return Ability->MakeEffectContext(Ability->GetCurrentAbilitySpecHandle(), Ability->GetCurrentActorInfo());
}
