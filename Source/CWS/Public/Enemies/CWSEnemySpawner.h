// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CWSEnemySpawner.generated.h"

class ACWSEnemyBase;
class APawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FCWSSpawnBatchCompletedSignature
);


UCLASS()
class CWS_API ACWSEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ACWSEnemySpawner();

	void StartSpawnBatch(int32 EnemyCount);
	void StopSpawning();
	void SetActiveEnemiesEnabled(bool bEnabled);


	UPROPERTY(
		BlueprintAssignable,
		Category = "Spawning"
	)
	FCWSSpawnBatchCompletedSignature OnSpawnBatchCompleted;

	
protected:
	virtual void BeginPlay() override;

private:
	void SpawnEnemy();

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Spawning",
		meta = (AllowPrivateAccess = true)
	)
	TSubclassOf<ACWSEnemyBase> EnemyClass;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Spawning",
		meta = (
			AllowPrivateAccess = true,
			ClampMin = "0.1"
		)
	)
	float SpawnInterval = 2.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Spawning",
		meta = (
			AllowPrivateAccess = true,
			ClampMin = "0.1"
		)
	)
	float SpawnRadius = 1200.0f;	

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Spawning",
		meta = (
			AllowPrivateAccess = true,
			ClampMin = "1"
		)
	)
	int32 MaxActiveEnemies = 2;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Spawning",
		meta = (
			AllowPrivateAccess = true,
			ClampMin = "1"
		)
	)
	int32 InitialWaveEnemyCount = 5;
	
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Spawning",
		meta = (AllowPrivateAccess = true)
	)
	bool bSpawnImmediately = true;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Spawning",
		meta = (AllowPrivateAccess = true)
	)
	bool bStartAutomatically = false;
	
	FTimerHandle SpawnTimerHandle;
	TWeakObjectPtr<APawn> TargetPawn;
	TArray<TWeakObjectPtr<ACWSEnemyBase>> ActiveEnemies;

	UFUNCTION()
	void HandleSpawnedEnemyDestroyed(AActor* DestroyedActor);

	void CheckSpawnBatchCompleted();
	
	int32 RemainingEnemiesToSpawn = 0;
	bool bWaveActive = false;

	
};
