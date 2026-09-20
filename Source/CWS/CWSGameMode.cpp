// Copyright Epic Games, Inc. All Rights Reserved.

#include "CWSGameMode.h"

#include "Combat/CWSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CWSPlayerCharacter.h"
#include "Player/CWSPlayerController.h"
#include "Waves/CWSWaveManager.h"

ACWSGameMode::ACWSGameMode()
{
	DefaultPawnClass = ACWSPlayerCharacter::StaticClass();
	PlayerControllerClass = ACWSPlayerController::StaticClass();
}

void ACWSGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!WaveManagerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaveManagerClass is not set!"));
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;

	WaveManager = GetWorld()->SpawnActor<ACWSWaveManager>(
		WaveManagerClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters
	);
	
	if (IsValid(WaveManager))
	{
		WaveManager->OnAllWavesCompleted.AddDynamic(
			this,
			&ACWSGameMode::HandleAllWavesCompleted
		);
		
		WaveManager->StartWaves();
	}
}

void ACWSGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	const APawn* PlayerPawn = NewPlayer->GetPawn();
	if (!IsValid(PlayerPawn))
	{
		return;
	}
	
	UCWSHealthComponent* HealthComponent = PlayerPawn->FindComponentByClass<UCWSHealthComponent>();
	if (!IsValid(HealthComponent))
	{
		return;
	}
	
	HealthComponent->OnDeath.AddUniqueDynamic(
		this,
		&ACWSGameMode::HandlePlayerDeath
	);

	
}

void ACWSGameMode::HandlePlayerDeath(AActor* DeadActor)
{
	if (CurrentPhase != ECWSGamePhase::Combat)
	{
		return;
	}

	CurrentPhase = ECWSGamePhase::GameOver;

	if (IsValid(WaveManager))
	{
		WaveManager->StopWaves();
	}
	UE_LOG(LogTemp, Log, TEXT("Game Over"));

	// Display Game Over screen
	if (ACWSPlayerController* PlayerController = Cast<ACWSPlayerController>(
		UGameplayStatics::GetPlayerController(this, 0))
	)
	{
		PlayerController->ShowGameOverScreen();
	}
	
}

void ACWSGameMode::HandleAllWavesCompleted()
{
	if (CurrentPhase != ECWSGamePhase::Combat)
	{
		return;
	}

	CurrentPhase = ECWSGamePhase::UpgradeSelection;
	
	UE_LOG(LogTemp, Log, TEXT("Entering upgrade selection"));
}
