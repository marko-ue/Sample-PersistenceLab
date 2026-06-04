// Copyright Epic Games, Inc. All Rights Reserved.

#include "Libraries/SaveFilePersistenceUtils.h"
#include "Framework/PersistenceGameSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "PersistenceUtils.h"

static UPersistenceGameSubsystem* GetSubsystem(const UObject* WorldContextObject)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	return GameInstance ? GameInstance->GetSubsystem<UPersistenceGameSubsystem>() : nullptr;
}

UPersistenceSaveGame* USaveFilePersistenceUtils::StartNewSaveFile(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex, TSubclassOf<UPersistenceSaveGame> SpecificClass)
{
	UPersistenceGameSubsystem* Subsystem = GetSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		UE_LOG(LogPersistenceUtils, Warning, TEXT("StartNewSaveFile: Could not resolve UPersistenceGameSubsystem."));
		return nullptr;
	}

	UPersistenceSaveGame* Result = Subsystem->StartNewSaveFile(SlotName, UserIndex, SpecificClass);
	if (Result)
	{
		UE_LOG(LogPersistenceUtils, Display, TEXT("StartNewSaveFile: Started new save '%s' (UserIndex=%d, Class=%s)."), *SlotName, UserIndex, *GetNameSafe(Result->GetClass()));
	}
	return Result;
}

UPersistenceSaveGame* USaveFilePersistenceUtils::LoadFromFile(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex)
{
	UPersistenceGameSubsystem* Subsystem = GetSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		UE_LOG(LogPersistenceUtils, Warning, TEXT("LoadFromFile: Could not resolve UPersistenceGameSubsystem."));
		return nullptr;
	}

	UPersistenceSaveGame* Result = Subsystem->LoadFromFile(SlotName, UserIndex);
	if (Result)
	{
		UE_LOG(LogPersistenceUtils, Display, TEXT("LoadFromFile: Loaded save '%s' (UserIndex=%d)."), *SlotName, UserIndex);
	}
	return Result;
}

UPersistenceSaveGame* USaveFilePersistenceUtils::ReloadFromFile(const UObject* WorldContextObject, bool bStartTravel)
{
	UPersistenceGameSubsystem* Subsystem = GetSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		UE_LOG(LogPersistenceUtils, Warning, TEXT("ReloadFromFile: Could not resolve UPersistenceGameSubsystem."));
		return nullptr;
	}

	UPersistenceSaveGame* Result = Subsystem->ReloadFromFile(bStartTravel);
	if (Result)
	{
		UE_LOG(LogPersistenceUtils, Display, TEXT("ReloadFromFile: Reloaded active save (bStartTravel=%s)."), bStartTravel ? TEXT("true") : TEXT("false"));
	}
	return Result;
}

void USaveFilePersistenceUtils::SaveToFile(const UObject* WorldContextObject)
{
	UPersistenceGameSubsystem* Subsystem = GetSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		UE_LOG(LogPersistenceUtils, Warning, TEXT("SaveToFile: Could not resolve UPersistenceGameSubsystem."));
		return;
	}

	// SaveToFile is asynchronous: it returns before the disk write completes (it waits on a Mass entity
	// snapshot at the next phase boundary). The completion log is emitted by ContinueSaveToFileToDisk.
	Subsystem->SaveToFile();
}

void USaveFilePersistenceUtils::TravelFromSaveFile(const UObject* WorldContextObject)
{
	UPersistenceGameSubsystem* Subsystem = GetSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		UE_LOG(LogPersistenceUtils, Warning, TEXT("TravelFromSaveFile: Could not resolve UPersistenceGameSubsystem."));
		return;
	}

	Subsystem->TravelFromSaveFile();
	UE_LOG(LogPersistenceUtils, Display, TEXT("TravelFromSaveFile: Triggered travel to saved map."));
}

namespace UE::PersistenceUtils::ConsoleCommands
{
	static bool ParseBoolArg(const FString& Arg, bool& OutValue)
	{
		if (Arg.Equals(TEXT("1")) || Arg.Equals(TEXT("true"), ESearchCase::IgnoreCase))
		{
			OutValue = true;
			return true;
		}
		if (Arg.Equals(TEXT("0")) || Arg.Equals(TEXT("false"), ESearchCase::IgnoreCase))
		{
			OutValue = false;
			return true;
		}
		return false;
	}

