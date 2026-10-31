#pragma once

#include "CoreMinimal.h"
#include "CWSPipeTypes.h"
#include "CWSResolvedPipe.generated.h"


USTRUCT(BlueprintType)
struct FCWSResolvedPipe
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Flow")
	FGuid InstanceId;
	
	UPROPERTY(BlueprintReadOnly, Category = "Flow")
	int32 PipeIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Flow")
	FIntPoint GridCell = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "Flow")
	ECWSPipeDirection Direction = ECWSPipeDirection::Down;

	UPROPERTY(BlueprintReadOnly, Category = "Flow")
	ECWSPipeType Type = ECWSPipeType::Closed;

	UPROPERTY(BlueprintReadOnly, Category = "Flow")
	float OutputWeight = 1.0f;
};