// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAbilityBlueprintFunctionLibrary.h"

#include "Abilities/GameplayAbilityTypes.h"

bool UMyAbilityBlueprintFunctionLibrary::IsForRemoteClient(const FGameplayAbilityActorInfo& ActorInfo)
{
	// const FGameplayAbilityActorInfo* const CurrentActorInfoPtr = GetCurrentActorInfo();
	if (ActorInfo.OwnerActor.IsValid())
	{
		bool bIsLocallyControlled = ActorInfo.IsLocallyControlled();
		bool bIsAuthority = ActorInfo.IsNetAuthority();

		if (bIsAuthority && !bIsLocallyControlled)
		{
			return true;
		}
	}

	return false;
}

bool UMyAbilityBlueprintFunctionLibrary::IsPredictingClient(const FGameplayAbilityActorInfo& ActorInfo)
{
	// const FGameplayAbilityActorInfo* const CurrentActorInfoPtr = ActorInfo;
	if (ActorInfo.OwnerActor.IsValid())
	{
		bool bIsLocallyControlled = ActorInfo.IsLocallyControlled();
		bool bIsAuthority = ActorInfo.IsNetAuthority();

		// LocalPredicted and ServerInitiated are both valid because in both those modes the ability also runs on the client
		if (!bIsAuthority && bIsLocallyControlled)
		{
			return true;
		}
	}

	return false;
}

bool UMyAbilityBlueprintFunctionLibrary::GetAttackDamageTargetDataFromHandle(FGameplayAbilityTargetDataHandle Handle,
	TSubclassOf<UDamageType>& OutDamageType, UObject*& OutAction, FGameplayTagContainer& OutDamageContext)
{
	if (const FAttackDamageTargetData* TargetData = static_cast<const FAttackDamageTargetData*>(Handle.Get(0)))
	{
		OutDamageType = TargetData->DamageType;
		OutAction = TargetData->Action;
		OutDamageContext = TargetData->DamageContext;
		return true;
	}
	return false;
}

bool UMyAbilityBlueprintFunctionLibrary::SetAttackDamageTargetDataToHandle(FGameplayAbilityTargetDataHandle Handle, const TSubclassOf<UDamageType>& InDamageType,
	UObject* InAction, const FGameplayTagContainer& InDamageContext)
{
	if (FAttackDamageTargetData* TargetData = static_cast<FAttackDamageTargetData*>(Handle.Get(0)))
	{
		TargetData->DamageType = InDamageType;
		TargetData->Action = InAction;
		TargetData->DamageContext = InDamageContext;
		return true;
	}
	return false;
}

FGameplayAbilityTargetDataHandle UMyAbilityBlueprintFunctionLibrary::MakeAttackDamageTargetDataHandle(const TSubclassOf<UDamageType>& InDamageType, UObject* InAction)
{
	FGameplayAbilityTargetDataHandle Handle;
	FAttackDamageTargetData* TargetData = new FAttackDamageTargetData();
	TargetData->DamageType = InDamageType;
	TargetData->Action = InAction;
	Handle.Add(TargetData);
	return Handle;
}
