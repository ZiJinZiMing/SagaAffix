// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageProcess/DAG/SagaDMDAG.h"

#include "GameplayTagContainer.h"
#include "SagaStatsLog.h"
#include "DamageProcess/DAG/SagaDMDAGNode.h"


bool USagaDMDAG::Build(const TArray<USagaDMDAGNode*>& InNodes)
{
	SS_LOG(Log,TEXT("=== 开始构建DAG，输入%d个Node ==="), InNodes.Num());

	AllNodes = InNodes;
	
    // 1. 验证Nodes有效性
	if (!ValidateNodes())
	{
		SS_LOG(Log,TEXT("DPU验证失败：存在无效的DPU"));
		return false;
	}
	
	// 2. 根据Token构建依赖图
	if (!BuildDependencyGraph())
	{
		SS_LOG(Error, TEXT("依赖图构建失败"));
		return false;
	}

	// 3. DFS统一处理：环检测 + 优先级排序
	if (!TopologicalSort())
	{
		SS_LOG(Error, TEXT("DFS拓扑排序失败"));
		return false;
	}

	//将组成的DAG结构绘制成MMD图标
	SaveToMMD();
	
	return true;
}

void USagaDMDAG::Execute()
{
	
}

void USagaDMDAG::SaveToMMD()
{
	
}

bool USagaDMDAG::ValidateNodes()
{
	for (TObjectPtr<USagaDMDAGNode> Node : AllNodes)
	{
		if (!Node)return false;		
	}
	return true;
	
}

bool USagaDMDAG::BuildDependencyGraph()
{
	bool Success = true;
	/*Token生产方*/
    TMap<FGameplayTag, TArray<USagaDMDAGNode*>> TokenProducedMap;

	/*Token需求方*/
    TMap<FGameplayTag, TArray<USagaDMDAGNode*>> TokenRequiredMap;
	

	//TokenProviderMap and TokenConsumerMap
	for ( TObjectPtr<USagaDMDAGNode> Node : AllNodes)
	{
		FGameplayTagContainer ProducedTokens;
		Node->GetRequiredTokens(ProducedTokens);
		for (const FGameplayTag& Token : ProducedTokens)
		{
			TArray<USagaDMDAGNode*>& Producers = TokenProducedMap.FindOrAdd(Token);
			Producers.Add(Node);
		}
		
		FGameplayTagContainer RequiredTokens;
		Node->GetRequiredTokens(RequiredTokens);
		for (const FGameplayTag& Token : RequiredTokens)
		{
			TArray<USagaDMDAGNode*>& Requirers = TokenRequiredMap.FindOrAdd(Token);
			Requirers.Add(Node);
		}
	}

	
	//检测是否所有的ProducedToken都有节点消费，所有的RequiredToken消费者都有节点提供
	for (TObjectPtr<USagaDMDAGNode> Node : AllNodes)
	{
		FGameplayTagContainer ProducedTokens;
		Node->GetRequiredTokens(ProducedTokens);
		for (const FGameplayTag& Token : ProducedTokens)
		{
			TArray<USagaDMDAGNode*>* Requirers = TokenRequiredMap.Find(Token);
			if (!Requirers || Requirers->Num() == 0)
			{
				SS_LOG(Error, TEXT("节点%s的ProducedToken:[%s]没有需求方"), *Node->GetNodeName(), *Token.ToString());
				Success = false;
			}
		}
		
		FGameplayTagContainer RequiredTokens;
		Node->GetRequiredTokens(RequiredTokens);
		for (const FGameplayTag& Token : RequiredTokens)
		{
			TArray<USagaDMDAGNode*>* Producers = TokenProducedMap.Find(Token);
			if (Producers && Producers->Num() > 0)
			{
				Node->PrerequisitesNodes.Append(*Producers);
			}

			Node->PrerequisitesNodes.Append(*Producers);
		}
	}

	return Success;
}

bool USagaDMDAG::TopologicalSort()
{
	TMap<USagaDMDAGNode*, ENodeState> NodeState;
    TArray<USagaDMDAGNode*> FinishOrder; // 记录DFS完成时间顺序
	
	// 初始化所有节点为白色
	for (TObjectPtr<USagaDMDAGNode> Node : AllNodes)
	{
		NodeState.Add(Node, ENodeState::White);
	}
	
	//开始DFS拓扑排序
	// 对每个白色节点启动DFS - Start DFS from each white node
	for (TObjectPtr<USagaDMDAGNode> Node : AllNodes)
	{
		if (NodeState[Node] == ENodeState::White)
		{
			if (CheckDFSVisit(Node,NodeState, FinishOrder))
			{
				//存在环依赖
				SS_LOG(Error, TEXT("DFS拓扑排序过程中发现环依赖"));
				return false;
			}
		}
	}

	AllNodes = FinishOrder;

	return true;
}

bool USagaDMDAG::CheckDFSVisit(USagaDMDAGNode* Node, TMap<USagaDMDAGNode*, ENodeState>& NodeState, TArray<USagaDMDAGNode*>& FinishOrder)
{
	check(Node);
	
	// 如果节点已经是灰色，说明发现了后向边（环） - Gray node means back edge (cycle)
	if (NodeState[Node] == ENodeState::Gray)
	{
		SS_LOG(Error, TEXT("DFS检测到环路：访问灰色节点 %s"),*Node->GetNodeName());
		return false;
	}

	// 如果节点已经是黑色，说明已经处理过，直接返回 - Black node is already processed
	if (NodeState[Node] == ENodeState::Black)
	{
		return true;
	}

	// 表示当前节点为白色，标记为灰色（正在访问） 
	NodeState[Node] = ENodeState::Gray;

	for (TObjectPtr<USagaDMDAGNode> PrerequisitesNode : Node->PrerequisitesNodes)
	{
		if (!CheckDFSVisit(PrerequisitesNode, NodeState, FinishOrder))
		{
			//存在环路
			return false;
		}
	}

	//访问完成，标记黑色
	NodeState[Node] = ENodeState::Black;
    FinishOrder.Add(Node); // 记录完成时间
	
	return true;
}
