// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "CWSPlayerCharacter.generated.h"

class UCameraComponent;

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

class ACWSProjectile;

UCLASS()
class CWS_API ACWSPlayerCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	ACWSPlayerCharacter();

protected:
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime) override;


private:
	void Move(const FInputActionValue& Value);
	void Fire();

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Camera",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UCameraComponent> TopDownCamera;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Input",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UInputMappingContext> PlayerMappingContext;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Input",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UInputAction> MoveAction;

	// Aiming
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Aiming",
		meta = (AllowPrivateAccess = "true")
	)
	float AimRotationOffsetDegrees = 0.0f;

	// Fire
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Input",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Combat",
		meta = (AllowPrivateAccess = "true")
	)
	TSubclassOf<ACWSProjectile> ProjectileClass;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Combat",
		meta = (AllowPrivateAccess = "true")
	)
	float ProjectileSpawnDistance = 70.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Combat",
		meta = (AllowPrivateAccess = "true")
	)
	float FireInterval = 0.15f;

	double NextAllowedFireTime = 0.0f;

	FVector CurrentAimDirection = FVector::ForwardVector;
};
