// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PersistentTimerComponent.generated.h"

UCLASS(ClassGroup=(PersistenceExamples), meta=(BlueprintSpawnableComponent))
class PERSISTENCEEXAMPLES_API UPersistentTimerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPersistentTimerComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Property path is added in DefaultEngine.ini so its persisted on map placed, spatially partitioned actors
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Timer")
	float TimeAlive = 0.0f;
};
