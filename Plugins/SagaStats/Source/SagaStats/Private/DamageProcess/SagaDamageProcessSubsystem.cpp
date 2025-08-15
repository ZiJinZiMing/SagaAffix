// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageProcess/SagaDamageProcessSubsystem.h"

#include "SagaGameplayEffectContext.h"
#include "DamageProcess/SagaDPU.h"
#include "DamageProcess/DAG/SagaDMDAG.h"

void USagaDamageProcessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USagaDamageProcessSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USagaDamageProcessSubsystem::RegisterDPU(USagaDPU* DPUPrototype)
{
	DPUPrototypes.Add(DPUPrototype);
}

USagaDMDAG* USagaDamageProcessSubsystem::BuildDAG(FGameplayEffectContextHandle ContextHandle) const
{
	TArray<USagaDPU*> SatisfiedDPUPrototypes = CollectRelevantDPUs(ContextHandle);
	if (SatisfiedDPUPrototypes.Num() > 0)
	{
		/*USagaDMDAG* DAG = NewObject<USagaDMDAG>();
		if (DAG->Build(ContextHandle, SatisfiedDPUPrototypes))
		{
			return DAG;
		}*/
	}
	return nullptr;
}

TArray<USagaDPU*> USagaDamageProcessSubsystem::CollectRelevantDPUs(FGameplayEffectContextHandle ContextHandle) const
{
	TArray<USagaDPU*> SatisfiedDPUPrototypes;
	for (TObjectPtr<USagaDPU> DPU : DPUPrototypes)
	{
		if (DPU->SatisfiedDamageProcess(ContextHandle))
		{
			SatisfiedDPUPrototypes.Add(DPU);
		}
	}
	return SatisfiedDPUPrototypes;
}
