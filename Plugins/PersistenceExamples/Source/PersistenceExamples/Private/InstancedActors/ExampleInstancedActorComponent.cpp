// Copyright Epic Games, Inc. All Rights Reserved.

#include "InstancedActors/ExampleInstancedActorComponent.h"
#include "InstancedActors/ExampleInstancedActorsData.h"
#include "InstancedActors/ExampleInstancedActorVersion.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/StaticMeshComponent.h"
#include "GAS/HealthAttributeSet.h"
#include "Mass/HealthFragment.h"
#include "PersistenceExamples.h"
#include "InstancedActorsData.h"
#include "InstancedActorsManager.h"
#include "InstancedActorsRepresentationSubsystem.h"
#include "InstancedActorsTypes.h"
#include "InstancedActorsVisualizationSwitcherProcessor.h"
#include "ISMPartition/ISMComponentDescriptor.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassRepresentationFragments.h"
#include "Framework/PersistenceWorldSubsystem.h"

namespace ExampleIAC
{
	constexpr uint32 PersistenceDataID = 0x45494143; // 'EIAC'

	static UHealthAttributeSet* FindOwnerHealthSet(AActor* Owner)
	{
		if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
		{
			if (const UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				return const_cast<UHealthAttributeSet*>(ASC->GetSet<UHealthAttributeSet>());
			}
		}
		return nullptr;
	}
}

void UExampleInstancedActorComponent::ModifyMassEntityConfig(FMassEntityManager& InMassEntityManager, UInstancedActorsData* InstancedActorData, FMassEntityConfig& InOutMassEntityConfig) const
{
	Super::ModifyMassEntityConfig(InMassEntityManager, InstancedActorData, InOutMassEntityConfig);

	if (bSupportsHealth && InstancedActorData)
	{
		UHealthTrait* HealthTrait = NewObject<UHealthTrait>(InstancedActorData);
		InOutMassEntityConfig.AddTrait(*HealthTrait);
	}
}

void UExampleInstancedActorComponent::ModifyMassEntityTemplate(FMassEntityManager& InMassEntityManager, UInstancedActorsData* InstancedActorData, FMassEntityTemplateData& InOutMassEntityTemplateData) const
{
	Super::ModifyMassEntityTemplate(InMassEntityManager, InstancedActorData, InOutMassEntityTemplateData);

	if (VisualDescriptionGenerationType != EVisualDescriptionGenerationType::PropertyConfigured || !InstancedActorData)
	{
		return;
	}

	const FInstancedActorsVisualizationInfo* DefaultViz = InstancedActorData->GetVisualization(0);
	if (!DefaultViz)
	{
		return;
	}

	// For each additional visual state, copy state 0's descriptors and apply the per-SMC-name overrides.
	// AdditionalVisualStates[K] configures visual state K+1; state 0 is engine-built and needs no entry.
	// Descriptors whose source SMC name isn't in the override map pass through unchanged. Descriptors
	// whose override has bVisible=false are dropped (no ISM contribution at this state).
	for (int32 EntryIndex = 0; EntryIndex < AdditionalVisualStates.Num(); ++EntryIndex)
	{
		const TMap<FName, FVisualStateMeshOverride>& Overrides = AdditionalVisualStates[EntryIndex].Overrides;

		FInstancedActorsVisualizationDesc Desc = DefaultViz->VisualizationDesc;
		Desc.ISMComponentDescriptors.RemoveAll([&Overrides](const FISMComponentDescriptor& Descriptor)
		{
			const UStaticMeshComponent* SourceSMC = Descriptor.GetStaticMeshComponent().Get();
			if (!SourceSMC)
			{
				return false;
			}
			const FVisualStateMeshOverride* Override = Overrides.Find(SourceSMC->GetFName());
			return Override && !Override->bVisible;
		});

		for (FISMComponentDescriptor& Descriptor : Desc.ISMComponentDescriptors)
		{
			const UStaticMeshComponent* SourceSMC = Descriptor.GetStaticMeshComponent().Get();
			if (!SourceSMC)
			{
				continue;
			}
			const FVisualStateMeshOverride* Override = Overrides.Find(SourceSMC->GetFName());
			if (!Override)
			{
				continue;
			}

			if (Override->StaticMeshOverride)
			{
				Descriptor.StaticMesh = Override->StaticMeshOverride;
			}
			switch (Override->TransformModification)
			{
			case EVisualStateTransformModification::Override:
				Descriptor.LocalTransform = Override->TransformToApply;
				break;
			case EVisualStateTransformModification::ApplyRelative:
				Descriptor.LocalTransform = Override->TransformToApply * Descriptor.LocalTransform;
				break;
			case EVisualStateTransformModification::None:
			default:
				break;
			}
		}

		InstancedActorData->AddVisualization(Desc);
	}
}

