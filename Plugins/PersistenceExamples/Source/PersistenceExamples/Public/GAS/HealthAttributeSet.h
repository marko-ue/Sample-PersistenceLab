// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Delegates/DelegateCombinations.h"
#include "Framework/PersistedObjectInterface.h"
#include "HealthAttributeSet.generated.h"

struct FGameplayEffectModCallbackData;

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAttributeChangedEvent, UAttributeSet*, AttributeSet, float, OldValue, float, NewValue);

/**
 * Minimal health attribute set for PersistenceLab demo. Lifted and pared down from AbilitiesLab's
 * ULabHealthAttributeSet. Tracks current Health and MaxHealth; Damage is a meta attribute that the
 * damage execution writes into and PostGameplayEffectExecute converts into a Health delta.
 *
 * Persistence: Health and MaxHealth are persisted directly by LSP (see Config/DefaultEngine.ini
 * entries under LevelStreamingPersistenceSettings). PostRestoreObject re-publishes the restored
 * values through SetHealth/SetMaxHealth so the ASC's aggregator state matches the directly-written
 * FGameplayAttributeData fields.
 */
UCLASS()
class PERSISTENCEEXAMPLES_API UHealthAttributeSet : public UAttributeSet, public IPersistedObject
{
	GENERATED_BODY()

public:
	UHealthAttributeSet();

	/** Current health. Clamped to [0, MaxHealth] via PreAttributeChange. Persisted directly by LSP. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;

	/** Upper limit for health. Persisted directly by LSP. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;

	/** Meta attribute the damage execution writes into; PostGameplayEffectExecute converts to -Health. */
	UPROPERTY(VisibleAnywhere, meta = (HideFromModifiers))
	FGameplayAttributeData Damage;

	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Damage);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	/** Fires when Health changes, or when MaxHealth changes (so health bars can rescale). */
	UPROPERTY(BlueprintAssignable)
	FAttributeChangedEvent OnHealthChanged;

	//~ IPersistedObject
	virtual void PostRestoreObject_Implementation(const TArray<FName>& RestoredPropertyNames) override;
};
