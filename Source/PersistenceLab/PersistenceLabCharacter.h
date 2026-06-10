// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "References/PersistableActorReference.h"
#include "Framework/PersistedObjectInterface.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "PersistenceLabCharacter.generated.h"

class UAbilitySystemComponent;
class UCameraComponent;
class UHealthAttributeSet;
class UInputAction;
class USpringArmComponent;
class UStateTree;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class APersistenceLabCharacter : public ACharacter, public IAbilitySystemInterface, public IPersistedObject
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

public:

	/** StateTree asset assigned to the controller's component when an APersistenceAIController possesses this character. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	TObjectPtr<UStateTree> StateTreeAsset;

	/** Persisted (via LSP) GUID of the StateTree state that was active at save time. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Persistence")
	FGuid StateTreeStateGuid;

	/** Persistable mirror of the StateTree's PatrolTarget global parameter. Updated in PrePersistObject. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Persistence")
	FPersistableActorReference PatrolTargetPersistable;

	/** Persistable mirror of the StateTree's AttackTarget global parameter. Updated in PrePersistObject. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Persistence")
	FPersistableActorReference AttackTargetPersistable;

	/** Whoever last attacked this character. Read by the StateTree's ChasePlayer task as the move-to target. */
	UPROPERTY(BlueprintReadOnly, Category="AI")
	TWeakObjectPtr<AActor> AttackerActor;

	/** GAS surface. Created in the constructor; InitAbilityActorInfo runs in PostInitializeComponents. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComp;

	/** Health attribute set registered with AbilitySystemComp. Created in the constructor. Health/MaxHealth persist directly via LSP. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UHealthAttributeSet> HealthSet;

public:

	/** Sets AttackerActor and sends AI.Event.Attacked to the StateTree. Call from Blueprint when the player attacks. */
	UFUNCTION(BlueprintCallable, Category="AI")
	void BeAttacked(AActor* Attacker);

	//~ IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

public:

	/** Constructor */
	APersistenceLabCharacter();

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Hooks the ASC's actor info to this actor (owner + avatar). Runs once at spawn. */
	virtual void PostInitializeComponents() override;

	/**
	 * Starts the StateTree once the controller has fully possessed this pawn (AController::OnPossess
	 * calls PossessedBy *before* SetPawn, so AIController->GetPawn() is null inside PossessedBy and
	 * the AI schema fails to resolve its context. NotifyRestarted fires after SetPawn).
	 */
	virtual void NotifyRestarted() override;

	/** Stops the StateTree when the controller is removed. */
	virtual void UnPossessed() override;

	//~ IPersistedObject
	virtual void PrePersistObject_Implementation() override;
	virtual void PostRestoreObject_Implementation(const TArray<FName>& RestoredPropertyNames) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

