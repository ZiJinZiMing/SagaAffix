#include "DAG/DAG.h"

#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformProcess.h"

// 构造函数：初始化DAG状态 / Constructor: Initialize DAG state
UDAG::UDAG()
{
	bIsExecuting = false;
}

// 添加节点到DAG / Add node to DAG
bool UDAG::AddNode(UDAGNode* Node)
{
	// 检查节点有效性和是否已存在 / Check node validity and if already exists
	if (!Node || Nodes.Contains(Node))
	{
		return false;
	}

	Nodes.Add(Node);
	return true;
}

// 从DAG移除节点 / Remove node from DAG
bool UDAG::RemoveNode(UDAGNode* Node)
{
	// 检查节点有效性和是否存在 / Check node validity and if exists
	if (!Node || !Nodes.Contains(Node))
	{
		return false;
	}

	// 移除连接到此节点的所有边 / Remove all edges connected to this node
	for (UDAGNode* Child : Node->GetChildren())
	{
		RemoveEdge(Node, Child);
	}

	for (UDAGNode* Parent : Node->GetParents())
	{
		RemoveEdge(Parent, Node);
	}

	Nodes.Remove(Node);
	return true;
}

// 添加边（依赖关系）/ Add edge (dependency relationship)
bool UDAG::AddEdge(UDAGNode* FromNode, UDAGNode* ToNode)
{
	// 验证边的有效性 / Validate edge validity
	if (!IsValidEdge(FromNode, ToNode))
	{
		return false;
	}

	// FromNode依赖ToNode / FromNode depends on ToNode
	FromNode->AddChild(ToNode);
	return true;
}

// 移除边（依赖关系）/ Remove edge (dependency relationship)
bool UDAG::RemoveEdge(UDAGNode* FromNode, UDAGNode* ToNode)
{
	// 检查节点有效性 / Check node validity
	if (!FromNode || !ToNode)
	{
		return false;
	}

	// 移除依赖关系 / Remove dependency relationship
	FromNode->RemoveChild(ToNode);
	return true;
}

// 执行DAG中的所有节点 / Execute all nodes in DAG
void UDAG::ExecuteDAG(EDAGTraversalOrder TraversalOrder)
{
	// 检查是否已在执行 / Check if already executing
	if (bIsExecuting)
	{
		return;
	}

	bIsExecuting = true;
	ResetAllNodeStates();
	ExecutionQueue.Empty();

	// 根据依赖图原则获取执行顺序 / Get execution order based on dependency graph principles:
	// 子节点（被依赖项）在父节点（依赖项）之前执行 / Child nodes (dependencies) execute before Parent nodes (dependents)
	switch (TraversalOrder)
	{
	case EDAGTraversalOrder::DepthFirst:
		ExecutionQueue = GetDFSTraversal();
		break;
	case EDAGTraversalOrder::TopologicalSort:
		ExecutionQueue = GetTopologicalSort();
		break;
	}

	// 按依赖顺序执行节点 / Execute nodes in dependency order: 被依赖项(children) before 依赖项(parents)
	for (UDAGNode* Node : ExecutionQueue)
	{
		if (CanExecuteNode(Node))
		{
			ExecuteNode(Node);
		}
	}

	bIsExecuting = false;
	OnDAGExecutionComplete.Broadcast();

	SaveToMMD();
}

// 检查DAG中是否有环 / Check if DAG has cycles
bool UDAG::HasCycles()
{
	// 重置所有节点状态 / Reset all node states
	ResetAllNodeStates();
	TArray<UDAGNode*> CyclePath;

	// 遍历所有节点检查环 / Traverse all nodes to check for cycles
	for (UDAGNode* Node : Nodes)
	{
		if (Node->GetState() == EDAGNodeState::Unvisited)
		{
			if (DFSCycleCheck(Node, CyclePath))
			{
				OnCycleDetected.Broadcast(CyclePath);
				return true;
			}
		}
	}

	return false;
}

