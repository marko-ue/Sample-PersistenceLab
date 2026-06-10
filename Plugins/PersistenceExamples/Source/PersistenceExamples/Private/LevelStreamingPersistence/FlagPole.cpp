// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelStreamingPersistence/FlagPole.h"
#include "References/PersistableReferencedActorComponent.h"

AFlagPole::AFlagPole()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	PersistableRef = CreateDefaultSubobject<UPersistableReferencedActorComponent>(TEXT("PersistableRef"));
}