	static TSubclassOf<UPersistenceSaveGame> ResolveSaveClass(const FString& ClassPath)
	{
		UClass* Found = FindObject<UClass>(nullptr, *ClassPath);
		if (!Found)
		{
			Found = LoadObject<UClass>(nullptr, *ClassPath);
		}
		if (Found && Found->IsChildOf(UPersistenceSaveGame::StaticClass()))
		{
			return Found;
		}
		return nullptr;
	}

	static void HandleNewSave(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogPersistenceUtils, Warning, TEXT("Usage: saveutils.newsave <slot> [userIndex=0] [saveClass]"));
			return;
		}

		const FString& SlotName = Args[0];
		const int32 UserIndex = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 0;

		TSubclassOf<UPersistenceSaveGame> SaveClass = UPersistenceSaveGame::StaticClass();
		if (Args.Num() > 2)
		{
			TSubclassOf<UPersistenceSaveGame> Resolved = ResolveSaveClass(Args[2]);
			if (!Resolved)
			{
				UE_LOG(LogPersistenceUtils, Warning, TEXT("saveutils.newsave: Could not resolve save class '%s' (must derive from UPersistenceSaveGame)."), *Args[2]);
				return;
			}
			SaveClass = Resolved;
		}

		USaveFilePersistenceUtils::StartNewSaveFile(World, SlotName, UserIndex, SaveClass);
	}

	static void HandleLoadFromFile(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() < 1)
		{
			UE_LOG(LogPersistenceUtils, Warning, TEXT("Usage: saveutils.loadfromfile <slot> [userIndex=0]"));
			return;
		}

		const FString& SlotName = Args[0];
		const int32 UserIndex = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 0;
		USaveFilePersistenceUtils::LoadFromFile(World, SlotName, UserIndex);
	}

	static void HandleReloadFromFile(const TArray<FString>& Args, UWorld* World)
	{
		bool bStartTravel = true;
		if (Args.Num() > 0 && !ParseBoolArg(Args[0], bStartTravel))
		{
			UE_LOG(LogPersistenceUtils, Warning, TEXT("saveutils.reloadfromfile: Could not parse '%s' as 0/1/true/false."), *Args[0]);
			return;
		}
		USaveFilePersistenceUtils::ReloadFromFile(World, bStartTravel);
	}

	static void HandleSaveToFile(const TArray<FString>& /*Args*/, UWorld* World)
	{
		USaveFilePersistenceUtils::SaveToFile(World);
	}

	static void HandleTravelFromSaveFile(const TArray<FString>& /*Args*/, UWorld* World)
	{
		USaveFilePersistenceUtils::TravelFromSaveFile(World);
	}
}

static FAutoConsoleCommandWithWorldAndArgs GSaveUtilsNewSaveCmd(
	TEXT("saveutils.newsave"),
	TEXT("Create a new save file. Usage: saveutils.newsave <slot> [userIndex=0] [saveClass]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UE::PersistenceUtils::ConsoleCommands::HandleNewSave));

static FAutoConsoleCommandWithWorldAndArgs GSaveUtilsLoadFromFileCmd(
	TEXT("saveutils.loadfromfile"),
	TEXT("Load a save file. Usage: saveutils.loadfromfile <slot> [userIndex=0]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UE::PersistenceUtils::ConsoleCommands::HandleLoadFromFile));

static FAutoConsoleCommandWithWorldAndArgs GSaveUtilsReloadFromFileCmd(
	TEXT("saveutils.reloadfromfile"),
	TEXT("Reload the active save, discarding unsaved changes. Usage: saveutils.reloadfromfile [bStartTravel=1]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UE::PersistenceUtils::ConsoleCommands::HandleReloadFromFile));

static FAutoConsoleCommandWithWorldAndArgs GSaveUtilsSaveToFileCmd(
	TEXT("saveutils.savetofile"),
	TEXT("Save the current state to the active save's slot. Usage: saveutils.savetofile"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UE::PersistenceUtils::ConsoleCommands::HandleSaveToFile));

static FAutoConsoleCommandWithWorldAndArgs GSaveUtilsTravelFromSaveFileCmd(
	TEXT("saveutils.travelfromsavefile"),
	TEXT("Travel to the map stored on the active save. Usage: saveutils.travelfromsavefile"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&UE::PersistenceUtils::ConsoleCommands::HandleTravelFromSaveFile));
