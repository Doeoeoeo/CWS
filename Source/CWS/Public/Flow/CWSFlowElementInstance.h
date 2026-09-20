#pragma once

#include "CoreMinimal.h"
#include "CWSFlowElementDefinition.h"
#include "CWSFlowElementInstance.generated.h"

USTRUCT(BlueprintType)
struct FCWSFlowElementInstance
{
    GENERATED_BODY()
    
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Flow"
    )
    FGuid InstanceId;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Flow"
    )
    TObjectPtr<UCWSFlowElementDefinition> Definition = nullptr;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Flow"
    )
    FIntPoint GridOrigin = FIntPoint::ZeroValue;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Flow"
    )
    uint8 RotationSteps = 0;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Transient,
        Category = "Flow"
    )
    TArray<FIntPoint> PlacedCells;
    
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Flow"
    )
    bool bIsPlaced = false;

};