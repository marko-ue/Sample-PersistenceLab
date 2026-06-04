// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/TimeAliveMassProcessor.h"
#include "Mass/ExampleMassFragments.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "DrawDebugHelpers.h"

UTimeAliveMassProcessor::UTimeAliveMassProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UTimeAliveMassProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	EntityQuery.AddRequirement<FTimeAliveFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);
}

void UTimeAliveMassProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = Context.GetDeltaTimeSeconds();

	UWorld* World = EntityManager.GetWorld();

	EntityQuery.ForEachEntityChunk(Context, [DeltaTime, World](FMassExecutionContext& Ctx)
	{
		TArrayView<FTimeAliveFragment> TimeAliveList = Ctx.GetMutableFragmentView<FTimeAliveFragment>();
		TConstArrayView<FTransformFragment> TransformList = Ctx.GetFragmentView<FTransformFragment>();

		for (int32 i = 0; i < Ctx.GetNumEntities(); ++i)
		{
			TimeAliveList[i].TimeAlive += DeltaTime;

//#if !UE_BUILD_SHIPPING
//			const FVector Location = TransformList[i].GetTransform().GetLocation() + FVector(0.f, 0.f, 50.f);
//			DrawDebugString(World, Location, FString::Printf(TEXT("%.1fs"), TimeAliveList[i].TimeAlive), nullptr, FColor::White, 0.f, true);
//#endif
		}
	});
}
