// Fill out your copyright notice in the Description page of Project Settings.


#include "Flow/CWSFlowGraph.h"

#include "Containers/Deque.h"
#include "Flow/CWSPipeDefinition.h"


CWSFlowGraph::CWSFlowGraph()
{
}

bool CWSFlowGraph::AddNode(FGuid NodeId, bool bIsRoot = false)
{
	if (!NodeId.IsValid() || Nodes.Contains(NodeId))
	{
		return false;
	}
	FCWSFlowNode Node;
	Node.NodeId = NodeId;
	
	Nodes.Add(NodeId, Node);
	if (bIsRoot)
	{
		RootNodes.Add(NodeId);
	}

	return true;
}

bool CWSFlowGraph::AddEdge(
	FGuid SourceId,
	FGuid TargetId,
	float EdgeWeight,
	ECWSPipeType SourceType,
	ECWSPipeType TargetType,
	ECWSPipeDirection SourceDirection,
	ECWSPipeDirection TargetDirection
) {
	const bool bCanFlowAToB = CanSendFlow(SourceType) && CanReceiveFlow(TargetType) && SourceDirection != ECWSPipeDirection::Up;
	const bool bCanFlowBToA = CanReceiveFlow(SourceType) && CanSendFlow(TargetType) && TargetDirection != ECWSPipeDirection::Up;
	
	if (!(bCanFlowAToB || bCanFlowBToA))
	{
		return false;
	}
	
	FCWSFlowNode* SourceNode = Nodes.Find(SourceId);
	FCWSFlowNode* TargetNode = Nodes.Find(TargetId);
	if (!SourceNode || !TargetNode)
	{
		return false;
	}

	FCWSFlowEdge Edge;
	Edge.Nodes.Add(SourceNode->NodeId, TargetNode->NodeId);
	Edge.Nodes.Add(TargetNode->NodeId, SourceNode->NodeId);
	Edge.bVisited = false;
	Edge.NumberOfDirections = static_cast<uint8>(bCanFlowAToB) + static_cast<uint8>(bCanFlowBToA);
	const int32 EdgeIndex = Edges.Add(Edge);

	
	if (bCanFlowAToB)
	{
		SourceNode->OutgoingEdges.Add(EdgeIndex);
		++SourceNode->NumberOfOutgoingEdges;
	}
	if (bCanFlowBToA)
	{
		TargetNode->OutgoingEdges.Add(EdgeIndex);
		++TargetNode->NumberOfOutgoingEdges;
	}
	UE_LOG(LogTemp, Warning, TEXT("NodeAdd Check %s, %d"), *SourceNode->NodeId.ToString(), SourceNode->NumberOfOutgoingEdges);
	UE_LOG(LogTemp, Warning, TEXT("NodeAdd Check %s, %d"), *TargetNode->NodeId.ToString(), TargetNode->NumberOfOutgoingEdges);

	return true;
}

void CWSFlowGraph::Reset()
{
	Nodes.Reset();
	NodesBackup.Reset();
	Edges.Reset();
	RootNodes.Reset();
	EndNodes.Reset();
	TraversalMap.Reset();
	CannonicalIds.Reset();
	NodeFlows.Reset();
	ActiveOutgoingEdgeCount.Reset();
}

int32 CWSFlowGraph::GetNodeCount() const
{
	return Nodes.Num();
}

int32 CWSFlowGraph::GetEdgeCount() const
{
	return Edges.Num();
}

int32 CWSFlowGraph::GetOutgoingEdgesCount(const FGuid& NodeId) const
{
	const FCWSFlowNode* Node = Nodes.Find(NodeId);
	
	return Node ? Node->OutgoingEdges.Num() : INDEX_NONE;
}


/**
 * Builds a flow graph by filling the traversal map.
 * 
 * @return True if the flow graph was successfully constructed.
 */
