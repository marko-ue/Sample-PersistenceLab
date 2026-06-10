// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/PersistedObjectInterface.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "PersistedGeometryCollectionComponent.generated.h"

USTRUCT(BlueprintType)
struct FPersistedGeometryCollectionState
{
	GENERATED_BODY()

	/**
	 * Per-piece transforms in DynamicCollection space at save time. For pieces still attached to
	 * a parent these are parent-relative; for pieces broken loose (parent == INDEX_NONE) they are
	 * component-relative. Read via FGeometryDynamicCollection::GetTransform; round-trips through
	 * SetInitialTransforms which writes back into the same DC slot.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Persistence")
	TArray<FTransform> PieceTransforms;

	/**
	 * Transform indices whose parent flag was severed at save time, i.e. broken loose from their
	 * authored parent. Computed at save by comparing GetParent(i) against GetParentArrayRest()[i];
	 * fed straight back to SetInitialClusterBreaks on restore.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Persistence")
	TArray<int32> BrokenIndices;
};

/**
 * Geometry collection component that survives save/load by capturing per-piece transforms and
 * per-piece broken state on PrePersistObject, then replaying them via the engine's "replication
 * helper" path on PostRestoreObject: SetRestState (-> SetInitialTransforms on DC) +
 * SetInitialClusterBreaks (-> per-piece parent flag flips on DC) + RecreatePhysicsState (rebuilds
 * the physics proxy from the now-mutated DC). Mirrors the engine's own ResetDynamicCollection +
 * RecreatePhysicsState pattern at GeometryCollectionComponent.cpp:5131-5134.
 *
 * Supports arbitrary fracture depth: any subset of cluster pieces can be marked broken, and
 * RecreatePhysicsState builds the proxy with that exact decomposition. No flatten-everything
 * fallback.
 *
 * Velocity is not persisted; pieces resume from a stop. Acceptable when saves are quiescent.
 */
UCLASS(ClassGroup=(Persistence), meta=(BlueprintSpawnableComponent))
class PERSISTENCEEXAMPLES_API UPersistedGeometryCollectionComponent : public UGeometryCollectionComponent, public IPersistedObject
{
	GENERATED_BODY()

public:
	/** Snapshot of fracture state. Written by PrePersistObject, persisted by LSP, applied by PostRestoreObject. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Persistence", SaveGame)
	FPersistedGeometryCollectionState PersistedState;

	//~ IPersistedObject
	virtual void PrePersistObject_Implementation() override;
	virtual void PostRestoreObject_Implementation(const TArray<FName>& RestoredPropertyNames) override;
};
