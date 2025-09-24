/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DamageProcessFunctionLibrary.generated.h"

struct FGameplayAttribute;
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class SAGASTATS_API UDamageProcessFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable)
	static void ShowDAGTest();


	UFUNCTION(BlueprintCallable)
	static bool GetMagnitudeFromGEModifiers(TSubclassOf<UGameplayEffect> EffectClass, FGameplayAttribute Attribute, float& OutMagnitude);
};
