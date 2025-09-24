/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageProcess/DamageProcess.h"

#include "DamageProcess/DamageProcessAttribute.h"
#include "DamageProcess/DamageProcessOperation.h"
#include "DamageProcess/DAG/DAG.h"

void UDamageProcess::Execute()
{
	DAG = NewObject<UDAG>();

	// 存储创建的节点以便后续建立连接
	TMap<FGameplayAttribute, UDamageProcessAttribute*> AttributeNodeMap;
	TArray<UDamageProcessOperation*> OperationNodes;

	//根据Attribute构造AttributeNodes，并添加进DAG
	for (const FGameplayAttribute& Attribute : Attributes)
	{
		if (Attribute.IsValid())
		{
			UDamageProcessAttribute* AttributeNode = NewObject<UDamageProcessAttribute>();
			AttributeNode->Attribute = Attribute;
			AttributeNode->OwnerActor = OwnerActor;
			AttributeNode->AttributeMagnitude = 0.0f; // 初始值
			
			// 添加到DAG和映射表
			DAG->AddNode(AttributeNode);
			AttributeNodeMap.Add(Attribute, AttributeNode);
		}
	}

	//根据OperationClasses构建OperationNode对象，并添加DAG
	for (TSubclassOf<UDamageProcessOperation> OperationClass : OperationClasses)
	{
		if (OperationClass)
		{
			UDamageProcessOperation* OperationNode = NewObject<UDamageProcessOperation>(this, OperationClass);
			OperationNode->OwnerActor = OwnerActor;
			
			// 添加到DAG和数组
			DAG->AddNode(OperationNode);
			OperationNodes.Add(OperationNode);
		}
	}

	//根据Attribute，为AttributeNode和OperationNode添加边
	//OperationNode根据Modifiers[i].Attribute匹配AttributeNode，OperationNode作为子节点
	//OperationNode根据BackingAttributes[i]匹配AttributeNode，OperationNode作为父节点
	for (UDamageProcessOperation* OperationNode : OperationNodes)
	{
		// 处理 Modifiers: Operation 输出到 Attribute，Operation 是 Attribute 的子节点
		for (const FDamageProcessModifierInfo& Modifier : OperationNode->Modifiers)
		{
			if (UDamageProcessAttribute** AttributeNodePtr = AttributeNodeMap.Find(Modifier.Attribute))
			{
				UDamageProcessAttribute* AttributeNode = *AttributeNodePtr;
				// Operation -> Attribute (Operation 是子节点，先执行)
				DAG->AddEdge(AttributeNode, OperationNode);
			}
		}

		// 处理 BackingAttributes: Operation 依赖于 Attribute，Operation 是 Attribute 的父节点
		for (const FGameplayAttribute& BackingAttribute : OperationNode->BackingAttributes)
		{
			if (UDamageProcessAttribute** AttributeNodePtr = AttributeNodeMap.Find(BackingAttribute))
			{
				UDamageProcessAttribute* AttributeNode = *AttributeNodePtr;
				// Attribute -> Operation (Attribute 是子节点，先执行)
				DAG->AddEdge(OperationNode, AttributeNode);
			}
		}
	}

	//DAG执行
	DAG->ExecuteDAG(EDAGTraversalOrder::TopologicalSort);
}
