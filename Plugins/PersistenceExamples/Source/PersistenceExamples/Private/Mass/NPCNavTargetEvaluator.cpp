// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/NPCNavTargetEvaluator.h"
#include "MassStateTreeDependency.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FNPCNavTargetEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(NavTargetHandle);
	return true;
}

void FNPCNavTargetEvaluator::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
	Builder.AddReadOnly(NavTargetHandle);
}

void FNPCNavTargetEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FNPCNavTargetFragment& Nav = Context.GetExternalData(NavTargetHandle);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	InstanceData.bHasValidTarget = Nav.bValid;
	InstanceData.TargetLocation.Reset();
	if (Nav.bValid)
	{
		InstanceData.TargetLocation.EndOfPathPosition = Nav.Target;
		InstanceData.TargetLocation.EndOfPathIntent = EMassMovementAction::Move;
	}
}
