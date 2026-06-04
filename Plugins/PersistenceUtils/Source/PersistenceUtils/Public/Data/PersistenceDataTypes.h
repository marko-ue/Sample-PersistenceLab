// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "MassEntityTypes.h"
#include "PersistenceDataTypes.generated.h"

// Serialized state for a single InstancedActorManager. Mainly a wrapper around TArray<uint8> since that cannot directly be a TMap value-type.
USTRUCT()
struct PERSISTENCEUTILS_API FInstancedActorManagerState
{
	GENERATED_BODY()

	// Serialized delta data produced by the InstancedActorManager.
	UPROPERTY()
	TArray<uint8> Data;
};

// Struct that encapsulates persistence data for a single World Partition streaming level.
USTRUCT()
struct PERSISTENCEUTILS_API FStreamingLevelPersistenceEntry
{
	GENERATED_BODY()

	// Serialized delta data per InstancedActorManager, keyed by manager name.
	UPROPERTY()
	TMap<FName, FInstancedActorManagerState> InstancedActorManagerDeltas;
};

// Opt-in marker tag: only Mass entities with this tag will be included in snapshot serialization.
// Entities owned by InstancedActors (IAM) do not carry this tag, preventing double-persistence.
USTRUCT()
struct PERSISTENCEUTILS_API FMassPersistenceSnapshotTag : public FMassTag
{
	GENERATED_BODY()
};

// Layout descriptor for a single fragment type, recorded at save time.
USTRUCT()
struct PERSISTENCEUTILS_API FMassFragmentLayout
{
	GENERATED_BODY()

	// Soft path to the UScriptStruct type (e.g. "/Script/MyModule.FMyFragment").
	UPROPERTY()
	FSoftObjectPath Type;

	// Byte size of this fragment at save time (UScriptStruct::GetStructureSize()).
	// Used at load time to skip past fragments that have been removed or whose layout has changed.
	UPROPERTY()
	int32 SizeInBytes = 0;
};

// Self-describing snapshot of all entities sharing one UMassEntityConfigAsset.
// FragmentLayout defines the byte layout of Data; SourceConfigAsset drives respawn at restore.
USTRUCT()
struct PERSISTENCEUTILS_API FMassEntityConfigGroupSnapshot
{
	GENERATED_BODY()

	// Ordered layout entries for each fragment type serialized for this config at save time.
	// Determines the schema for reading Data back: each entity occupies sum(Entry.SizeInBytes) bytes.
	// At load time, entries whose type can no longer be resolved (or whose size has changed) are skipped.
	UPROPERTY()
	TArray<FMassFragmentLayout> FragmentLayout;

	// Number of entities serialized. Used to drive spawning on restore.
	UPROPERTY()
	int32 EntityCount = 0;

	// Raw fragment data: EntityCount x (all FragmentTypes in order), written by FMemoryWriter.
	UPROPERTY()
	TArray<uint8> Data;

	// The UMassEntityConfigAsset this snapshot originated from. Required: restore calls
	// SpawnEntities(Template, N) via this config. Snapshots whose config no longer resolves are skipped.
	UPROPERTY()
	FSoftObjectPath SourceConfigAsset;
};

// Struct that encapsulates persistence data for a World Partitioned map.
USTRUCT()
struct PERSISTENCEUTILS_API FWorldPersistenceEntry
{
	GENERATED_BODY()

	// Serialized persistent property data for all World Partition streaming levels, produced by ULevelStreamingPersistenceManager::SerializeTo.
	UPROPERTY()
	TArray<uint8> StreamingLevelData;

	// Persistence data keyed by streaming level package name (sourced from ULevelStreaming::GetWorldAssetPackageFName to ensure PIE-stability).
	UPROPERTY()
	TMap<FName, FStreamingLevelPersistenceEntry> SavedStatePerStreamingLevel;

	// Serialized Mass entity snapshots, one per distinct UMassEntityConfigAsset.
	UPROPERTY()
	TArray<FMassEntityConfigGroupSnapshot> MassEntitySnapshots;
};

// Stores the data required to travel to a map and restore the pawn's position.
USTRUCT()
struct PERSISTENCEUTILS_API FPersistenceTravelData
{
	GENERATED_BODY()

	// The map to travel to. Uses FSoftObjectPath to allow asset loading.
	UPROPERTY()
	FSoftObjectPath Map;

	// Whether PawnTransform contains a valid saved transform.
	UPROPERTY()
	bool bHasPawnTransform = false;

	// The location of the player-pawn at save-time.
	UPROPERTY()
	FTransform PawnTransform = FTransform::Identity;

	// Whether ControlRotation contains a valid saved rotation.
	UPROPERTY()
	bool bHasControlRotation = false;

	// The player controller's control rotation at save-time.
	UPROPERTY()
	FRotator ControlRotation = FRotator::ZeroRotator;
};
