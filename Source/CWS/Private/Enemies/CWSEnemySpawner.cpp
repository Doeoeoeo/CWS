// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/CWSEnemySpawner.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Enemies/CWSEnemyBase.h"

ACWSEnemySpawner::ACWSEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ACWSEnemySpawner::StartSpawning()
{
	TargetPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	if (bSpawnImmediately)
	{
		SpawnEnemy();
	}

	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&ACWSEnemySpawner::SpawnEnemy,
		SpawnInterval,
		true
	);
}

void ACWSEnemySpawner::StopSpawning()
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
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

	StartSpawning();
	
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
	if (IsValid(SpawnedEnemy))
	{
		ActiveEnemies.Add(SpawnedEnemy);
	}

	

	
}



