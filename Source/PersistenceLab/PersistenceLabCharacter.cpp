// Copyright Epic Games, Inc. All Rights Reserved.

#include "PersistenceLabCharacter.h"
#include "AbilitySystemComponent.h"
#include "AI/PersistedStateTreeComponent.h"
#include "AI/PersistenceAIController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "GAS/HealthAttributeSet.h"
#include "GAS/PersistedAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NativeGameplayTags.h"
#include "PersistenceLab.h"
#include "StateTree.h"
#include "StructUtils/PropertyBag.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(Tag_AI_Event_Attacked, "AI.Event.Attacked");

namespace
{
	const FName Name_PatrolTarget(TEXT("PatrolTarget"));
	const FName Name_AttackTarget(TEXT("AttackTarget"));
}

APersistenceLabCharacter::APersistenceLabCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// AI is driven by an APersistenceAIController which owns the StateTree component.
	AIControllerClass = APersistenceAIController::StaticClass();

	// GAS surface. Player pawn -> Full replication mode (vs Mixed on world actors like ADestructibleCrate).
	AbilitySystemComp = CreateDefaultSubobject<UPersistedAbilitySystemComponent>(TEXT("AbilitySystemComp"));
	AbilitySystemComp->SetIsReplicated(true);
	AbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Full);
	HealthSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void APersistenceLabCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AbilitySystemComp->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* APersistenceLabCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

void APersistenceLabCharacter::NotifyRestarted()
{
	Super::NotifyRestarted();

	APersistenceAIController* AIController = Cast<APersistenceAIController>(GetController());
	if (!AIController || !AIController->StateTreeComponent || !StateTreeAsset)
	{
		return;
	}

	// Apply and start the state tree
	AIController->StateTreeComponent->SetStateTree(StateTreeAsset);
	AIController->StateTreeComponent->StartLogic();

	// Restore actor-typed globals from the persistables before forcing the tree to the saved leaf.
	const auto WriteGlobalFromPersistable = [&](FName Name, const FPersistableActorReference& Ref)
	{
		AActor* Resolved = Ref.TryResolve(GetWorld());
		AIController->StateTreeComponent->SetGlobalObject(Name, Resolved);
		if (Ref.Type != EPersistableActorReferenceType::None && !Resolved)
		{
			UE_LOG(LogPersistenceLab, Warning, TEXT("[%s] NotifyRestarted: %s persistable (Type=%d) failed to resolve"),
				*GetName(), *Name.ToString(), (int32)Ref.Type);
		}
		else
		{
			UE_LOG(LogPersistenceLab, Log, TEXT("[%s] NotifyRestarted: %s -> %s"),
				*GetName(), *Name.ToString(), *GetNameSafe(Resolved));
		}
	};

	WriteGlobalFromPersistable(Name_PatrolTarget, PatrolTargetPersistable);
	WriteGlobalFromPersistable(Name_AttackTarget, AttackTargetPersistable);

	// If we have reloaded a state guid, force the state tree to that state now.
	if (StateTreeStateGuid.IsValid())
	{
		// Force the transition to the saved state from previous session, represented by its guid
		const EStateTreeRunStatus Status = AIController->StateTreeComponent->ForceTransitionToState(StateTreeStateGuid);
		UE_LOG(LogPersistenceLab, Log, TEXT("[%s] Restored StateTree state %s (status=%d)"), *GetName(), *StateTreeStateGuid.ToString(), (int32)Status);

		// Saved guid is only applied once
		StateTreeStateGuid = FGuid();
	}
}

void APersistenceLabCharacter::UnPossessed()
{
	if (APersistenceAIController* AIController = Cast<APersistenceAIController>(GetController()))
	{
		if (AIController->StateTreeComponent)
		{
			AIController->StateTreeComponent->StopLogic(TEXT("Unpossessed"));
		}
	}

	Super::UnPossessed();
}

