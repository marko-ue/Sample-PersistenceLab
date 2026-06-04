// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "MassNavigationTypes.h"
#include "Mass/NPCMassFragments.h"
#include "NPCNavTargetEvaluator.generated.h"

namespace UE::MassBehavior { struct FStateTreeDependencyBuilder; }

/**
 * Output struct surfaced to the StateTree property-binding panel. Bind TargetLocation to
 * FMassNavMeshPathFollowTask::TargetLocation and use bHasValidTarget on a transition condition
 * to enter/exit the path-follow state.
 */
USTRUCT()
struct FNPCNavTargetEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = Output)
	FMassTargetLocation TargetLocation;

	UPROPERTY(VisibleAnywhere, Category = Output)
	bool bHasValidTarget = false;
};

/**
 * Mass StateTree evaluator that reads FNPCNavTargetFragment and emits an FMassTargetLocation.
 * Plug the fragment-side data (written by NPCMassActorComponent / hydrated AI) into the engine's
 * MassNavMeshPathFollowTask without a custom task or processor.
 */
USTRUCT(meta = (DisplayName = "NPC Nav Target Eval"))
struct PERSISTENCEEXAMPLES_API FNPCNavTargetEvaluator : public FMassStateTreeEvaluatorBase
{
	GENERATED_BODY()

	using FInstanceDataType = FNPCNavTargetEvaluatorInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;

	TStateTreeExternalDataHandle<FNPCNavTargetFragment> NavTargetHandle;
};
