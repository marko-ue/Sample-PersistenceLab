// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/NPCActorTransformToMassTranslator.h"
#include "Mass/NPCMassFragments.h"
#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassEntityManager.h"

UNPCActorTransformToMassTranslator::UNPCActorTransformToMassTranslator()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::SyncWorldToMass;
	bRequiresGameThreadExecution = true;
}

void UNPCActorTransformToMassTranslator::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FNPCHydratedTag>(EMassFragmentPresence::All);
}

void UNPCActorTransformToMassTranslator::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	//EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	//{
	//	const TConstArrayView<FMassActorFragment> ActorList = Context.GetFragmentView<FMassActorFragment>();
	//	const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
	//
	//	for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
	//	{
	//		if (const AActor* Actor = ActorList[EntityIt].Get())
	//		{
	//			TransformList[EntityIt].GetMutableTransform() = Actor->GetActorTransform();
	//		}
	//	}
	//});
}