// 获取拓扑排序（依赖顺序）/ Get topological sort (dependency order)
TArray<UDAGNode*> UDAG::GetTopologicalSort()
{
	// 检查是否有环，有环则无法排序 / Check for cycles, cannot sort if cycles exist
	if (HasCycles())
	{
		return TArray<UDAGNode*>();
	}

	ResetAllNodeStates();
	TArray<UDAGNode*> DependencyOrder;

	// 从根节点开始（依赖项 - 需要依赖其他节点）/ Start from root nodes (dependents - need other nodes)
	TArray<UDAGNode*> RootNodes = GetRootNodes();
	for (UDAGNode* RootNode : RootNodes)
	{
		if (RootNode->GetState() == EDAGNodeState::Unvisited)
		{
			TopologicalSortUtil(RootNode, DependencyOrder);
		}
	}

	// 处理剩余的未访问节点（处理断开的组件）/ Handle any remaining unvisited nodes (in case of disconnected components)
	for (UDAGNode* Node : Nodes)
	{
		if (Node->GetState() == EDAGNodeState::Unvisited)
		{
			TopologicalSortUtil(Node, DependencyOrder);
		}
	}

	// 返回依赖顺序 / Return in dependency order: children (被依赖项) before parents (依赖项)
	return DependencyOrder;
}

// 获取DFS遍历结果 / Get DFS traversal result
TArray<UDAGNode*> UDAG::GetDFSTraversal(UDAGNode* StartNode)
{
	ResetAllNodeStates();
	TArray<UDAGNode*> VisitedOrder;

	// 指定起始节点 / If start node is specified
	if (StartNode)
	{
		DFSVisit(StartNode, VisitedOrder);
	}
	else
	{
		// 对于依赖图，从根节点开始 / For dependency graphs, start from root nodes (依赖项 - dependent tasks)
		TArray<UDAGNode*> RootNodes = GetRootNodes();
		for (UDAGNode* Root : RootNodes)
		{
			if (Root->GetState() == EDAGNodeState::Unvisited)
			{
				DFSVisit(Root, VisitedOrder);
			}
		}

		// 处理剩余的未访问节点 / Handle any remaining unvisited nodes
		for (UDAGNode* Node : Nodes)
		{
			if (Node->GetState() == EDAGNodeState::Unvisited)
			{
				DFSVisit(Node, VisitedOrder);
			}
		}
	}

	return VisitedOrder;
}

// 获取根节点（依赖项，没有父节点）/ Get root nodes (dependents, no parent nodes)
TArray<UDAGNode*> UDAG::GetRootNodes() const
{
	TArray<UDAGNode*> RootNodes;
	
	// 遍历所有节点寻找根节点 / Traverse all nodes to find root nodes
	for (UDAGNode* Node : Nodes)
	{
		if (Node->IsRootNode())
		{
			RootNodes.Add(Node);
		}
	}

	return RootNodes;
}

// 获取叶节点（被依赖项，没有子节点）/ Get leaf nodes (dependencies, no child nodes)
TArray<UDAGNode*> UDAG::GetLeafNodes() const
{
	TArray<UDAGNode*> LeafNodes;
	
	// 遍历所有节点寻找叶节点 / Traverse all nodes to find leaf nodes
	for (UDAGNode* Node : Nodes)
	{
		if (Node->IsLeafNode())
		{
			LeafNodes.Add(Node);
		}
	}

	return LeafNodes;
}

// 重置所有节点状态 / Reset all node states
void UDAG::ResetAllNodeStates()
{
	// 遍历所有节点并重置状态 / Traverse all nodes and reset states
	for (UDAGNode* Node : Nodes)
	{
		Node->ResetState();
	}
}

// 清空DAG所有数据 / Clear all DAG data
void UDAG::Clear()
{
	// 清空执行队列和节点列表 / Clear execution queue and node list
	ExecutionQueue.Empty();
	Nodes.Empty();
	bIsExecuting = false;
}

// 检查DAG是否包含指定节点 / Check if DAG contains specified node
bool UDAG::ContainsNode(UDAGNode* Node) const
{
	return Node && Nodes.Contains(Node);
}

