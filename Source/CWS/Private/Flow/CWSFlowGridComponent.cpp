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
	if (Instance->bIsPlaced)
	{
		if (!UnplaceElement(InstanceId))
		{
			return false;
		}
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

	UnplaceElement(InstanceId);
	
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
	if (!Instance->bIsPlaced)
	{
		if (!UnplaceElement(InstanceId))
		{
			return false;
		}
	}
	
	return FlowElementInstances.Remove(InstanceId) > 0;
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






