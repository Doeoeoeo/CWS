// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "CWSPlayerCharacter.generated.h"

class UCameraComponent;

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class CWS_API ACWSPlayerCharacter : public APaperCharacter
{
	GENERATED_BODY()

	public:
		ACWSPlayerCharacter();

	protected:
		virtual void BeginPlay() override;

		virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	private:
		void Move(const FInputActionValue& Value);
	
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
};
