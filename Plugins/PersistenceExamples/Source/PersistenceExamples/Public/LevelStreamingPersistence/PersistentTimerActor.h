// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelStreamingPersistence/PersistentTimerComponent.h"
#include "PersistentTimerActor.generated.h"

UCLASS()
class PERSISTENCEEXAMPLES_API APersistentTimerActor : public AActor
{
	GENERATED_BODY()

public:
	APersistentTimerActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Timer")
	TObjectPtr<UPersistentTimerComponent> TimerComponent;

	// Property path is added in DefaultEngine.ini so its persisted on map placed, spatially partitioned actors
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Timer")
	float TimeAlive = 0.0f;
};
