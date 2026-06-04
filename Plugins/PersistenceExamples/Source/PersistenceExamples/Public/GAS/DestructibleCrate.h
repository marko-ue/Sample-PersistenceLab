// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructibleCrate.generated.h"

class UAbilitySystemComponent;
class UHealthAttributeSet;
class UPersistableGeometryCollectionComponent;

/**
 * Actor base for the destructible-crate demo. Owns an ASC + UHealthAttributeSet. LSP persists
 * Health / MaxHealth directly on the AttributeSet subobject, and per-piece fracture pose on the
 * UPersistableGeometryCollectionComponent (see Config/DefaultEngine.ini entries under
 * LevelStreamingPersistenceSettings). The crate itself carries no LSP-persisted actor properties.
 */
UCLASS()
class PERSISTENCEEXAMPLES_API ADestructibleCrate : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADestructibleCrate();

	//~ IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Fracturable mesh + persistence root. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Crate")
	TObjectPtr<UPersistableGeometryCollectionComponent> GeometryCollection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UHealthAttributeSet> HealthSet;

protected:
	virtual void PostInitializeComponents() override;
};
