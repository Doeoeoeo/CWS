// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CWSGameMode.generated.h"



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
	
	bool bRoundOver = false;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Waves",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<ACWSWaveManager> WaveManagerClass;

	UPROPERTY(Transient)
	TObjectPtr<ACWSWaveManager> WaveManager;
};


