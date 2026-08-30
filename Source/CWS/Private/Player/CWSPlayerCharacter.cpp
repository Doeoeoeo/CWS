// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CWSPlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

ACWSPlayerCharacter::ACWSPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Prevent the character from rotating the camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->GravityScale = 0.0f;
	Movement->bOrientRotationToMovement = false;

	// Restrict movement to the plane XY
	Movement->SetPlaneConstraintEnabled(true);
	Movement->SetPlaneConstraintNormal(FVector::UpVector);

	// Movement data
	Movement->MaxFlySpeed = 600.0f;
	Movement->BrakingDecelerationFlying = 2000.0f;
	
	GetSprite()->SetRelativeRotation(FRotator(0, 0, -90.0f));
	GetSprite()->SetRelativeLocation(FVector(0, 0, 1.0));
	GetSprite()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(
		TEXT("TopDownCamera")
	);

	TopDownCamera->SetupAttachment(RootComponent);

	TopDownCamera->SetUsingAbsoluteRotation(true);
	TopDownCamera->SetRelativeLocation(FVector(0, 0, 1000.0f));
	TopDownCamera->SetRelativeRotation(FRotator(-90.0, -90.0, 0.0f));

	TopDownCamera->ProjectionMode = ECameraProjectionMode::Orthographic;

	TopDownCamera->OrthoWidth = 1920.0f;
}

void ACWSPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	UCharacterMovementComponent* Movement = GetCharacterMovement();

	// Fixate the character on the spawned plane
	Movement->SetPlaneConstraintOrigin(GetActorLocation());

	// Flying ignores gravity and floor requirements (The plane constraint keeps it on the plane)
	Movement->SetMovementMode(MOVE_Flying);
}

void ACWSPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()); PlayerController && PlayerMappingContext)
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				InputSubsystem->AddMappingContext(
					PlayerMappingContext,
					0
				);
			}
		}
	}


	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent); EnhancedInput && MoveAction)
	{
		EnhancedInput->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&ACWSPlayerCharacter::Move
		);
	}
}

void ACWSPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Direction = Value.Get<FVector2D>();

	AddMovementInput(FVector::ForwardVector, Direction.X);
	AddMovementInput(FVector::RightVector, Direction.Y);
}
