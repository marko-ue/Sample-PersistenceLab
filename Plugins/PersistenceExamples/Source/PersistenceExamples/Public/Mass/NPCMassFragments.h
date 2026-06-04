// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "NPCMassFragments.generated.h"

/** Last move target written by gameplay code. Read by the dehydrated-nav processor to drive FMassMoveTargetFragment. */
USTRUCT()
struct FNPCNavTargetFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "NPC|Mass")
	FVector Target = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "NPC|Mass")
	bool bValid = false;
};

/** Snapshot of StateTree state captured at flush time, so the hydrated actor can ForceTransitionToState on respawn. */
USTRUCT()
struct FNPCStateFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "NPC|Mass")
	FGuid ActiveStateId;
};

/** Present while the entity has a high-LOD actor representation in the world. Custom processors filter on this. */
USTRUCT()
struct FNPCHydratedTag : public FMassTag
{
	GENERATED_BODY()
};

/** Set when the NPC's health reaches 0. Nav/steering processors filter this out. */
USTRUCT()
struct FNPCDeadTag : public FMassTag
{
	GENERATED_BODY()
};

/** BP-friendly bundle of all hydrated-actor-authored state. Used by UNPCMassActorComponent to read/write fragments en bloc. */
USTRUCT(BlueprintType)
struct FNPCMassSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mass")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mass")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mass")
	FVector NavTarget = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mass")
	bool bNavTargetValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mass")
	FGuid ActiveStateId;
};

UCLASS(meta = (DisplayName = "NPC Nav Target"))
class UNPCNavTargetTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

UCLASS(meta = (DisplayName = "NPC StateTree State"))
class UNPCStateTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
