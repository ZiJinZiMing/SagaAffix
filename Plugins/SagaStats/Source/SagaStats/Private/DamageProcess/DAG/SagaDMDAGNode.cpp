// Fill out your copyright notice in the Description page of Project Settings.

#include "DamageProcess/DAG/SagaDMDAGNode.h"


USagaDMDAGNode::USagaDMDAGNode()
{
}

void USagaDMDAGNode::Execute()
{
}

FString USagaDMDAGNode::GetNodeName() const
{
	return GetName();
}

void USagaDMDAGNode::GetRequiredTokens(FGameplayTagContainer& OutRequiredTokens) const
{

}

void USagaDMDAGNode::GetProducedTokens(FGameplayTagContainer& OutProducedTokens) const
{
	
}

int USagaDMDAGNode::GetPriority() const
{
	return 0;
}
