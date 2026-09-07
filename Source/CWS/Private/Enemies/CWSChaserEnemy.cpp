// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/CWSChaserEnemy.h"

#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"

ACWSChaserEnemy::ACWSChaserEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(
		TEXT("FloatingMovement")
	);

	FloatingMovement->SetUpdatedComponent(
		GetEnemyCollision()
	);
	FloatingMovement->MaxSpeed = 250.0f;
	FloatingMovement->Acceleration = 800.0f;
	FloatingMovement->Deceleration = 800.0f;
	FloatingMovement->TurningBoost = 8.0f;

	FloatingMovement->SetPlaneConstraintEnabled(true);
	FloatingMovement->SetPlaneConstraintNormal(FVector::UpVector);
	FloatingMovement->bSnapToPlaneAtStart = true;

	// Enemy statistics
	SetContactDamage(1.0f);
}

void ACWSChaserEnemy::BeginPlay()
{
	Super::BeginPlay();

	TargetPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	FloatingMovement->SetPlaneConstraintOrigin(
		GetActorLocation()
	);
	
}

void ACWSChaserEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TargetPawn.IsValid())
	{
		TargetPawn = UGameplayStatics::GetPlayerPawn(this, 0);

		if (!TargetPawn.IsValid())
		{
			return;
		}
	}

	FVector PathToTarget = TargetPawn->GetActorLocation() - GetActorLocation();
	PathToTarget.Z = 0;

	// Can this be a 3d vector on a 2d function?
	if (PathToTarget.SizeSquared2D() < FMath::Square(StopDistance))
	{
		return;
	}

	const FVector MoveDirection = PathToTarget.GetSafeNormal2D();

	AddMovementInput(MoveDirection, 1.0f);

	const float TurnYaw = MoveDirection.Rotation().Yaw + FacingRotationOffsetDegrees;
	SetActorRotation(FRotator(0.0f, TurnYaw, 0.0f));
	
}

void ACWSChaserEnemy::OnPlayerOverlap(APawn* PlayerPawn)
{
	Super::OnPlayerOverlap(PlayerPawn);

	Die();
}


