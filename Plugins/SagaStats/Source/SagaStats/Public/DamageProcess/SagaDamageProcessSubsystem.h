// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SagaDamageProcessSubsystem.generated.h"

class USagaDMDAG;
struct FGameplayEffectContextHandle;
class USagaDPU;
/**
 * 
 */
UCLASS()
class SAGASTATS_API USagaDamageProcessSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	/**/
	UFUNCTION(BlueprintCallable, Category = "Saga|DamageProcess")
	void RegisterDPU(USagaDPU* DPUPrototype);

	
	/**/
	UFUNCTION(BlueprintCallable, Category = "Saga|DamageProcess")
	USagaDMDAG* BuildDAG(FGameplayEffectContextHandle ContextHandle) const;

	TArray<USagaDPU*> CollectRelevantDPUs(FGameplayEffectContextHandle ContextHandle) const;
	
protected:
	/**/
	TArray<TObjectPtr<USagaDPU>> DPUPrototypes;
};
