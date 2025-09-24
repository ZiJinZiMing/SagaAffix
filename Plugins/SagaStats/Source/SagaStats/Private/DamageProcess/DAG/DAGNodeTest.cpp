/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageProcess/DAG/DAGNodeTest.h"

void UDAGNodeTest::Execute()
{
	UE_LOG(LogTemp, Log, TEXT("Executing node: %s"), *NodeName);
}

FString UDAGNodeTest::GetNodeDisplayName() const
{
	return NodeName;
}
