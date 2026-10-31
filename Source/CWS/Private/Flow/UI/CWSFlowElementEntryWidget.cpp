// Fill out your copyright notice in the Description page of Project Settings.


#include "Flow/UI/CWSFlowElementEntryWidget.h"

#include "Components/Button.h"

void UCWSFlowElementEntryWidget::InitializeEntry(
	FGuid InInstanceId,
	UCWSFlowElementDefinition* InDefinition
) {
	InstanceId = InInstanceId;
	Definition = InDefinition;

	RefreshEntryVisuals();
}

void UCWSFlowElementEntryWidget::SetSelected(bool bNewSelected)
{
	if (bIsSelected == bNewSelected)
	{
		return;
	}

	bIsSelected = bNewSelected;
	RefreshEntryVisuals();
}

FGuid UCWSFlowElementEntryWidget::GetInstanceId() const
{
	return InstanceId;
}

void UCWSFlowElementEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(SelectButton))
	{
		SelectButton->OnClicked.AddUniqueDynamic(
			this,
			&ThisClass::HandleSelected
		);
	}
}

void UCWSFlowElementEntryWidget::NativeDestruct()
{
	if (IsValid(SelectButton))
	{
		SelectButton->OnClicked.RemoveDynamic(
			this,
			&ThisClass::HandleSelected
		);
	}
	
	Super::NativeDestruct();
}

void UCWSFlowElementEntryWidget::HandleSelected()
{
	if (InstanceId.IsValid())
	{
		OnSelected.Broadcast(InstanceId);
	}
}
