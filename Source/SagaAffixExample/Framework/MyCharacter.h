// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"
#include "MyCharacter.generated.h"

SAGAAFFIXEXAMPLE_API DECLARE_LOG_CATEGORY_EXTERN(VLogMyGAS, Display, All);


class UACStateGameplayAbility;
class UMyAbilitySystemComponent;

UCLASS()
class SAGAAFFIXEXAMPLE_API AMyCharacter : public ACharacter, public IAbilitySystemInterface, public IVisualLoggerDebugSnapshotInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

	UPROPERTY(BlueprintReadOnly, Category=Ability)
	TObjectPtr<UMyAbilitySystemComponent> AbilitySystemComponent;

public:

	UAbilitySystemComponent* GetAbilitySystemComponent() const;
protected:

	//从PlayerState中初始化AbilitySystemComponent
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	UFUNCTION(BlueprintNativeEvent)
	void OnAbilitySystemInitialized();
};
