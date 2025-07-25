// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

class UMyAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class SAGAAFFIXEXAMPLE_API AMyPlayerState : public APlayerState ,public IAbilitySystemInterface
{
	GENERATED_BODY()

	AMyPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
public:
	UPROPERTY(BlueprintReadOnly,Category=Ability)
	TObjectPtr<UMyAbilitySystemComponent> AbilitySystemComponent;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
};