void UExampleInstancedActorComponent::InitializeComponentForInstance(FInstancedActorsInstanceHandle InInstanceHandle)
{
	Super::InitializeComponentForInstance(InInstanceHandle);

	AActor* Owner = GetOwner();
	UInstancedActorsData* InstanceData = InInstanceHandle.GetInstanceActorData();
	if (!Owner || !InstanceData)
	{
		return;
	}

	UMassEntitySubsystem* EntitySubsystem = GetWorld() ? GetWorld()->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!EntitySubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle EntityHandle = GetMassEntityHandle();
	if (!EntityManager.IsEntityValid(EntityHandle))
	{
		return;
	}

	// Restore visual state by mapping the entity's current StaticMeshDescHandle back to a state index.
	const FMassRepresentationFragment& RepFrag = EntityManager.GetFragmentDataChecked<FMassRepresentationFragment>(EntityHandle);
	const FStaticMeshInstanceVisualizationDescHandle CurrentHandle = RepFrag.StaticMeshDescHandle;

	int32 VisualStateIndex = 0;
	InstanceData->ForEachVisualization([&](uint8 VizIndex, const FInstancedActorsVisualizationInfo& VizInfo)
	{
		if (VizInfo.MassStaticMeshDescHandle == CurrentHandle)
		{
			VisualStateIndex = static_cast<int32>(VizIndex);
			return false;
		}
		return true;
	});

	if (Owner->Implements<UExampleVisualStateRestorable>())
	{
		IExampleVisualStateRestorable::Execute_OnVisualStateRestored(Owner, VisualStateIndex);
	}

	if (bSupportsHealth)
	{
		if (UHealthAttributeSet* HealthSet = ExampleIAC::FindOwnerHealthSet(Owner))
		{
			if (const FHealthFragment* HealthFrag = EntityManager.GetFragmentDataPtr<FHealthFragment>(EntityHandle))
			{
				HealthSet->SetMaxHealth(HealthFrag->Max);
				HealthSet->SetHealth(HealthFrag->Current);
			}
		}

		// When the persistence world subsystem is about to save, ensure that a hydrated actor's health is flushed back to the Mass entity fragment.
		// Then, SerializeInstancePersistenceData will write those values into a binary blob for the owning AInstancedActorsManager which manages
		// this instance. For dehydrated actors, the Mass fragment is already updated on EndPlay of this component. Alternatively, you could sync
		// the health values on every change.
		if (UPersistenceWorldSubsystem* PersistenceSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UPersistenceWorldSubsystem>() : nullptr)
		{
			PreIAMFlushHandle = PersistenceSubsystem->OnPreFlushInstancedActorsData.AddLambda([this]()
			{
				if (UHealthAttributeSet* HealthSet = ExampleIAC::FindOwnerHealthSet(GetOwner()))
				{
					NotifyHealthChanged(HealthSet->GetHealth(), HealthSet->GetMaxHealth());
				}
			});
		}
	}
}

void UExampleInstancedActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Last chance to flush actor-authoritative health back to the entity fragment before despawn.
	if (bSupportsHealth && HasMassEntity())
	{
		if (UHealthAttributeSet* HealthSet = ExampleIAC::FindOwnerHealthSet(GetOwner()))
		{
			UMassEntitySubsystem* EntitySubsystem = GetWorld() ? GetWorld()->GetSubsystem<UMassEntitySubsystem>() : nullptr;
			if (EntitySubsystem)
			{
				FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
				const FMassEntityHandle EntityHandle = GetMassEntityHandle();
				if (EntityManager.IsEntityValid(EntityHandle))
				{
					if (FHealthFragment* HealthFrag = EntityManager.GetFragmentDataPtr<FHealthFragment>(EntityHandle))
					{
						HealthFrag->Current = HealthSet->GetHealth();
						HealthFrag->Max = HealthSet->GetMaxHealth();
					}
				}
			}
		}
	}

	if (PreIAMFlushHandle.IsValid())
	{
		if (UPersistenceWorldSubsystem* PersistenceSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UPersistenceWorldSubsystem>() : nullptr)
		{
			PersistenceSubsystem->OnPreFlushInstancedActorsData.Remove(PreIAMFlushHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

uint32 UExampleInstancedActorComponent::GetInstancePersistenceDataID() const
{
	return ExampleIAC::PersistenceDataID;
}

bool UExampleInstancedActorComponent::ShouldSerializeInstancePersistenceData(const FArchive& Archive, UInstancedActorsData* InstanceData, int64 TimeDelta) const
{
	return InstanceData != nullptr;
}

void UExampleInstancedActorComponent::SerializeInstancePersistenceData(FStructuredArchive::FRecord Record, UInstancedActorsData* InstanceData, int64 TimeDelta) const
{
	FArchive& Ar = Record.GetUnderlyingArchive();

	// Versioning via the archive's custom-version container instead of a hand-rolled byte. On a saving
	// archive UsingCustomVersion copies LatestVersion from the process-global registry into the container;
	// on a loading archive it's a no-op and CustomVer returns the version that was saved (the transport,
	// UPersistenceWorldSubsystem, persists the FCustomVersionContainer alongside this blob and restores it
	// onto the reader before Serialize runs). See FExampleInstancedActorVersion.
	Ar.UsingCustomVersion(FExampleInstancedActorVersion::GUID);
	const int32 Version = Ar.CustomVer(FExampleInstancedActorVersion::GUID);

	// On load, a record older than our first versioned layout can't be trusted; warn (the archive seek
	// past this record's body is handled by the IAM caller via the IAC size header).
	if (Ar.IsLoading() && Version < FExampleInstancedActorVersion::InitialVersion)
	{
		UE_LOG(LogPersistenceExamples, Warning,
			TEXT("ExampleInstancedActorComponent: unsupported persistence version %d (expected >= %d) — skipping record."),
			Version, static_cast<int32>(FExampleInstancedActorVersion::InitialVersion));
	}

	// Save-time: gather sparse overrides vs. the IAD's defaults (viz 0, health 100/100).
	TArray<TPair<FInstancedActorsInstanceIndex, uint8>> VizOverrides;
	TArray<TTuple<FInstancedActorsInstanceIndex, float, float>> HealthOverrides;

	if (Ar.IsSaving())
	{
		AInstancedActorsManager* Manager = InstanceData ? InstanceData->GetTypedOuter<AInstancedActorsManager>() : nullptr;
		TSharedPtr<FMassEntityManager> EntityManagerPtr = Manager ? Manager->GetMassEntityManager() : nullptr;
		if (EntityManagerPtr && InstanceData->HasSpawnedEntities())
		{
			const FMassEntityManager& EntityManager = *EntityManagerPtr;
			for (int32 i = 0; i < static_cast<int32>(InstanceData->NumValidInstances); ++i)
			{
				const FInstancedActorsInstanceIndex InstanceIndex(i);
				const FMassEntityHandle EntityHandle = InstanceData->GetEntity(InstanceIndex);
				if (!EntityManager.IsEntityValid(EntityHandle))
				{
					continue;
				}

				const FMassRepresentationFragment& RepFrag = EntityManager.GetFragmentDataChecked<FMassRepresentationFragment>(EntityHandle);
				uint8 VizIndex = 0;
				InstanceData->ForEachVisualization([&](uint8 Idx, const FInstancedActorsVisualizationInfo& VizInfo)
				{
					if (VizInfo.MassStaticMeshDescHandle == RepFrag.StaticMeshDescHandle)
					{
						VizIndex = Idx;
						return false;
					}
					return true;
				});

				if (VizIndex != 0)
				{
					VizOverrides.Emplace(InstanceIndex, VizIndex);
				}

				if (bSupportsHealth)
				{
					if (const FHealthFragment* HealthFrag = EntityManager.GetFragmentDataPtr<FHealthFragment>(EntityHandle))
					{
						if (HealthFrag->Current != 100.f || HealthFrag->Max != 100.f)
						{
							HealthOverrides.Emplace(InstanceIndex, HealthFrag->Current, HealthFrag->Max);
						}
					}
				}
			}
		}
	}

	int32 NumVizOverrides = VizOverrides.Num();
	FStructuredArchive::FArray VizArray = Record.EnterArray(TEXT("VisualizationOverrides"), NumVizOverrides);
	if (Ar.IsSaving())
	{
		for (TPair<FInstancedActorsInstanceIndex, uint8>& Override : VizOverrides)
		{
			FStructuredArchive::FRecord Entry = VizArray.EnterElement().EnterRecord();
			Entry << SA_VALUE(TEXT("InstanceIndex"), Override.Key);
			Entry << SA_VALUE(TEXT("VizIndex"), Override.Value);
		}
	}
	else
	{
		UExampleInstancedActorsData* ExampleData = Cast<UExampleInstancedActorsData>(InstanceData);
		for (int32 j = 0; j < NumVizOverrides; ++j)
		{
			FStructuredArchive::FRecord Entry = VizArray.EnterElement().EnterRecord();
			FInstancedActorsInstanceIndex InstanceIndex;
			uint8 VizIndex = 0;
			Entry << SA_VALUE(TEXT("InstanceIndex"), InstanceIndex);
			Entry << SA_VALUE(TEXT("VizIndex"), VizIndex);
			// Phase 1: stage into the IAD rather than touching Mass - entities are not yet spawned at
			// restore time. Applied in UExampleInstancedActorsData::OnSpawnEntities.
			if (ExampleData)
			{
				ExampleData->StagePersistedVisualization(InstanceIndex.GetIndex(), VizIndex);
			}
		}
	}

	int32 NumHealthOverrides = HealthOverrides.Num();
	FStructuredArchive::FArray HealthArray = Record.EnterArray(TEXT("HealthOverrides"), NumHealthOverrides);
	if (Ar.IsSaving())
	{
		for (TTuple<FInstancedActorsInstanceIndex, float, float>& Override : HealthOverrides)
		{
			FStructuredArchive::FRecord Entry = HealthArray.EnterElement().EnterRecord();
			Entry << SA_VALUE(TEXT("InstanceIndex"), Override.Get<0>());
			Entry << SA_VALUE(TEXT("Health"), Override.Get<1>());
			Entry << SA_VALUE(TEXT("MaxHealth"), Override.Get<2>());
		}
	}
	else
	{
		UExampleInstancedActorsData* ExampleData = Cast<UExampleInstancedActorsData>(InstanceData);
		for (int32 j = 0; j < NumHealthOverrides; ++j)
		{
			FStructuredArchive::FRecord Entry = HealthArray.EnterElement().EnterRecord();
			FInstancedActorsInstanceIndex InstanceIndex;
			float Health = 100.f;
			float MaxHealth = 100.f;
			Entry << SA_VALUE(TEXT("InstanceIndex"), InstanceIndex);
			Entry << SA_VALUE(TEXT("Health"), Health);
			Entry << SA_VALUE(TEXT("MaxHealth"), MaxHealth);
			// Phase 1: stage into the IAD; applied to FHealthFragment in OnSpawnEntities once entities exist.
			if (ExampleData)
			{
				ExampleData->StagePersistedHealth(InstanceIndex.GetIndex(), Health, MaxHealth);
			}
		}
	}

	UE_LOG(LogPersistenceExamples, Log,
		TEXT("ExampleIAC SerializeInstancePersistenceData [%s] Viz: %d override(s), Health: %d override(s)"),
		Ar.IsSaving() ? TEXT("Save") : TEXT("Load"), NumVizOverrides, NumHealthOverrides);
}

void UExampleInstancedActorComponent::NotifyVisualStateChanged(int32 NewVisualState)
{
	if (!HasMassEntity())
	{
		return;
	}

	UInstancedActorsData* InstanceData = InstanceHandle.GetInstanceActorData();
	if (!InstanceData)
	{
		return;
	}

	const FInstancedActorsInstanceIndex InstanceIndex = InstanceHandle.GetInstanceIndex();
	InstanceData->SwitchInstanceVisualization(InstanceIndex, static_cast<uint8>(NewVisualState));

	// Server: record the new visualization index as the instance's lifecycle phase. This is the producer
	// for late-joining-client replication: SetInstanceCurrentLifecyclePhase flushes net dormancy and writes
	// into the replicated InstanceDeltas FastArray, so the phase reaches ALL clients (incl. late joiners)
	// and survives this actor dehydrating. Convention: viz index == lifecycle phase index. The base
	// ApplyInstanceDelta ignores the phase on the server (it only acts on destruction), so this has no
	// server-side gameplay effect; UExampleInstancedActorsData::ApplyInstanceDelta consumes it client-side.
	// SetInstanceCurrentLifecyclePhase asserts on HasAuthority(), so guard on the manager's authority.
	const AInstancedActorsManager* Manager = InstanceData->GetManager();
	if (Manager && Manager->HasAuthority())
	{
		InstanceData->SetInstanceCurrentLifecyclePhase(InstanceIndex, static_cast<uint8>(NewVisualState));
	}

	// Client-side workaround for the visualization switcher processor (server-only by default — see
	// project-ia-viz-switcher-client-gap memory). Apply the switch synchronously on clients so the
	// dehydration path's stationary-ISM switcher reads a fresh handle.
	if (GetNetMode() == NM_Client)
	{
		UInstancedActorsRepresentationSubsystem* RepSubsystem = GetWorld()->GetSubsystem<UInstancedActorsRepresentationSubsystem>();
		UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
		const FInstancedActorsVisualizationInfo* TargetViz = InstanceData->GetVisualization(static_cast<uint8>(NewVisualState));
		if (RepSubsystem && EntitySubsystem && TargetViz)
		{
			FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
			const FMassEntityHandle EntityHandle = GetMassEntityHandle();
			if (EntityManager.IsEntityValid(EntityHandle))
			{
				FMassInstancedStaticMeshInfoArrayView ISMInfosView = RepSubsystem->GetMutableInstancedStaticMeshInfos();
				FMassRepresentationFragment& RepFrag = EntityManager.GetFragmentDataChecked<FMassRepresentationFragment>(EntityHandle);
				UInstancedActorsVisualizationSwitcherProcessor::SwitchEntityMeshDesc(
					ISMInfosView, RepFrag, EntityHandle, TargetViz->MassStaticMeshDescHandle);
			}
		}
	}
}

void UExampleInstancedActorComponent::NotifyHealthChanged(float NewHealth, float NewMaxHealth)
{
	if (!HasMassEntity())
	{
		return;
	}

	UMassEntitySubsystem* EntitySubsystem = GetWorld() ? GetWorld()->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!EntitySubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	const FMassEntityHandle EntityHandle = GetMassEntityHandle();
	if (!EntityManager.IsEntityValid(EntityHandle))
	{
		return;
	}

	if (FHealthFragment* HealthFrag = EntityManager.GetFragmentDataPtr<FHealthFragment>(EntityHandle))
	{
		HealthFrag->Current = NewHealth;
		HealthFrag->Max = NewMaxHealth;
	}
}

void UExampleInstancedActorComponent::DestroyInstance()
{
	if (HasMassEntity())
	{
		InstanceHandle.GetInstanceActorDataChecked().DestroyInstance(InstanceHandle.GetInstanceIndex());
	}
}

void UExampleInstancedActorComponent::EjectInstance()
{
	AActor* Owner = GetOwner();
	if (Owner && HasMassEntity())
	{
		InstanceHandle.GetInstanceActorDataChecked().EjectInstanceActor(InstanceHandle.GetInstanceIndex(), *Owner);
	}
}
