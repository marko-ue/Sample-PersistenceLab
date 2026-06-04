// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/DamageExecutionCalculation.h"
#include "GAS/HealthAttributeSet.h"
#include "NativeGameplayTags.h"
#include "PersistenceExamples.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(Tag_Abilities_Parameters_Damage, "Abilities.Parameters.Damage");

UDamageExecutionCalculation::UDamageExecutionCalculation()
{
}

void UDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);

	const float DamageValue = ExecutionParams.GetOwningSpec().GetSetByCallerMagnitude(
		Tag_Abilities_Parameters_Damage, /*WarnIfNotFound=*/true, /*DefaultIfNotFound=*/0.0f);

	UE_LOG(LogPersistenceExamples, Log, TEXT("DamageExec: SetByCaller(%s) = %.2f"),
		*Tag_Abilities_Parameters_Damage.GetTag().ToString(), DamageValue);

	if (DamageValue > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UHealthAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, DamageValue));
	}
}
