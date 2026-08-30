// Copyright Epic Games, Inc. All Rights Reserved.

#include "CWSGameMode.h"
#include "Player/CWSPlayerCharacter.h"

ACWSGameMode::ACWSGameMode()
{
	// Avoid spawning Unreal's default flying pawn while the 2D player is not yet defined.
	DefaultPawnClass = ACWSPlayerCharacter::StaticClass();
}
