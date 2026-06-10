// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/PersistedAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "NativeGameplayTags.h"
#include "PersistenceExamples.h"
#include "References/PersistableActorReferenceManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// Asset tag authors set on a GameplayEffect (GameplayEffectAssetTags) to opt it into persistence.
UE_DEFINE_GAMEPLAY_TAG_STATIC(Tag_Gameplay_Effect_Persistable, "Gameplay.Effect.Persistable");

const FGameplayTag UPersistedAbilitySystemComponent::GetPersistableEffectTag()
{
	return Tag_Gameplay_Effect_Persistable;
}

void UPersistedAbilitySystemComponent::PrePersistObject_Implementation()
{
	PersistedState.ActiveEffects.Reset();

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const float WorldTime = World->GetTimeSeconds();

	// Query active effects whose own (asset) tags include the persistable tag, rather than walking all
	// effects and filtering by hand.
	FGameplayTagContainer TagFilter;
	TagFilter.AddTag(GetPersistableEffectTag());
	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyEffectTags(TagFilter);

	for (const FActiveGameplayEffectHandle& Handle : GetActiveEffects(Query))
	{
		const FActiveGameplayEffect* AGE = GetActiveGameplayEffect(Handle);
		if (!AGE || !AGE->Spec.Def)
		{
			continue;
		}
		const FGameplayEffectSpec& Spec = AGE->Spec;

		// Instant effects don't persist as active state; skip anything without a real duration that
		// isn't explicitly infinite.
		const float Remaining = AGE->GetTimeRemaining(WorldTime); // -1 for infinite
		const bool bInfinite = (Remaining < 0.f);
		if (!bInfinite && Remaining <= 0.f)
		{
			continue;
		}

		FPersistedGameplayEffect Entry;
		Entry.EffectClass = TSoftClassPtr<UGameplayEffect>(Spec.Def->GetClass());
		Entry.Instigator.SetFromActor(Spec.GetEffectContext().GetInstigator());
		Entry.RemainingDuration = bInfinite ? -1.f : Remaining;
		Entry.Level = Spec.GetLevel();
		Entry.StackCount = Spec.GetStackCount();
		Entry.SetByCallerTagMagnitudes = Spec.SetByCallerTagMagnitudes;
		Entry.SetByCallerNameMagnitudes = Spec.SetByCallerNameMagnitudes;

		PersistedState.ActiveEffects.Add(MoveTemp(Entry));
	}

	UE_LOG(LogPersistenceExamples, Log, TEXT("[%s] PrePersistObject: captured %d persistable gameplay effect(s)"),
		*GetNameSafe(GetOwner()), PersistedState.ActiveEffects.Num());
}

void UPersistedAbilitySystemComponent::PostRestoreObject_Implementation(const TArray<FName>& RestoredPropertyNames)
{
	if (PersistedState.ActiveEffects.Num() == 0)
	{
		return;
	}

	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	// PostRestoreObject runs before BeginPlay for map-placed actors, so instigators may not exist yet.
	// Defer re-application to the owning level's LSP post-restore flush. We key the deferred callback on
	// the OWNER's own (level, name) identity purely to borrow that timing - the resolved-actor argument
	// is ignored; all instigator resolution happens inside ApplyAllRestoredEffects. ResolveOrRegister
	// fires synchronously if the owner is already resolvable. Lifetime=this drops the callback if this
	// component is destroyed first.
	UPersistableActorReferenceManager* PersistableReferenceManager = World->GetSubsystem<UPersistableActorReferenceManager>();
	if (!PersistableReferenceManager)
	{
		UE_LOG(LogPersistenceExamples, Warning, TEXT("[%s] PostRestoreObject: no UPersistableActorReferenceManager; cannot restore gameplay effects."), *GetNameSafe(Owner));
		return;
	}

	ULevel* Level = Owner->GetLevel();
	const FSoftObjectPath LevelPath(Level);
	PersistableReferenceManager->ResolveOrRegister(LevelPath, Owner->GetFName(),
		UPersistableActorReferenceManager::FOnRuntimeActorResolved::CreateUObject(this, &UPersistedAbilitySystemComponent::ApplyAllRestoredEffects), /*Lifetime*/ this);
}

void UPersistedAbilitySystemComponent::ApplyAllRestoredEffects(AActor* ResolvedOwner, bool bSuccess)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	int32 NumApplied = 0;
	for (const FPersistedGameplayEffect& Entry : PersistedState.ActiveEffects)
	{
		// Resolve the instigator. None -> self; otherwise resolve through the reference machinery, which
		// by the level post-restore flush can resolve player- and level-actor references alike.
		AActor* Instigator = Owner;
		if (Entry.Instigator.Type != EPersistableActorReferenceType::None)
		{
			Instigator = Entry.Instigator.TryResolve(World);
			if (!Instigator)
			{
				UE_LOG(LogPersistenceExamples, Warning, TEXT("[%s] Restore: instigator unresolved for effect '%s'; skipping."), *GetNameSafe(Owner), *Entry.EffectClass.ToString());
				continue;
			}
		}

		UClass* EffectClass = Entry.EffectClass.LoadSynchronous();
		if (!EffectClass)
		{
			UE_LOG(LogPersistenceExamples, Warning, TEXT("[%s] Restore: could not load effect class '%s'; skipping."), *GetNameSafe(Owner), *Entry.EffectClass.ToString());
			continue;
		}

		FGameplayEffectContextHandle Context = MakeEffectContext();
		Context.AddInstigator(Instigator, Instigator);

		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(EffectClass, Entry.Level, Context);
		if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
		{
			continue;
		}
		FGameplayEffectSpec& Spec = *SpecHandle.Data;

		// Re-apply captured SetByCaller magnitudes BEFORE pinning duration, since the duration magnitude
		// can itself be SetByCaller-driven.
		for (const TPair<FGameplayTag, float>& Pair : Entry.SetByCallerTagMagnitudes)
		{
			Spec.SetSetByCallerMagnitude(Pair.Key, Pair.Value);
		}
		for (const TPair<FName, float>& Pair : Entry.SetByCallerNameMagnitudes)
		{
			Spec.SetSetByCallerMagnitude(Pair.Key, Pair.Value);
		}

		// Infinite effects (RemainingDuration == -1) keep the CDO's infinite policy; finite ones get the
		// captured remaining time pinned so it isn't recomputed from the level-based magnitude.
		if (Entry.RemainingDuration > 0.f)
		{
			Spec.SetDuration(Entry.RemainingDuration, /*bLockDuration*/ true);
		}

		if (Entry.StackCount > 1)
		{
			Spec.SetStackCount(Entry.StackCount);
		}

		ApplyGameplayEffectSpecToSelf(Spec);
		++NumApplied;
	}

	UE_LOG(LogPersistenceExamples, Log, TEXT("[%s] Restore: re-applied %d/%d persisted gameplay effect(s)"), *GetNameSafe(Owner), NumApplied, PersistedState.ActiveEffects.Num());
}
