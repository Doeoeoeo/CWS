// Fill out your copyright notice in the Description page of Project Settings.


#include "Flow/CWSFlowGraph.h"

#include "Elements/Framework/TypedElementQueryBuilder.h"
#include "Flow/CWSPipeDefinition.h"
#include "Navigation/PathFollowingComponent.h"
#include "Storage/Bundles/V2/PacketHandle.h"


CWSFlowGraph::CWSFlowGraph()
{
}

bool CWSFlowGraph::AddNode(FGuid NodeId)
{
	if (!NodeId.IsValid() || Nodes.Contains(NodeId))
	{
		return false;
	}
	FCWSFlowNode Node;
	Node.NodeId = NodeId;
	
	Nodes.Add(NodeId, Node);

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
	UE_LOG(LogTemp, Warning, TEXT("Stop 1"));
	
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
	UE_LOG(LogTemp, Warning, TEXT("Stop 2"));

	FCWSFlowEdge Edge;
	Edge.Nodes.Add(SourceNode->NodeId, TargetNode->NodeId);
	Edge.Nodes.Add(TargetNode->NodeId, SourceNode->NodeId);
	Edge.bVisited = false;
	Edge.OutputWeight = EdgeWeight;

	int32 EdgeIndex = Edges.Add(Edge);

	
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
	UE_LOG(LogTemp, Warning, TEXT("Stop 3"));

	return true;
}

void CWSFlowGraph::Reset()
{
	Nodes.Reset();
	Edges.Reset();
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


