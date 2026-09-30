// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CWSPipeTypes.h"
#include "CoreMinimal.h"
#include "CWSPipeDefinition.h"

struct FCWSFlowNode;

struct FCWSFlowEdge
{
	// FGUID points to the other node
	TMap<FGuid, FGuid> Nodes;

	// Any potential stats of the connection
	float OutputWeight;

	bool bVisited = false;
};

struct FCWSFlowNode
{
	FGuid NodeId;
	TArray<int32> OutgoingEdges;
	
	int32 NumberOfOutgoingEdges = 0;
};

class CWS_API CWSFlowGraph
{
public:
	CWSFlowGraph();

	bool AddNode(FGuid NodeId);
	bool AddEdge(FGuid SourceId, FGuid TargetId, float EdgeWeight, ECWSPipeType SourceType, ECWSPipeType TargetType, ECWSPipeDirection
	             SourceDirection, ECWSPipeDirection TargetDirection);
	void Reset();
	int32 GetNodeCount() const;
	int32 GetEdgeCount() const;
	int32 GetOutgoingEdgesCount(const FGuid& NodeId) const;
private:
	static bool CanReceiveFlow(ECWSPipeType Type)
	{
		return (Type == ECWSPipeType::Input) || (Type == ECWSPipeType::Bidirectional);
	}

	static bool CanSendFlow(ECWSPipeType Type)
	{
		return (Type == ECWSPipeType::Output) || (Type == ECWSPipeType::Bidirectional);
	}
	
	TMap<FGuid, FCWSFlowNode> Nodes;
	TArray<FCWSFlowEdge> Edges;
};
