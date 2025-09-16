#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DAGNode.h"
#include "DAG.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNodeExecuted, UDAGNode*, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCycleDetected, const TArray<UDAGNode*>&, CyclePath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDAGExecutionComplete);

UENUM(BlueprintType)
enum class EDAGTraversalOrder : uint8
{
	DepthFirst,
	TopologicalSort
};

/**
 * 依赖图 (DAG) - 用于任务依赖管理的有向无环图 / Dependency Graph (DAG) - A directed acyclic graph for task dependency management
 * 
 * 图结构概念 / Graph Structure Concept:
 * - 父节点 = 依赖任务 / Parent Node = Dependent Task - needs other tasks to complete first
 * - 子节点 = 被依赖任务 / Child Node = Dependency Task - can execute independently 
 * - 边的方向 / Edge Direction: Parent → Child (依赖项指向被依赖项)
 * 
 * 执行顺序 / Execution Order: Children first, then Parents (被依赖项先执行，依赖项后执行)
 * 
 * 示例 / Example: Task A depends on tasks B and C
 *   图结构 / Graph: A → B, A → C
 *   执行顺序 / Execution: B and C execute first, then A
 *   
 * 树形视图 / Tree View:
 *     A (依赖项/父节点)
 *    / \
 *   B   C (被依赖项/子节点)
 */
UCLASS(BlueprintType, Blueprintable)
class SAGASTATS_API UDAG : public UObject
{
	GENERATED_BODY()

public:
	UDAG();

	// 核心DAG操作 / Core DAG operations
	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual bool AddNode(UDAGNode* Node);

	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual bool RemoveNode(UDAGNode* Node);

	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual bool AddEdge(UDAGNode* FromNode, UDAGNode* ToNode);

	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual bool RemoveEdge(UDAGNode* FromNode, UDAGNode* ToNode);

	// 依赖图算法 / Dependency Graph algorithms (被依赖项先执行，依赖项后执行)
	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual void ExecuteDAG(EDAGTraversalOrder TraversalOrder = EDAGTraversalOrder::TopologicalSort);

	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual bool HasCycles();

	// 返回执行顺序 / Returns execution order: Child nodes (被依赖项) before Parent nodes (依赖项)
	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual TArray<UDAGNode*> GetTopologicalSort();

	// DFS遍历，从根节点开始 / DFS traversal starting from root nodes (依赖项优先遍历，被依赖项优先执行)
	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual TArray<UDAGNode*> GetDFSTraversal(UDAGNode* StartNode = nullptr);

	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual TArray<UDAGNode*> GetRootNodes() const;

	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual TArray<UDAGNode*> GetLeafNodes() const;

	// 工具函数 / Utility functions
	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual void ResetAllNodeStates();

	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual void Clear();

	// 可视化与调试 / Visualization and debugging
	UFUNCTION(BlueprintCallable, Category = "DAG")
	virtual void SaveToMMD();

	UFUNCTION(BlueprintPure, Category = "DAG")
	virtual int32 GetNodeCount() const { return Nodes.Num(); }

	UFUNCTION(BlueprintPure, Category = "DAG")
	virtual bool IsEmpty() const { return Nodes.Num() == 0; }

	UFUNCTION(BlueprintPure, Category = "DAG")
	virtual bool ContainsNode(UDAGNode* Node) const;

	// 蓝图事件 / Blueprint events
	UPROPERTY(BlueprintAssignable, Category = "DAG")
	FOnNodeExecuted OnNodeExecuted;

	UPROPERTY(BlueprintAssignable, Category = "DAG")
	FOnCycleDetected OnCycleDetected;

	UPROPERTY(BlueprintAssignable, Category = "DAG")
	FOnDAGExecutionComplete OnDAGExecutionComplete;

protected:
	// 核心DFS实现 / Core DFS implementation
	virtual void DFSVisit(UDAGNode* Node, TArray<UDAGNode*>& VisitedOrder);
	virtual bool DFSCycleCheck(UDAGNode* Node, TArray<UDAGNode*>& CyclePath);
	virtual void TopologicalSortUtil(UDAGNode* Node, TArray<UDAGNode*>& Stack);

	// 执行辅助函数 / Execution helpers
	virtual void ExecuteNode(UDAGNode* Node);
	virtual bool CanExecuteNode(UDAGNode* Node) const;

	// 节点管理 / Node management
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DAG")
	TArray<UDAGNode*> Nodes;

	// 执行状态 / Execution state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DAG")
	bool bIsExecuting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DAG")
	TArray<UDAGNode*> ExecutionQueue;

private:
	// 内部辅助函数 / Internal helpers
	void ValidateDAGIntegrity();
	bool IsValidEdge(UDAGNode* FromNode, UDAGNode* ToNode) const;
	
	// 可视化辅助函数 / Visualization helpers
	static void SaveMermaidToFile(const FString& Content);
};