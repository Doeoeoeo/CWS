// Fill out your copyright notice in the Description page of Project Settings.


#include "Flow/UI/CWSFlowElementWidget.h"

void UCWSFlowElementWidget::SetElementInstance(
	const FCWSFlowElementInstance& InElementInstance,
	const TArray<FCWSResolvedPipe>& InResolvedPipes,
	float InCellSize
)
{
	ElementInstance = InElementInstance;
	ResolvedPipes = InResolvedPipes;
	DisplayCellSize = InCellSize;

	if (!ElementInstance.PlacedCells.IsEmpty())
	{
		DisplayMinCell = ElementInstance.PlacedCells[0];

		for (const FIntPoint& Cell : ElementInstance.PlacedCells)
		{
			DisplayMinCell.X = FMath::Min(DisplayMinCell.X, Cell.X);
			DisplayMinCell.Y = FMath::Min(DisplayMinCell.Y, Cell.Y);
		}
	}
	else
	{
		DisplayMinCell = FIntPoint::ZeroValue;
	}
	
	RefreshElementVisuals();
}
