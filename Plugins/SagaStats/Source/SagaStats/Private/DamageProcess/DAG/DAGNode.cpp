/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/

#include "DamageProcess/DAG/DAGNode.h"
// 构造函数：初始化节点状态和ID / Constructor: Initialize node state and ID
UDAGNode::UDAGNode()
{
	State = EDAGNodeState::Unvisited;
}

// 添加子节点（被依赖项）/ Add child node (dependency)
void UDAGNode::AddChild(UDAGNode* ChildNode)
{
	// 检查节点有效性和是否已存在 / Check node validity and if already exists
	if (!ChildNode || Children.Contains(ChildNode))
	{
		return;
	}

	// 添加双向连接 / Add bidirectional connection
	Children.AddUnique(ChildNode);
	ChildNode->AddParent(this);
}

// 移除子节点（被依赖项）/ Remove child node (dependency)
void UDAGNode::RemoveChild(UDAGNode* ChildNode)
{
	// 检查节点有效性 / Check node validity
	if (!ChildNode)
	{
		return;
	}

	// 移除双向连接 / Remove bidirectional connection
	Children.Remove(ChildNode);
	ChildNode->RemoveParent(this);
}

// 添加父节点（依赖项）/ Add parent node (dependent)
void UDAGNode::AddParent(UDAGNode* ParentNode)
{
	// 检查节点有效性和是否已存在 / Check node validity and if already exists
	if (!ParentNode || Parents.Contains(ParentNode))
	{
		return;
	}

	// 添加到父节点列表 / Add to parent node list
	Parents.AddUnique(ParentNode);
}

// 移除父节点（依赖项）/ Remove parent node (dependent)
void UDAGNode::RemoveParent(UDAGNode* ParentNode)
{
	// 检查节点有效性 / Check node validity
	if (!ParentNode)
	{
		return;
	}

	// 从父节点列表移除 / Remove from parent node list
	Parents.Remove(ParentNode);
}

void UDAGNode::GetOrderedChildren(TArray<UDAGNode*>& OutOrderedChildren) const
{
	 OutOrderedChildren = Children;
}

FString UDAGNode::GetNodeDisplayName() const
{
	return GetName();
}

FString UDAGNode::GetMermaidNodeShape(const FString& NodeLabel) const
{
	// 默认使用方框格式 / Default to rectangle format
	return FString::Printf(TEXT("[\"%s\"]"), *NodeLabel);
}
