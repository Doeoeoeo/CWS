// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CWSFlowElementInstance.h"
#include "Components/ActorComponent.h"
#include "CWSFlowGridComponent.generated.h"

class UCWSFlowElementDefinition;

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

};
