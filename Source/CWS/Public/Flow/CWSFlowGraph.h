// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CWSPipeTypes.h"
#include "CoreMinimal.h"
#include "CWSPipeDefinition.h"
#include "Containers/Deque.h"

struct FCWSFlowNode;


// FLow graph structs
struct FCWSFlowEdge
{
	// FGUID points to the other node
	TMap<FGuid, FGuid> Nodes;

	uint8 NumberOfDirections = 0;

	// Any potential stats of the connection
	bool bVisited = false;
};

struct FCWSFlowNode
{
	FGuid NodeId;
	TSet<int32> OutgoingEdges;
	
	int32 NumberOfOutgoingEdges = 0;

	FCWSFlowNode MergeWith(const FCWSFlowNode& Other, const int32 EdgeId) const
	{
		FCWSFlowNode Result;
		Result.NodeId = FGuid::NewGuid();
		Result.OutgoingEdges.Reserve(OutgoingEdges.Num() + Other.OutgoingEdges.Num());
		Result.OutgoingEdges.Append(OutgoingEdges);
		Result.OutgoingEdges.Append(Other.OutgoingEdges);
		Result.OutgoingEdges.Remove(EdgeId);
		Result.NumberOfOutgoingEdges = NumberOfOutgoingEdges + Other.NumberOfOutgoingEdges - 2;
		
		return Result;	
	}

	
	FCWSFlowNode Consume(const FCWSFlowNode& Other, const int32 EdgeId)
	{
		OutgoingEdges.Append(Other.OutgoingEdges);
		OutgoingEdges.Remove(EdgeId);
		NumberOfOutgoingEdges += Other.NumberOfOutgoingEdges - 2;

		return *this;
	}

	void RemoveInternalEdge(const int32 EdgeId, int32 NEdgesToRemove)
	{
		OutgoingEdges.Remove(EdgeId);
		NumberOfOutgoingEdges -= NEdgesToRemove;
	}
};

// FLow calculation structs
struct FCWSFlow
{
	float Amount = 0;

	FCWSFlow& operator+=(const FCWSFlow& Other)
	{
		Amount += Other.Amount;

		return *this;
	}
	
	FCWSFlow operator+(const FCWSFlow& Other) const
	{
		FCWSFlow Result = *this;
		Result.Amount += Other.Amount;

		return Result;
	}

	FCWSFlow operator/(int32 Divider) const
	{
		FCWSFlow Result = *this;
		Result.Amount /= Divider;

		return Result;
	}
	
	FCWSFlow& operator/=(int32 Divider)
	{
		Amount /= Divider;
		
		return *this;
	}
};

struct FCWSFlowDependancy
{
	FGuid SourceNodeId;
	int32 EdgeIndex = INDEX_NONE;
};

enum class ECWSFlowVisitState : uint8
{
	Visited,
	Processing,
	Entrypoint
};

class CWS_API CWSFlowGraph
{
public:
	CWSFlowGraph();
	
	bool AddNode(FGuid NodeId, bool bIsRoot);
	bool AddEdge(FGuid SourceId, FGuid TargetId, float EdgeWeight, ECWSPipeType SourceType, ECWSPipeType TargetType, ECWSPipeDirection
	             SourceDirection, ECWSPipeDirection TargetDirection);
	void Reset();
	int32 GetNodeCount() const;
	int32 GetEdgeCount() const;
	int32 GetOutgoingEdgesCount(const FGuid& NodeId) const;

	bool ConstructFlowGraph();
	bool ComputeFlow();
	bool TraverseNode(
		const FCWSFlowNode* Node,
		TDeque<FGuid>& NextProcessingQueue, TSet<FGuid>& VisitedNodes, TMap<int32, FGuid>& Connections, TArray<int32>&
		EdgeConflicts, const TSet<int32>& UsedEdges
	);
	void FinalizeBuild();

	TArray<FCWSFlow> GetEndPointFlows() const;

	void LogComputedFlows() const;
private:
	static bool CanReceiveFlow(ECWSPipeType Type)
	{
		return (Type == ECWSPipeType::Input) || (Type == ECWSPipeType::Bidirectional);
	}

	static bool CanSendFlow(ECWSPipeType Type)
	{
		return (Type == ECWSPipeType::Output) || (Type == ECWSPipeType::Bidirectional);
	}

	void MergeNodes(
		FGuid Node1Id,
		FGuid Node2Id,
		int32 EdgeId
	);

	void FindEndpoints();
	
	FGuid ResolveCannonicalId(const FGuid& NodeId);
	
	void ResetGraphState();
	void ResetFlowState();
	
	TMap<FGuid, FCWSFlowNode> Nodes;
	TMap<FGuid, FCWSFlowNode> NodesBackup;
	TArray<FCWSFlowEdge> Edges;

	TArray<FGuid> RootNodes;
	TArray<FGuid> EndNodes;

	TMap<FGuid, TArray<FCWSFlowDependancy>> TraversalMap;
	TMap<FGuid, FGuid> CannonicalIds;
	TMap<FGuid, FCWSFlow> NodeFlows;
	TMap<FGuid, int32> ActiveOutgoingEdgeCount;
	
};
