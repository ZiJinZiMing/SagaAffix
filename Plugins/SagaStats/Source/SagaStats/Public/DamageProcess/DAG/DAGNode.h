/***************************************************************************************************************
* Plugin:       SagaStats
* Author:       Jinming Zhang
* Description:  SagaStats offers modular damage process and meter systems to support adaptable status management
****************************************************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "DAGNode.generated.h"

UENUM(BlueprintType)
enum class EDAGNodeState : uint8
{
	Unvisited,
	Visiting,
	Visited
};

/**
 * DAG节点 - 依赖图节点的抽象基类 / DAG Node - Abstract base class for dependency graph nodes
 * 
 * 依赖图中的节点概念 / Node Concept in Dependency Graph:
 * - 此节点可以是依赖项或被依赖项 / This node can be either a Dependent Task or Dependency Task
 * - Children = 此节点依赖的任务(被依赖项) / Tasks this node depends on (dependencies)
 * - Parents = 依赖此节点的任务(依赖项) / Tasks that depend on this node (dependents)
 * 
 * 执行规则 / Execution Rule:
 * - 节点只能在所有子节点完成后才能执行 / A node can only execute after ALL its children have completed
 * - 子节点(被依赖项)在父节点(依赖项)之前执行 / Children (dependencies) execute before Parents (dependents)
 * 
 * 示例 / Example:
 * - 任务A依赖任务B和C / TaskA depends on TaskB and TaskC
 * - TaskA.AddChild(TaskB), TaskA.AddChild(TaskC)
 * - 执行顺序 / Execution order: TaskB, TaskC, then TaskA
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SAGASTATS_API UDAGNode : public UObject
{
	GENERATED_BODY()

public:
	UDAGNode();

	// 派生类必须实现的纯虚函数 / Pure virtual functions that must be implemented by derived classes
	virtual void Execute() PURE_VIRTUAL(UDAGNode::Execute, );
	virtual bool CanExecute() const {return  true;};
	virtual void OnExecutionComplete() {};

	// 依赖关系连接 / Dependency connections
	// 添加被依赖项 / Add a dependency: This node depends on ChildNode
	UFUNCTION(BlueprintCallable, Category = "DAG")
	void AddChild(UDAGNode* ChildNode);

	UFUNCTION(BlueprintCallable, Category = "DAG")
	void RemoveChild(UDAGNode* ChildNode);

	UFUNCTION(BlueprintCallable, Category = "DAG")
	void AddParent(UDAGNode* ParentNode);

	UFUNCTION(BlueprintCallable, Category = "DAG")
	void RemoveParent(UDAGNode* ParentNode);

	// 获取器 / Getters
	// 获取被依赖项 / Get dependencies: Tasks this node depends on
	UFUNCTION(BlueprintPure, Category = "DAG")
	const TArray<UDAGNode*>& GetChildren() const { return Children; }

	virtual void GetOrderedChildren(TArray<UDAGNode*>& OutOrderedChildren) const;
	
	// 获取依赖项 / Get dependents: Tasks that depend on this node
	UFUNCTION(BlueprintPure, Category = "DAG")
	const TArray<UDAGNode*>& GetParents() const { return Parents; }

	UFUNCTION(BlueprintPure, Category = "DAG")
	EDAGNodeState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "DAG")
	virtual FString GetNodeDisplayName() const;

	// Mermaid节点形状格式 / Mermaid node shape format
	virtual FString GetMermaidNodeShape(const FString& NodeLabel) const;

	// 独立任务 / Independent task: No dependencies, executes first
	UFUNCTION(BlueprintPure, Category = "DAG")
	bool IsLeafNode() const { return Children.Num() == 0; }

	// 最终依赖任务 / Final dependent task: Depends on others, executes last
	UFUNCTION(BlueprintPure, Category = "DAG")
	bool IsRootNode() const { return Parents.Num() == 0; }

	// DFS状态管理 / State management for DFS
	void SetState(EDAGNodeState NewState) { State = NewState; }
	void ResetState() { State = EDAGNodeState::Unvisited; }

	/*
	UFUNCTION(BlueprintImplementableEvent, Category = "DAG")
	void OnNodeStateChanged(EDAGNodeState OldState, EDAGNodeState NewState);
	*/

protected:
	// 依赖关系连接 / Dependency connections
	// 被依赖项 / Dependencies: Tasks this node depends on
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DAG")
	TArray<UDAGNode*> Children;

	// 依赖项 / Dependents: Tasks that depend on this node
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DAG")
	TArray<UDAGNode*> Parents;

	// DFS遍历的节点状态 / Node state for DFS traversal
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DAG")
	EDAGNodeState State;
};