// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "SagaGameplayEffectContext.generated.h"


USTRUCT()
struct SAGASTATS_API FSagaGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_USTRUCT_BODY()

public:
	virtual ~FSagaGameplayEffectContext() override
	{
		TargetData.Clear();
	}

	virtual FGameplayAbilityTargetDataHandle GetTargetData() const
	{
		return TargetData;
	}

	virtual FGameplayAbilityTargetDataHandle& GetTargetData_Mutable() 
	{
		return TargetData;
	}

	virtual void AddTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
	{
		TargetData.Append(TargetDataHandle);
	}

	/**
	* Functions that subclasses of FGameplayEffectContext need to override
	*/

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	virtual FSagaGameplayEffectContext* Duplicate() const override
	{
		FSagaGameplayEffectContext* NewContext = new FSagaGameplayEffectContext();
		*NewContext = *this;
		NewContext->AddActors(Actors);
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		// Shallow copy of TargetData, is this okay?
		NewContext->TargetData.Append(TargetData);
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

protected:
	UPROPERTY()
	FGameplayAbilityTargetDataHandle TargetData;
};


template <>
struct TStructOpsTypeTraits<FSagaGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FSagaGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true // Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};
