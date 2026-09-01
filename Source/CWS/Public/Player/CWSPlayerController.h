// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CWSPlayerController.generated.h"


UCLASS()
class CWS_API ACWSPlayerController : public APlayerController
{
	GENERATED_BODY()


	public:
	ACWSPlayerController();

	bool GetMouseAimDirection(
		const FVector& PlaneOrigin,
		FVector& OutDirection	
	) const;

	protected:
		virtual void BeginPlay() override;
};
