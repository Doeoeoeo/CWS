// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Flow/CWSFlowElementInstance.h"
#include "Flow/UI/CWSFlowCellWidget.h"
#include "CWSFlowBoardWidget.generated.h"

class UCWSFlowGridComponent;
class UCWSFlowCellWidget;
class UUniformGridPanel;
class USizeBox;
class UPanelWidget;
class UCWSFlowElementEntryWidget;
class UCanvasPanel;
class UCWSFlowElementWidget;


UCLASS(Abstract, Blueprintable)
class CWS_API UCWSFlowBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Flow Board")
	bool InitializeBoard(UCWSFlowGridComponent* InFlowGrid);

	// Widget interaction
	UFUNCTION(BlueprintCallable, Category = "Flow Board")
	bool SelectElement(FGuid InstanceId);

	UFUNCTION(BlueprintCallable, Category = "Flow Board")
	void ClearSelection();

	UFUNCTION(BlueprintCallable, Category = "Flow Board")
	void RotateSelectionClockwise();
	
protected:
	virtual void NativeDestruct() override;

	// R-click cancellation
	virtual FReply NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent
	) override;

	// Rotation Key mapping
	virtual FReply NativeOnMouseWheel(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent
	) override;

	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent
	) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel;

	UPROPERTY(EditDefaultsOnly, Category = "Flow Board")
	TSubclassOf<UCWSFlowCellWidget> CellWidgetClass;

	UPROPERTY(
		BlueprintReadOnly,
		Transient,
		Category = "Flow Board"
	)
	TObjectPtr<UCWSFlowGridComponent> FlowGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> GridSizeBox;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Flow Board",
		meta = (ClampMin = "16.0")
	)
	float CellSize = 64.0f;

	// Element palette
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PalettePanel;

	UPROPERTY(EditDefaultsOnly, Category = "Flow Board")
	TSubclassOf<UCWSFlowElementEntryWidget> ElementEntryWidgetClass;
	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UCWSFlowElementEntryWidget>> ElementEntryWidgets;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ElementCanvas;

	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UCWSFlowElementWidget>> ElementWidgets;

	UPROPERTY(EditDefaultsOnly, Category = "Flow Board")
	TSubclassOf<UCWSFlowElementWidget> ElementWidgetClass;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCWSFlowCellWidget>> CellWidgets;

	FIntPoint CachedGridSize = FIntPoint::ZeroValue;

	bool BuildGrid();

	UFUNCTION()
	void RefreshGrid();

	UFUNCTION()
	void HandleCellClicked(FIntPoint GridCoordinate);

	UFUNCTION()
	void HandleCellHovered(FIntPoint GridCoordinate);
	
	UFUNCTION()
	void HandleCellUnhovered(FIntPoint GridCoordinate);

	// Destructor helper
	void UnbindCellDelegates();
	
	// Grid helpers
	int32 CoordinateToIndex(FIntPoint GridCoordinate) const;

	UCWSFlowCellWidget* GetCellWidget(FIntPoint GridCoordinate) const;

	FIntPoint GetCellCoordinate(int32 Index) const;

	// Interaction states
	FGuid SelectedInstanceId;
	uint8 PendingRotationSteps = 0;

	bool bHasHoveredCell = false;
	FIntPoint HoveredCell = FIntPoint::ZeroValue;

	void UpdatePlacementPreview();

	// Palette helpers
	void RefreshPalette();
	void RefreshPaletteSelection();

	bool ShowPaletteEntry(FGuid InstanceId);
	void HidePaletteEntry(FGuid InstanceId);
	void RemovePaletteEntry(FGuid InstanceId);

	
	void UnbindEntryDelegates();
	void BindGridDelegates();
	void UnbindGridDelegates();

	UFUNCTION()
	void HandleEntrySelected(FGuid InstanceId);

	UFUNCTION()
	void HandleStructureChanged();

	UFUNCTION()
	void HandleElementCreated(FGuid InstanceId);

	UFUNCTION()
	void HandleElementDeleted(FGuid InstanceId);

	UFUNCTION()
	void HandleElementPlaced(FGuid InstanceId);

	UFUNCTION()
	void HandleElementUnplaced(FGuid InstanceId);

	void RefreshPlacedElements();

	bool ShowPlacedElement(FGuid InstanceId);

	bool ShowHoveredElement(
		const FGuid InstanceId,
		const TArray<FIntPoint>& PreviewCells
	);
	
	void HidePlacedElement(FGuid InstanceId);

	void RemoveElementWidget(FGuid InstanceId);

	bool ShowElementAtCells(
		const FCWSFlowElementInstance& DisplayInstance,
		const TArray<FIntPoint>& Cells,
		float InRenderOpacity
	);

	// Rotation helper
	void RotateSelectionBy(int32 Rotations);
};
