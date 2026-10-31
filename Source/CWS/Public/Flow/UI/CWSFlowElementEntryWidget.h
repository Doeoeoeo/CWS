// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CWSFlowElementEntryWidget.generated.h"

class UButton;
class UCWSFlowElementDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FCWSFlowElementEntrySelectedSignature,
	FGuid,
	InstanceId
);

UCLASS(Abstract, Blueprintable)
class CWS_API UCWSFlowElementEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeEntry(
		FGuid InInstanceId,
		UCWSFlowElementDefinition* InDefinition
	);

	UPROPERTY(
		BlueprintAssignable,
		Category = "Flow Entry"
	)
	FCWSFlowElementEntrySelectedSignature OnSelected;

	UFUNCTION()
	void SetSelected(bool bNewSelected);

	UFUNCTION()
	FGuid GetInstanceId() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Flow Entry"
	)
	FGuid InstanceId;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Flow Entry"
	)
	TObjectPtr<UCWSFlowElementDefinition> Definition;

	UPROPERTY(BlueprintReadOnly)
	bool bIsSelected = false;

	UFUNCTION(
		BlueprintImplementableEvent,
		Category="Flow Entry"
	)
	void RefreshEntryVisuals();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SelectButton;

	UFUNCTION()
	void HandleSelected();
};
