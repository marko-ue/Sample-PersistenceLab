// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UPersistenceGameSubsystem;
class UPersistenceSaveGame;

struct FPersistenceUtilsDelegates
{
	// Broadcast before the SaveGame is written to disk. Use this to flush any pending state into the SaveGame.
	DECLARE_MULTICAST_DELEGATE_TwoParams(FPreSave, UPersistenceGameSubsystem*, UPersistenceSaveGame*);
	static PERSISTENCEUTILS_API FPreSave OnPreSave;
};