void APersistenceLabCharacter::BeAttacked(AActor* Attacker)
{
	AttackerActor = Attacker;

	if (APersistenceAIController* AIController = Cast<APersistenceAIController>(GetController()))
	{
		if (UPersistedStateTreeComponent* Comp = AIController->StateTreeComponent)
		{
			// Update the global before sending the event, so the transition's target state
			// reads the new value during its EnterState (e.g., MoveTo reads AttackTarget).
			const EPropertyBagResult Result = Comp->SetGlobalObject(Name_AttackTarget, Attacker);
			const TCHAR* ResultStr =
				Result == EPropertyBagResult::Success ? TEXT("Success") :
				Result == EPropertyBagResult::PropertyNotFound ? TEXT("PropertyNotFound (asset is missing 'AttackTarget' global)") :
				Result == EPropertyBagResult::TypeMismatch ? TEXT("TypeMismatch (asset's 'AttackTarget' type rejects this actor)") :
				Result == EPropertyBagResult::OutOfBounds ? TEXT("OutOfBounds") :
				Result == EPropertyBagResult::DuplicatedValue ? TEXT("DuplicatedValue") :
				TEXT("Unknown");
			const UObject* Readback = Comp->GetGlobalObject(Name_AttackTarget);
			UE_LOG(LogPersistenceLab, Log,
				TEXT("[%s] BeAttacked: SetGlobalObject(AttackTarget, %s) -> %s; readback=%s"),
				*GetName(), *GetNameSafe(Attacker), ResultStr, *GetNameSafe(Readback));

			Comp->SendStateTreeEvent(Tag_AI_Event_Attacked);
		}
	}
}

void APersistenceLabCharacter::PrePersistObject_Implementation()
{
	StateTreeStateGuid = FGuid();
	PatrolTargetPersistable = FPersistableActorReference();
	AttackTargetPersistable = FPersistableActorReference();

	if (APersistenceAIController* AIController = Cast<APersistenceAIController>(GetController()))
	{
		if (UPersistedStateTreeComponent* Comp = AIController->StateTreeComponent)
		{
			StateTreeStateGuid = Comp->GetActiveLeafStateId();
			PatrolTargetPersistable.SetFromActor(Cast<AActor>(Comp->GetGlobalObject(Name_PatrolTarget)));
			AttackTargetPersistable.SetFromActor(Cast<AActor>(Comp->GetGlobalObject(Name_AttackTarget)));
		}
	}

	UE_LOG(LogPersistenceLab, Log,
		TEXT("[%s] PrePersistObject: StateTreeStateGuid=%s, Patrol(Type=%d,Idx=%d,Level=%s,Name=%s), Attack(Type=%d,Idx=%d,Level=%s,Name=%s)"),
		*GetName(), *StateTreeStateGuid.ToString(),
		(int32)PatrolTargetPersistable.Type, PatrolTargetPersistable.RuntimeIndex,
		*PatrolTargetPersistable.LevelPath.ToString(), *PatrolTargetPersistable.ActorName.ToString(),
		(int32)AttackTargetPersistable.Type, AttackTargetPersistable.RuntimeIndex,
		*AttackTargetPersistable.LevelPath.ToString(), *AttackTargetPersistable.ActorName.ToString());
}

void APersistenceLabCharacter::PostRestoreObject_Implementation(const TArray<FName>& RestoredPropertyNames)
{
	UWorld* World = GetWorld();
	const AActor* ResolvedPatrol = PatrolTargetPersistable.TryResolve(World);
	const AActor* ResolvedAttack = AttackTargetPersistable.TryResolve(World);

	UE_LOG(LogPersistenceLab, Log,
		TEXT("[%s] PostRestoreObject: StateTreeStateGuid=%s, %d properties restored. Patrol(Type=%d) -> %s. Attack(Type=%d) -> %s."),
		*GetName(), *StateTreeStateGuid.ToString(),
		RestoredPropertyNames.Num(),
		(int32)PatrolTargetPersistable.Type, *GetNameSafe(ResolvedPatrol),
		(int32)AttackTargetPersistable.Type, *GetNameSafe(ResolvedAttack));
}

void APersistenceLabCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APersistenceLabCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APersistenceLabCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APersistenceLabCharacter::Look);
	}
	else
	{
		UE_LOG(LogPersistenceLab, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APersistenceLabCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void APersistenceLabCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void APersistenceLabCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void APersistenceLabCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void APersistenceLabCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void APersistenceLabCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
