// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGameplayAbilityTargetData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyAbilityBlueprintFunctionLibrary.generated.h"

struct FGameplayAbilityActorInfo;

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType)
class SAGAAFFIXEXAMPLE_API UMyAbilityBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure)
	static bool IsForRemoteClient(const FGameplayAbilityActorInfo& ActorInfo);

	UFUNCTION(BlueprintPure)
	static bool IsPredictingClient(const FGameplayAbilityActorInfo& ActorInfo);

	
	UFUNCTION(BlueprintPure)
	static UObject* GetCDO(const UClass* Class){return Class->GetDefaultObject();}
	

	UFUNCTION(BlueprintCallable)
	static UPARAM(DisplayName=OutDamageType) UDamageType* GetAttackDamageTargetDataFromHandle(FGameplayAbilityTargetDataHandle Handle, bool& IsValid,UObject*& OutAction,FGameplayTagContainer& OutDamageContext);

	
	UFUNCTION(BlueprintCallable)
	static bool SetAttackDamageTargetDataToHandle(FGameplayAbilityTargetDataHandle Handle, UDamageType* InDamageType, UObject* InAction, FGameplayTagContainer InDamageContext);

	
	UFUNCTION(BlueprintCallable)
	static FGameplayAbilityTargetDataHandle MakeAttackDamageTargetDataHandle(UDamageType* InDamageType, UObject* InAction, FGameplayTagContainer InDamageContext);

	

	
	
};
