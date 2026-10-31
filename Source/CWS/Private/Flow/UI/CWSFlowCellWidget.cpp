// Fill out your copyright notice in the Description page of Project Settings.


#include "Flow/UI/CWSFlowCellWidget.h"

#include "Components/Button.h"

void UCWSFlowCellWidget::InitializeCell(FIntPoint InGridCoordinate)
{
	GridCoordinate = InGridCoordinate;
}

void UCWSFlowCellWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(CellButton))
	{
		CellButton->OnClicked.AddUniqueDynamic(
			this,
			&ThisClass::HandleClicked
		);

		CellButton->OnHovered.AddUniqueDynamic(
			this,
			&ThisClass::HandleHovered
		);

		CellButton->OnUnhovered.AddUniqueDynamic(
			this,
			&ThisClass::HandleUnhovered
		);
	}
}

void UCWSFlowCellWidget::NativeDestruct()
{
	if (IsValid(CellButton))
	{
		CellButton->OnClicked.RemoveDynamic(
			this,
			&ThisClass::HandleClicked
		);

		CellButton->OnHovered.RemoveDynamic(
			this,
			&ThisClass::HandleHovered
		);

		CellButton->OnUnhovered.RemoveDynamic(
			this,
			&ThisClass::HandleUnhovered
		);
	}
	
	Super::NativeDestruct();
}


void UCWSFlowCellWidget::HandleClicked()
{
	OnCellClicked.Broadcast(GridCoordinate);
}

void UCWSFlowCellWidget::HandleHovered()
{
	OnCellHovered.Broadcast(GridCoordinate);
}

void UCWSFlowCellWidget::HandleUnhovered()
{
	OnCellUnhovered.Broadcast(GridCoordinate);
}
