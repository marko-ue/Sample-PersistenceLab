// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "TestActorForJson.generated.h"

UCLASS()
class PERSISTENCEEXAMPLES_API ATestActorForJson : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	float GetHealth() const { return 50.0f; }
	float GetMaxHealth() const { return 100.0f; }
	FGameplayTag GetTargetID() const { return FGameplayTag::RequestGameplayTag(FName(TEXT("NPC.Demo.Json")), /*ErrorIfNotFound=*/false); }
};
