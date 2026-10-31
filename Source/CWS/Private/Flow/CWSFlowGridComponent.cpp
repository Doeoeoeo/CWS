// Fill out your copyright notice in the Description page of Project Settings.

#include "Flow/CWSFlowGridComponent.h"

UCWSFlowGridComponent::UCWSFlowGridComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

/**
 * Getter for instance ID at a coordinate.
 * @param Cell Coordinate.
 * @param OutInstanceId ID to be set.
 * @return True if valid.
 */
bool UCWSFlowGridComponent::TryGetInstanceAtCell(FIntPoint Cell, FGuid& OutInstanceId) const
{
	const FGuid* InstanceId = OccupiedCells.Find(Cell);
		if (!InstanceId) return false;
	OutInstanceId = *InstanceId;
	return true;
}

/**
 * Getter for instance at a coordinate.
 * @param Cell Coordinate.
 * @param OutInstance Instance to be set.
 * @return True if valid.
 */
bool UCWSFlowGridComponent::TryGetElementInstance(FIntPoint Cell, FCWSFlowElementInstance& OutInstance) const
{
	const FGuid* InstanceId = OccupiedCells.Find(Cell);
		if (!InstanceId) return false;
	const FCWSFlowElementInstance* Instance = FlowElementInstances.Find(*InstanceId);
		if (!Instance) return false;
	OutInstance = *Instance;
	return true;
}

/**
 * Getter for instance belonging to an ID.
 * @param InstanceId ID of the instance.
 * @param OutInstance Instance to be set.
 * @return True if valid.
 */
bool UCWSFlowGridComponent::TryGetInstanceById(FGuid InstanceId, FCWSFlowElementInstance& OutInstance) const
{
	const FCWSFlowElementInstance* Instance = FlowElementInstances.Find(InstanceId);
		if (!Instance) return false;

	OutInstance = *Instance;
	return true;
}

/**
 * Returns the size of the grid.
 * @return Size of the grid.
 */
FIntPoint UCWSFlowGridComponent::GetGridSize() const
{
	return FIntPoint(GridWidth, GridHeight);
}

/**
 * Fills the given array with IDs of instances that are not placed.
 * @param OutInstanceIds Array that is filled with IDS.
 */
void UCWSFlowGridComponent::GetUnplacedElementIds(TArray<FGuid>& OutInstanceIds) const
{
	OutInstanceIds.Reset();

	for (const TPair<FGuid, FCWSFlowElementInstance>& Pair : FlowElementInstances)
	{
		if (!Pair.Value.bIsPlaced)
		{
			OutInstanceIds.Add(Pair.Key);
		}
	}
}

/**
 * Fills the given array with IDs of placed instances.
 * @param OutInstanceIds Array that is filled with IDS.
 */
void UCWSFlowGridComponent::GetPlacedElementIds(TArray<FGuid>& OutInstanceIds) const
{
	OutInstanceIds.Reset();

	for (const TPair<FGuid, FCWSFlowElementInstance>& Pair : FlowElementInstances)
	{
		if (Pair.Value.bIsPlaced)
		{
			OutInstanceIds.Add(Pair.Key);
		}
	}
	
}

/**
 * Public wrapper to get the coordinate covered by a block instance.
 * @param InstanceId ID if the instance.
 * @param GridOrigin Center coordinate.
 * @param RotationSteps Number of 90° rotations.
 * @param OutCells Array to be filled with the covered coordinates.
 * @return True if valid.
 */
bool UCWSFlowGridComponent::GetPlacementCells(
	FGuid InstanceId,
	FIntPoint GridOrigin,
	uint8 RotationSteps,
	TArray<FIntPoint>& OutCells
) const {
	OutCells.Reset();

	const FCWSFlowElementInstance* Instance = FlowElementInstances.Find(InstanceId);

	if (!Instance || !IsValid(Instance->Definition))
	{
		return false;
	}

	GetElementCells(
		Instance->Definition.Get(),
		GridOrigin,
		RotationSteps,
		OutCells
	);
	return true;
}

/**
 * Checks if The coordinate is inside the grid.
 * @param Cell Coordinate to check.
 * @return True if valid.
 */
bool UCWSFlowGridComponent::IsCellInsideGrid(FIntPoint Cell) const
{
	return Cell.X >= 0
		&& Cell.Y >= 0
		&& Cell.X < GridWidth
		&& Cell.Y < GridHeight;
}

/**
 * Checks if cell is occupied by any instance
 * @param Cell Coordinate to be checked
 * @return True if occupied
 */
bool UCWSFlowGridComponent::IsCellOccupied(FIntPoint Cell) const
{
	return OccupiedCells.Contains(Cell);
}

