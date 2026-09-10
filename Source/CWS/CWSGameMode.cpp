// Copyright Epic Games, Inc. All Rights Reserved.

#include "CWSGameMode.h"

#include "Combat/CWSHealthComponent.h"
#include "Enemies/CWSEnemySpawner.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CWSPlayerCharacter.h"
#include "Player/CWSPlayerController.h"

ACWSGameMode::ACWSGameMode()
{
	DefaultPawnClass = ACWSPlayerCharacter::StaticClass();
	PlayerControllerClass = ACWSPlayerController::StaticClass();
}

void ACWSGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundSpawners;
	UGameplayStatics::GetAllActorsOfClass(
		this,
		ACWSEnemySpawner::StaticClass(),
		FoundSpawners
	);

	for (AActor* Actor : FoundSpawners)
	{
		if (ACWSEnemySpawner* Spawner = Cast<ACWSEnemySpawner>(Actor))
		{
			EnemySpawners.Add(Spawner);
		}
	}
	
}

void ACWSGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	const APawn* PlayerPawn = NewPlayer->GetPawn();
	UCWSHealthComponent* HealthComponent = PlayerPawn->FindComponentByClass<UCWSHealthComponent>();
	HealthComponent->OnDeath.AddUniqueDynamic(
		this,
		&ACWSGameMode::HandlePlayerDeath
	);

	
}

void ACWSGameMode::HandlePlayerDeath(AActor* DeadActor)
{
	if (bRoundOver)
	{
		return;
	}

	bRoundOver = true;

	// Stop enemy spawning and movement
	for (const TWeakObjectPtr<ACWSEnemySpawner>& Spawner: EnemySpawners)
	{
		if (Spawner.IsValid())
		{
			Spawner->StopSpawning();
			Spawner->SetActiveEnemiesEnabled(false);
		}

		UE_LOG(LogTemp, Log, TEXT("Game Over"));
	}

	// Display Game Over screen
	if (ACWSPlayerController* PlayerController = Cast<ACWSPlayerController>(
		UGameplayStatics::GetPlayerController(this, 0))
	)
	{
		PlayerController->ShowGameOverScreen();
	}
	
}
