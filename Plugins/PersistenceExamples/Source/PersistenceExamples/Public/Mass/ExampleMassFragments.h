// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "ExampleMassFragments.generated.h"

/** Tracks how long this Mass entity has been alive, in seconds. Incremented by UTimeAliveMassProcessor. */
USTRUCT()
struct FTimeAliveFragment : public FMassFragment
{
	GENERATED_BODY()

	float TimeAlive = 0.f;
};
