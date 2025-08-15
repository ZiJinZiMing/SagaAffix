// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageProcess/SagaDPU.h"

#include "SagaGameplayEffectContext.h"

void USagaDPU::Execute_Implementation(FGameplayEffectContextHandle ContextHandle)
{
	
}

void USagaDPU::GetRequiredTokens_Implementation(FGameplayTagContainer& OutRequiredTokens) const
{
	OutRequiredTokens = RequiredTokens;
}

void USagaDPU::GetProducedTokens_Implementation(FGameplayTagContainer& OutProducedTokens) const
{
	OutProducedTokens = ProducedTokens;
}

FString USagaDPU::GetDisplayName_Implementation() const
{
	return GetName();
}

bool USagaDPU::SatisfiedDamageProcess_Implementation(FGameplayEffectContextHandle ContextHandle) const
{
	return true;
}
