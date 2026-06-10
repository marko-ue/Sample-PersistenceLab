// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/DestructibleCrate.h"
#include "AbilitySystemComponent.h"
#include "GAS/HealthAttributeSet.h"
#include "GeometryCollection/PersistedGeometryCollectionComponent.h"
#include "PersistenceExamples.h"

ADestructibleCrate::ADestructibleCrate()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GeometryCollection = CreateDefaultSubobject<UPersistedGeometryCollectionComponent>(TEXT("GeometryCollection"));
	SetRootComponent(GeometryCollection);

	AbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));
	AbilitySystemComp->SetIsReplicated(true);
	// Mixed: GE list replicates to owning client only; tags/attributes replicate to all. Right default for
	// non-pawn world actors. Player ASCs use Full (see APersistenceLabCharacter).
	AbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));
}

UAbilitySystemComponent* ADestructibleCrate::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

void ADestructibleCrate::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AbilitySystemComp->InitAbilityActorInfo(this, this);
}
