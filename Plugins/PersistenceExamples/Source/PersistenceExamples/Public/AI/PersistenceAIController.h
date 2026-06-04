// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PersistenceAIController.generated.h"

class UGameplayTasksComponent;
class UPersistedStateTreeComponent;

/**
 * AIController that owns a UPersistedStateTreeComponent. Hosting the component on the controller
 * (rather than the pawn) lets UStateTreeAIComponent's AIOwner resolve correctly via the base
 * UBrainComponent::OnRegister path.
 *
 * The controller itself is intentionally passive: the possessed pawn is responsible for driving
 * StartLogic / StopLogic / ForceTransition through the component, so this module avoids depending
 * on any specific pawn class.
 */
UCLASS()
class PERSISTENCEEXAMPLES_API APersistenceAIController : public AAIController
{
	GENERATED_BODY()

public:
	APersistenceAIController();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UPersistedStateTreeComponent> StateTreeComponent;

	/** Hosts AI gameplay tasks (e.g., UAITask_MoveTo). Without this on the pawn or controller, AI MoveTo fails to start. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UGameplayTasksComponent> GameplayTasksComponent;
};
