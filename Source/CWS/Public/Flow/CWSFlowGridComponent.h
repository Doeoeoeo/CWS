// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CWSFlowElementInstance.h"
#include "Components/ActorComponent.h"
#include "CWSFlowGraph.h"
#include "CWSFlowGridComponent.generated.h"


class UCWSFlowElementDefinition;


struct FCWSPipeSocketKey
{
	FIntPoint Cell;
	ECWSPipeDirection Direction;

	bool operator==(const FCWSPipeSocketKey& Other) const
	{
		return Cell == Other.Cell && Direction == Other.Direction;
	}

	friend uint32 GetTypeHash(const FCWSPipeSocketKey& Key)
	{
		return HashCombine(GetTypeHash(Key.Cell), GetTypeHash(static_cast<uint8>(Key.Direction)));
	}
};

struct FCWSResolvedPipe
{
	FGuid InstanceId;
	int32 PipeIndex = INDEX_NONE;

	FIntPoint GridCell = FIntPoint::ZeroValue;

	ECWSPipeDirection Direction = ECWSPipeDirection::Down;
	ECWSPipeType Type = ECWSPipeType::Closed;

	float OutputWeight = 1.0f;
};




UCLASS(ClassGroup = (CWS), meta = (BlueprintSpawnableComponent))
class CWS_API UCWSFlowGridComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	UCWSFlowGridComponent();

	UFUNCTION(BlueprintPure, Category = "Flow Grid")
	bool IsCellInsideGrid(FIntPoint Cell) const;

	UFUNCTION(BlueprintPure, Category = "Flow Grid")
	bool IsCellOccupied(FIntPoint Cell) const;

	UFUNCTION(BlueprintPure, Category = "Flow Grid")
	bool IsCellOccupiedByOther(FIntPoint Cell, FGuid InstanceId) const;
	
	UFUNCTION(BlueprintPure, Category = "Flow Grid")
	bool CanPlaceElement(
		FGuid InstanceId,
		FIntPoint GridOrigin, uint8 RotationSteps
	) const;

	UFUNCTION(BlueprintCallable, Category = "Flow Grid")
	bool PlaceElement(
		FGuid InstanceId,
		FIntPoint GridOrigin,
		uint8 RotationSteps
	);

	UFUNCTION(BlueprintCallable, Category = "Flow Grid")
	bool CreateElementInstance(
		UCWSFlowElementDefinition* Definition,
		FGuid& OutInstanceId
	);
	
	UFUNCTION(BlueprintCallable, Category = "Flow Grid")
	bool UnplaceElement(FGuid InstanceId);

	UFUNCTION(BlueprintCallable, Category = "Flow Grid")
	bool DeleteElementInstance(FGuid InstanceId);

	UFUNCTION(BlueprintCallable, Category = "Flow Grid")
	bool CreateFlowGraph();

	virtual void BeginPlay() override;

	//Debugg
	UFUNCTION(BlueprintCallable, Category = "Flow Grid|Debug")
	void PrintGraphDebug() const;

private:
	static FIntPoint RotateLocalCoordinate(
		FIntPoint Coordinate,
		uint8 RotationSteps
	);

	static void GetElementCells(
		const UCWSFlowElementDefinition* Definition,
		FIntPoint GridOrigin,
		uint8 RotationSteps,
		TArray<FIntPoint>& OutCells
	);

	bool CanPlaceOnCells(
		const TArray<FIntPoint>& Cells,
		FGuid InstanceId
	) const;

	FIntPoint FindNeighbourCell(
		FGuid InstanceId,
		FIntPoint Cell,
		ECWSPipeDirection Direction
	) const;

	ECWSPipeDirection RotatePipeDirection(
		ECWSPipeDirection Direction,
		int32 RotationSteps
	) const;

	ECWSPipeDirection GetOppositeDirection(ECWSPipeDirection Direction) const;

	FIntPoint GetDirectionOffset(ECWSPipeDirection Direction) const;
	
	UPROPERTY(
		EditAnywhere,
		Category = "Flow Grid",
		meta = (ClampMin = "1")
	)
	int32 GridWidth = 8;

	UPROPERTY(
		EditAnywhere,
		Category = "Flow Grid",
		meta = (ClampMin = "1")
	)
	int32 GridHeight = 8;

	UPROPERTY(Transient)
	TMap<FGuid, FCWSFlowElementInstance> FlowElementInstances;

	UPROPERTY(Transient)
	TMap<FIntPoint, FGuid> OccupiedCells;

	CWSFlowGraph FlowGraph;

	// Root
	UPROPERTY(EditDefaultsOnly, Category = "Flow Grid|Root")
	TObjectPtr<UCWSFlowElementDefinition> RootDefinition = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Flow Grid|Root")
	FIntPoint RootGridCell = FIntPoint(5, 0);

	FGuid RootInstanceId;
	TArray<FGuid> RootInstanceIds;

	bool AddRoot(
		UCWSFlowElementDefinition* Definition,
		FIntPoint GridCell,
		uint8 RotationSteps,
		FGuid& RootId
	);
	
	void InitializeRoots();
};