/**
 * Check if cell is occupied by a block other than given.
 * @param Cell Coordinate to be checked.
 * @param InstanceId Owning instance ID
 * @return True if occupied
 */
bool UCWSFlowGridComponent::IsCellOccupiedByOther(FIntPoint Cell, FGuid InstanceId) const
{
	const FGuid* OccupyingInstanceId = OccupiedCells.Find(Cell);
	return OccupyingInstanceId && *OccupyingInstanceId != InstanceId;
}

/**
 * Checks if a placement is valid.
 * @param InstanceId ID of the placed Instance.
 * @param GridOrigin Center coordinate.
 * @param RotationSteps Number of 90° rotations.
 * @return True if placement is valid.
 */
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

/**
 * Place an element instance of the given coordinate at the given rotation.
 * @param InstanceId ID of the placed Instance.
 * @param GridOrigin Center coordinate.
 * @param RotationSteps Number of 90° rotations.
 * @return True if placement succeeds / False if placement is invalid. 
 */
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
		ClearPlacement(*Instance);
	}
	
	Instance->GridOrigin = GridOrigin;
	Instance->RotationSteps = RotationSteps % 4;
	Instance->bIsPlaced = true;
	Instance->PlacedCells = Cells;

		
	for (const FIntPoint& Cell : Cells)
	{
		OccupiedCells.Add(Cell, InstanceId);
	}

	OnElementPlaced.Broadcast(InstanceId);
	OnStructureChanged.Broadcast();
	return true;
}

/**
 * Create block instance.
 * @param Definition Definition of the instance.
 * @param OutInstanceId ID of the instance.
 * @return True if successful.
 */
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

	OnElementCreated.Broadcast(OutInstanceId);
	return true;
}

/**
 * Unplace the block instance for the given ID.
 * @param InstanceId Block ID of the unplaced instance. 
 * @return True if successful.
 */
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
	if (RootInstanceIds.Contains(InstanceId))
	{
		return false;
	}
	
	
	ClearPlacement(*Instance);

	OnElementUnplaced.Broadcast(InstanceId);
	OnStructureChanged.Broadcast();
	
	return true;
}

/**
 * Deletes the block instance for given ID
 * @param InstanceId ID of the instance to be deleted
 * @return True if successful
 */
bool UCWSFlowGridComponent::DeleteElementInstance(FGuid InstanceId)
{
	FCWSFlowElementInstance* Instance = FlowElementInstances.Find(InstanceId);
	if (!Instance)
	{
		return false;
	}
	if (RootInstanceIds.Contains(InstanceId))
	{
		return false;
	}
	
	const bool bWasPlaced = Instance->bIsPlaced;
	if (bWasPlaced)
	{
		ClearPlacement(*Instance);
	}
	
	if (FlowElementInstances.Remove(InstanceId) == 0)
	{
		return false;
	}

	OnElementDeleted.Broadcast(InstanceId);

	if (bWasPlaced)
	{
		OnStructureChanged.Broadcast();
	}

	return true;
}


/**
* Recreates the flow graph. Currently called after every placement change.
*
* @return Returns true if successful
*/
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
		NewGraph.AddNode(Batch.Key, RootInstanceIds.Contains(Batch.Key));
		const TArray<FCWSPipeDefinition>& PipeDefinitions = Instance->Definition->Pipes;
		
		TArray<FCWSResolvedPipe> InstancePipes;

		if (!ResolvePipesForInstance(*Instance, InstancePipes))
		{
			return false;
		}

		for (const FCWSResolvedPipe& ResolvedPipe : InstancePipes)
		{
			const FCWSPipeSocketKey SocketKey {
				ResolvedPipe.GridCell,
				ResolvedPipe.Direction
			};

			if (ResolvedPipes.Contains(SocketKey))
			{
				return false;
			}

			ResolvedPipes.Add(SocketKey, ResolvedPipe);
		}
	}

	
	for (const TPair<FCWSPipeSocketKey, FCWSResolvedPipe>& Batch : ResolvedPipes)
	{
		const FCWSResolvedPipe* Pipe = &Batch.Value;
		// Ignore up and left facing pipes
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
	FlowGraph.FinalizeBuild();
	
	return true;
}

void UCWSFlowGridComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeRoots();
}

/**
 * Orchestration function. Manages the entire flow computation process.
 * 
 * @return true if successful.
 */
