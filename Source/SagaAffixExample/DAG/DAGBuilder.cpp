// Fill out your copyright notice in the Description page of Project Settings.

#include "DAGBuilder.h"

UDAGBuilder::UDAGBuilder()
{
    ClearState();
}

FDAGBuildResult UDAGBuilder::BuildDAG(const TArray<USimpleDPU*>& DPUs)
{
    FDAGBuildResult Result;
    ClearState();
    
    AddDiagnosticLog(FString::Printf(TEXT("=== 开始构建DAG，输入%d个DPU ==="), DPUs.Num()));
    
    // 1. 验证DPU有效性 - Validate DPU validity
    if (!ValidateDPUs(DPUs))
    {
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("DPU验证失败：存在无效的DPU");
        Result.DiagnosticInfo = DiagnosticLog;
        return Result;
    }
    
    // 2. 构建依赖图 - Build dependency graph
    AddDiagnosticLog(TEXT("步骤1: 构建依赖图..."));
    if (!BuildDependencyGraph(DPUs))
    {
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("依赖图构建失败：存在冲突的提供者或缺失的依赖");
        Result.DiagnosticInfo = DiagnosticLog;
        return Result;
    }
    
    // 3. 纯DFS拓扑排序：环检测 + 拓扑序列生成 - Pure DFS Topological Sort: Cycle Detection + Topological Ordering
    AddDiagnosticLog(TEXT("步骤2: 纯DFS拓扑排序（环检测+拓扑序列生成）..."));
    FDFSResult DFSResult = ProcessDAGWithPureDFS(DPUs);
    
    if (DFSResult.bHasCycle)
    {
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("DAG构建失败：检测到循环依赖");
        Result.DiagnosticInfo = DiagnosticLog + TEXT("\n环路信息: ") + DFSResult.CycleInfo;
        return Result;
    }
    
    // 4. 直接使用DFS拓扑排序结果 - Direct use of DFS topological sort result  
    AddDiagnosticLog(TEXT("步骤3: 存储DFS拓扑序列..."));
    Result.ExecutionOrder = DFSResult.ExecutionOrder;
    
    Result.bSuccess = true;
    Result.ErrorMessage = TEXT("DAG构建成功");
    Result.DiagnosticInfo = DiagnosticLog;
    
    AddDiagnosticLog(FString::Printf(TEXT("=== DAG构建完成，DFS拓扑序列包含%d个DPU ==="), Result.ExecutionOrder.Num()));
    
    return Result;
}

