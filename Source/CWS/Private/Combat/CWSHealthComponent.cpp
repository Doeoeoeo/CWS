// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CWSHealthComponent.h"
#include "GameFramework/Actor.h"

UCWSHealthComponent::UCWSHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCWSHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	
	AActor* owner = GetOwner();
	owner->OnTakeAnyDamage.AddDynamic(
		this,
		&UCWSHealthComponent::HandleOwnerTakeAnyDamage
	);
}

void UCWSHealthComponent::HandleOwnerTakeAnyDamage(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	class AController* InstigatedBy,
	AActor* DamageCauser
)
{
	if (Damage <= 0.0f || IsDead())
	{
		return;
	}

	CurrentHealth = FMath::Clamp(
		CurrentHealth - Damage,
		0.0f,
		MaxHealth
	);

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		OnDeath.Broadcast(DamagedActor);
	}

}

