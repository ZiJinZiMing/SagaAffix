// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAbilitySystemGlobals.h"

#include "MyGameplayAbilityTypes.h"

FGameplayEffectContext* UMyAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FMyGameplayEffectContext();
}
