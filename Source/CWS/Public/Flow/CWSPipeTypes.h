// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CWSPipeTypes.generated.h"

UENUM(BlueprintType)
enum class ECWSPipeDirection : uint8
{
	Up,
	Right,
	Down,
	Left,
};

UENUM(BlueprintType)
enum class ECWSPipeType : uint8
{
	Closed,
	Input,
	Output,
	Bidirectional
};




