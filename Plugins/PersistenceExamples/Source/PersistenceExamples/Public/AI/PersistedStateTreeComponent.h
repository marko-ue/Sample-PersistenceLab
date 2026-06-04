// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeAIComponent.h"
#include "StateTreeExecutionTypes.h"
#include "PersistedStateTreeComponent.generated.h"

enum class EPropertyBagResult : uint8;

/**
 * UStateTreeAIComponent subclass adding accessors for persistence: reading the active leaf state's
 * GUID, and forcing the tree into a state identified by GUID. Both require access to the protected
 * FStateTreeInstanceData on the base class, which is only reachable from a subclass.
 *
 * Intended to live on an AAIController (UStateTreeAIComponent's design assumption); when it does,
 * UBrainComponent::OnRegister automatically resolves AIOwner from GetOwner().
 */
UCLASS(ClassGroup = AI, meta = (BlueprintSpawnableComponent))
class PERSISTENCEEXAMPLES_API UPersistedStateTreeComponent : public UStateTreeAIComponent
{
	GENERATED_BODY()

public:
	/** Invalid FGuid if the tree is not running or has no active states. */
	UFUNCTION(BlueprintPure, Category = "Persistence|StateTree")
	FGuid GetActiveLeafStateId();

	/**
	 * Forces the running tree into the state identified by StateId, bypassing enter conditions
	 * and entering all ancestor states. Must be called only when the tree is running and not mid-tick.
	 * Returns the resulting run status, or Failed if the GUID could not be resolved or the tree is not running.
	 */
	UFUNCTION(BlueprintCallable, Category = "Persistence|StateTree")
	EStateTreeRunStatus ForceTransitionToState(const FGuid StateId);

	/** Reads a UObject-typed global parameter by name. Returns nullptr if the bag is uninitialized or the name is unknown. */
	UObject* GetGlobalObject(FName Name) const;

	/** Writes a UObject-typed global parameter by name. Returns the underlying property bag result for diagnostics. */
	EPropertyBagResult SetGlobalObject(FName Name, UObject* Object);
};