bool CWSFlowGraph::ConstructFlowGraph()
{
	if (RootNodes.Num() < 1)
	{
		return false;
	}
	ResetGraphState();

	// Two queues
	// Curret -> k depth we are currently processing
	// Next -> k+1 depth to process next
	TDeque<FGuid> CurrentProcessingQueue;
	TDeque<FGuid> NextProcessingQueue;

	// Create flow dependency traversal map 
	// Set to prevent revisiting nodes
	TSet<FGuid> VisitedNodes;
	TSet<int32> UsedEdges;
	
	// Add roots to queue
	for (const FGuid& RootNodeID : RootNodes)
	{
		NextProcessingQueue.EmplaceFirst(RootNodeID);
		VisitedNodes.Add(RootNodeID);
	
	}


	
	while (!NextProcessingQueue.IsEmpty())
	{
		// Move Next to Current
		Swap(CurrentProcessingQueue, NextProcessingQueue);
		NextProcessingQueue.Reset();
		
		// Map of connections to process
		TMap<int32, FGuid> Connections;
		// List of conflicts to resolve before handling edges.
		TArray<int32> EdgeConflicts;

		
		while (!CurrentProcessingQueue.IsEmpty())
		{
			const FGuid CurrentNodeId = CurrentProcessingQueue.First();
			
			CurrentProcessingQueue.PopFirst();

			const FCWSFlowNode* CurrentNode = Nodes.Find(CurrentNodeId);
			if (!CurrentNode)
			{
				return false;
			}
			

			// Check if we reached an end Node and store it
			if (CurrentNode->NumberOfOutgoingEdges == 0)
			{
				EndNodes.Add(CurrentNode->NodeId);
				continue;
			}
			
			// Visits one node
			// Updates NextQueue, Connections and EdgeConflicts
			if (!TraverseNode(
				CurrentNode,
				NextProcessingQueue,
				VisitedNodes,
				Connections,
				EdgeConflicts,
				UsedEdges
			)) {
				return false;
			}
		}

		// Mark Newly traversed connections as used
		TSet<int32> VisitedEdges;
		Connections.GetKeys(VisitedEdges);
		UsedEdges.Append(VisitedEdges);
		
		// Handle every conflict
		// Each member in MergeGroups has to be remapped
		for (const int32 EdgeIndex : EdgeConflicts)
		{
			
			// Obtain NodeIds and Nodes from edge
			const FCWSFlowEdge& CurrentEdge = Edges[EdgeIndex];
			TArray<FGuid> NodesIds; 
			CurrentEdge.Nodes.GetKeys(NodesIds);

			FGuid& Node1Id = NodesIds[0];				
			FGuid& Node2Id = NodesIds[1];				

			// Merge conflicting Nodes.
			MergeNodes(Node1Id, Node2Id, EdgeIndex);
			Connections.Remove(EdgeIndex);
		}
	
		// Finalize the CannonicalId map so all Id mappings are compressed
		for (const TPair<FGuid, FGuid>& Pair : CannonicalIds)
		{
			ResolveCannonicalId(Pair.Key);
		}

		// Update the Traversal Map
		for (const TPair<int32, FGuid>& Pair : Connections)
		{
			
			const FCWSFlowEdge& CurrentEdge = Edges[Pair.Key];
			const FGuid OriginalSourceId = Pair.Value;
			const FGuid* OriginalTargetId = CurrentEdge.Nodes.Find(OriginalSourceId);
				if (!OriginalTargetId) return false;

			const FGuid SourceId = ResolveCannonicalId(OriginalSourceId);
			const FGuid TargetId = ResolveCannonicalId(*OriginalTargetId);

			// An internal edge within a merged group has been found
			if (SourceId == TargetId)
			{
				FCWSFlowNode* Node = Nodes.Find(SourceId);
					if (!Node) return false;
				if (CurrentEdge.NumberOfDirections != 1)
				{
					UE_LOG(LogTemp, Log, TEXT("Illegal edge slipped past merge and got caught in Traversal fill"));
				}
				Node->RemoveInternalEdge(Pair.Key, 1);
				continue;
			}

			FCWSFlowDependancy Dependency {
				SourceId,
				Pair.Key
			};
			
			TraversalMap.FindOrAdd(TargetId).Add(Dependency);

			FCWSFlowNode* TargetNode = Nodes.Find(TargetId);
				if (!TargetNode) return false;
			
			// Find end Nodes
			if (CurrentEdge.NumberOfDirections == 2)
			{
				UE_LOG(LogTemp, Warning, TEXT("Edge Check %s"), *TargetNode->NodeId.ToString())
				--TargetNode->NumberOfOutgoingEdges;
			}

			--ActiveOutgoingEdgeCount.FindOrAdd(SourceId, 1);
		}
	}

	FindEndpoints();
	
	return true;
}

/**
 * Pushes flow through the flow graph and stores all graph endpoints. 
 * Must be called after ConstructFlowGraph.
 * 
 * @return True if the function successfully executes
 */
