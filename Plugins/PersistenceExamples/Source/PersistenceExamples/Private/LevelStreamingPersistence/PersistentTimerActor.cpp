// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelStreamingPersistence/PersistentTimerActor.h"
#include "PersistenceExamples.h"

APersistentTimerActor::APersistentTimerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	TimerComponent = CreateDefaultSubobject<UPersistentTimerComponent>(TEXT("TimerComponent"));
}

void APersistentTimerActor::BeginPlay()
{
	Super::BeginPlay();

	if (TimeAlive == 0.0f)
	{
		UE_LOG(LogPersistenceExamples, Warning, TEXT("APersistentTimerActor '%s': Starting with a fresh TimeAlive."), *GetName());
	}
	else
	{
		UE_LOG(LogPersistenceExamples, Warning, TEXT("APersistentTimerActor '%s': Using a restored TimeAlive of %.2f."), *GetName(), TimeAlive);
	}
}

void APersistentTimerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeAlive += DeltaTime;
}
