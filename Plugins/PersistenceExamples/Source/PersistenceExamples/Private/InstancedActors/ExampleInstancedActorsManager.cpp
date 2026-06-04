// Copyright Epic Games, Inc. All Rights Reserved.

#include "InstancedActors/ExampleInstancedActorsManager.h"
#include "InstancedActors/ExampleInstancedActorsData.h"

AExampleInstancedActorsManager::AExampleInstancedActorsManager()
{
	// AInstancedActorsManager::CreateNextInstanceActorData NewObject's this class for every per-actor-class instance group.
	InstancedActorsDataClass = UExampleInstancedActorsData::StaticClass();
}
