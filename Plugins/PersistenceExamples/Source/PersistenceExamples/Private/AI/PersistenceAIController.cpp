// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/PersistenceAIController.h"

#include "AI/PersistedStateTreeComponent.h"
#include "GameplayTasksComponent.h"

APersistenceAIController::APersistenceAIController()
{
	StateTreeComponent = CreateDefaultSubobject<UPersistedStateTreeComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetStartLogicAutomatically(false);

	// Although a GameplayTasksComponent will be created on-demand when the AIController possesses a pawn,
	// we'll create it ahead of time so that during the possession callbacks/overrides, we can already call 
	// ForceTransition on the StateTreeComponent and any gameplay tasks firing from restoring a state can
	// already be executed by this GameplayTasksComponent.
	GameplayTasksComponent = CreateDefaultSubobject<UGameplayTasksComponent>(TEXT("GameplayTasksComponent"));
	CachedGameplayTasksComponent = GameplayTasksComponent;
}
