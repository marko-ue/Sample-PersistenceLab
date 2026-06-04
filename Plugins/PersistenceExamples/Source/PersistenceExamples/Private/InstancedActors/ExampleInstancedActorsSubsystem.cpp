// Copyright Epic Games, Inc. All Rights Reserved.

#include "InstancedActors/ExampleInstancedActorsSubsystem.h"
#include "InstancedActors/ExampleInstancedActorsManager.h"

UExampleInstancedActorsSubsystem::UExampleInstancedActorsSubsystem()
{
	// Base sets this to AInstancedActorsManager in its constructor; override it here so runtime-spawned
	// managers use our subclass (which in turn spawns UExampleInstancedActorsData).
	InstancedActorsManagerClass = AExampleInstancedActorsManager::StaticClass();
}
