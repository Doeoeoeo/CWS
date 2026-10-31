// Fill out your copyright notice in the Description page of Project Settings.


#include "Flow/UI/CWSFlowBoardWidget.h"

#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Flow/CWSFlowGridComponent.h"
#include "Flow/UI/CWSFlowElementEntryWidget.h"
#include "Flow/UI/CWSFlowElementWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

bool UCWSFlowBoardWidget::InitializeBoard(UCWSFlowGridComponent* InFlowGrid)
{
	// Unbind existing grid if already defined
	UnbindGridDelegates();
	
	FlowGrid = InFlowGrid;

	if (!IsValid(FlowGrid) ||
		!IsValid(GridPanel) ||
		!IsValid(GridSizeBox) ||
		!IsValid(PalettePanel) ||
		!IsValid(ElementCanvas) || 
		!CellWidgetClass ||
		!ElementEntryWidgetClass ||
		!ElementWidgetClass
	) {
		return false;	
	}

	BindGridDelegates();
	
	if (!BuildGrid())
	{
		UnbindGridDelegates();
		return false;
	}

	RefreshGrid();
	RefreshPalette();
	RefreshPlacedElements();
	
	return true;
}

bool UCWSFlowBoardWidget::SelectElement(FGuid InstanceId)
{
	if (!IsValid(FlowGrid) ||
		!InstanceId.IsValid()
	)
	{
		return false;
	}

	FCWSFlowElementInstance Instance;

	if (!FlowGrid->TryGetInstanceById(InstanceId, Instance) ||
		Instance.bIsPlaced
	)
	{
		return false;
	}
	if (SelectedInstanceId == InstanceId)
	{
		HidePaletteEntry(InstanceId);
		UpdatePlacementPreview();
		return true;
	}

	// If a new element is selected, return the old one to the palette and hide
	// the newly selected element from the palette
	const FGuid PreviousSelectionId = SelectedInstanceId;
	SelectedInstanceId = InstanceId;
	PendingRotationSteps = Instance.RotationSteps;
	if (PreviousSelectionId.IsValid())
	{
		ShowPaletteEntry(PreviousSelectionId);
	}

	HidePaletteEntry(SelectedInstanceId);

	RefreshPaletteSelection();
	UpdatePlacementPreview();
	return true;
}

void UCWSFlowBoardWidget::ClearSelection()
{
	const FGuid PreviousSelectionId = SelectedInstanceId;
	
	SelectedInstanceId.Invalidate();
	PendingRotationSteps = 0;

	if (PreviousSelectionId.IsValid())
	{
		ShowPaletteEntry(PreviousSelectionId);
	}
	
	RefreshGrid();
	RefreshPaletteSelection();
}

void UCWSFlowBoardWidget::RotateSelectionClockwise()
{
	RotateSelectionBy(1);
}

void UCWSFlowBoardWidget::NativeDestruct()
{
	UnbindGridDelegates();
	UnbindCellDelegates();
	UnbindEntryDelegates();
	
	Super::NativeDestruct();
}