// DFS访问节点（递归实现）/ DFS visit node (recursive implementation)
void UDAG::DFSVisit(UDAGNode* Node, TArray<UDAGNode*>& VisitedOrder)
{
	// 检查节点有效性和状态 / Check node validity and state
	if (!Node || Node->GetState() != EDAGNodeState::Unvisited)
	{
		return;
	}

	// 标记为正在访问 / Mark as visiting
	Node->SetState(EDAGNodeState::Visiting);

	// 递归访问所有子节点 / Recursively visit all child nodes
	for (UDAGNode* Child : Node->GetChildren())
	{
		if (Child->GetState() == EDAGNodeState::Unvisited)
		{
			DFSVisit(Child, VisitedOrder);
		}
	}

	// 标记为已访问并加入结果 / Mark as visited and add to result
	Node->SetState(EDAGNodeState::Visited);
	VisitedOrder.Add(Node);
}

// DFS环检查（递归实现）/ DFS cycle check (recursive implementation)
bool UDAG::DFSCycleCheck(UDAGNode* Node, TArray<UDAGNode*>& CyclePath)
{
	// 检查节点有效性 / Check node validity
	if (!Node)
	{
		return false;
	}

	// 如果节点正在被访问，发现后向边，检测到环 / If node is being visited, found back edge - cycle detected
	if (Node->GetState() == EDAGNodeState::Visiting)
	{
		CyclePath.Add(Node);
		return true;
	}

	// 如果节点已访问，无需再检查 / If node already visited, no need to check again
	if (Node->GetState() == EDAGNodeState::Visited)
	{
		return false;
	}

	// 标记为正在访问并加入路径 / Mark as visiting and add to path
	Node->SetState(EDAGNodeState::Visiting);
	CyclePath.Add(Node);

	// 递归检查所有子节点 / Recursively check all child nodes
	for (UDAGNode* Child : Node->GetChildren())
	{
		if (DFSCycleCheck(Child, CyclePath))
		{
			return true;
		}
	}

	// 从路径中移除并标记为已访问 / Remove from path and mark as visited
	CyclePath.Remove(Node);
	Node->SetState(EDAGNodeState::Visited);
	return false;
}

// 拓扑排序辅助函数（递归实现）/ Topological sort utility function (recursive implementation)
void UDAG::TopologicalSortUtil(UDAGNode* Node, TArray<UDAGNode*>& Stack)
{
	// 检查节点有效性和状态 / Check node validity and state
	if (!Node || Node->GetState() != EDAGNodeState::Unvisited)
	{
		return;
	}

	// 标记为已访问 / Mark as visited
	Node->SetState(EDAGNodeState::Visited);

	// 递归处理所有子节点 / Recursively process all child nodes
	for (UDAGNode* Child : Node->GetChildren())
	{
		if (Child->GetState() == EDAGNodeState::Unvisited)
		{
			TopologicalSortUtil(Child, Stack);
		}
	}

	// 将节点加入栈中（后序遍历）/ Add node to stack (post-order traversal)
	Stack.Add(Node);
}

// 执行节点 / Execute node
void UDAG::ExecuteNode(UDAGNode* Node)
{
	// 检查节点有效性和可执行性 / Check node validity and executability
	if (!Node || !CanExecuteNode(Node))
	{
		return;
	}

	// 执行节点逻辑 / Execute node logic
	Node->Execute();
	Node->OnExecutionComplete();
	// 广播节点执行事件 / Broadcast node execution event
	OnNodeExecuted.Broadcast(Node);
}

// 检查节点是否可执行 / Check if node can execute
bool UDAG::CanExecuteNode(UDAGNode* Node) const
{
	return Node && Node->CanExecute();
}

// 验证DAG完整性 / Validate DAG integrity
void UDAG::ValidateDAGIntegrity()
{
	// 移除所有空节点 / Remove any null nodes
	Nodes.RemoveAll([](UDAGNode* Node) { return Node == nullptr; });

	// 验证所有边指向DAG中的有效节点 / Validate all edges point to valid nodes in the DAG
	for (UDAGNode* Node : Nodes)
	{
		const TArray<UDAGNode*>& Children = Node->GetChildren();
		for (UDAGNode* Child : Children)
		{
			if (!ContainsNode(Child))
			{
				Node->RemoveChild(Child);
			}
		}
	}
}

