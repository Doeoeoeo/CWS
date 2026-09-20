#pragma once

#include "CoreMinimal.h"
#include "CWSPipeTypes.h"
#include "CWSPipeDefinition.generated.h"

USTRUCT(BlueprintType)
struct FCWSPipeDefinition
{
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Flow"
	)
	FIntPoint LocalCoordinate = FIntPoint::ZeroValue;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Flow"
	)
	ECWSPipeDirection Direction = ECWSPipeDirection::Down;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Flow"
	)
	ECWSPipeType Type = ECWSPipeType::Closed;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Flow",
		meta = (ClampMin = "0")
	)
	float OutputWeight = 1.0f;
};