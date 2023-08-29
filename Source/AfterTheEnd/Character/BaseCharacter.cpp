// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AfterTheEnd/Components/InteractionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);

	bUseControllerRotationYaw = true;

	GetMesh()->SetOwnerNoSee(true);
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseCharacter::MoveTriggered);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseCharacter::LookTriggered);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABaseCharacter::CrouchStarted);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this,
		                                   &ABaseCharacter::CrouchCompleted);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ABaseCharacter::AttackStarted);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this,
		                                   &ABaseCharacter::InteractStarted);
	}
}

void ABaseCharacter::MoveTriggered(const FInputActionValue& InputValue)
{
	const FVector2D Value = InputValue.Get<FVector2D>();

	FRotator ControlRotation = GetControlRotation();
	FVector ForwardVector = UKismetMathLibrary::GetForwardVector({0.f,ControlRotation.Yaw,0.f});
	FVector RightVector = UKismetMathLibrary::GetRightVector({0.f, ControlRotation.Yaw, ControlRotation.Roll});

	AddMovementInput(ForwardVector, Value.Y);
	AddMovementInput(RightVector, Value.X);
}

void ABaseCharacter::LookTriggered(const FInputActionValue& InputValue)
{
	const FVector2D Value = InputValue.Get<FVector2D>();

	AddControllerYawInput(Value.X);
	AddControllerPitchInput(Value.Y);
}

void ABaseCharacter::CrouchStarted()
{
	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}

	Crouch();
}

void ABaseCharacter::CrouchCompleted()
{
	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}

	UnCrouch();
}

void ABaseCharacter::AttackStarted()
{
}

void ABaseCharacter::InteractStarted()
{
	// Move to GA later
	const FVector TraceStart = FirstPersonCamera->GetComponentLocation();
	const FVector TraceEnd = FirstPersonCamera->GetComponentLocation() + FirstPersonCamera->GetForwardVector() *
		InteractionDistance;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility);

	if (HitResult.bBlockingHit)
	{
		UActorComponent* OutComponent = HitResult.GetActor()->GetComponentByClass(UInteractionComponent::StaticClass());
		UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(OutComponent);
		if (InteractionComponent)
		{
			InteractionComponent->Interact(this);
		}
	}
}