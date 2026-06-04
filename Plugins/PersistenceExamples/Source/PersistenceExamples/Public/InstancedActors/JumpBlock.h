// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "InstancedActors/ExampleInstancedActorComponent.h"
#include "JumpBlock.generated.h"

UCLASS()
class PERSISTENCEEXAMPLES_API AJumpBlock : public AActor, public IExampleVisualStateRestorable
{
	GENERATED_BODY()

public:
	AJumpBlock();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ IExampleVisualStateRestorable
	virtual void OnVisualStateRestored_Implementation(int32 VisualStateIndex) override;

	UFUNCTION(BlueprintCallable)
	void SetVisualState(int32 NewVisualState);

protected:
	virtual void BeginPlay() override;

private:
	void ApplyVisualState();

	UFUNCTION()
	void OnRep_VisualState();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UExampleInstancedActorComponent> InstancedActorsComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", ReplicatedUsing = OnRep_VisualState)
	int32 VisualState = 0;
};
