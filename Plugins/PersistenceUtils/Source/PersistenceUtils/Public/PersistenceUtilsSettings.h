// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PersistenceUtilsSettings.generated.h"

UCLASS(config=Engine, defaultconfig, meta=(DisplayName="Persistence Utils", CategoryName="Plugins"))
class PERSISTENCEUTILS_API UPersistenceUtilsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Whether to flush the current map's data to the active SaveGame when leaving maps. Doesn't write to disk, but updates the SaveGame object in memory
	// which applies to next time loading the level in the same session. If you then save it to disk, it also applies to future sessions.
	UPROPERTY(config, EditAnywhere, Category="Persistence")
	bool bAutoSaveWhenLeavingMap = true;

	// Whether to save and restore the player pawn's transform when traveling to a saved map.
	UPROPERTY(config, EditAnywhere, Category="Persistence")
	bool bRestorePawnTransform = true;

	// Whether to save and restore the player controller's control rotation when traveling to a saved map.
	UPROPERTY(config, EditAnywhere, Category="Persistence")
	bool bRestoreControlRotation = true;

	// At save-time, ALL Mass Entities are searched to see which have been spawned by MassEntityConfigAssets with the 
	// UPersistableEntityConfigTrait. For those that have that trait, the fragments of these types are serialized.
	// At restore-time, entities are respawned using the config asset and then the values of these fragment types are 
	// deserialized back onto the entity fragments.
	UPROPERTY(config, EditAnywhere, Category="Mass", meta=(GetOptions="GetMassFragmentOptions"))
	TSet<FName> MassFragmentsToSerialize;

protected:
	// Returns the set of FMassFragment-derived USTRUCT path names available in the loaded modules.
	// Used by the editor to populate the picker for MassFragmentsToSerialize.
	UFUNCTION()
	TArray<FString> GetMassFragmentOptions() const;
};