bool CWSFlowGraph::ComputeFlow()
{
	if (TraversalMap.IsEmpty())
	{
		return false;
	}
	
	ResetFlowState();
	// Queue of flow search order
	TDeque<FGuid> FlowQueue;
	// Set of visited Nodes
	TSet<FGuid> VisitedNodes;
	
	for (const FGuid& RootNodeId : RootNodes)
	{
		FCWSFlowNode* Root = Nodes.Find(RootNodeId);
		if (!Root)
		{
			continue;
		}

		VisitedNodes.Add(Root->NodeId);
		
		// For now, use Default root flow of 1
		// TODO Give variable flow to Root Nodes.
		FCWSFlow Flow;
		Flow.Amount = 1.0f;
		if (Root->NumberOfOutgoingEdges != 0)
		{
			Flow /= Root->NumberOfOutgoingEdges;
		}
		NodeFlows.Add(RootNodeId, Flow);
	}

	// Pass through every Node
	for (const TPair<FGuid, TArray<FCWSFlowDependancy>>& Pair : TraversalMap)
	{
		const FCWSFlowNode* StartingNode = Nodes.Find(Pair.Key);
		if (!StartingNode){return false;}

		if (NodeFlows.Contains(StartingNode->NodeId))
		{
			continue;
		}
		
		FlowQueue.PushFirst(StartingNode->NodeId);
		VisitedNodes.Reset();
		
		// Move in the reversed flow direction to compute incoming flow values
		while (!FlowQueue.IsEmpty())
		{
			const FGuid NodeId = FlowQueue.First();
			
			const FCWSFlowNode* Node = Nodes.Find(NodeId);
			if (!Node) {return false;}

			VisitedNodes.Add(Node->NodeId);

			// Add unprocessed Nodes to the array to be added to the queue
			TArray<FGuid> UnprocessedNodeIds;
			FCWSFlow Flow;
			const TArray<FCWSFlowDependancy>* Dependencies = TraversalMap.Find(Node->NodeId);
			if (!Dependencies) {return false;}
			for (const auto [SourceNodeId, EdgeIndex] : *Dependencies)
			{
				if (!NodeFlows.Contains(SourceNodeId))
				{
					// If a visited Node is attempted to be added, a cycle is found
					if (VisitedNodes.Contains(SourceNodeId))
					{
						// TODO CYCLE LOGIC
						continue;
					}
					
					UnprocessedNodeIds.Add(SourceNodeId);
					continue;
				}
				Flow += NodeFlows[SourceNodeId];
			}
			UE_LOG(LogTemp, Warning, TEXT("Flow Check %s"), *NodeId.ToString());
			UE_LOG(LogTemp, Warning, TEXT("Flow Check %s"), *NodeId.ToString());

			// If all input nodes have already been processed, store the combined flow
			if (UnprocessedNodeIds.IsEmpty())
			{
				if (Node->NumberOfOutgoingEdges != 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("Flow Check %s"), *NodeId.ToString());
					
					Flow /= Node->NumberOfOutgoingEdges;
				}
				UE_LOG(LogTemp, Warning, TEXT("Flow Check %s %d"), *NodeId.ToString(), Node->NumberOfOutgoingEdges);
				
				FlowQueue.PopFirst();
				NodeFlows.Add(Node->NodeId, Flow);
				continue;
			}

			// Add unprocessed Nodes to the queue while keeping the source Node
			for (const FGuid& SourceNodeId : UnprocessedNodeIds)
			{
				FlowQueue.PushFirst(SourceNodeId);
			}
		}
	}

	
	return true;
}


/**
 * 
 * @param Node The node to be traversed at depth k.
 * @param NextProcessingQueue The processing queue at depth k + 1.
 * @param VisitedNodes Already visited nodes.
 * @param Connections A map of used connections and the node from which they were used.
 * @param EdgeConflicts An array of edges that got used from both vertices at depth k.
 * @param UsedEdges A set of already used edges
 * @return True if the node was successfully traversed.
 */
bool CWSFlowGraph::TraverseNode(
	const FCWSFlowNode* Node,
	TDeque<FGuid>& NextProcessingQueue,
	TSet<FGuid>& VisitedNodes,
	TMap<int32, FGuid>& Connections,
	TArray<int32>& EdgeConflicts,
	const TSet<int32>& UsedEdges
) {
	for (const int32& EdgeId : Node->OutgoingEdges)
	{
		// Get Targeted node
		const FCWSFlowEdge& Edge = Edges[EdgeId];
		UE_LOG(LogTemp, Warning, TEXT("Traversal %s Rest"), *Node->NodeId.ToString());
		const FGuid* TargetNodeId = Edge.Nodes.Find(Node->NodeId);
		if (!TargetNodeId){return false;}
		
		// Ignore used edges
		if (UsedEdges.Contains(EdgeId))
		{
			continue;
		}
		
		// Add the node to the traversal map and reduce
		// its number of open edges
		if (Connections.Contains(EdgeId))
		{
			EdgeConflicts.Add(EdgeId);
		}
		else
		{
			Connections.Add(EdgeId, Node->NodeId);
		}
		
		// Add the node to the queue to be processed
		// if it has not been processed yet.
		if (!VisitedNodes.Contains(*TargetNodeId))
		{
			NextProcessingQueue.PushLast(*TargetNodeId);
			VisitedNodes.Add(*TargetNodeId);
		}
	}
	return true;
}


