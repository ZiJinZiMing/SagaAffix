// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "MyGameplayAbilityTargetData.generated.h"




USTRUCT(BlueprintType)
struct FAttackDamageTargetData : public FGameplayAbilityTargetData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UDamageType> DamageType;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> Action;
	
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer DamageContext;
	
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}
};


template <>
struct TStructOpsTypeTraits<FAttackDamageTargetData> : public TStructOpsTypeTraitsBase2<FAttackDamageTargetData>
{
	enum
	{
		WithNetSerializer = true // For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};




