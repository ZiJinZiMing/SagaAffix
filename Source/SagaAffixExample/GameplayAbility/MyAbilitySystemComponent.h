// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "SagaAbilitySystemComponent.h"
#include "MyAbilitySystemComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SAGAAFFIXEXAMPLE_API UMyAbilitySystemComponent : public USagaAbilitySystemComponent
{
	GENERATED_BODY()

public:

	// Sets default values for this component's properties
	UMyAbilitySystemComponent();

	
public:
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool TryActivateAbilityByClassWithTargetData(TSubclassOf<UGameplayAbility> InAbilityToActivate, const FGameplayAbilityTargetDataHandle& TargetData);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool TryActivateAbilityByClassWithGEContext(TSubclassOf<UGameplayAbility> InAbilityToActivate, const FGameplayEffectContextHandle& GEContextHandle);
	
	
	UFUNCTION(BlueprintPure, Category = "Abilities")
	FGameplayAbilityActorInfo GetGameplayAbilityActorInfo() const;

	
};
