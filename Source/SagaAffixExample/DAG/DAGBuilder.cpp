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
    
    // 3. DFS统一处理：环检测 + 优先级排序 - DFS Unified Processing: Cycle Detection + Priority-driven Sorting
    AddDiagnosticLog(TEXT("步骤2: DFS统一处理（环检测+优先级排序）..."));
    FDFSResult DFSResult = ProcessDAGWithDFS(DPUs);
    
    if (DFSResult.bHasCycle)
    {
        Result.bSuccess = false;
        Result.ErrorMessage = TEXT("DAG构建失败：检测到循环依赖");
        Result.DiagnosticInfo = DiagnosticLog + TEXT("\n环路信息: ") + DFSResult.CycleInfo;
        return Result;
    }
    
    // 4. 直接使用DFS线性执行顺序 - Direct use of DFS linear execution order
    AddDiagnosticLog(TEXT("步骤3: 存储线性执行顺序..."));
    Result.ExecutionOrder = DFSResult.ExecutionOrder;
    
    Result.bSuccess = true;
    Result.ErrorMessage = TEXT("DAG构建成功");
    Result.DiagnosticInfo = DiagnosticLog;
    
    AddDiagnosticLog(FString::Printf(TEXT("=== DAG构建完成，线性执行顺序包含%d个DPU ==="), Result.ExecutionOrder.Num()));
    
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
    
    UE_LOG(LogTemp, Warning, TEXT("=== 开始执行DAG，线性顺序包含%d个DPU ==="), BuildResult.ExecutionOrder.Num());
    
    // 直接按DFS确定的顺序执行 - Execute directly in DFS-determined order
    for (int32 i = 0; i < BuildResult.ExecutionOrder.Num(); ++i)
    {
        USimpleDPU* DPU = BuildResult.ExecutionOrder[i];
        if (DPU)
        {
            UE_LOG(LogTemp, Log, TEXT("[%d/%d] 执行: %s (P:%d)"), 
                i + 1, BuildResult.ExecutionOrder.Num(), *DPU->DPUId, DPU->Priority);
            DPU->Execute();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== DAG线性执行完成 ==="));
}

FString UDAGBuilder::GetBuildReport() const
{
    return DiagnosticLog;
}

bool UDAGBuilder::BuildDependencyGraph(const TArray<USimpleDPU*>& DPUs)
{
    // 初始化数据结构 - Initialize data structures
    AdjacencyList.Empty();
    IndegreeMap.Empty();
    TokenProviderMap.Empty();
    
    // 第一遍：建立提供者映射 - First pass: build provider map
    for (USimpleDPU* DPU : DPUs)
    {
        if (!DPU) continue;
        
        // 初始化入度为0 - Initialize indegree to 0
        IndegreeMap.Add(DPU, 0);
        AdjacencyList.Add(DPU, TArray<USimpleDPU*>());
        
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
            
            // 建立边：所有提供该令牌的DPU -> 当前DPU
            // Create edges: All DPUs providing this token -> Current DPU
            for (USimpleDPU* Provider : *ProvidersPtr)
            {
                AdjacencyList[Provider].Add(DPU);
                IndegreeMap[DPU]++;
                
                AddDiagnosticLog(FString::Printf(TEXT("建立令牌依赖：%s -> %s (通过令牌'%s')"), *Provider->DPUId, *DPU->DPUId, *RequiredToken));
            }
        }
    }
    
    return true;
}

FDFSResult UDAGBuilder::ProcessDAGWithDFS(const TArray<USimpleDPU*>& DPUs)
{
    FDFSResult Result;
    TMap<USimpleDPU*, ENodeState> NodeState;
    TSet<USimpleDPU*> ExecutedNodes;
    
    // 初始化所有节点为白色 - Initialize all nodes as White
    for (USimpleDPU* DPU : DPUs)
    {
        if (DPU)
        {
            NodeState.Add(DPU, ENodeState::White);
        }
    }
    
    AddDiagnosticLog(TEXT("开始优先级驱动的迭代处理..."));
    
    // 首先进行环检测（使用DFS） - First perform cycle detection using DFS
    if (HasCycleDFS(DPUs, NodeState))
    {
        Result.bHasCycle = true;
        Result.CycleInfo = TEXT("检测到循环依赖");
        AddDiagnosticLog(TEXT("环检测失败：存在循环依赖"));
        return Result;
    }
    
    AddDiagnosticLog(TEXT("环检测通过，开始优先级排序..."));
    
    // 重置节点状态用于优先级排序 - Reset node state for priority-driven sorting
    for (auto& Pair : NodeState)
    {
        Pair.Value = ENodeState::White;
    }
    
    // 迭代选择就绪的最高优先级节点 - Iteratively select ready nodes with highest priority
    while (Result.ExecutionOrder.Num() < DPUs.Num())
    {
        // 找到所有就绪的节点 - Find all ready nodes
        TArray<USimpleDPU*> ReadyNodes;
        for (USimpleDPU* DPU : DPUs)
        {
            if (DPU && NodeState[DPU] == ENodeState::White && IsNodeReady(DPU, NodeState))
            {
                ReadyNodes.Add(DPU);
            }
        }
        
        if (ReadyNodes.Num() == 0)
        {
            // 没有就绪节点但还有未处理节点，说明存在问题
            Result.bHasCycle = true;
            Result.CycleInfo = TEXT("无法找到就绪节点，可能存在未检测到的依赖问题");
            AddDiagnosticLog(Result.CycleInfo);
            return Result;
        }
        
        // 按优先级排序（数值小优先） - Sort by priority (smaller value first)
        ReadyNodes.Sort([](const USimpleDPU& A, const USimpleDPU& B) {
            return A.Priority < B.Priority;
        });
        
        // 选择最高优先级的就绪节点 - Select the highest priority ready node
        USimpleDPU* SelectedNode = ReadyNodes[0];
        
        // 执行选中的节点 - Execute selected node
        NodeState[SelectedNode] = ENodeState::Black;
        ExecutedNodes.Add(SelectedNode);
        Result.ExecutionOrder.Add(SelectedNode);
        
        AddDiagnosticLog(FString::Printf(TEXT("执行节点: %s (P:%d) [第%d个]"), 
            *SelectedNode->DPUId, SelectedNode->Priority, Result.ExecutionOrder.Num()));
    }
    
    Result.bHasCycle = false;
    AddDiagnosticLog(FString::Printf(TEXT("优先级排序完成，执行顺序包含%d个节点"), Result.ExecutionOrder.Num()));
    
    // 输出最终执行顺序 - Output final execution order
    for (int32 i = 0; i < Result.ExecutionOrder.Num(); ++i)
    {
        USimpleDPU* DPU = Result.ExecutionOrder[i];
        AddDiagnosticLog(FString::Printf(TEXT("执行顺序[%d]: %s (P:%d)"), i + 1, *DPU->DPUId, DPU->Priority));
    }
    
    return Result;
}

