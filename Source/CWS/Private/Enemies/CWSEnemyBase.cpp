// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/CWSEnemyBase.h"

#include "PaperFlipbookComponent.h"
#include "Components/SphereComponent.h"
#include "Combat/CWSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
ACWSEnemyBase::ACWSEnemyBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
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
	UGameplayStatics::ApplyDamage(
		PlayerPawn,
		ContactDamage,
		GetController(),
		this,
		UDamageType::StaticClass()
	);
	
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




