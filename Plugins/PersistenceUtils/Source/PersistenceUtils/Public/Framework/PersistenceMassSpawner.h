// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassSpawner.h"
#include "PersistenceMassSpawner.generated.h"

/**
 * A Mass spawner that skips spawning if ShouldSpawnEntities() returns false.
 * On first spawn, sets bHasEverSpawned = true after DoSpawning completes.
 */
UCLASS(Blueprintable)
class PERSISTENCEUTILS_API APersistenceMassSpawner : public AMassSpawner
{
	GENERATED_BODY()

public:
	APersistenceMassSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Returns true if entities should be spawned on BeginPlay.
	// Default implementation returns !bHasEverSpawned.
	// Override to implement custom checks, e.g. polling from a USaveGame.
	virtual bool ShouldSpawnEntities() const;

	UFUNCTION()
	void OnSpawningFinished();

	// Iterates AllSpawnedEntities, resolves each batch's UMassEntityConfigAsset by matching template ID against
	// EntityTypes, and adds FMassSpawnedFromEntityConfigFragment (with the config's soft path) to each entity.
	// Run from OnSpawningFinished so the fragment is in place before any save flush happens.
	void StampOriginFragmentOnSpawnedEntities();

	// To avoid double spawning, this property should be persisted and restored before BeginPlay.
	// For example, use the Level Streaming Persistence plugin to persist spatially-loaded instances
	// of this actor and add this section to your DefaultEngine.ini:
	//
	// [/Script/LevelStreamingPersistence.LevelStreamingPersistenceSettings]
	// +Properties=(Path="/Script/PersistenceUtils.PersistenceMassSpawner:bHasEverSpawned",bIsPublic=False)
	//
	// Alternatively, override ShouldSpawnEntities() to implement custom checks,
	// like polling from your custom USaveGame.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Persistence")
	bool bHasEverSpawned = false;
};
