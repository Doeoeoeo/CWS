// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CWSFlowCellWidget.generated.h"

class UButton;

UENUM(BlueprintType)
enum class ECWSFlowCellVisualState : uint8
{
	Empty,
	Occupied,
	ValidPreview,
	InvalidPreview
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FCWSFlowCellInteractionSignature,
	FIntPoint,
	GridCoordinate
);

UCLASS(Abstract, Blueprintable, BlueprintType)
class CWS_API UCWSFlowCellWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeCell(FIntPoint InGridCoordinate);

	UPROPERTY(BlueprintAssignable, Category = "Flow Cell")
	FCWSFlowCellInteractionSignature OnCellClicked;

	UPROPERTY(BlueprintAssignable, Category = "Flow Cell")
	FCWSFlowCellInteractionSignature OnCellHovered;

	UPROPERTY(BlueprintAssignable, Category = "Flow Cell")
	FCWSFlowCellInteractionSignature OnCellUnhovered;

	UFUNCTION(BlueprintPure, Category = "Flow Cell")
	FIntPoint GetGridCoordinate() const
	{
		return GridCoordinate;
	}

	UFUNCTION(BlueprintImplementableEvent, Category = "Flow Cell")
	void SetVisualState(ECWSFlowCellVisualState NewState);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CellButton;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Flow Cell",
		meta = (AllowPrivateAccess = "true")
	)
	FIntPoint GridCoordinate = FIntPoint::ZeroValue;

	UFUNCTION()
	void HandleClicked();

	UFUNCTION()
	void HandleHovered();
	
	UFUNCTION()
	void HandleUnhovered();
};
