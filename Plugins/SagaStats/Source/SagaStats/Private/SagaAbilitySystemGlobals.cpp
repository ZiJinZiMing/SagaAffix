// Fill out your copyright notice in the Description page of Project Settings.


#include "SagaAbilitySystemGlobals.h"

#include "SagaGameplayEffectContext.h"


FGameplayEffectContext* USagaAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FSagaGameplayEffectContext();
}
