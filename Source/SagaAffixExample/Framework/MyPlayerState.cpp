// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerState.h"

#include "AbilitySystemComponent.h"
#include "SagaAffixExample/GameplayAbility/MyAbilitySystemComponent.h"

AMyPlayerState::AMyPlayerState(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	//初始化AbilitySystemComponent
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UMyAbilitySystemComponent>(this,TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AbilitySystemComponent->SetIsReplicated(true);
	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* AMyPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}
