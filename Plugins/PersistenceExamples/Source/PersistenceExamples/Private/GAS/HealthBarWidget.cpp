// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/HealthBarWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/HealthAttributeSet.h"

void UHealthBarWidget::SetTargetActor(AActor* InTarget)
{
	if (!InTarget)
	{
		ClearTarget();
		return;
	}

	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InTarget))
	{
		SetTargetAbilitySystemComponent(ASI->GetAbilitySystemComponent());
	}
	else
	{
		// Fallback: scan the actor for a UAbilitySystemComponent.
		UAbilitySystemComponent* ASC = InTarget->FindComponentByClass<UAbilitySystemComponent>();
		SetTargetAbilitySystemComponent(ASC);
	}
}

void UHealthBarWidget::SetTargetAbilitySystemComponent(UAbilitySystemComponent* InASC)
{
	// Detach from any prior target so we don't double-bind.
	if (BoundHealthSet.IsValid())
	{
		BoundHealthSet->OnHealthChanged.RemoveDynamic(this, &UHealthBarWidget::HandleHealthChanged);
		BoundHealthSet.Reset();
	}

	if (!InASC)
	{
		Health = 0.f;
		MaxHealth = 0.f;
		OnHealthValuesChanged.Broadcast(Health, MaxHealth);
		return;
	}

	UHealthAttributeSet* NewSet = const_cast<UHealthAttributeSet*>(InASC->GetSet<UHealthAttributeSet>());
	if (!NewSet)
	{
		Health = 0.f;
		MaxHealth = 0.f;
		OnHealthValuesChanged.Broadcast(Health, MaxHealth);
		return;
	}

	BoundHealthSet = NewSet;
	NewSet->OnHealthChanged.AddDynamic(this, &UHealthBarWidget::HandleHealthChanged);
	RefreshFromSet();
}

void UHealthBarWidget::ClearTarget()
{
	SetTargetAbilitySystemComponent(nullptr);
}

float UHealthBarWidget::GetHealthFraction() const
{
	return MaxHealth > 0.f ? FMath::Clamp(Health / MaxHealth, 0.f, 1.f) : 0.f;
}

void UHealthBarWidget::NativeDestruct()
{
	ClearTarget();
	Super::NativeDestruct();
}

void UHealthBarWidget::HandleHealthChanged(UAttributeSet* AttributeSet, float OldValue, float NewValue)
{
	// OnHealthChanged fires for both Health and MaxHealth changes (see UHealthAttributeSet),
	// so re-read both rather than trusting NewValue alone.
	RefreshFromSet();
}

void UHealthBarWidget::RefreshFromSet()
{
	if (UHealthAttributeSet* Set = BoundHealthSet.Get())
	{
		Health = Set->GetHealth();
		MaxHealth = Set->GetMaxHealth();
	}
	else
	{
		Health = 0.f;
		MaxHealth = 0.f;
	}
	OnHealthValuesChanged.Broadcast(Health, MaxHealth);
}
