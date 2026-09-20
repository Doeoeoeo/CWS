// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CWSGameMode.generated.h"

UENUM(BlueprintType)
enum class ECWSGamePhase : uint8
{
	Combat,
	UpgradeSelection,
	GameOver
};

class ACWSWaveManager;

UCLASS()
class CWS_API ACWSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACWSGameMode();

protected:
	virtual void BeginPlay() override;

	virtual void RestartPlayer(AController* NewPlayer) override;

private:
	UFUNCTION()
	void HandlePlayerDeath(AActor* DeadActor);
	
	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Transient,
		Category = "Game",
		meta = (AllowPrivateAccess = "true")
	)
	ECWSGamePhase CurrentPhase = ECWSGamePhase::Combat;
	
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Waves",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<ACWSWaveManager> WaveManagerClass;

	UPROPERTY(Transient)
	TObjectPtr<ACWSWaveManager> WaveManager;

	UFUNCTION()
	void HandleAllWavesCompleted();
};


