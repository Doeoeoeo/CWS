// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CWSEnemyBase.generated.h"

class UCWSHealthComponent;
class USphereComponent;
class UPaperFlipbookComponent;

UCLASS(Abstract)
class CWS_API ACWSEnemyBase : public APawn
{
	GENERATED_BODY()
	
public:	
	ACWSEnemyBase();

protected:
	virtual void BeginPlay() override;

	virtual void Die();

	virtual void OnPlayerOverlap(APawn* PlayerPawn);
	
	USphereComponent* GetEnemyCollision() const
	{
		return EnemyCollision;
	}

	UCWSHealthComponent* GetHealthComponent() const
	{
		return HealthComponent;
	}

	void SetContactDamage(float Damage)
	{
		ContactDamage = FMath::Max(Damage, 0.0f);	
	}
	
private:
	UFUNCTION()
	void HandleDeath(AActor* DeadActor);

	UFUNCTION()
	void HandlePlayerOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult
	);
	
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Enemy",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<USphereComponent> EnemyCollision;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Enemy",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UCWSHealthComponent> HealthComponent;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Enemy",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UPaperFlipbookComponent> EnemyVisual;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Combat",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "0.0"
		)
	)
	float ContactDamage = 0.0f;
};
