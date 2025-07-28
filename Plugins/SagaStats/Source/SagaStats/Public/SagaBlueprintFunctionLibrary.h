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
	static bool GetTargetDataHandleFromGEContext(FGameplayEffectContextHandle EffectContextHandle,FGameplayAbilityTargetDataHandle& TargetDataHandle);

	
};
