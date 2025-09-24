/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "UObject/Object.h"
#include "DamageProcess.generated.h"

class UDAG;
class UDamageProcessOperation;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class SAGASTATS_API UDamageProcess : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FGameplayAttribute> Attributes;
	
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UDamageProcessOperation>> OperationClasses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UDAG* DAG;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* OwnerActor;


	UFUNCTION(BlueprintCallable)
	void SetOwnerActor(AActor* NewOwnerActor) { OwnerActor = NewOwnerActor; };

	UFUNCTION(BlueprintCallable)
	void Execute();
};
