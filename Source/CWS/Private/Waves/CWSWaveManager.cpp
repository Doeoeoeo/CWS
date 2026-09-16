// Fill out your copyright notice in the Description page of Project Settings.


#include "Waves/CWSWaveManager.h"

#include "Enemies/CWSEnemySpawner.h"

ACWSWaveManager::ACWSWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ACWSWaveManager::StartWaves()
{
	if (bWavesRunning || SpawnerByClass.IsEmpty())
	{
		return;
	}
	bWavesRunning = true;

	StartNextWave();
	
}

void ACWSWaveManager::StopWaves()
{
	bWavesRunning = false;
	GetWorldTimerManager().ClearTimer(NextWaveTimerHandle);
	ExpectedSpawnBatchCount = 0;
	CompletedSpawnBatchCount = 0;
	
	for (const TPair<TSubclassOf<ACWSEnemySpawner>, TObjectPtr<ACWSEnemySpawner>>& Pair : SpawnerByClass)
	{
		ACWSEnemySpawner* Spawner = Pair.Value.Get();
		if (IsValid(Spawner))
		{
			Spawner->StopSpawning();
			Spawner->SetActiveEnemiesEnabled(false);
			
		}
	}
}

void ACWSWaveManager::BeginPlay()
{
	Super::BeginPlay();
	CreateSpawners();
}

void ACWSWaveManager::CreateSpawners()
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	
	for (const FCWSWaveDefinition& Wave : WaveDefinitions)
	{
		for (const FCWSSpawnBatchDefinition& Batch : Wave.SpawnBatches)
		{
			UClass* SpawnerClass = Batch.SpawnerClass.Get();
			if (!SpawnerClass || SpawnerByClass.Contains(SpawnerClass))
			{
				continue;
			}

			ACWSEnemySpawner* Spawner = GetWorld()->SpawnActor<ACWSEnemySpawner>(
				Batch.SpawnerClass,
				GetActorTransform(),
				SpawnParameters
			);

			if (!IsValid(Spawner))
			{
				return;
			}

			Spawner->OnSpawnBatchCompleted.AddUniqueDynamic(
				this,
				&ACWSWaveManager::HandleSpawnBatchCompleted
			);

			SpawnerByClass.Add(Batch.SpawnerClass, Spawner);
		}
	}
}

void ACWSWaveManager::StartNextWave()
{
	if (!bWavesRunning)
	{
		return;
	}

	if (!WaveDefinitions.IsValidIndex(CurrentWaveNumber))
	{
		bWavesRunning = false;
		UE_LOG(LogTemp, Log, TEXT("All waves have completed"));
	}
	

	FCWSWaveDefinition& Wave = WaveDefinitions[CurrentWaveNumber];
	
	TArray<TPair<ACWSEnemySpawner*, int32>> BatchesToStart;
	TSet<ACWSEnemySpawner* > ScheduledSpawners;
	
	for (const FCWSSpawnBatchDefinition& Batch : Wave.SpawnBatches)
	{
		if (Batch.EnemyCount <= 0)
		{
			continue;
		}

		ACWSEnemySpawner* Spawner = FindSpawner(Batch.SpawnerClass);
		
		if (!IsValid(Spawner))
		{
			continue;
		}

		if (ScheduledSpawners.Contains(Spawner))
		{
			UE_LOG(LogTemp, Warning, TEXT("Wave %d contains two instances of the same spawner."), CurrentWaveNumber + 1);
			continue;
		}

		ScheduledSpawners.Add(Spawner);
		BatchesToStart.Emplace(Spawner, Batch.EnemyCount);
	}

	CompletedSpawnBatchCount = 0;
	ExpectedSpawnBatchCount = BatchesToStart.Num();
	
	if (ExpectedSpawnBatchCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Wave %d has no batches to spawn"), CurrentWaveNumber + 1);
		bWavesRunning = false;
		return;
	}

	++CurrentWaveNumber;
	OnWaveStarted.Broadcast(CurrentWaveNumber);
	for (const TPair<ACWSEnemySpawner*, int32>& Batch : BatchesToStart)
	{
		Batch.Key->StartSpawnBatch(Batch.Value);
	}
}

void ACWSWaveManager::HandleSpawnBatchCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("On batch complete being handled"));
	
	if (!bWavesRunning || ExpectedSpawnBatchCount == 0)
	{
		return;
	}
	
	++CompletedSpawnBatchCount;
	if (CompletedSpawnBatchCount < ExpectedSpawnBatchCount)
	{
		return;
	}

	ExpectedSpawnBatchCount = 0;
	OnWaveCompleted.Broadcast(CurrentWaveNumber);
	
	if (!WaveDefinitions.IsValidIndex(CurrentWaveNumber))
	{
		bWavesRunning = false;
		UE_LOG(LogTemp, Log, TEXT("All waves have completed"));
		return;
	}
	
	GetWorldTimerManager().SetTimer(
		NextWaveTimerHandle,
		this,
		&ACWSWaveManager::StartNextWave,
		TimeBetweenWaves,
		false
	);
	
}

ACWSEnemySpawner* ACWSWaveManager::FindSpawner(TSubclassOf<ACWSEnemySpawner> SpawnerClass) const
{
	const TObjectPtr<ACWSEnemySpawner>* FoundSpawner = SpawnerByClass.Find(SpawnerClass);
	
	if (!FoundSpawner || !IsValid(FoundSpawner->Get()))
	{
		return nullptr;
	}

	return FoundSpawner->Get();
}



