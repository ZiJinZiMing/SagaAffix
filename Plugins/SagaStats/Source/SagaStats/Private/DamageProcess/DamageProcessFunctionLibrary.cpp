// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageProcess/DamageProcessFunctionLibrary.h"

#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "DAG/DAG.h"
#include "DAG/DAGNodeTest.h"


void UDamageProcessFunctionLibrary::ShowDAGTest()
{
	UDAG* DAGInstance = NewObject<UDAG>();
	
	// 创建测试节点
	UDAGNodeTest* NodeA = NewObject<UDAGNodeTest>();
	NodeA->NodeName = "A";
	UDAGNodeTest* NodeB = NewObject<UDAGNodeTest>();
	NodeB->NodeName = "B";
	UDAGNodeTest* NodeC = NewObject<UDAGNodeTest>();
	NodeC->NodeName = "C";
	UDAGNodeTest* NodeD = NewObject<UDAGNodeTest>();
	NodeD->NodeName = "D";
	UDAGNodeTest* NodeE = NewObject<UDAGNodeTest>();
	NodeE->NodeName = "E";

	// 添加到DAG
	DAGInstance->AddNode(NodeA);
	DAGInstance->AddNode(NodeB);
	DAGInstance->AddNode(NodeD);
	DAGInstance->AddNode(NodeC);
	DAGInstance->AddNode(NodeE);

	// 构建依赖关系（Edge）/ Build dependency relationships (Edges)
	// A依赖C、D / A depends on C, D
	DAGInstance->AddEdge(NodeA, NodeC);
	DAGInstance->AddEdge(NodeA, NodeD);
	
	// C依赖E / C depends on E
	DAGInstance->AddEdge(NodeC, NodeE);
	
	// B依赖D、E / B depends on D, E
	DAGInstance->AddEdge(NodeB, NodeD);
	DAGInstance->AddEdge(NodeB, NodeE);

	// 执行DAG测试 / Execute DAG test
	UE_LOG(LogTemp, Warning, TEXT("=== DAG执行开始 / DAG Execution Start ==="));
	DAGInstance->ExecuteDAG(EDAGTraversalOrder::TopologicalSort);
	UE_LOG(LogTemp, Warning, TEXT("=== DAG执行完成 / DAG Execution Complete ==="));
}

bool UDamageProcessFunctionLibrary::GetMagnitudeFromGEModifiers(TSubclassOf<UGameplayEffect> EffectClass, FGameplayAttribute Attribute, float& OutMagnitude)
{
	//从GE中获取去Modifiers中对应Attribute的ScalableFloat情况中的Magnitude数值
	if (!EffectClass)
	{
		return false;
	}

	// 获取GameplayEffect的CDO
	const UGameplayEffect* GameplayEffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
	if (!GameplayEffectCDO)
	{
		return false;
	}

	// 遍历Modifiers查找匹配的Attribute
	for (const FGameplayModifierInfo& ModifierInfo : GameplayEffectCDO->Modifiers)
	{
		if (ModifierInfo.Attribute == Attribute)
		{
			// 获取ScalableFloat中的基础Magnitude值
			const FGameplayEffectModifierMagnitude& ModifierMagnitude = ModifierInfo.ModifierMagnitude;

			// 如果是ScalableFloat类型，获取其基础值
			if (ModifierMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::ScalableFloat)
			{
				FGameplayEffectSpec Spec;
				ModifierMagnitude.AttemptCalculateMagnitude(Spec, OutMagnitude);
				return true;
			}
		}
	}

	// 没有找到匹配的Attribute
	return false;
}
