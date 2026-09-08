// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/CWSEnemyBase.h"

#include "AIController.h"
#include "PaperFlipbookComponent.h"
#include "Components/SphereComponent.h"
#include "Combat/CWSHealthComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ACWSEnemyBase::ACWSEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	EnemyCollision = CreateDefaultSubobject<USphereComponent>(
		TEXT("Collision")
	);

	SetRootComponent(EnemyCollision);
	EnemyCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EnemyCollision->SetCollisionObjectType(ECC_Pawn);
	EnemyCollision->SetGenerateOverlapEvents(true);
	EnemyCollision->InitSphereRadius(32.0f);
	EnemyCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	EnemyCollision->SetCollisionResponseToChannel(
		ECC_WorldDynamic,
		ECR_Overlap
	);
	EnemyCollision->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Overlap
	);


	EnemyVisual = CreateDefaultSubobject<UPaperFlipbookComponent>(
		TEXT("EnemyVisual")
	);
	EnemyVisual->SetupAttachment(EnemyCollision);
	EnemyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnemyVisual->SetRelativeRotation(FRotator(0, -90.0f, -90.0f));


	HealthComponent = CreateDefaultSubobject<UCWSHealthComponent>(
		TEXT("Health")
	);

	// Gives spawned enemies an AI controller to move them
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();
}

void ACWSEnemyBase::SetEnemyActive(bool bNewActive)
{
	if (bNewActive == bIsEnemyActive)
	{
		return;
	}

	bIsEnemyActive = bNewActive;

	// Disable enemy
	SetActorTickEnabled(bIsEnemyActive);
	SetActorEnableCollision(bIsEnemyActive);

	// Stop the remaining movement
	ConsumeMovementInputVector();
	if (UPawnMovementComponent* Movement = GetMovementComponent())
	{
		Movement->StopMovementImmediately();
	}

	OnEnemyActiveChanged(bIsEnemyActive);
	
}

void ACWSEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// Death hook
	HealthComponent->OnDeath.AddDynamic(
		this,
		&ACWSEnemyBase::HandleDeath
	);

	// Collision hood
	EnemyCollision->OnComponentBeginOverlap.AddDynamic(
		this,
		&ACWSEnemyBase::HandlePlayerOverlap
	);
}

void ACWSEnemyBase::Die()
{
	Destroy();
}

void ACWSEnemyBase::OnPlayerOverlap(APawn* PlayerPawn)
{
	// Ignore collisions if enemies are stopped
	if (!bIsEnemyActive)
	{
		return;
	}
	
	UGameplayStatics::ApplyDamage(
		PlayerPawn,
		ContactDamage,
		GetController(),
		this,
		UDamageType::StaticClass()
	);
	
}

void ACWSEnemyBase::OnEnemyActiveChanged(bool bNewActive)
{
}

void ACWSEnemyBase::HandleDeath(AActor* DeadActor)
{
	if (DeadActor == this)
	{
		Die();
	}
	
}

void ACWSEnemyBase::HandlePlayerOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || PlayerPawn != OtherActor)
	{
		return;
	}

	OnPlayerOverlap(PlayerPawn);
}