FReply UCWSFlowBoardWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton &&
		SelectedInstanceId.IsValid()
	)
	{
		ClearSelection();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UCWSFlowBoardWidget::NativeOnMouseWheel(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent
)
{
	if (!SelectedInstanceId.IsValid())
	{
		return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
	}

	const float WheelDelta = InMouseEvent.GetWheelDelta();

	if (WheelDelta > 0.0f)
	{
		RotateSelectionBy(1);
		return FReply::Handled();
	}

	if (WheelDelta < 0.0f)
	{
		RotateSelectionBy(-1);
		return FReply::Handled();
	}

	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

FReply UCWSFlowBoardWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent
)
{
	if (InKeyEvent.GetKey() == EKeys::R &&
		SelectedInstanceId.IsValid()
	)
	{
		RotateSelectionBy(1);
		return FReply::Handled();
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UCWSFlowBoardWidget::UnbindCellDelegates()
{
	for (UCWSFlowCellWidget* CellWidget : CellWidgets)
	{
		if (!IsValid(CellWidget)) continue;

		CellWidget->OnCellClicked.RemoveDynamic(
			this,
			&ThisClass::HandleCellClicked
		);

		CellWidget->OnCellHovered.RemoveDynamic(
			this,
			&ThisClass::HandleCellHovered
		);

		CellWidget->OnCellUnhovered.RemoveDynamic(
			this,
			&ThisClass::HandleCellUnhovered
		);
	}
}

bool UCWSFlowBoardWidget::BuildGrid()
{
	UnbindCellDelegates();
	
	GridPanel->ClearChildren();
	CellWidgets.Reset();

	CachedGridSize = FlowGrid->GetGridSize();
	GridSizeBox->SetWidthOverride(CachedGridSize.X * CellSize);
	GridSizeBox->SetHeightOverride(CachedGridSize.Y * CellSize);
	GridPanel->SetMinDesiredSlotWidth(CellSize);
	GridPanel->SetMinDesiredSlotHeight(CellSize);
	
	CellWidgets.Reserve(CachedGridSize.X * CachedGridSize.Y);

	for (int32 Y = 0; Y < CachedGridSize.Y; ++Y)
	{
		for (int32 X = 0; X < CachedGridSize.X; ++X)
		{
			UCWSFlowCellWidget* CellWidget =
				CreateWidget<UCWSFlowCellWidget>(
					GetOwningPlayer(),
					CellWidgetClass
				);
				if (!IsValid(CellWidget)) return false;

			const FIntPoint Coordinate(X, Y);
			CellWidget->InitializeCell(Coordinate);
			
			CellWidget->OnCellClicked.AddDynamic(
				this,
				&ThisClass::HandleCellClicked
			);

			CellWidget->OnCellHovered.AddDynamic(
				this,
				&ThisClass::HandleCellHovered
			);

			CellWidget->OnCellUnhovered.AddDynamic(
				this,
				&ThisClass::HandleCellUnhovered
			);

			GridPanel->AddChildToUniformGrid(
				CellWidget,
				Y,
				X
			);

			CellWidgets.Add(CellWidget);
		}
	}
	
	return true;	
}

void UCWSFlowBoardWidget::RefreshGrid()
{
	if (!IsValid(FlowGrid))
	{
		return;
	}

	for (int32 X = 0; X < CachedGridSize.X; ++X)
	{
		for (int32 Y = 0; Y < CachedGridSize.Y; ++Y)
		{
			FGuid InstanceId;
			FIntPoint CellCoordinate = FIntPoint(X, Y);

			const ECWSFlowCellVisualState State =
				FlowGrid->TryGetInstanceAtCell(CellCoordinate, InstanceId)
				? ECWSFlowCellVisualState::Occupied
				: ECWSFlowCellVisualState::Empty;

			UCWSFlowCellWidget* CellWidget = GetCellWidget(CellCoordinate);
				if (!CellWidget) return;
			
			CellWidget->SetVisualState(State);
		}
	}
}

void UCWSFlowBoardWidget::HandleCellClicked(FIntPoint GridCoordinate)
{
	if (!IsValid(FlowGrid)
	) {
		return;	
	}

	// If we are holding an element place it
	if (SelectedInstanceId.IsValid())
	{
		FlowGrid->PlaceElement(
			SelectedInstanceId,
			GridCoordinate,
			PendingRotationSteps
		);

		return;
	}

	// We are not holding anything. Try to pick up the clicked element
	FGuid ClickedInstanceId;

	if (!FlowGrid->TryGetInstanceAtCell(
		GridCoordinate,
		ClickedInstanceId
	))
	{
		return;
	}

	if (FlowGrid->UnplaceElement(ClickedInstanceId))
	{
		SelectElement(ClickedInstanceId);
	}
	
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Cell clicked: (%d, %d)"),
		GridCoordinate.X,
		GridCoordinate.Y
	);
}

void UCWSFlowBoardWidget::HandleCellHovered(FIntPoint GridCoordinate)
{
	HoveredCell = GridCoordinate;
	bHasHoveredCell = true;
	
	UE_LOG(
		LogTemp,
		Verbose,
		TEXT("Cell hovered: (%d, %d)"),
		GridCoordinate.X,
		GridCoordinate.Y
	);
	UpdatePlacementPreview();
}

void UCWSFlowBoardWidget::HandleCellUnhovered(FIntPoint GridCoordinate)
{
	if (bHasHoveredCell && HoveredCell == GridCoordinate)
	{
		bHasHoveredCell = false;
		RefreshGrid();
	}
	UE_LOG(
		LogTemp,
		Verbose,
		TEXT("Cell unhovered: (%d, %d)"),
		GridCoordinate.X,
		GridCoordinate.Y
	);
}

int32 UCWSFlowBoardWidget::CoordinateToIndex(FIntPoint GridCoordinate) const
{
	return GridCoordinate.Y * CachedGridSize.X + GridCoordinate.X;
}

UCWSFlowCellWidget* UCWSFlowBoardWidget::GetCellWidget(FIntPoint GridCoordinate) const
{
	if (GridCoordinate.X < 0 ||
		GridCoordinate.Y < 0 ||
		GridCoordinate.X >= CachedGridSize.X ||
		GridCoordinate.Y >= CachedGridSize.Y
	)
	{
		return nullptr;
	}

	const int32 Index = CoordinateToIndex(GridCoordinate);
	
	return CellWidgets.IsValidIndex(Index) ? CellWidgets[Index] : nullptr;
}

FIntPoint UCWSFlowBoardWidget::GetCellCoordinate(int32 Index) const
{
	return FIntPoint(Index % CachedGridSize.X, Index / CachedGridSize.X);
}

void UCWSFlowBoardWidget::UpdatePlacementPreview()
{
	RefreshGrid();

	if (!IsValid(FlowGrid) ||
		!SelectedInstanceId.IsValid() ||
		!bHasHoveredCell
	) {
		return;	
	}

	TArray<FIntPoint> PreviewCells;

	if (!FlowGrid->GetPlacementCells(
		SelectedInstanceId,
		HoveredCell,
		PendingRotationSteps,
		PreviewCells
	)) {
		return;
	}

	const bool bCanPlace = FlowGrid->CanPlaceElement(
		SelectedInstanceId,
		HoveredCell,
		PendingRotationSteps
	);

	const ECWSFlowCellVisualState PreviewState =
		bCanPlace
		? ECWSFlowCellVisualState::ValidPreview
		: ECWSFlowCellVisualState::InvalidPreview;

	for (const FIntPoint& Cell : PreviewCells)
	{
		if (UCWSFlowCellWidget* CellWidget = GetCellWidget(Cell))
		{
			CellWidget->SetVisualState(PreviewState);
		}
	}

	ShowHoveredElement(SelectedInstanceId, PreviewCells);
}

void UCWSFlowBoardWidget::RefreshPalette()
{
	if (!IsValid(FlowGrid) ||
		!IsValid(PalettePanel) ||
		!ElementEntryWidgetClass
	)
	{
		return;
	}

	UnbindEntryDelegates();
	PalettePanel->ClearChildren();
	ElementEntryWidgets.Reset();

	TArray<FGuid> UnplacedIds;
	FlowGrid->GetUnplacedElementIds(UnplacedIds);
	
	for (const FGuid& InstanceId : UnplacedIds)
	{
		ShowPaletteEntry(InstanceId);
	}

	RefreshPaletteSelection();
}

void UCWSFlowBoardWidget::RefreshPaletteSelection()
{
	for (const TPair<FGuid, TObjectPtr<UCWSFlowElementEntryWidget>>& Pair: ElementEntryWidgets)
	{
		if (UCWSFlowElementEntryWidget* Entry = Pair.Value.Get())
		{
			Entry->SetSelected(Entry->GetInstanceId() == SelectedInstanceId);
		}
	}
}

bool UCWSFlowBoardWidget::ShowPaletteEntry(FGuid InstanceId)
{
	if (!IsValid(FlowGrid) ||
		!IsValid(PalettePanel)
	)
	{
		return false;		
	}
	FCWSFlowElementInstance Instance;
	if (!FlowGrid->TryGetInstanceById(InstanceId,Instance) ||
		Instance.bIsPlaced ||
		!IsValid(Instance.Definition)
	)
	{
		return false;
	}
	
	// If the widget exists, show it if not already shown
	if (TObjectPtr<UCWSFlowElementEntryWidget>* WidgetPtr = ElementEntryWidgets.Find(InstanceId))
	{
		UCWSFlowElementEntryWidget* EntryWidget = WidgetPtr->Get();

		if (IsValid(EntryWidget))
		{
			// Check if the widget is already shown in the palette
			if (!EntryWidget->GetParent())
			{
				PalettePanel->AddChild(EntryWidget);
			}
			
			EntryWidget->SetVisibility(ESlateVisibility::Visible);
			EntryWidget->SetSelected(InstanceId == SelectedInstanceId);
			return true;
		}

		ElementEntryWidgets.Remove(InstanceId);
	}

	// The widget doesn't exist, create it.
	UCWSFlowElementEntryWidget* EntryWidget =
		CreateWidget<UCWSFlowElementEntryWidget>(
			GetOwningPlayer(),
			ElementEntryWidgetClass
		);
		if (!IsValid(EntryWidget)) return false;

	EntryWidget->InitializeEntry(
		InstanceId,
		Instance.Definition.Get()
	);

	EntryWidget->OnSelected.AddUniqueDynamic(
		this,
		&ThisClass::HandleEntrySelected
	);

	ElementEntryWidgets.Add(InstanceId,EntryWidget);
	PalettePanel->AddChild(EntryWidget);
	EntryWidget->SetVisibility(ESlateVisibility::Visible);
	EntryWidget->SetSelected(InstanceId == SelectedInstanceId);
	return true;
}

void UCWSFlowBoardWidget::HidePaletteEntry(FGuid InstanceId)
{
	if (TObjectPtr<UCWSFlowElementEntryWidget>* FoundEntry = ElementEntryWidgets.Find(InstanceId))
	{
		if (UCWSFlowElementEntryWidget* Entry = FoundEntry->Get())
		{
			Entry->SetVisibility(ESlateVisibility::Collapsed);
			// Entry->RemoveFromParent();
		}
		
	}
}

void UCWSFlowBoardWidget::RemovePaletteEntry(FGuid InstanceId)
{
	TObjectPtr<UCWSFlowElementEntryWidget>* FoundEntry = ElementEntryWidgets.Find(InstanceId);
		if (!FoundEntry) return;

	if (UCWSFlowElementEntryWidget* Entry = FoundEntry->Get())
	{
		Entry->OnSelected.RemoveDynamic(
			this,
			&ThisClass::HandleEntrySelected
		);

		Entry->RemoveFromParent();
	}

	ElementEntryWidgets.Remove(InstanceId);
}

void UCWSFlowBoardWidget::UnbindEntryDelegates()
{
	for (const TPair<FGuid, TObjectPtr<UCWSFlowElementEntryWidget>>& Pair: ElementEntryWidgets)
	{
		if (UCWSFlowElementEntryWidget* Entry = Pair.Value.Get())
		{
			Entry->OnSelected.RemoveDynamic(
				this,
				&ThisClass::HandleEntrySelected
			);
		}
		
	}
}

void UCWSFlowBoardWidget::BindGridDelegates()
{
	if (!IsValid(FlowGrid))
	{
		return;
	}

	FlowGrid->OnStructureChanged.AddUniqueDynamic(
		this,
		&ThisClass::HandleStructureChanged
	);

	FlowGrid->OnElementCreated.AddUniqueDynamic(
		this,
		&ThisClass::HandleElementCreated
	);

	FlowGrid->OnElementPlaced.AddUniqueDynamic(
		this,
		&ThisClass::HandleElementPlaced
	);

	FlowGrid->OnElementUnplaced.AddUniqueDynamic(
		this,
		&ThisClass::HandleElementUnplaced
	);

	FlowGrid->OnElementDeleted.AddUniqueDynamic(
		this,
		&ThisClass::HandleElementDeleted
	);
}

void UCWSFlowBoardWidget::UnbindGridDelegates()
{
	if (!IsValid(FlowGrid))
	{
		return;
	}

	FlowGrid->OnStructureChanged.RemoveDynamic(
		this,
		&ThisClass::HandleStructureChanged
	);

	FlowGrid->OnElementCreated.RemoveDynamic(
		this,
		&ThisClass::HandleElementCreated
	);

	FlowGrid->OnElementPlaced.RemoveDynamic(
		this,
		&ThisClass::HandleElementPlaced
	);

	FlowGrid->OnElementUnplaced.RemoveDynamic(
		this,
		&ThisClass::HandleElementUnplaced
	);

	FlowGrid->OnElementDeleted.RemoveDynamic(
		this,
		&ThisClass::HandleElementDeleted
	);
}

void UCWSFlowBoardWidget::HandleEntrySelected(FGuid InstanceId)
{
	SelectElement(InstanceId);
}

void UCWSFlowBoardWidget::HandleStructureChanged()
{
	RefreshGrid();
}

void UCWSFlowBoardWidget::HandleElementCreated(FGuid InstanceId)
{
	ShowPaletteEntry(InstanceId);
}

void UCWSFlowBoardWidget::HandleElementDeleted(FGuid InstanceId)
{
	const bool bWasSelected = SelectedInstanceId == InstanceId;

	RemoveElementWidget(InstanceId);
	RemovePaletteEntry(InstanceId);

	if (bWasSelected)
	{
		ClearSelection();
	}
}

void UCWSFlowBoardWidget::HandleElementPlaced(FGuid InstanceId)
{
	ShowPlacedElement(InstanceId);
	HidePaletteEntry(InstanceId);

	if (SelectedInstanceId == InstanceId)
	{
		SelectedInstanceId.Invalidate();
		PendingRotationSteps = 0;
		RefreshPaletteSelection();
	}
}

void UCWSFlowBoardWidget::HandleElementUnplaced(FGuid InstanceId)
{
	HidePlacedElement(InstanceId);
	ShowPaletteEntry(InstanceId);
}

void UCWSFlowBoardWidget::RefreshPlacedElements()
{
	if (!IsValid(FlowGrid) ||
		!IsValid(ElementCanvas)
	)
	{
		return;
	}

	ElementCanvas->ClearChildren();
	ElementWidgets.Reset();

	TArray<FGuid> PlacedIds;
	FlowGrid->GetPlacedElementIds(PlacedIds);

	for (const FGuid& InstanceId : PlacedIds)
	{
		ShowPlacedElement(InstanceId);
	}
}

bool UCWSFlowBoardWidget::ShowPlacedElement(FGuid InstanceId)
{
	if (!IsValid(FlowGrid) ||
		!IsValid(ElementCanvas) ||
		!ElementWidgetClass
	)
	{
		return false;
	}

	// Check if instance exists and is placed/occupies any cells
	FCWSFlowElementInstance Instance;

	if (!FlowGrid->TryGetInstanceById(InstanceId, Instance) ||
		!Instance.bIsPlaced ||
		Instance.PlacedCells.IsEmpty()
	)
	{
		return false;
	}

	return ShowElementAtCells(
		Instance,
		Instance.PlacedCells,
		1.0f
	);
}

bool UCWSFlowBoardWidget::ShowHoveredElement(
	const FGuid InstanceId,
	const TArray<FIntPoint>& PreviewCells
)
{
	FCWSFlowElementInstance PreviewInstance;

	if (!IsValid(FlowGrid) ||
		!FlowGrid->TryGetInstanceById(InstanceId, PreviewInstance)
	)
	{
		return false;
	}

	// Local copy for displaying the preview

	PreviewInstance.GridOrigin = HoveredCell;
	PreviewInstance.PlacedCells = PreviewCells;
	PreviewInstance.bIsPlaced = false;
	PreviewInstance.RotationSteps = PendingRotationSteps;

	return ShowElementAtCells(
		PreviewInstance,
		PreviewCells,
		0.65f
	);
}


void UCWSFlowBoardWidget::HidePlacedElement(FGuid InstanceId)
{
	if (TObjectPtr<UCWSFlowElementWidget>* FoundWidget = ElementWidgets.Find(InstanceId))
	{
		if (UCWSFlowElementWidget* Widget = FoundWidget->Get())
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UCWSFlowBoardWidget::RemoveElementWidget(FGuid InstanceId)
{
	TObjectPtr<UCWSFlowElementWidget>* FoundWidget = ElementWidgets.Find(InstanceId);
		if (!FoundWidget) return;

	if (UCWSFlowElementWidget* Widget = FoundWidget->Get())
	{
		Widget->RemoveFromParent();
	}

	ElementWidgets.Remove(InstanceId);
}

bool UCWSFlowBoardWidget::ShowElementAtCells(
	const FCWSFlowElementInstance& DisplayInstance,
	const TArray<FIntPoint>& Cells,
	float InRenderOpacity
)
{
	const FGuid& InstanceId = DisplayInstance.InstanceId;
	// If the ElementWidget already exists, find it
	UCWSFlowElementWidget* ElementWidget = nullptr;

	if (TObjectPtr<UCWSFlowElementWidget>* ExistingWidget = ElementWidgets.Find(InstanceId))
	{
		ElementWidget = ExistingWidget->Get();
	}

	// The ElementWidget doesn't exist or is invalid, so create a new one
	if (!IsValid(ElementWidget))
	{
		ElementWidget =
			CreateWidget<UCWSFlowElementWidget>(
				GetOwningPlayer(),
				ElementWidgetClass
			);

		if (!IsValid(ElementWidget))
		{
			return false;
		}

		ElementWidgets.Add(InstanceId, ElementWidget);
		ElementCanvas->AddChildToCanvas(ElementWidget);
	}

	// Find boundary of occupied cells
	int32 MaxX = Cells[0].X;
	int32 MinX = Cells[0].X;
	int32 MaxY = Cells[0].Y;
	int32 MinY = Cells[0].Y;

	for (const FIntPoint& Cell : DisplayInstance.PlacedCells)
	{
		MaxX = FMath::Max(MaxX, Cell.X);
		MinX = FMath::Min(MinX, Cell.X);
		MaxY = FMath::Max(MaxY, Cell.Y);
		MinY = FMath::Min(MinY, Cell.Y);
	}

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ElementWidget->Slot);

	if (!CanvasSlot)
	{
		return false;
	}

	const FVector2D Position(
		MinX * CellSize,
		MinY * CellSize
	);

	const FVector2D Size(
		(MaxX - MinX + 1) * CellSize,
		(MaxY - MinY + 1) * CellSize
	);
	
	CanvasSlot->SetPosition(Position);
	CanvasSlot->SetSize(Size);
	CanvasSlot->SetAlignment(FVector2D::ZeroVector);
	CanvasSlot->SetAutoSize(false);

	TArray<FCWSResolvedPipe> ResolvedPipes;
	if (!FlowGrid->ResolvePipesForInstance(
		DisplayInstance,
		ResolvedPipes
	))
	{
		return false;
	}
	
	ElementWidget->SetElementInstance(
		DisplayInstance,
		ResolvedPipes,
		CellSize
	);
	ElementWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	ElementWidget->SetRenderOpacity(RenderOpacity);

	return true;	
}

void UCWSFlowBoardWidget::RotateSelectionBy(int32 Rotations)
{
	if (!SelectedInstanceId.IsValid())
	{
		return;
	}

	constexpr int32 RotationCount = 4;

	int32 NewRotation = static_cast<int32>(PendingRotationSteps) + Rotations;

	NewRotation %= RotationCount;

	if (NewRotation < 0)
	{
		NewRotation += RotationCount;
	}

	PendingRotationSteps = static_cast<uint8>(NewRotation);

	UpdatePlacementPreview();
}
