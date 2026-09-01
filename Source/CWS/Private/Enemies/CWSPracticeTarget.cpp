// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/CWSPracticeTarget.h"

#include "PaperFlipbookComponent.h"
#include "Components/SphereComponent.h"
#include "Combat/CWSHealthComponent.h"

// Sets default values
ACWSPracticeTarget::ACWSPracticeTarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Collision = CreateDefaultSubobject<USphereComponent>(
		TEXT("Collision")
	);
	SetRootComponent(Collision);
	
	Collision->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly
	);
	Collision->SetCollisionObjectType(ECC_Pawn);
	Collision->SetGenerateOverlapEvents(true);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(
		ECC_WorldDynamic,
		ECR_Overlap
	);


	TargetVisual = CreateDefaultSubobject<UPaperFlipbookComponent>(
		TEXT("TargetVisual")
	);

	TargetVisual->SetupAttachment(Collision);
	TargetVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TargetVisual->SetRelativeRotationExact(FRotator(0.0f, -90.0f, -90.0f));

	HealthComponent = CreateDefaultSubobject<UCWSHealthComponent>(
		TEXT("HealthComponent")
	);
}

void ACWSPracticeTarget::BeginPlay()
{
	Super::BeginPlay();


	
	
	HealthComponent->OnDeath.AddDynamic(
		this,
		&ACWSPracticeTarget::HandleDeath
	);
}

void ACWSPracticeTarget::HandleDeath(AActor* DeadActor)
{
	if (DeadActor == this)
	{
		Destroy();
	}
}




