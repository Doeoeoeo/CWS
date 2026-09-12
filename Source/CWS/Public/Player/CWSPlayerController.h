// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CWSPlayerController.generated.h"


class UUserWidget;
class UCWSPlayerHUDWidget;
class UCWSHUDWidget;
class UCWSHealthComponent;

UCLASS()
class CWS_API ACWSPlayerController : public APlayerController
{
	GENERATED_BODY()


public:
	ACWSPlayerController();

	bool GetMouseAimDirection(
		const FVector& PlaneOrigin,
		FVector& OutDirection	
	) const;

	UFUNCTION(
		BlueprintCallable,
		Category = "Game"
	)
	void ShowGameOverScreen();
	
	UFUNCTION(
			BlueprintCallable,
			Category = "Game"
		)
	void RestartCurrentLevel();
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "UI",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> GameOverWidget;

	// Health UI
	UFUNCTION()
	void HandlePlayerHealthChanged(
		float CurrentHealth,
		float MaxHealth
	);

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "UI",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<UCWSPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UCWSPlayerHUDWidget> PlayerHUDWidget;
};
