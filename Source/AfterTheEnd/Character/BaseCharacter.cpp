// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AfterTheEnd/Components/InteractionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(SpringArmComponent);
	ThirdPersonCamera->SetActive(false);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
	                                     FName("head"));
	FirstPersonCamera->SetActive(true);

	bUseControllerRotationYaw = true;
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

	CurrentCamera = MakeWeakObjectPtr(FirstPersonCamera);
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

		EnhancedInputComponent->BindAction(SwitchCameraAction, ETriggerEvent::Started, this,
		                                   &ABaseCharacter::SwitchCameraStarted);
		EnhancedInputComponent->BindAction(ZoomCameraAction, ETriggerEvent::Triggered, this,
		                                   &ABaseCharacter::ZoomCameraTriggered);
	}
}

void ABaseCharacter::MoveTriggered(const FInputActionValue& InputValue)
{
	const FVector2D Value = InputValue.Get<FVector2D>();

	AddMovementInput(GetActorForwardVector(), Value.Y);
	AddMovementInput(GetActorRightVector(), Value.X);
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
	const FVector TraceStart = CurrentCamera->GetComponentLocation();
	const FVector TraceEnd = CurrentCamera->GetComponentLocation() + CurrentCamera->GetForwardVector() *
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

void ABaseCharacter::SwitchCameraStarted()
{
	if (CurrentCamera.Get() == FirstPersonCamera)
	{
		ActivateThirdPersonCamera();
	}
	else
	{
		ActivateFirstPersonCamera();
	}

	SpringArmComponent->TargetArmLength = MinCameraDistance;
}

void ABaseCharacter::ActivateFirstPersonCamera()
{
	FirstPersonCamera->SetActive(true);
	ThirdPersonCamera->SetActive(false);
	CurrentCamera = MakeWeakObjectPtr(FirstPersonCamera);
}

void ABaseCharacter::ActivateThirdPersonCamera()
{
	FirstPersonCamera->SetActive(false);
	ThirdPersonCamera->SetActive(true);
	CurrentCamera = MakeWeakObjectPtr(ThirdPersonCamera);
}

void ABaseCharacter::ZoomCameraTriggered(const FInputActionValue& InputValue)
{
	const float Value = InputValue.Get<float>();

	SpringArmComponent->TargetArmLength += Value * CameraZoomRate;
	SpringArmComponent->TargetArmLength = FMath::Clamp(SpringArmComponent->TargetArmLength, 0,
	                                                   MaxCameraDistance);

	if (CurrentCamera.Get() == ThirdPersonCamera && SpringArmComponent->TargetArmLength <= MinCameraDistance)
	{
		SwitchCameraStarted();
	}
	else if (CurrentCamera.Get() == FirstPersonCamera && SpringArmComponent->TargetArmLength >= MinCameraDistance)
	{
		SwitchCameraStarted();
	}
}