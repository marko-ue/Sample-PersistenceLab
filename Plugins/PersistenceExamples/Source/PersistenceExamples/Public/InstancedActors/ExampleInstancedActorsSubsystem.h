// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedActorsSubsystem.h"
#include "ExampleInstancedActorsSubsystem.generated.h"

/**
 * Example InstancedActors subsystem that spawns AExampleInstancedActorsManager instead of the base manager.
 *
 * The InstancedActors framework resolves "the" subsystem by the class configured in
 * UInstancedActorsProjectSettings::InstancedActorsSubsystemClass (all registration and processors fetch it via
 * UE::InstancedActors::Utils::GetInstancedActorsSubsystem). Point that setting at this class to make it the
 * active subsystem - see Config/DefaultInstancedActors.ini.
 */
UCLASS()
class PERSISTENCEEXAMPLES_API UExampleInstancedActorsSubsystem : public UInstancedActorsSubsystem
{
	GENERATED_BODY()

public:
	UExampleInstancedActorsSubsystem();
};
