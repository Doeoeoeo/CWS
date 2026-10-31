// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Flow/CWSFlowElementInstance.h"
#include "Flow/CWSResolvedPipe.h"
#include "CWSFlowElementWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class CWS_API UCWSFlowElementWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	void SetElementInstance(
		const FCWSFlowElementInstance& InElementInstance,
		const TArray<FCWSResolvedPipe>& InResolvedPipes, float InCellSize
	);

	UFUNCTION(BlueprintPure, Category = "Flow Element")
	FGuid GetElementId() const
	{
		return ElementInstance.InstanceId;
	}
	
protected:
	UPROPERTY(
		BlueprintReadOnly,
		Transient,
		Category = "Flow Element"
	)
	FCWSFlowElementInstance ElementInstance;

	UPROPERTY(
		BlueprintReadOnly,
		Transient,
		Category = "Flow Element"
	)
	TArray<FCWSResolvedPipe> ResolvedPipes;

	UPROPERTY(
		BlueprintReadOnly,
		Transient,
		Category = "Flow Element"
	)
	FIntPoint DisplayMinCell = FIntPoint::ZeroValue;

	UPROPERTY(
		BlueprintReadOnly,
		Transient,
		Category = "Flow Element"
	)
	float DisplayCellSize = 0.0f;

	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Flow Element"
	)
	void RefreshElementVisuals();
};