bool UDAGBuilder::HasCycleDFS(const TArray<USimpleDPU*>& DPUs, TMap<USimpleDPU*, ENodeState>& NodeState)
{
    // 重置所有节点状态为白色 - Reset all nodes to White
    for (auto& Pair : NodeState)
    {
        Pair.Value = ENodeState::White;
    }
    
    AddDiagnosticLog(TEXT("开始DFS环检测..."));
    
    // 对每个白色节点进行DFS - DFS from each white node
    for (USimpleDPU* DPU : DPUs)
    {
        if (DPU && NodeState[DPU] == ENodeState::White)
        {
            if (DFSVisitForCycleCheck(DPU, NodeState))
            {
                AddDiagnosticLog(FString::Printf(TEXT("在节点 %s 的DFS中检测到环路"), *DPU->DPUId));
                return true;
            }
        }
    }
    
    AddDiagnosticLog(TEXT("DFS环检测完成：未发现环路"));
    return false;
}

bool UDAGBuilder::DFSVisitForCycleCheck(USimpleDPU* Node, TMap<USimpleDPU*, ENodeState>& NodeState)
{
    if (!Node) return false;
    
    // 如果节点已经是灰色，说明发现了后向边（环） - Gray node means back edge (cycle)
    if (NodeState[Node] == ENodeState::Gray)
    {
        AddDiagnosticLog(FString::Printf(TEXT("检测到环路：访问灰色节点 %s"), *Node->DPUId));
        return true;
    }
    
    // 如果节点已经是黑色，说明已经处理过，直接返回 - Black node is already processed
    if (NodeState[Node] == ENodeState::Black)
    {
        return false;
    }
    
    // 标记节点为灰色（正在访问） - Mark node as Gray (visiting)
    NodeState[Node] = ENodeState::Gray;
    
    // 访问所有子节点 - Visit all children
    TArray<USimpleDPU*>* ChildrenPtr = AdjacencyList.Find(Node);
    if (ChildrenPtr)
    {
        for (USimpleDPU* Child : *ChildrenPtr)
        {
            if (Child && DFSVisitForCycleCheck(Child, NodeState))
            {
                return true; // 发现环路
            }
        }
    }
    
    // 标记节点为黑色（已完成） - Mark node as Black (completed)
    NodeState[Node] = ENodeState::Black;
    
    return false;
}

bool UDAGBuilder::IsNodeReady(USimpleDPU* Node, const TMap<USimpleDPU*, ENodeState>& NodeState)
{
    if (!Node) return false;
    
    // 检查所有需要的令牌是否都已被提供 - Check if all required tokens are provided
    for (const FString& RequiredToken : Node->RequiredTokens)
    {
        TArray<USimpleDPU*>* ProvidersPtr = TokenProviderMap.Find(RequiredToken);
        if (!ProvidersPtr || ProvidersPtr->Num() == 0)
        {
            return false; // 没有提供者
        }
        
        // 检查所有提供该令牌的DPU是否都已完成（黑色） - Check if all providers are completed (Black)
        for (USimpleDPU* Provider : *ProvidersPtr)
        {
            const ENodeState* ProviderStatePtr = NodeState.Find(Provider);
            if (!ProviderStatePtr || *ProviderStatePtr != ENodeState::Black)
            {
                return false; // 还有提供者未完成
            }
        }
    }
    
    return true; // 所有依赖都已满足
}





void UDAGBuilder::ClearState()
{
    AdjacencyList.Empty();
    IndegreeMap.Empty();
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

