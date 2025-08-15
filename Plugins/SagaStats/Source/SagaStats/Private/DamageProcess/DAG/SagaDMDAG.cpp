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
	if (!TopoSort())
	{
		SS_LOG(Error, TEXT("DFS拓扑排序失败"));
		return false;
	}
	
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
			if (!Producers || Producers->Num() == 0)
			{
				SS_LOG(Error, TEXT("节点%s的RequiredToken:[%s]没有生产者"), *Node->GetNodeName(), *Token.ToString());
				Success = false;
			}
			
			Node->PrerequisitesNodes.Append(*Producers);
		}
	}

	return Success;
}

bool USagaDMDAG::TopoSort()
{
	TMap<USagaDMDAGNode*, ENodeState> NodeState;
	// 初始化所有节点为白色
	for (TObjectPtr<USagaDMDAGNode> Node : AllNodes)
	{
		NodeState.Add(Node, ENodeState::White);
	}

	//开始检测环
	if (!CheckCycle(NodeState))
	{
		SS_LOG(Error, TEXT("检测到循环依赖"));
		return false;
	}

	//环检测完毕，开始进行拓扑排序
	//重置颜色
	for (auto& Pair : NodeState)
	{
		Pair.Value = ENodeState::White;
	}


	
	TArray<USagaDMDAGNode*> SortedNodes;
	//todo:优先级排序，需要作用在同一节点下

	

	
	
	

	return true;
}

bool USagaDMDAG::CheckCycle(TMap<USagaDMDAGNode*, ENodeState>& NodeState)
{
	//开始环检测
	for (TObjectPtr<USagaDMDAGNode> Node : AllNodes)
	{
		if (NodeState[Node] == ENodeState::White)
		{
			if (!CheckNodeCycle(Node, NodeState))
			{
				SS_LOG(Error, TEXT("在节点 %s 中检测到环路"), *Node->GetNodeName());
				return false;
			}
		}
	}

	return true;
}

bool USagaDMDAG::CheckNodeCycle(USagaDMDAGNode* Node, TMap<USagaDMDAGNode*, ENodeState>& NodeState)
{
	// 如果节点已经是灰色，说明发现了后向边
	if (NodeState[Node] == ENodeState::Gray)
	{
		SS_LOG(Error, TEXT("检测到环路：访问灰色节点 %s"), *Node->GetNodeName());
		return false;
	}

	// 如果节点已经是黑色，说明已经处理过，直接返回 
	if (NodeState[Node] == ENodeState::Black)
	{
		return true;
	}

	// 标记节点为灰色（正在访问）
	NodeState[Node] = ENodeState::Gray;

	//继续访问前置节点
	for (TObjectPtr<USagaDMDAGNode> Prerequisites : Node->PrerequisitesNodes)
	{
		if (!CheckNodeCycle(Prerequisites, NodeState))
		{
			return false;
		}
	}

	// 标记节点为黑色（已完成）
	NodeState[Node] = ENodeState::Black;
	return true;
}
