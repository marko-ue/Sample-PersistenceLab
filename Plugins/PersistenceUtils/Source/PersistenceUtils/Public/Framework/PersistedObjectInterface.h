// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PersistedObjectInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UPersistedObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * Per-object hooks for LevelStreamingPersistence save/restore.
 *
 * Implementers receive a callback immediately before LSP serializes their
 * SaveGame properties, and again after those properties are restored. The
 * pre-persist callback is the place to convert transient runtime state into
 * UPROPERTY(SaveGame) members. The post-restore callback fires after the
 * restored values are written to the object; for map-placed actors it runs
 * before BeginPlay, so heavier reactions that depend on full actor
 * initialization should be deferred (e.g. to BeginPlay reading the now-restored
 * properties).
 */
class PERSISTENCEUTILS_API IPersistedObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Persistence")
	void PrePersistObject();
	virtual void PrePersistObject_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Persistence")
	void PostRestoreObject(const TArray<FName>& RestoredPropertyNames);
	virtual void PostRestoreObject_Implementation(const TArray<FName>& RestoredPropertyNames) {}
};
