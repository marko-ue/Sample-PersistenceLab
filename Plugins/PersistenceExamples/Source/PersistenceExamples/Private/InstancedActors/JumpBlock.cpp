// Copyright Epic Games, Inc. All Rights Reserved.

#include "InstancedActors/JumpBlock.h"
#include "Net/UnrealNetwork.h"

AJumpBlock::AJumpBlock()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetNotifyRigidBodyCollision(true);
	SetRootComponent(MeshComponent);

	InstancedActorsComp = CreateDefaultSubobject<UExampleInstancedActorComponent>(TEXT("ExampleIAComp"));
}

void AJumpBlock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AJumpBlock, VisualState);
}

void AJumpBlock::BeginPlay()
{
	Super::BeginPlay();
	ApplyVisualState();
}

// Game code callable function that changes the actor's visual state and notifies this actor's instanced actor component,
// so that the visual state value on the Mass entity will be updates as well.
void AJumpBlock::SetVisualState(int32 NewVisualState)
{
	VisualState = NewVisualState;
	ApplyVisualState();
	InstancedActorsComp->NotifyVisualStateChanged(NewVisualState);
}

// Update server or client-side visual after receiving the state from this actor's instanced actor component.
// For example, on hydration from Mass entity into this actor.
void AJumpBlock::OnVisualStateRestored_Implementation(int32 VisualStateIndex)
{
	VisualState = VisualStateIndex;
	ApplyVisualState();
}

void AJumpBlock::ApplyVisualState()
{
	// Mirror the IAC's ModifyMassEntityTemplate logic on the hydrated actor: for each of the actor's
	// SMCs, look up its FName in the current visual state's override map and apply mesh + transform.
	// Initial transforms are read from each SMC's archetype (the CDO's same-named default subobject for
	// native components, the SCS template for BP-added components) so ApplyRelative always composes
	// against the base, and no-override / state 0 restores cleanly.
	// AdditionalVisualStates[K] configures visual state K+1; state 0 has no overrides (engine-built).
	const TArray<FVisualStateSMCOverrides>& States = InstancedActorsComp->AdditionalVisualStates;
	const TMap<FName, FVisualStateMeshOverride>* Overrides = nullptr;
	const int32 EntryIndex = VisualState - 1;
	if (EntryIndex >= 0 && States.IsValidIndex(EntryIndex))
	{
		Overrides = &States[EntryIndex].Overrides;
	}

	TArray<UStaticMeshComponent*> SMCs;
	GetComponents<UStaticMeshComponent>(SMCs);
	for (UStaticMeshComponent* SMC : SMCs)
	{
		if (!SMC)
		{
			continue;
		}
		// For the root component RelativeTransform is the actor's world transform — touching it would
		// teleport the actor. Apply mesh/visibility to the root, but skip transform changes.
		const bool bIsRoot = (SMC == GetRootComponent());
		const UStaticMeshComponent* Archetype = Cast<UStaticMeshComponent>(SMC->GetArchetype());
		const FTransform InitialTransform = Archetype ? Archetype->GetRelativeTransform() : FTransform::Identity;
		const bool bArchetypeVisible = Archetype ? Archetype->IsVisible() : true;
		const FVisualStateMeshOverride* Override = Overrides ? Overrides->Find(SMC->GetFName()) : nullptr;

		if (Override)
		{
			SMC->SetVisibility(Override->bVisible);
			if (!Override->bVisible)
			{
				continue;
			}
			if (Override->StaticMeshOverride)
			{
				SMC->SetStaticMesh(Override->StaticMeshOverride);
			}
			if (bIsRoot)
			{
				continue;
			}
			switch (Override->TransformModification)
			{
			case EVisualStateTransformModification::Override:
				SMC->SetRelativeTransform(Override->TransformToApply);
				break;
			case EVisualStateTransformModification::ApplyRelative:
				SMC->SetRelativeTransform(Override->TransformToApply * InitialTransform);
				break;
			case EVisualStateTransformModification::None:
			default:
				SMC->SetRelativeTransform(InitialTransform);
				break;
			}
		}
		else
		{
			// No override at this state — restore archetype mesh + visibility + transform in case a prior
			// state modified them. Skip transform on the root for the same reason as above.
			if (Archetype)
			{
				SMC->SetStaticMesh(Archetype->GetStaticMesh());
			}
			SMC->SetVisibility(bArchetypeVisible);
			if (!bIsRoot)
			{
				SMC->SetRelativeTransform(InitialTransform);
			}
		}
	}
}

// Update client-side visual after receiving the state via replication from the server-side actor.
void AJumpBlock::OnRep_VisualState()
{
	SetVisualState(VisualState);
}
