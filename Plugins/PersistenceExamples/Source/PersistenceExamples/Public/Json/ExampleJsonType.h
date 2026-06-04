// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ExampleJsonType.generated.h"

USTRUCT(BlueprintType)
struct PERSISTENCEEXAMPLES_API FExampleJsonDataStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JSON Demo")
	float Health = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JSON Demo")
	float MaxHealth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JSON Demo")
	FGameplayTag TargetID;
};
