// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CWSProjectile.generated.h"

class UProjectileMovementComponent;
class UPaperFlipbookComponent;
class USphereComponent;
class UPrimitiveComponent;

UCLASS()
class CWS_API ACWSProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ACWSProjectile();

	void Launch(const FVector& Direction);

private:	
	UPROPERTY(
		VisibleAnywhere, 
		BlueprintReadOnly, 
		Category = "Projectile",
		meta = (AllowPrivateAccess = true)
	)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(
		VisibleAnywhere, 
		BlueprintReadOnly, 
		Category = "Projectile",
		meta = (AllowPrivateAccess = true)
	)
	TObjectPtr<UPaperFlipbookComponent> ProjectileVisual;
	
	UPROPERTY(
    	VisibleAnywhere, 
    	BlueprintReadOnly, 
    	Category = "Projectile",
    	meta = (AllowPrivateAccess = true)
    )
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Projectile",
		meta = (AllowPrivateAccess = "true")
	)
	float Damage = 1.0f;

	UFUNCTION()
	void HandleOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
