// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CWSPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Combat/CWSHealthComponent.h"
#include "UI/CWSPlayerHUDWidget.h"

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

	// Health bar
	if (!IsLocalController() || !PlayerHUDWidgetClass)
	{
		return;
	}

	PlayerHUDWidget = CreateWidget<UCWSPlayerHUDWidget>(
		this,
		PlayerHUDWidgetClass
	);
	if (!IsValid(PlayerHUDWidget))
	{
		return;
	}
	PlayerHUDWidget->AddToViewport();

	APawn* PlayerPawn = GetPawn();
	if (!IsValid(PlayerPawn))
	{
		return;
	}
	UCWSHealthComponent* HealthComponent = PlayerPawn->FindComponentByClass<UCWSHealthComponent>();
	if (!IsValid(HealthComponent))
	{
		return;
	}
	HealthComponent->OnHealthChanged.AddUniqueDynamic(
		this,
		&ACWSPlayerController::HandlePlayerHealthChanged
	);

	HandlePlayerHealthChanged(
		HealthComponent->GetCurrentHealth(),
		HealthComponent->GetMaxHealth()
	);
	
}

void ACWSPlayerController::HandlePlayerHealthChanged(float CurrentHealth, float MaxHealth)
{
	if (!IsValid(PlayerHUDWidget))
	{
		return;
	}
	PlayerHUDWidget->SetHealth(CurrentHealth, MaxHealth);
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

