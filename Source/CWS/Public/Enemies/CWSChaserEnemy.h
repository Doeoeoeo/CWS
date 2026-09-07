// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/CWSEnemyBase.h"
#include "CWSChaserEnemy.generated.h"
class UFloatingPawnMovement;
class APawn;


UCLASS()
class CWS_API ACWSChaserEnemy : public ACWSEnemyBase
{
	GENERATED_BODY()

public:
	ACWSChaserEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPlayerOverlap(APawn* PlayerPawn) override;

private:
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Movement",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UFloatingPawnMovement> FloatingMovement;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movement",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "0.0"	
		)
	)
	float StopDistance = 10.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movement",
		meta = (AllowPrivateAccess = "true")
	)
	float FacingRotationOffsetDegrees = 0.0f;
	
	TWeakObjectPtr<APawn> TargetPawn;	
};
