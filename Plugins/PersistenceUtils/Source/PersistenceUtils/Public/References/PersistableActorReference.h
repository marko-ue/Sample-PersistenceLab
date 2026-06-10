// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "PersistableActorReference.generated.h"

class AActor;
class UWorld;

/** Discriminator for FPersistableActorReference. */
UENUM(BlueprintType)
enum class EPersistableActorReferenceType : uint8
{
	None,
	/** Any actor that belongs to a ULevel (map-placed or runtime-spawned). Identified by (LevelPath, ActorName). */
	LevelActor,
	PlayerPawn,
	PlayerController,
};

/**
 * Serializable hint for resolving a runtime actor reference across sessions. The reference is not a direct
 * UObject pointer (which would not survive serialization across maps/loads); instead the struct stores a
 * discriminator plus enough information to re-look-up the actor in a later world.
 *
 * For LevelActor, resolution goes through UPersistableActorReferenceManager which checks its registry
 * (populated by UPersistableReferencedActorComponent on actors whose names aren't stable across sessions)
 * and falls back to scanning the owning level by FName (covers map-placed actors).
 */
USTRUCT(BlueprintType)
struct PERSISTENCEUTILS_API FPersistableActorReference
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Persistence")
	EPersistableActorReferenceType Type = EPersistableActorReferenceType::None;

	/** Set when Type == LevelActor. Soft path to the level that owns/owned the actor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Persistence")
	FSoftObjectPath LevelPath;

	/** Set when Type == LevelActor. The actor's FName at save time (stable for map-placed, previous-session name for runtime). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Persistence")
	FName ActorName;

	/** Set when Type == PlayerPawn or PlayerController. Index into the world's player array. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Persistence")
	int32 RuntimeIndex = INDEX_NONE;

	/** Resolves the live actor in the given world, or returns nullptr if it cannot yet be found. */
	AActor* TryResolve(UWorld* World) const;

	/** Classifies Actor and populates this reference. Pass nullptr to reset to None. */
	bool SetFromActor(AActor* Actor);
};