bool UCWSFlowGridComponent::RecalculateFlow()
{
	if (!CreateFlowGraph())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Create Graph"));
		return false;
	}

	if (!FlowGraph.ConstructFlowGraph())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Construct Graph"));
		return false;
	}

	if (!FlowGraph.ComputeFlow())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Compute Flow"));
		return false;
	}

	FlowGraph.LogComputedFlows();

	return true;
}

/**
 * Generates the ResolvedPipe structure for each outgoing flow pipe from an instance.
 * 
 * @param Instance Instance to be processed.
 * @param OutResolvedPipes Empty array to be filled.
 * @return true if successful
 */
bool UCWSFlowGridComponent::ResolvePipesForInstance(
	const FCWSFlowElementInstance& Instance,
	TArray<FCWSResolvedPipe>& OutResolvedPipes
) const
{
	OutResolvedPipes.Reset();

	if (!IsValid(Instance.Definition))
	{
		return false;
	}

	const TArray<FCWSPipeDefinition>& PipeDefinitions = Instance.Definition->Pipes;

	OutResolvedPipes.Reserve(PipeDefinitions.Num());
	for (int32 PipeIndex = 0; PipeIndex < PipeDefinitions.Num(); PipeIndex++)
	{
		const FCWSPipeDefinition& Pipe = PipeDefinitions[PipeIndex];
		FCWSResolvedPipe& ResolvedPipe = OutResolvedPipes.AddDefaulted_GetRef();

		ResolvedPipe.InstanceId = Instance.InstanceId;
		ResolvedPipe.PipeIndex = PipeIndex;
		ResolvedPipe.GridCell =
			Instance.GridOrigin +
			RotateLocalCoordinate(
				Pipe.LocalCoordinate,
				Instance.RotationSteps
			);

		ResolvedPipe.Direction = RotatePipeDirection(
			Pipe.Direction,
			Instance.RotationSteps
		);

		ResolvedPipe.Type = Pipe.Type;
		ResolvedPipe.OutputWeight = Pipe.OutputWeight;
	}
	
	return true;
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

/**
 * Recomputes the coordinate after n-90° rotations.
 * 
 * @param Coordinate Coordinate to rotate.
 * @param RotationSteps Number of 90° rotations.
 * @return The rotated coordinate.
 */
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

/**
 * Computes the cells occupied by a block on a given center coordinate after rotation.
 * 
 * @param Definition The definition of the block.
 * @param GridOrigin The center coordinate of the placement.
 * @param RotationSteps Number of 90° rotations.
 * @param OutCells An array where computed occupied cells are stored.
 */
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


/**
 * Checks if given coordinates are valid for placement (empty or occupied by this block).
 * 
 * @param Cells Coordinates to be checked.
 * @param InstanceId ID of the block being placed.
 * @return True if placement is valid / False if not.
 */
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

/**
 * Rotates the Direction structure by n-90° rotations.
 * 
 * @param Direction The direction to be rotated 
 * @param RotationSteps Number of 90° rotations.
 * @return The rotated direction.
 */
ECWSPipeDirection UCWSFlowGridComponent::RotatePipeDirection(
	ECWSPipeDirection Direction,
	int32 RotationSteps
) const {
	const uint8 DirectionValue = static_cast<uint8>(Direction);
	return static_cast<ECWSPipeDirection>((DirectionValue + RotationSteps) % 4); 
}

/**
 * Computes the opposite direction
 * @param Direction The direction to be reversed.
 * @return The opposite rotation.
 */
ECWSPipeDirection UCWSFlowGridComponent::GetOppositeDirection(ECWSPipeDirection Direction) const
{
	return RotatePipeDirection(Direction, 2);
}

/**
 * Returns an increment coordinate in the given direction. 
 * @param Direction The direction to increment.
 * @return An incremented coordinate.
 */
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

/**
 * Clears the cells occupied by an instance.
 * @param Instance Instance to clear.
 */
void UCWSFlowGridComponent::ClearPlacement(FCWSFlowElementInstance& Instance)
{
	for (const FIntPoint& Cell : Instance.PlacedCells)
	{
		OccupiedCells.Remove(Cell);
	}

	Instance.bIsPlaced = false;
	Instance.PlacedCells.Reset();
}

/**
 * Placed a root node and stores it as a root.
 * @param Definition Block definition
 * @param GridCell Center coordinate of the placement
 * @param RotationSteps Number of 90° rotations.
 * @param RootId ID of the block
 * @return True if successful
 */
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

/**
 * Creates all roots
 */
void UCWSFlowGridComponent::InitializeRoots()
{
	FGuid RootId;
	AddRoot(
		RootDefinition,
		RootGridCell,
		0,
		RootId
	);
	UE_LOG(LogTemp, Warning, TEXT("Flow Check %s"), *RootId.ToString());

}







