// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Framework/PersistedObjectInterface.h"
#include "GameplayTagContainer.h"
#include "References/PersistableActorReference.h"
#include "PersistedAbilitySystemComponent.generated.h"

class UGameplayEffect;

/**
 * A single active GameplayEffect captured for persistence. This is a lossy snapshot: it records the
 * effect class, who applied it, how much time was left, the level, stack count, and any SetByCaller
 * magnitudes. It intentionally does NOT capture dynamically granted tags, modifier overrides, or the
 * full effect context beyond the instigator - on restore the effect is rebuilt from its CDO via
 * MakeOutgoingSpec, so anything not listed here comes back at the asset's authored defaults.
 */
USTRUCT()
struct FPersistedGameplayEffect
{
	GENERATED_BODY()

	/** Who applied the effect. Resolved on restore through UPersistableActorReferenceManager. None == self/no source. */
	UPROPERTY(SaveGame)
	FPersistableActorReference Instigator;

	/** Soft path to the (often Blueprint) UGameplayEffect subclass to re-apply. */
	UPROPERTY(SaveGame)
	TSoftClassPtr<UGameplayEffect> EffectClass;

	/** Seconds of duration left at save time. -1 means infinite / no duration policy. */
	UPROPERTY(SaveGame)
	float RemainingDuration = -1.f;

	/** Effect level (GE spec level) at save time. */
	UPROPERTY(SaveGame)
	float Level = 1.f;

	/** Active stack count at save time. */
	UPROPERTY(SaveGame)
	int32 StackCount = 1;

	/** SetByCaller magnitudes keyed by gameplay tag, copied wholesale from the spec. */
	UPROPERTY(SaveGame)
	TMap<FGameplayTag, float> SetByCallerTagMagnitudes;

	/** SetByCaller magnitudes keyed by FName, copied wholesale from the spec. */
	UPROPERTY(SaveGame)
	TMap<FName, float> SetByCallerNameMagnitudes;
};

/** Outer container persisted on the ASC: the set of active effects to restore. */
USTRUCT()
struct FPersistedAbilitySystemState
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FPersistedGameplayEffect> ActiveEffects;
};

/**
 * Example AbilitySystemComponent that persists a subset of its active GameplayEffects across save/load.
 *
 * Opt-in is per-effect: only GEs whose asset tags contain GetPersistableEffectTag()
 * ("Gameplay.Effect.Persistable") are captured. On PrePersistObject the component snapshots those
 * effects into PersistedState (a UPROPERTY(SaveGame) the LevelStreamingPersistence plugin serializes).
 * On PostRestoreObject it defers re-application to the owning level's LSP post-restore flush - at which
 * point each effect's instigator actor is resolvable - then rebuilds and re-applies each effect.
 *
 * Wire it in by creating it as the ASC subobject (see APersistenceLabCharacter) and registering
 * PersistedState in DefaultEngine.ini under LevelStreamingPersistenceSettings.
 */
UCLASS(ClassGroup=(Persistence), meta=(BlueprintSpawnableComponent))
class PERSISTENCEEXAMPLES_API UPersistedAbilitySystemComponent : public UAbilitySystemComponent, public IPersistedObject
{
	GENERATED_BODY()

public:
	/** Asset tag that marks a GameplayEffect as persistable. Set this on the GE asset's GameplayEffectAssetTags. */
	static const FGameplayTag GetPersistableEffectTag();

	//~ IPersistedObject
	virtual void PrePersistObject_Implementation() override;
	virtual void PostRestoreObject_Implementation(const TArray<FName>& RestoredPropertyNames) override;

protected:
	/** Snapshot of persistable active effects. Written by PrePersistObject, serialized by LSP, consumed by PostRestoreObject. */
	UPROPERTY()
	FPersistedAbilitySystemState PersistedState;

private:
	/** Deferred callback fired at the owning level's LSP post-restore flush; re-applies every entry in PersistedState. */
	void ApplyAllRestoredEffects(AActor* ResolvedOwner, bool bSuccess);
};
