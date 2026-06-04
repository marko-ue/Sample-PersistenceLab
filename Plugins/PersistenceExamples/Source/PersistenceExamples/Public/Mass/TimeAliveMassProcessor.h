// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TimeAliveMassProcessor.generated.h"

/** Increments FTimeAliveFragment::TimeAlive by DeltaTime each tick for all matching entities. */
UCLASS()
class PERSISTENCEEXAMPLES_API UTimeAliveMassProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UTimeAliveMassProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
