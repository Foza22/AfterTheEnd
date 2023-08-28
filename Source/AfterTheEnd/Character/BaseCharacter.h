// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class AFTERTHEEND_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*
	 * CAMERA
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera)
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera)
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera)
	UCameraComponent* ThirdPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera)
	float MinCameraDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera)
	float MaxCameraDistance = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera)
	float CameraZoomRate = 10.f;

	/*
	 * INTERACTION
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Interaction)
	float InteractionDistance = 500.f;
	
	/*
	 * INPUT
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* CrouchAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction;
    	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InteractAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* SwitchCameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ZoomCameraAction;

	void MoveTriggered(const FInputActionValue& InputValue);
	void LookTriggered(const FInputActionValue& InputValue);

	void CrouchStarted();
	void CrouchCompleted();
	
	void AttackStarted();
	void InteractStarted();

	void SwitchCameraStarted();
	void ZoomCameraTriggered(const FInputActionValue& InputValue);

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UCameraComponent> CurrentCamera;

	void ActivateFirstPersonCamera();
	void ActivateThirdPersonCamera();
};
