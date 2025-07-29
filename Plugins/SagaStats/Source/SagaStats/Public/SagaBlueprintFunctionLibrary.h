// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SagaGameplayEffectContext.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SagaBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SAGASTATS_API USagaBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
		
	UFUNCTION(BlueprintCallable)
	static bool GetTargetDataHandleFromGEContext(FGameplayEffectContextHandle EffectContextHandle,FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	
	UFUNCTION(BlueprintCallable)
	static bool GEContextAppendTargetData(FGameplayEffectContextHandle EffectContextHandle,const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION(BlueprintCallable)
	static FGameplayEffectContextHandle AbilityMakeEffectContext(const UGameplayAbility* Ability);
	
	
};
