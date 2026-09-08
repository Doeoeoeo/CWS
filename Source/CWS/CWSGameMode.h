// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CWSGameMode.generated.h"



class ACWSEnemySpawner;

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

	TArray<TWeakObjectPtr<ACWSEnemySpawner>> EnemySpawners;

	bool bRoundOver = false;
	
};


