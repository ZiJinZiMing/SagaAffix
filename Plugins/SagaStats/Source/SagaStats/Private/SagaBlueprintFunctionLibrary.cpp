// Fill out your copyright notice in the Description page of Project Settings.


#include "SagaBlueprintFunctionLibrary.h"

#include "SagaGameplayEffectContext.h"


bool USagaBlueprintFunctionLibrary::GetTargetDataHandleFromGEContext(FGameplayEffectContextHandle EffectContextHandle,FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if (FSagaGameplayEffectContext* GEContext = static_cast<FSagaGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		TargetDataHandle = GEContext->GetTargetData_Mutable();
		return true;
	}
	return false;
}
