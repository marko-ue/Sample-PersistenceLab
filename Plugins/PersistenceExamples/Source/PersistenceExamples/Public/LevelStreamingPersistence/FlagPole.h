// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlagPole.generated.h"

class UPersistableReferencedActorComponent;

UCLASS()
class PERSISTENCEEXAMPLES_API AFlagPole : public AActor
{
	GENERATED_BODY()

public:
	AFlagPole();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FlagPole")
	TObjectPtr<USceneComponent> SceneRoot;

	// Gives this actor a stable cross-session identity (LSP captures the level path + the actor's FName at save
	// time, restores them on load). Other actors persist a FPersistableActorReference pointing here and resolve
	// through UPersistableActorReferenceManager.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FlagPole")
	TObjectPtr<UPersistableReferencedActorComponent> PersistableRef;
};
