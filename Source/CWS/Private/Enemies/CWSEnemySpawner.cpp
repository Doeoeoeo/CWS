// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/CWSEnemySpawner.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Enemies/CWSEnemyBase.h"

// Sets default values
ACWSEnemySpawner::ACWSEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACWSEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

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



