// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "HealthBarWidget.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UHealthAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthValuesChanged, float, NewHealth, float, NewMaxHealth);

/**
 * UserWidget base for a health bar bound to a UHealthAttributeSet. Call SetTargetActor (or
 * SetTargetAbilitySystemComponent) from BP after construction; the widget hooks
 * UHealthAttributeSet::OnHealthChanged and exposes Health / MaxHealth / GetHealthFraction +
 * an OnHealthValuesChanged BP event so BP can drive visual bindings.
 */
UCLASS(Blueprintable)
class PERSISTENCEEXAMPLES_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Health")
	void SetTargetActor(AActor* InTarget);

	UFUNCTION(BlueprintCallable, Category="Health")
	void SetTargetAbilitySystemComponent(UAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintCallable, Category="Health")
	void ClearTarget();

	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealthFraction() const;

	UPROPERTY(BlueprintReadOnly, Category="Health")
	float Health = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Health")
	float MaxHealth = 0.f;

	UPROPERTY(BlueprintAssignable, Category="Health")
	FHealthValuesChanged OnHealthValuesChanged;

protected:
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleHealthChanged(UAttributeSet* AttributeSet, float OldValue, float NewValue);

private:
	void RefreshFromSet();

	UPROPERTY()
	TWeakObjectPtr<UHealthAttributeSet> BoundHealthSet;
};
