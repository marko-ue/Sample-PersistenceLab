// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "DamageExecutionCalculation.generated.h"

/**
 * Damage execution. Reads SetByCaller magnitude via tag "Abilities.Parameters.Damage" and emits
 * it as an additive modifier on UHealthAttributeSet::Damage. The attribute set's
 * PostGameplayEffectExecute then converts the meta Damage into a clamped Health delta.
 *
 * Trimmed from AbilitiesLab's ULabEffectDamageExecution: no DamageBoost/DamageResistance capture,
 * no damage-type tag filtering, no HandleGameplayEvent feedback. Add those back if the demo grows.
 */
UCLASS()
class PERSISTENCEEXAMPLES_API UDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

	UDamageExecutionCalculation();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
