// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CWSWaveManager.generated.h"

class ACWSEnemySpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FCWSWaveStateSignature,
	int32,
	WaveNumber
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FCWSAllWavesCompletedSignature
);

USTRUCT(BlueprintType)
struct FCWSSpawnBatchDefinition
{
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Waves"
	)
	TSubclassOf<ACWSEnemySpawner> SpawnerClass;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Waves",
		meta = (ClampMin = "1")
	)
	int32 EnemyCount = 1;
};

USTRUCT(BlueprintType)
struct FCWSWaveDefinition
{
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Waves"
	)
	TArray<FCWSSpawnBatchDefinition> SpawnBatches;
};

UCLASS()
class CWS_API ACWSWaveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ACWSWaveManager();

	void StartWaves();
	void StopWaves();

	UPROPERTY(
		BlueprintAssignable,
		Category = "Waves"
	)
	FCWSWaveStateSignature OnWaveStarted;

	UPROPERTY(
		BlueprintAssignable,
		Category = "Waves"
	)
	FCWSWaveStateSignature OnWaveCompleted;

	UPROPERTY(
		BlueprintAssignable,
		Category = "Waves"
	)
	FCWSAllWavesCompletedSignature OnAllWavesCompleted;
	
protected:
	virtual void BeginPlay() override;

private:	
	void CreateSpawners();
	void StartNextWave();

	UFUNCTION()
	void HandleSpawnBatchCompleted();

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Waves",
		meta=(AllowPrivateAccess = "true")
	)
	TArray<FCWSWaveDefinition> WaveDefinitions;

	UPROPERTY(Transient)
	TMap<TSubclassOf<ACWSEnemySpawner>, TObjectPtr<ACWSEnemySpawner>> SpawnerByClass;

	ACWSEnemySpawner* FindSpawner(TSubclassOf<ACWSEnemySpawner> SpawnerClass) const;
	
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Waves",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "0.0")
	)
	float TimeBetweenWaves = 3.0f;

	FTimerHandle NextWaveTimerHandle;

	int32 CurrentWaveNumber = 0;
	int32 CompletedSpawnBatchCount = 0;
	int32 ExpectedSpawnBatchCount = 0;

	bool bWavesRunning = false;
};
