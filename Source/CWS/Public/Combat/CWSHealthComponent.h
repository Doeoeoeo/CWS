// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CWSHealthComponent.generated.h"


class UDamageType;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FCWSHealthChangedSignature,
	float,
	CurrentHealth,
	float,
	MaxHealth
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FCWSDeathSignature,
	AActor*,
	DeadActor
);

UCLASS( ClassGroup=(CWS), meta=(BlueprintSpawnableComponent) )
class CWS_API UCWSHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCWSHealthComponent();

	UFUNCTION(BlueprintPure, Category="Health")
	float GetMaxHealth() const {return MaxHealth;}
	
	UFUNCTION(BlueprintPure, Category="Health")
	float GetCurrentHealth() const {return CurrentHealth;}

	UFUNCTION(BlueprintPure, Category="Health")
	bool IsDead() const {return CurrentHealth <= 0.0f;}
	
	UPROPERTY(BlueprintAssignable, Category="Health")
	FCWSHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="Health")
	FCWSDeathSignature OnDeath;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleOwnerTakeAnyDamage(
		AActor* DamagedActor,
		float Damage,
		const UDamageType* DamageType,
		class AController* InstigatedBy,
		AActor* DamageCauser
	);

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category="Health",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "0.0"
		)
	)
	float MaxHealth = 3.0f;

	UPROPERTY(
		VisibleInstanceOnly,
		BlueprintReadOnly,
		Transient,
		Category = "Health",
		meta = (AllowPrivateAccess = "true")
	)
	float CurrentHealth = 0.0f;
};
