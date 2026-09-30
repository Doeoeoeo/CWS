// Fill out your copyright notice in the Description page of Project Settings.


#include "Flow/CWSFlowGridComponent.h"

UCWSFlowGridComponent::UCWSFlowGridComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCWSFlowGridComponent::IsCellInsideGrid(FIntPoint Cell) const
{
	return Cell.X >= 0
		&& Cell.Y >= 0
		&& Cell.X < GridWidth
		&& Cell.Y < GridHeight;
}

bool UCWSFlowGridComponent::IsCellOccupied(FIntPoint Cell) const
{
	return OccupiedCells.Contains(Cell);
}

bool UCWSFlowGridComponent::IsCellOccupiedByOther(FIntPoint Cell, FGuid InstanceId) const
{
	const FGuid* OccupyingInstanceId = OccupiedCells.Find(Cell);
	return OccupyingInstanceId && *OccupyingInstanceId != InstanceId;
}

bool UCWSFlowGridComponent::CanPlaceElement(
	FGuid InstanceId,
	FIntPoint GridOrigin,
	uint8 RotationSteps
) const {
	const FCWSFlowElementInstance* Instance = FlowElementInstances.Find(InstanceId);
	if (!Instance)
	{
		return false;
	}
	
	TArray<FIntPoint> Cells;
	GetElementCells(Instance->Definition.Get(), GridOrigin, RotationSteps, Cells);
	return CanPlaceOnCells(Cells, InstanceId);
}

bool UCWSFlowGridComponent::PlaceElement(
	FGuid InstanceId,
	FIntPoint GridOrigin,
	uint8 RotationSteps
) {
	FCWSFlowElementInstance* Instance = FlowElementInstances.Find(InstanceId);
	if (!Instance)
	{
		return false;
	}

	
	TArray<FIntPoint> Cells;
	GetElementCells(
		Instance->Definition.Get(),
		GridOrigin,
		RotationSteps,
		Cells
	);
	if (!CanPlaceOnCells(Cells, InstanceId))
	{
		return false;
	}
	if (Instance->bIsPlaced)
	{
		if (!UnplaceElement(InstanceId))
		{
			return false;
		}
	}
	
	Instance->GridOrigin = GridOrigin;
	Instance->RotationSteps = RotationSteps % 4;
	Instance->bIsPlaced = true;
	Instance->PlacedCells = Cells;

		
	for (const FIntPoint& Cell : Cells)
	{
		OccupiedCells.Add(Cell, InstanceId);
	}
	
	return true;
}

bool UCWSFlowGridComponent::CreateElementInstance(
	UCWSFlowElementDefinition* Definition,
	FGuid& OutInstanceId
) {
	OutInstanceId = FGuid();
	if (!IsValid(Definition) || Definition->FootprintCells.IsEmpty())
	{
		return false;
	}

	OutInstanceId = FGuid::NewGuid();
	FCWSFlowElementInstance PlacedElement;
	PlacedElement.Definition = Definition;
	PlacedElement.InstanceId = OutInstanceId;
	PlacedElement.bIsPlaced = false;

	FlowElementInstances.Add(OutInstanceId, PlacedElement);

	return true;
}

bool UCWSFlowGridComponent::UnplaceElement(FGuid InstanceId)
{
	FCWSFlowElementInstance* Instance = FlowElementInstances.Find(InstanceId);
	if (!Instance) {
		return false;
	}
	if (!Instance->bIsPlaced)
	{
		return false;
	}

	for (const FIntPoint& Cell : Instance->PlacedCells)
	{
		OccupiedCells.Remove(Cell);
	}
	Instance->bIsPlaced = false;
	Instance->PlacedCells.Reset();
	
	return true;
}

bool UCWSFlowGridComponent::DeleteElementInstance(FGuid InstanceId)
{
	const FCWSFlowElementInstance* Instance = FlowElementInstances.Find(InstanceId);
	if (!Instance)
	{
		return false;
	}
	if (Instance->bIsPlaced)
	{
		if (!UnplaceElement(InstanceId))
		{
			return false;
		}
	}
	
	return FlowElementInstances.Remove(InstanceId) > 0;
}

