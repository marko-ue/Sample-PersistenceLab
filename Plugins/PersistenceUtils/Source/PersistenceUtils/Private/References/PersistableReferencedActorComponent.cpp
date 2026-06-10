// Copyright Epic Games, Inc. All Rights Reserved.

#include "References/PersistableReferencedActorComponent.h"

#include "Engine/Level.h"
#include "Engine/World.h"
#include "References/PersistableActorReferenceManager.h"
#include "GameFramework/Actor.h"
#include "PersistenceUtils.h"

UPersistableReferencedActorComponent::UPersistableReferencedActorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPersistableReferencedActorComponent::PrePersistObject_Implementation()
{
	AActor* Owner = GetOwner();
	ULevel* Level = Owner ? Owner->GetLevel() : nullptr;
	if (!Owner || !Level)
	{
		return;
	}

	LastSessionLevelPath = FSoftObjectPath(Level);
	LastSessionActorName = Owner->GetFName();
}

void UPersistableReferencedActorComponent::PostRestoreObject_Implementation(const TArray<FName>& RestoredPropertyNames)
{
	if (LastSessionActorName.IsNone() || LastSessionLevelPath.IsNull())
	{
		return;
	}

	AActor* Owner = GetOwner();
	UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	UPersistableActorReferenceManager* Mgr = World ? World->GetSubsystem<UPersistableActorReferenceManager>() : nullptr;
	if (!Mgr)
	{
		UE_LOG(LogPersistenceUtils, Warning, TEXT("UPersistableReferencedActorComponent::PostRestoreObject: Manager missing on '%s'."), *GetPathNameSafe(Owner));
		return;
	}

	Mgr->RegisterActor(LastSessionLevelPath, LastSessionActorName, Owner);
	bRegisteredWithManager = true;
}

void UPersistableReferencedActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bRegisteredWithManager)
	{
		if (UWorld* World = GetWorld())
		{
			if (UPersistableActorReferenceManager* Mgr = World->GetSubsystem<UPersistableActorReferenceManager>())
			{
				Mgr->UnregisterActor(LastSessionLevelPath, LastSessionActorName);
			}
		}
		bRegisteredWithManager = false;
	}

	Super::EndPlay(EndPlayReason);
}