// 检查边是否有效 / Check if edge is valid
bool UDAG::IsValidEdge(UDAGNode* FromNode, UDAGNode* ToNode) const
{
	// 检查节点有效性 / Check node validity
	if (!FromNode || !ToNode)
	{
		return false;
	}

	// 检查节点是否在DAG中 / Check if nodes are in DAG
	if (!ContainsNode(FromNode) || !ContainsNode(ToNode))
	{
		return false;
	}

	// 禁止自环 / Prohibit self-loops
	if (FromNode == ToNode)
	{
		return false;
	}

	return true;
}

// 生成并保存Mermaid格式的DAG可视化图表 / Generate and save Mermaid format DAG visualization
void UDAG::SaveToMMD()
{
	FString MermaidContent = TEXT("graph TD\n");
	MermaidContent += FString::Printf(TEXT("    %%%% DAG Generated: %s\n\n"), 
		*FDateTime::Now().ToString());
	
	// 定义所有节点（显示节点显示名称和执行顺序，使用节点特定形状）/ Define all nodes (show node display name and execution order, using node-specific shapes)
	for (int32 Index = 0; Index < ExecutionQueue.Num(); ++Index)
	{
		UDAGNode* Node = ExecutionQueue[Index];
		if (!Node) continue;
		
		FString NodeDisplayName = Node->GetNodeDisplayName();
		FString NodeLabel = FString::Printf(TEXT("%s</br><b>Exec: %d</b>"), *NodeDisplayName, Index);
		FString NodeShape = Node->GetMermaidNodeShape(NodeLabel);
		MermaidContent += FString::Printf(TEXT("    %s%s\n"), *NodeDisplayName, *NodeShape);
	}
	
	MermaidContent += TEXT("\n");
	
	// 定义依赖关系（父节点 -> 子节点）/ Define dependency relationships (Parent -> Child)
	for (UDAGNode* Node : ExecutionQueue)
	{
		if (!Node) continue;
		
		FString ParentId = Node->GetNodeDisplayName();
		
		// 遍历该节点的所有子节点（被依赖项）/ Traverse all child nodes (dependencies)
		for (UDAGNode* Child : Node->GetChildren())
		{
			if (!Child) continue;
			
			FString ChildId = Child->GetNodeDisplayName();
			MermaidContent += FString::Printf(TEXT("    %s --> %s\n"), *ParentId, *ChildId);
		}
	}
	
	SaveMermaidToFile(MermaidContent);
}

// 将Mermaid内容保存到项目Saved/Graphs目录下的.mmd文件 / Save Mermaid content to .mmd file in project Saved/Graphs directory
void UDAG::SaveMermaidToFile(const FString& Content)
{
	// 生成.mmd文件名 / Generate .mmd filename
	FString FileName = FString::Printf(TEXT("Generic_DAG_Graph_%s.mmd"), 
		*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	
	// 保存到项目的 Saved/Graphs 目录 / Save to project's Saved/Graphs directory
	FString FilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Graphs"), FileName);
	
	// 确保目录存在 / Ensure directory exists
	FString DirectoryPath = FPaths::GetPath(FilePath);
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*DirectoryPath))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*DirectoryPath);
	}
	
	// 写入文件 / Write to file
	if (FFileHelper::SaveStringToFile(Content, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8))
	{
		UE_LOG(LogTemp, Log, TEXT("Generic DAG Mermaid图表已保存到: %s"), *FilePath);
		
		// 可选：在Windows上自动打开文件 / Optional: Auto-open file on Windows
		#if PLATFORM_WINDOWS
		FString FullPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FilePath);
		FPlatformProcess::LaunchFileInDefaultExternalApplication(*FullPath);
		#endif
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("保存Generic DAG Mermaid图表失败: %s"), *FilePath);
	}
}