// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SagaDMDAGNode.generated.h"

struct FGameplayTagContainer;

/**
 */
UCLASS(BlueprintType)
class SAGASTATS_API USagaDMDAGNode : public UObject
{
	GENERATED_BODY()

public:
	USagaDMDAGNode();

	virtual void GetRequiredTokens(FGameplayTagContainer& OutRequiredTokens) const;

	virtual void GetProducedTokens(FGameplayTagContainer& OutProducedTokens) const;

	virtual void Execute();

	virtual FString GetNodeName() const;

protected:
	friend class USagaDMDAG;
	
	/*前置依赖项*/
	TArray<TObjectPtr<USagaDMDAGNode>> PrerequisitesNodes;
	
};
