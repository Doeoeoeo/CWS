// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CWSProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACWSProjectile::ACWSProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(
		TEXT("Collision")
	);

	SetRootComponent(Collision);

	Collision->InitSphereRadius(12.0f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);

	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetGenerateOverlapEvents(true);
	Collision->SetCollisionResponseToChannel(
		ECC_Pawn,
		ECR_Overlap
	);
	Collision->OnComponentBeginOverlap.AddDynamic(
		this,
		&ACWSProjectile::HandleOverlap
	);

	ProjectileVisual = CreateDefaultSubobject<UPaperFlipbookComponent>(
		TEXT("ProjectileVisual")
	);

	ProjectileVisual->SetupAttachment(Collision);
	ProjectileVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileVisual->SetRelativeRotation(FRotator(0.0f, -90.0f, -90.0f));
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(
		TEXT("ProjectileMovement")
	);
	
	ProjectileMovement->UpdatedComponent = Collision;
	ProjectileMovement->InitialSpeed = 1200.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	ProjectileMovement->bConstrainToPlane = true;
	ProjectileMovement->SetPlaneConstraintNormal(FVector::UpVector);

	InitialLifeSpan = 2.0f;
}

void ACWSProjectile::Launch(const FVector& Direction)
{
	const FVector FlatDirection = Direction.GetSafeNormal2D();

	ProjectileMovement->Velocity = FlatDirection * ProjectileMovement->InitialSpeed;
}

void ACWSProjectile::HandleOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	const float AppliedDamage = UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		GetInstigatorController(),
		this,
		UDamageType::StaticClass()
	);

	if (AppliedDamage > 0.0f)
	{
		Destroy();
	}
}



