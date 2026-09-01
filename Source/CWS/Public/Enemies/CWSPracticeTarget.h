// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CWSPracticeTarget.generated.h"

class USphereComponent;
class UPaperFlipbookComponent;
class UCWSHealthComponent;

UCLASS()
class CWS_API ACWSPracticeTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACWSPracticeTarget();

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(
		VisibleAnywhere, 
		BlueprintReadOnly, 
		Category = "Enemy",
		meta = (AllowPrivateAccess = true)
	)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(
		VisibleAnywhere, 
		BlueprintReadOnly, 
		Category = "Enemy",
		meta = (AllowPrivateAccess = true)
	)
	TObjectPtr<UPaperFlipbookComponent> TargetVisual;
	
	UPROPERTY(
		VisibleAnywhere, 
		BlueprintReadOnly, 
		Category = "Enemy",
		meta = (AllowPrivateAccess = true)
	)
	TObjectPtr<UCWSHealthComponent> HealthComponent;

	UFUNCTION()
	void HandleDeath(AActor* DeadActor);
};
