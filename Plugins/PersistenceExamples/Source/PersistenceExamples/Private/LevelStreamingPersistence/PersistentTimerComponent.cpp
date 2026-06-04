// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelStreamingPersistence/PersistentTimerComponent.h"
#include "PersistenceExamples.h"

UPersistentTimerComponent::UPersistentTimerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPersistentTimerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (TimeAlive == 0.0f)
	{
		UE_LOG(LogPersistenceExamples, Warning, TEXT("UPersistentTimerComponent '%s' on '%s': Starting with a fresh TimeAlive."), *GetName(), *GetOwner()->GetName());
	}
	else
	{
		UE_LOG(LogPersistenceExamples, Warning, TEXT("UPersistentTimerComponent '%s' on '%s': Using a restored TimeAlive of %.2f."), *GetName(), *GetOwner()->GetName(), TimeAlive);
	}
}

void UPersistentTimerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeAlive += DeltaTime;
}
