// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CWSGameMode.generated.h"

/**
 * Base game mode for CWS.
 *
 * The player pawn will be assigned when the top-down character is implemented.
 */
UCLASS()
class CWS_API ACWSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACWSGameMode();
};
