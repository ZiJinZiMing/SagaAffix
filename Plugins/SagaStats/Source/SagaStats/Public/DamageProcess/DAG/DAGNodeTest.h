/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DAGNode.h"
#include "DAGNodeTest.generated.h"

/**
 * 
 */
UCLASS()
class SAGASTATS_API UDAGNodeTest : public UDAGNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FString NodeName;

	virtual void Execute() override;

	virtual void OnExecutionComplete() override{};

	virtual bool CanExecute() const override{return true;};

	virtual FString GetNodeDisplayName() const override;
};
