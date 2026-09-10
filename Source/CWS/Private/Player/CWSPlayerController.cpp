// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CWSPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ACWSPlayerController::ACWSPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void ACWSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);

	SetInputMode(InputMode);
}

void ACWSPlayerController::ShowGameOverScreen()
{
	if (!IsLocalController() || !GameOverWidgetClass || GameOverWidget)
	{
		return;
	}

	GameOverWidget = CreateWidget<UUserWidget>(
		this,
		GameOverWidgetClass
	);

	if (!IsValid(GameOverWidget))
	{
		return;
	}

	GameOverWidget->AddToViewport(100);

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = true;
}

void ACWSPlayerController::RestartCurrentLevel()
{
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(
		this,
		true
	);

	UGameplayStatics::OpenLevel(
		this,
		FName(*CurrentLevelName)
	);
}

bool ACWSPlayerController::GetMouseAimDirection(
	const FVector& PlaneOrigin, FVector& OutDirection
) const {
	FVector RayOrigin;
	FVector RayDirection;

	if (!DeprojectMousePositionToWorld(RayOrigin, RayDirection))
	{
		return false;
	}

	const FVector PlaneNormal = FVector::UpVector;
	const double Denominator = FVector::DotProduct(PlaneNormal, RayDirection);
	if (FMath::IsNearlyZero(Denominator))
	{
		return false;
	}

	const double Distance = FVector::DotProduct(PlaneOrigin - RayOrigin, PlaneNormal) / Denominator;

	if (Distance < 0.0)
	{
		return false;
	}

	const FVector AimPosition = RayOrigin + RayDirection * Distance;
	OutDirection = (AimPosition - PlaneOrigin).GetSafeNormal2D();

	return !OutDirection.IsNearlyZero();

	
}

