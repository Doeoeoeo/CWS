// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CWSPlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS(Abstract)
class CWS_API UCWSPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealth(
		float CurrentHealth,
		float MaxHealth
	);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthText;
};
