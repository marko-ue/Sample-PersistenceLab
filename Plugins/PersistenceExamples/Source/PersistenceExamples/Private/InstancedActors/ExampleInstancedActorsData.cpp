// Copyright Epic Games, Inc. All Rights Reserved.

#include "InstancedActors/ExampleInstancedActorsData.h"
#include "InstancedActorsManager.h"
#include "InstancedActorsRepresentationSubsystem.h"
#include "InstancedActorsTypes.h"
#include "InstancedActorsVisualizationSwitcherProcessor.h"
#include "Mass/HealthFragment.h"
#include "MassEntityManager.h"
#include "MassRepresentationFragments.h"

void UExampleInstancedActorsData::ApplyInstanceDelta(FMassEntityManager& EntityManager, const FInstancedActorsDelta& InstanceDelta, TArray<FInstancedActorsInstanceIndex>& OutEntitiesToRemove)
{
	// Let the engine handle destruction (the only thing the base implementation acts on).
	Super::ApplyInstanceDelta(EntityManager, InstanceDelta, OutEntitiesToRemove);

	if (!HasSpawnedEntities())
	{
		return;
	}

	// A destroyed instance is on its way out; the lifecycle phase is irrelevant.
	if (InstanceDelta.IsDestroyed())
	{
		return;
	}

	// The only piece of replicated state we can use to recover Visual State is the lifecycle phase.
	if (!InstanceDelta.HasCurrentLifecyclePhase())
	{
		return;
	}

	// SwitchInstanceVisualization defers a Mass command that is consumed by UInstancedActorsVisualizationSwitcherProcessor,
	// which runs server-only. On the server/standalone the gameplay-driven path already swaps the mesh, so the only place
	// that needs help is the client - where we apply the switch synchronously (mirrors the workaround in
	// UExampleInstancedActorComponent::NotifyVisualStateChanged).
	AInstancedActorsManager* Manager = GetManager();
	if (!Manager || Manager->GetNetMode() != NM_Client)
	{
		return;
	}

	// Convention: visualization index == lifecycle phase index (see class comment).
	const uint8 PhaseIndex = InstanceDelta.GetCurrentLifecyclePhaseIndex();
	const FInstancedActorsVisualizationInfo* TargetViz = GetVisualization(PhaseIndex);
	if (!TargetViz)
	{
		return;
	}

	const FMassEntityHandle EntityHandle = GetEntity(InstanceDelta.GetInstanceIndex());
	if (!EntityManager.IsEntityValid(EntityHandle))
	{
		return;
	}

	UWorld* World = Manager->GetWorld();
	UInstancedActorsRepresentationSubsystem* RepSubsystem = World ? World->GetSubsystem<UInstancedActorsRepresentationSubsystem>() : nullptr;
	FMassRepresentationFragment* RepFrag = EntityManager.GetFragmentDataPtr<FMassRepresentationFragment>(EntityHandle);
	if (!RepSubsystem || !RepFrag)
	{
		return;
	}

	FMassInstancedStaticMeshInfoArrayView ISMInfosView = RepSubsystem->GetMutableInstancedStaticMeshInfos();
	UInstancedActorsVisualizationSwitcherProcessor::SwitchEntityMeshDesc(
		ISMInfosView, *RepFrag, EntityHandle, TargetViz->MassStaticMeshDescHandle);
}

void UExampleInstancedActorsData::OnSpawnEntities()
{
	Super::OnSpawnEntities();

	if (PendingVisualization.IsEmpty() && PendingHealth.IsEmpty())
	{
		return;
	}

	// Staged persistence state is server-authoritative. On clients (where these maps are never filled,
	// since restore only runs on the authority) there is nothing to do; clear and bail defensively.
	const AInstancedActorsManager* Manager = GetManager();
	if (!Manager || !Manager->HasAuthority())
	{
		PendingVisualization.Empty();
		PendingHealth.Empty();
		return;
	}

	// Phase 2: entities now exist (OnSpawnEntities runs at the end of SpawnEntities). Apply the data
	// staged during deserialization onto the freshly spawned Mass entities - once, on the game thread,
	// before any simulation processor ticks.
	for (const TPair<int32, uint8>& VizPair : PendingVisualization)
	{
		const FInstancedActorsInstanceIndex InstanceIndex(VizPair.Key);
		SwitchInstanceVisualization(InstanceIndex, VizPair.Value);

		// Re-seed the replicated lifecycle-phase delta from restored viz. Persistence serializes only
		// destruction (AInstancedActorsManager::SerializeInstancePersistenceData), not the phase, so after a
		// server save/load InstanceDeltas carries no phase entry for instances whose viz changed pre-save.
		// Without this, a client joining a restored session sees the default mesh on dehydrated instances.
		// Mirrors the live producer in UExampleInstancedActorComponent::NotifyVisualStateChanged. Authority
		// is guaranteed here (early-out above); SetInstanceCurrentLifecyclePhase asserts on it. Convention:
		// viz index == lifecycle phase index. Server-side this is a no-op for gameplay (base ApplyInstanceDelta
		// ignores the phase), so there is no double-apply with SwitchInstanceVisualization above.
		SetInstanceCurrentLifecyclePhase(InstanceIndex, VizPair.Value);
	}

	if (!PendingHealth.IsEmpty())
	{
		FMassEntityManager& EntityManager = GetMassEntityManagerChecked();
		for (const TPair<int32, FVector2f>& HealthPair : PendingHealth)
		{
			const FMassEntityHandle EntityHandle = GetEntity(FInstancedActorsInstanceIndex(HealthPair.Key));
			if (EntityManager.IsEntityValid(EntityHandle))
			{
				if (FHealthFragment* HealthFrag = EntityManager.GetFragmentDataPtr<FHealthFragment>(EntityHandle))
				{
					HealthFrag->Current = HealthPair.Value.X;
					HealthFrag->Max = HealthPair.Value.Y;
				}
			}
		}
	}

	PendingVisualization.Empty();
	PendingHealth.Empty();
}

void UExampleInstancedActorsData::StagePersistedVisualization(int32 InstanceIndex, uint8 VisualizationIndex)
{
	PendingVisualization.Add(InstanceIndex, VisualizationIndex);
}

void UExampleInstancedActorsData::StagePersistedHealth(int32 InstanceIndex, float Current, float Max)
{
	PendingHealth.Add(InstanceIndex, FVector2f(Current, Max));
}