void UDAGBuilder::ExecuteDAG(const FDAGBuildResult& BuildResult)
{
    if (!BuildResult.bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("[DAG Execution] 执行失败：%s"), *BuildResult.ErrorMessage);
        return;
    }
    
    if (BuildResult.ExecutionOrder.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DAG Execution] 执行顺序为空，无DPU需要执行"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== 开始执行DAG，DFS拓扑序列包含%d个DPU ==="), BuildResult.ExecutionOrder.Num());
    
    // 直接按DFS拓扑序列执行 - Execute directly in DFS topological order
    for (int32 i = 0; i < BuildResult.ExecutionOrder.Num(); ++i)
    {
        USimpleDPU* DPU = BuildResult.ExecutionOrder[i];
        if (DPU)
        {
            UE_LOG(LogTemp, Log, TEXT("[%d/%d] 执行: %s"), 
                i + 1, BuildResult.ExecutionOrder.Num(), *DPU->DPUId);
            DPU->Execute();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== DAG拓扑序列执行完成 ==="));
}

FString UDAGBuilder::GetBuildReport() const
{
    return DiagnosticLog;
}

bool UDAGBuilder::BuildDependencyGraph(const TArray<USimpleDPU*>& DPUs)
{
    // 初始化数据结构 - Initialize data structures
    AdjacencyList.Empty();
    TokenProviderMap.Empty();
    
    // 第一遍：建立提供者映射 - First pass: build provider map
    for (USimpleDPU* DPU : DPUs)
    {
        if (!DPU) continue;
        
        TWeakObjectPtr<USimpleDPU> WeakDPU(DPU);
        AdjacencyList.Add(WeakDPU, TArray<TWeakObjectPtr<USimpleDPU>>());
        
        // 注册令牌提供者 - Register token providers
        for (const FString& TokenType : DPU->ProducedTokens)
        {
            if (!TokenProviderMap.Contains(TokenType))
            {
                TokenProviderMap.Add(TokenType, TArray<USimpleDPU*>());
            }
            TokenProviderMap[TokenType].Add(DPU);
            AddDiagnosticLog(FString::Printf(TEXT("注册令牌提供者：%s -> %s"), *TokenType, *DPU->DPUId));
        }
    }
    
    // 第二遍：建立依赖关系 - Second pass: build dependencies
    for (USimpleDPU* DPU : DPUs)
    {
        if (!DPU) continue;
        
        for (const FString& RequiredToken : DPU->RequiredTokens)
        {
            TArray<USimpleDPU*>* ProvidersPtr = TokenProviderMap.Find(RequiredToken);
            if (!ProvidersPtr || ProvidersPtr->Num() == 0)
            {
                AddDiagnosticLog(FString::Printf(TEXT("错误：%s需要的令牌'%s'没有提供者"), *DPU->DPUId, *RequiredToken));
                return false;
            }
            
            // 建立边：当前DPU -> 所有提供该令牌的DPU  
            // Create edges: Current DPU -> All DPUs providing this token
            for (USimpleDPU* Provider : *ProvidersPtr)
            {
                TWeakObjectPtr<USimpleDPU> ConsumerWeak(DPU);
                TWeakObjectPtr<USimpleDPU> ProviderWeak(Provider);
                AdjacencyList[ConsumerWeak].Add(ProviderWeak);
                
                AddDiagnosticLog(FString::Printf(TEXT("建立依赖关系：%s 依赖 %s (通过令牌'%s')"), *DPU->DPUId, *Provider->DPUId, *RequiredToken));
            }
        }
    }
    
    return true;
}

FDFSResult UDAGBuilder::ProcessDAGWithPureDFS(const TArray<USimpleDPU*>& DPUs)
{
    FDFSResult Result;
    TMap<USimpleDPU*, ENodeState> NodeState;
    TArray<USimpleDPU*> FinishOrder; // 记录DFS完成时间顺序
    
    // 初始化所有节点为白色 - Initialize all nodes as White
    for (USimpleDPU* DPU : DPUs)
    {
        if (DPU)
        {
            NodeState.Add(DPU, ENodeState::White);
        }
    }
    
    AddDiagnosticLog(TEXT("开始纯DFS拓扑排序..."));
    
    // 对每个白色节点启动DFS - Start DFS from each white node
    for (USimpleDPU* DPU : DPUs)
    {
        if (DPU && NodeState[DPU] == ENodeState::White)
        {
            if (DFSVisitForTopologicalSort(DPU, NodeState, FinishOrder))
            {
                Result.bHasCycle = true;
                Result.CycleInfo = TEXT("DFS过程中检测到循环依赖");
                AddDiagnosticLog(TEXT("环检测失败：DFS发现循环依赖"));
                return Result;
            }
        }
    }
    
    // DFS完成时间顺序直接就是拓扑序列 - DFS finish time order is directly the topological order
    Result.ExecutionOrder = FinishOrder;
    
    Result.bHasCycle = false;
    AddDiagnosticLog(FString::Printf(TEXT("DFS拓扑排序完成，拓扑序列包含%d个节点"), Result.ExecutionOrder.Num()));
    
    // 输出最终拓扑序列 - Output final topological order
    for (int32 i = 0; i < Result.ExecutionOrder.Num(); ++i)
    {
        USimpleDPU* DPU = Result.ExecutionOrder[i];
        AddDiagnosticLog(FString::Printf(TEXT("拓扑序列[%d]: %s"), i + 1, *DPU->DPUId));
    }
    
    return Result;
}


bool UDAGBuilder::DFSVisitForTopologicalSort(USimpleDPU* Node, TMap<USimpleDPU*, ENodeState>& NodeState, TArray<USimpleDPU*>& FinishOrder)
{
    if (!Node) return false;
    
    // 如果节点已经是灰色，说明发现了后向边（环） - Gray node means back edge (cycle)
    if (NodeState[Node] == ENodeState::Gray)
    {
        AddDiagnosticLog(FString::Printf(TEXT("DFS检测到环路：访问灰色节点 %s"), *Node->DPUId));
        return true;
    }
    
    // 如果节点已经是黑色，说明已经处理过，直接返回 - Black node is already processed
    if (NodeState[Node] == ENodeState::Black)
    {
        return false;
    }
    
    // 标记节点为灰色（正在访问） - Mark node as Gray (visiting)
    NodeState[Node] = ENodeState::Gray;
    AddDiagnosticLog(FString::Printf(TEXT("DFS开始访问: %s"), *Node->DPUId));
    
    // 递归访问当前节点依赖的所有节点 - Recursively visit all dependencies of current node
    TArray<USimpleDPU*> Dependencies = GetDirectDependencies(Node);
    
    for (USimpleDPU* Dependency : Dependencies)
    {
        if (Dependency && DFSVisitForTopologicalSort(Dependency, NodeState, FinishOrder))
        {
            return true; // 发现环路，传播错误
        }
    }
    
    // 标记节点为黑色（已完成） - Mark node as Black (completed)
    NodeState[Node] = ENodeState::Black;
    FinishOrder.Add(Node); // 记录完成时间
    
    AddDiagnosticLog(FString::Printf(TEXT("DFS完成访问: %s [完成序号: %d]"), 
        *Node->DPUId, FinishOrder.Num()));
    
    return false;
}

TArray<USimpleDPU*> UDAGBuilder::GetDirectDependencies(USimpleDPU* Node) const
{
    TArray<USimpleDPU*> Dependencies;
    
    if (!Node) 
    {
        return Dependencies;
    }
    
    // 查找邻接表中当前节点的直接依赖 - Find direct dependencies in adjacency list
    TWeakObjectPtr<USimpleDPU> WeakNode(Node);
    const TArray<TWeakObjectPtr<USimpleDPU>>* DependencyPtr = AdjacencyList.Find(WeakNode);
    
    if (DependencyPtr)
    {
        for (const TWeakObjectPtr<USimpleDPU>& WeakDependency : *DependencyPtr)
        {
            if (WeakDependency.IsValid())
            {
                USimpleDPU* Dependency = WeakDependency.Get();
                if (Dependency)
                {
                    Dependencies.Add(Dependency);
                }
            }
        }
    }
    
    return Dependencies;
}





void UDAGBuilder::ClearState()
{
    AdjacencyList.Empty();
    TokenProviderMap.Empty();
    DiagnosticLog.Empty();
}

void UDAGBuilder::AddDiagnosticLog(const FString& Message)
{
    DiagnosticLog += Message + TEXT("\n");
    UE_LOG(LogTemp, Log, TEXT("[DAG Builder] %s"), *Message);
}

bool UDAGBuilder::ValidateDPUs(const TArray<USimpleDPU*>& DPUs)
{
    for (USimpleDPU* DPU : DPUs)
    {
        if (!DPU)
        {
            AddDiagnosticLog(TEXT("错误：发现空的DPU指针"));
            return false;
        }
        
        if (DPU->DPUId.IsEmpty())
        {
            AddDiagnosticLog(FString::Printf(TEXT("错误：DPU '%s' 缺少ID"), *DPU->DisplayName));
            return false;
        }
        
        AddDiagnosticLog(FString::Printf(TEXT("验证DPU：%s"), *DPU->GetDebugInfo()));
    }
    
    return true;
}

