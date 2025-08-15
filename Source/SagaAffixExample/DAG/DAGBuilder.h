// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SimpleDPU.h"
#include "DAGBuilder.generated.h"

/**
 * DFS节点状态 - DFS Node State (三色标记法)
 * White: 未访问, Gray: 正在访问(当前DFS路径中), Black: 已完成
 */
UENUM(BlueprintType)
enum class ENodeState : uint8
{
    White   UMETA(DisplayName = "未访问"),
    Gray    UMETA(DisplayName = "访问中"),
    Black   UMETA(DisplayName = "已完成")
};

/**
 * DFS处理结果 - DFS Processing Result
 * 统一完成环检测和线性排序
 */
USTRUCT(BlueprintType)
struct SAGAAFFIXEXAMPLE_API FDFSResult
{
    GENERATED_BODY()

    // 是否检测到环路 - Has Cycle Detected
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasCycle;

    // 线性执行顺序 - Linear Execution Order
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<USimpleDPU*> ExecutionOrder;

    // 环路诊断信息 - Cycle Diagnostic Info
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CycleInfo;

    FDFSResult()
    {
        bHasCycle = false;
        CycleInfo = TEXT("");
    }
};



/**
 * DAG构建结果 - DAG Build Result
 */
USTRUCT(BlueprintType)
struct SAGAAFFIXEXAMPLE_API FDAGBuildResult
{
    GENERATED_BODY()

    // 是否构建成功 - Build Success
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSuccess;

    // 错误信息 - Error Message
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ErrorMessage;

    // 线性执行顺序 - Linear Execution Order (DFS Result)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<USimpleDPU*> ExecutionOrder;

    // 诊断信息 - Diagnostic Info
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DiagnosticInfo;

    FDAGBuildResult()
    {
        bSuccess = false;
        ErrorMessage = TEXT("");
        DiagnosticInfo = TEXT("");
    }
};

/**
 * DAG构建器 - DAG Builder
 * 核心功能：依赖图构建、环检测、优先级驱动的线性排序
 */
UCLASS(BlueprintType)
class SAGAAFFIXEXAMPLE_API UDAGBuilder : public UObject
{
    GENERATED_BODY()

public:
    UDAGBuilder();

    /**
     * 构建DAG并生成执行计划 - Build DAG and Generate Execution Plan
     * @param DPUs 输入的DPU列表
     * @return DAG构建结果
     */
    UFUNCTION(BlueprintCallable, Category = "DAG Builder")
    FDAGBuildResult BuildDAG(const TArray<USimpleDPU*>& DPUs);

    /**
     * 执行DAG计划 - Execute DAG Plan
     * @param BuildResult DAG构建结果
     */
    UFUNCTION(BlueprintCallable, Category = "DAG Builder")
    void ExecuteDAG(const FDAGBuildResult& BuildResult);

    /**
     * 获取详细的构建报告 - Get Detailed Build Report
     */
    UFUNCTION(BlueprintCallable, Category = "DAG Builder")
    FString GetBuildReport() const;

private:
    // 邻接表：DPU -> 依赖它的DPU列表 - Adjacency List: DPU -> List of dependent DPUs
    // 使用TWeakObjectPtr避免GC问题 - Use TWeakObjectPtr to avoid GC issues
    TMap<TWeakObjectPtr<USimpleDPU>, TArray<TWeakObjectPtr<USimpleDPU>>> AdjacencyList;


    // 令牌提供者映射：令牌类型 -> 提供该令牌的DPU列表 - Token Provider Map: Token Type -> List of DPUs that provide it
    TMap<FString, TArray<USimpleDPU*>> TokenProviderMap;

    // 构建诊断信息 - Build diagnostic information
    FString DiagnosticLog;

private:
    /**
     * 构建依赖图 - Build Dependency Graph
     * @param DPUs 输入的DPU列表
     * @return 是否构建成功
     */
    bool BuildDependencyGraph(const TArray<USimpleDPU*>& DPUs);

    /**
     * 统一DFS处理：环检测 + 优先级排序 - Unified DFS Processing: Cycle Detection + Priority-driven Sorting
     * @param DPUs 输入的DPU列表
     * @return DFS处理结果（环检测+线性执行顺序）
     */
    FDFSResult ProcessDAGWithDFS(const TArray<USimpleDPU*>& DPUs);

    /**
     * DFS环检测 - DFS Cycle Detection
     * @param DPUs 输入的DPU列表
     * @param NodeState 节点状态映射
     * @return 是否检测到环路
     */
    bool HasCycleDFS(const TArray<USimpleDPU*>& DPUs, TMap<USimpleDPU*, ENodeState>& NodeState);

    /**
     * DFS递归访问节点（环检测专用） - DFS Recursive Node Visit (for cycle detection)
     * @param Node 当前访问的节点
     * @param NodeState 节点状态映射
     * @return 是否检测到环路
     */
    bool DFSVisitForCycleCheck(USimpleDPU* Node, TMap<USimpleDPU*, ENodeState>& NodeState);

    /**
     * 检查节点是否就绪执行 - Check if Node is Ready for Execution
     * @param Node 待检查的节点
     * @param NodeState 节点状态映射
     * @return 是否所有依赖的令牌提供者都已完成
     */
    bool IsNodeReady(USimpleDPU* Node, const TMap<USimpleDPU*, ENodeState>& NodeState);



    /**
     * 清理内部状态 - Clear Internal State
     */
    void ClearState();

    /**
     * 添加诊断日志 - Add Diagnostic Log
     */
    void AddDiagnosticLog(const FString& Message);

    /**
     * 验证DPU有效性 - Validate DPU Validity
     */
    bool ValidateDPUs(const TArray<USimpleDPU*>& DPUs);
};
