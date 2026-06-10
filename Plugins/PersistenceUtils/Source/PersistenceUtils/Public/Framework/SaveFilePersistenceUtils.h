// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/PersistenceSaveGame.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveFilePersistenceUtils.generated.h"

UCLASS()
class PERSISTENCEUTILS_API USaveFilePersistenceUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Creates a new UPersistenceSaveGame instance of the given class, registers it with UPersistenceGameSubsystem, and returns it.
	// SpecificClass must be provided and must be UPersistenceSaveGame or a subclass; returns null otherwise.
	// If SlotName is empty, "DefaultSlot" is used.
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UPersistenceSaveGame* StartNewSaveFile(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex, TSubclassOf<UPersistenceSaveGame> SpecificClass);

	// Loads a UPersistenceSaveGame from the given slot and registers it with UPersistenceGameSubsystem.
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UPersistenceSaveGame* LoadFromFile(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex);

	// Reloads the active UPersistenceSaveGame from its stored slot, discarding any unsaved changes.
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UPersistenceSaveGame* ReloadFromFile(const UObject* WorldContextObject, bool bStartTravel = true);

	// Saves the current game state to disk using the slot stored on the active UPersistenceSaveGame instance.
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void SaveToFile(const UObject* WorldContextObject);

	// Triggers travel to the map stored in the active SaveGame's travel data.
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void TravelFromSaveFile(const UObject* WorldContextObject);
};
