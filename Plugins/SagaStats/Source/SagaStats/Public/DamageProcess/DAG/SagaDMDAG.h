// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SagaDMDAG.generated.h"


class USagaDMDAGNode;
/**
 * DFS节点状态 - DFS Node State (三色标记法)
 * White: 未访问, Gray: 正在访问(当前DFS路径中), Black: 已完成
 */



/**
 * 用于构建DamageProcess的DAG结构
 */
UCLASS(BlueprintType)
class SAGASTATS_API USagaDMDAG : public UObject
{
	GENERATED_BODY()

	enum class ENodeState : uint8
	{
		White   UMETA(DisplayName = "未访问"),
		Gray    UMETA(DisplayName = "访问中"),
		Black   UMETA(DisplayName = "已完成")
	};
	
public:
	bool Build(const TArray<USagaDMDAGNode*>& InNodes);

	/**/
	void Execute();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<USagaDMDAGNode>> AllNodes;


	UPROPERTY(BlueprintReadOnly)
	bool IsCycle;
		
private:


	/**/
    bool ValidateNodes();

	/**/
    bool BuildDependencyGraph();

	/**/
    bool TopoSort();

	/**/
	bool CheckCycle(TMap<USagaDMDAGNode*, ENodeState>& NodeState);

	/**/
	static bool CheckNodeCycle(USagaDMDAGNode* Node, TMap<USagaDMDAGNode*, ENodeState>& NodeState);

	/**/
	void SaveToMMD();
};
