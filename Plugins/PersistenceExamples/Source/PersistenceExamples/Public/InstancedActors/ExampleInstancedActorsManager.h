// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedActorsManager.h"
#include "ExampleInstancedActorsManager.generated.h"

/**
 * Example manager that spawns UExampleInstancedActorsData instead of the base UInstancedActorsData,
 * so dehydrated instances recover their Visual State on late-joining clients (see UExampleInstancedActorsData).
 *
 * Managers are spawned at EDITOR conversion time (Convert Actors to IAs) by UInstancedActorsSubsystem, which
 * reads its InstancedActorsManagerClass. The manager and its IADs are then serialized into the map
 * (AInstancedActorsManager::PerActorClassInstanceData is an Instanced, non-transient UPROPERTY), so the IAD
 * class is baked in at conversion time. UExampleInstancedActorsSubsystem points InstancedActorsManagerClass at
 * this class; the project settings point the active subsystem at it (see Config/DefaultInstancedActors.ini).
 *
 * NOTE: this only affects NEW conversions. JumpBlocks already converted/saved as base classes must be
 * re-converted in-editor (with the config active) for the override to take effect.
 */
UCLASS()
class PERSISTENCEEXAMPLES_API AExampleInstancedActorsManager : public AInstancedActorsManager
{
	GENERATED_BODY()

public:
	AExampleInstancedActorsManager();
};
