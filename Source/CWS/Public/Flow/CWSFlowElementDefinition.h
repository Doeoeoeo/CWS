#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CWSPipeDefinition.h"
#include "CWSFlowElementDefinition.generated.h"

UCLASS()
class CWS_API UCWSFlowElementDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Element"
	)
	FText DisplayName;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Element"
	)
	TArray<FIntPoint> FootprintCells;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Element"
	)
	TArray<FCWSPipeDefinition> Pipes;
};
