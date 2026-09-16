// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/CWSEnemySpawner.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Enemies/CWSEnemyBase.h"

ACWSEnemySpawner::ACWSEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ACWSEnemySpawner::StartSpawnBatch(int32 EnemyCount)
{
	if (bWaveActive || EnemyCount <= 0)
	{
		return;
	}

	// Start the wave spawn
	RemainingEnemiesToSpawn = EnemyCount;
	bWaveActive = true;
		
	TargetPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	if (bSpawnImmediately)
	{
		SpawnEnemy();
	}

	if (RemainingEnemiesToSpawn > 0)
	{
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&ACWSEnemySpawner::SpawnEnemy,
			SpawnInterval,
			true
		);
	}

}

void ACWSEnemySpawner::StopSpawning()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	RemainingEnemiesToSpawn = 0;
	bWaveActive = false;
}

void ACWSEnemySpawner::SetActiveEnemiesEnabled(bool bEnabled)
{
	for (const TWeakObjectPtr<ACWSEnemyBase>& EnemyPtr: ActiveEnemies)
	{
		if (ACWSEnemyBase* Enemy = EnemyPtr.Get())
		{
			Enemy->SetEnemyActive(bEnabled);
		}
	}
}

// Called when the game starts or when spawned
void ACWSEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAutomatically)
	{
		StartSpawnBatch(InitialWaveEnemyCount);
	}
}

void ACWSEnemySpawner::SpawnEnemy()
{

	// Check if all spawn conditions are met
	if (!EnemyClass.Get())
	{
		return;	
	}

	if (!TargetPawn.IsValid())
	{
		TargetPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		if (!TargetPawn.IsValid())
		{
			return;
		}
	}

	if (!bWaveActive || RemainingEnemiesToSpawn <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		CheckSpawnBatchCompleted();
		return;
	}
	// Remove dead enemies
	ActiveEnemies.RemoveAll(
		[](const TWeakObjectPtr<ACWSEnemyBase>& Enemy)
		{
			return !Enemy.IsValid();
		}
	);

	if (ActiveEnemies.Num() >= MaxActiveEnemies)
	{
		return;
	}

	// Define a new spawn position
	const float Angle = FMath::RandRange(0.0f, 2.0f * PI);
	const FVector SpawnDirection = FVector(
		FMath::Cos(Angle),
		FMath::Sin(Angle),
		0.0f
	);

	const FVector SpawnLocation = TargetPawn->GetActorLocation() + SpawnDirection * SpawnRadius;

	// Set spawn parameters
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACWSEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<ACWSEnemyBase>(
			EnemyClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParameters
	);
	
	if (!IsValid(SpawnedEnemy))
	{
		return;
	}
	
	ActiveEnemies.Add(SpawnedEnemy);
	SpawnedEnemy->OnDestroyed.AddUniqueDynamic(
		this,
		&ACWSEnemySpawner::HandleSpawnedEnemyDestroyed
	);

	--RemainingEnemiesToSpawn;

	if (RemainingEnemiesToSpawn <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void ACWSEnemySpawner::HandleSpawnedEnemyDestroyed(AActor* DestroyedActor)
{
	ActiveEnemies.RemoveAll(
		[DestroyedActor](
			const TWeakObjectPtr<ACWSEnemyBase>& Enemy
		)
		{
			return !Enemy.IsValid() || Enemy.Get() == DestroyedActor;
		}
	);
	CheckSpawnBatchCompleted();
}

void ACWSEnemySpawner::CheckSpawnBatchCompleted()
{
	// Do nothing (the wave is not over)
	// if 1 - wave isn't active, 2 - enemies are left to spawn, 3 - enemies are still alive.
	if (
		!bWaveActive ||
		RemainingEnemiesToSpawn > 0 ||
		!ActiveEnemies.IsEmpty())
	{
		return;
	}

	bWaveActive = false;
	UE_LOG(LogTemp, Log, TEXT("Spawner wave cleared"));
	OnSpawnBatchCompleted.Broadcast();
	
}