bool UCWSFlowGridComponent::CreateFlowGraph()
{

	CWSFlowGraph NewGraph;
	TMap<FCWSPipeSocketKey, FCWSResolvedPipe> ResolvedPipes;
	
	for (const TPair<FGuid, FCWSFlowElementInstance>& Batch : FlowElementInstances)
	{
		const FCWSFlowElementInstance* Instance = &Batch.Value;
		if (!Instance->bIsPlaced || !IsValid(Instance->Definition.Get()))
		{
			continue;
		}
		NewGraph.AddNode(Batch.Key);
		UE_LOG(LogTemp, Warning, TEXT("Made a node"));
		const TArray<FCWSPipeDefinition>& PipeDefinitions = Instance->Definition->Pipes;
		
		for (int32 PipeIndex = 0; PipeIndex < Instance->Definition->Pipes.Num(); PipeIndex++)
		{
			const FCWSPipeDefinition& Pipe = PipeDefinitions[PipeIndex];
			const FIntPoint PipeCell =
				Instance->GridOrigin +
				RotateLocalCoordinate(
					Pipe.LocalCoordinate,
					Instance->RotationSteps
			);
			
			const ECWSPipeDirection PipeDirection = RotatePipeDirection(
				Pipe.Direction,
				Instance->RotationSteps
			);
			FCWSPipeSocketKey SocketKey = FCWSPipeSocketKey{
				PipeCell,
				PipeDirection
			};
			
			if (ResolvedPipes.Contains(SocketKey))
			{
				return false;
			}
			FCWSResolvedPipe ResolvedPipe;
			ResolvedPipe.InstanceId = Instance->InstanceId;
			ResolvedPipe.GridCell = PipeCell;
			ResolvedPipe.Direction = PipeDirection;
			ResolvedPipe.Type = Pipe.Type;
			ResolvedPipe.PipeIndex = PipeIndex;
			ResolvedPipe.OutputWeight = Pipe.OutputWeight;

			ResolvedPipes.Add(
				SocketKey,
				ResolvedPipe
			);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Starting on Pipes"));
	
	for (const TPair<FCWSPipeSocketKey, FCWSResolvedPipe>& Batch : ResolvedPipes)
	{
		const FCWSResolvedPipe* Pipe = &Batch.Value;
		if (Pipe->Direction == ECWSPipeDirection::Up || Pipe->Direction == ECWSPipeDirection::Left)
		{
			continue;
		}

		const FIntPoint NeighbourCell = Pipe->GridCell + GetDirectionOffset(Pipe->Direction);

		const FCWSPipeSocketKey NeighbourSocketKey = FCWSPipeSocketKey{
			NeighbourCell,
			GetOppositeDirection(Pipe->Direction)
		};
		FCWSResolvedPipe* NeighbourPipe = ResolvedPipes.Find(NeighbourSocketKey);
		if (!NeighbourPipe || NeighbourPipe->InstanceId == Pipe->InstanceId)
		{
			continue;
		}
		UE_LOG(LogTemp, Warning, TEXT("Trying to add Pipe "));		
		NewGraph.AddEdge(
			Pipe->InstanceId,
			NeighbourPipe->InstanceId,
			Pipe->OutputWeight * NeighbourPipe->OutputWeight,
			Pipe->Type,
			NeighbourPipe->Type,
			Pipe->Direction,
			NeighbourPipe->Direction
		);
	}

	FlowGraph = MoveTemp(NewGraph);
	
	return true;
}

void UCWSFlowGridComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeRoots();
}

void UCWSFlowGridComponent::PrintGraphDebug() const
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Flow graph: Nodes=%d Edges=%d"),
		FlowGraph.GetNodeCount(),
		FlowGraph.GetEdgeCount()
	);
}

FIntPoint UCWSFlowGridComponent::RotateLocalCoordinate(
	FIntPoint Coordinate,
	uint8 RotationSteps
) {
	switch (RotationSteps % 4)
	{
		case 0:
			return Coordinate;
		case 1:
			return FIntPoint(-Coordinate.Y, Coordinate.X);
		case 2:
			return FIntPoint(-Coordinate.X, -Coordinate.Y);
		case 3:
			return FIntPoint(Coordinate.Y, -Coordinate.X);
		default:
			return Coordinate;
	}
}

void UCWSFlowGridComponent::GetElementCells (
	const UCWSFlowElementDefinition* Definition,
	FIntPoint GridOrigin,
	uint8 RotationSteps,
	TArray<FIntPoint>& OutCells
)
{
	OutCells.Reset();
	for (const FIntPoint& Cell : Definition->FootprintCells)
	{
		OutCells.Add(GridOrigin + RotateLocalCoordinate(Cell, RotationSteps));
	}
}

bool UCWSFlowGridComponent::CanPlaceOnCells (
	const TArray<FIntPoint>& Cells,
	FGuid InstanceId
) const {
	for (const FIntPoint& Cell : Cells)
	{
		if (!IsCellInsideGrid(Cell) || IsCellOccupiedByOther(Cell, InstanceId))
		{
			return false;
		}
	}

	return true;
}

ECWSPipeDirection UCWSFlowGridComponent::RotatePipeDirection(
	ECWSPipeDirection Direction,
	int32 RotationSteps
) const {
	const uint8 DirectionValue = static_cast<uint8>(Direction);
	return static_cast<ECWSPipeDirection>((DirectionValue + RotationSteps) % 4); 
}

ECWSPipeDirection UCWSFlowGridComponent::GetOppositeDirection(ECWSPipeDirection Direction) const
{
	return RotatePipeDirection(Direction, 2);
}

FIntPoint UCWSFlowGridComponent::GetDirectionOffset(ECWSPipeDirection Direction) const
{
	switch (static_cast<uint8>(Direction))
	{
		case 0:
			return FIntPoint(0, -1);
		case 1:
			return FIntPoint(1, 0);
		case 2:
			return FIntPoint(0, 1);
		case 3:
			return FIntPoint(-1, 0);
		default:
			return FIntPoint(0, 0);
	}
}

bool UCWSFlowGridComponent::AddRoot(
	UCWSFlowElementDefinition* Definition,
	FIntPoint GridCell,
	uint8 RotationSteps,
	FGuid& RootId
) {
	RootId = FGuid();

	if (!CreateElementInstance(Definition, RootId))
	{
		return false;
	}

	if (!PlaceElement(RootId, GridCell, RotationSteps))
	{
		return false;
	}

	RootInstanceIds.Add(RootId);
	return true;
}

void UCWSFlowGridComponent::InitializeRoots()
{
	FGuid RootId;
	AddRoot(
		RootDefinition,
		RootGridCell,
		0,
		RootId
	);
}