/**
 * Merges two nodes by either creating a new node to merge two primary nodes or
 * uses an existing secondary merged node.
 * 
 * @param Node1Id The first node to be merged.
 * @param Node2Id The second node to be merged.
 * @param EdgeId ID of the edge connecting the merge
 */
void CWSFlowGraph::MergeNodes(
	FGuid Node1Id,
	FGuid Node2Id,
	int32 EdgeId
) {
	Node1Id = ResolveCannonicalId(Node1Id);
	Node2Id = ResolveCannonicalId(Node2Id);

	FCWSFlowNode* Node1 = Nodes.Find(Node1Id);
	FCWSFlowNode* Node2 = Nodes.Find(Node2Id);
	
	if (Node1Id == Node2Id)
	{
		Node1->RemoveInternalEdge(EdgeId, 2);
		return;
	}
	// Remap TraversalMap
	TArray<FCWSFlowDependancy> Dependencies;
	const TArray<FCWSFlowDependancy>* Node1Dependencies = TraversalMap.Find(Node1Id);
	const TArray<FCWSFlowDependancy>* Node2Dependencies = TraversalMap.Find(Node2Id);
	
	if (Node1Dependencies)
	{
		Dependencies.Append(*Node1Dependencies);
		TraversalMap.Remove(Node1Id);
	}
	if (Node2Dependencies)
	{
		Dependencies.Append(*Node2Dependencies);
		TraversalMap.Remove(Node2Id);
	}
	
	// Create a new Node and redirect them to it
	const FCWSFlowNode MergedNode = Node1->MergeWith(*Node2, EdgeId);
	CannonicalIds.Add(Node1Id, MergedNode.NodeId);
	CannonicalIds.Add(Node2Id, MergedNode.NodeId);
	CannonicalIds.Add(MergedNode.NodeId, MergedNode.NodeId);

	TraversalMap.Add(MergedNode.NodeId, Dependencies);
	
	
	Nodes.Add(MergedNode.NodeId, MergedNode);
}

void CWSFlowGraph::FindEndpoints()
{
	for (const TPair<FGuid, int32>& Pair : ActiveOutgoingEdgeCount)
	{
		if (Pair.Value == 0)
		{
			EndNodes.Add(Pair.Key);
		}
	}
}

FGuid CWSFlowGraph::ResolveCannonicalId(const FGuid& NodeId)
{
	FGuid* CannonicalId = CannonicalIds.Find(NodeId);
	if (!CannonicalId)
	{
		CannonicalIds.Add(NodeId, NodeId);
		return NodeId;
	}
	if (*CannonicalId == NodeId)
	{
		return NodeId;
	}

	const FGuid ResolvedId = ResolveCannonicalId(*CannonicalId);
	CannonicalIds.Add(NodeId, ResolvedId);
	return ResolvedId;
}

void CWSFlowGraph::ResetGraphState()
{
	Nodes = NodesBackup;
	TraversalMap.Reset();
	EndNodes.Reset();
	CannonicalIds.Reset();
	ActiveOutgoingEdgeCount.Reset();
}

void CWSFlowGraph::ResetFlowState()
{
	NodeFlows.Reset();
}


void CWSFlowGraph::FinalizeBuild()
{
	NodesBackup = Nodes;
}

TArray<FCWSFlow> CWSFlowGraph::GetEndPointFlows() const
{
	TArray<FCWSFlow> EndPointFlows;
	for (const FGuid& NodeId : EndNodes)
	{
		const FCWSFlow* Flow = NodeFlows.Find(NodeId);
		if (!Flow)
		{
			UE_LOG(LogTemp, Error, TEXT("NodeFlow not found for %s"), *NodeId.ToString());
			continue;
		}
		EndPointFlows.Add(*Flow);
	}

	return EndPointFlows;
}

void CWSFlowGraph::LogComputedFlows() const
{
	for (const TPair<FGuid, FCWSFlow>& Pair : NodeFlows)
	{
		UE_LOG(LogTemp, Warning, TEXT("Node %s: Flow = %f"), *Pair.Key.ToString(), Pair.Value.Amount);
	}
}



