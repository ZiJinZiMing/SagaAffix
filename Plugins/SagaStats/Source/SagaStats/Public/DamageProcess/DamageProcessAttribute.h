// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "DAG/DAGNode.h"
#include "DamageProcessAttribute.generated.h"

/**
 * 
 */
UCLASS()
class SAGASTATS_API UDamageProcessAttribute : public UDAGNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FGameplayAttribute Attribute;

	UPROPERTY(visibleAnywhere)
	float AttributeMagnitude;

	UPROPERTY(BlueprintReadOnly)
	AActor* OwnerActor;
	
protected:

	virtual void GetOrderedChildren(TArray<UDAGNode*>& OutOrderedChildren) const override;

	virtual FString GetNodeDisplayName() const override;

	virtual FString GetMermaidNodeShape(const FString& NodeLabel) const override;
	
	virtual void Execute() override;
};
