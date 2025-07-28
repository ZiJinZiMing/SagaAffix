// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DamageFlowPreset.generated.h"

class UDamageProcessFlow;
/**
 * 
 */
UCLASS(BlueprintType)
class SAGASTATS_API UDamageFlowPreset : public UDataAsset
{
	GENERATED_BODY()


public:

	/** */
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Damage"/*, meta = (ShowOnlyInnerProperties)*/)
	TArray<TObjectPtr<UDamageProcessFlow>> DamageFlows;
	
};
