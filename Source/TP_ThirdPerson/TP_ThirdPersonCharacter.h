// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TP_ThirdPersonCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UCommandReceiverComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ATP_ThirdPersonCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** IPC command receiver */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="IPC", meta = (AllowPrivateAccess = "true"))
	UCommandReceiverComponent* CommandReceiver;

	/** Diamond counters */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Diamonds")
	int32 AutomaticDiamondCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Diamonds")
	int32 ManualDiamondCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Diamonds")
	bool bUsedManualInputSinceLastReset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Submission")
	FString SubmissionBaseUrl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Submission")
	FString SubmissionStudentId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Submission")
	FString SubmissionStudentMail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Submission")
	FString SubmissionFirstName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Submission")
	FString SubmissionLastName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Submission")
	FString SubmissionGroup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Submission")
	FString SubmissionShareLink;

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
	ATP_ThirdPersonCharacter();	

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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
	UFUNCTION(BlueprintCallable, Category="Diamonds")
	void RegisterDiamondCollected(int32 DiamondIndex, const FString& SecretCode);

	UFUNCTION(BlueprintCallable, Category="Diamonds")
	void RegisterManualInputUsed();

	UFUNCTION(BlueprintCallable, Category="Diamonds")
	void ResetRunTracking();

private:
	bool LoadSubmissionConfig();
	void SendFinalAnswerIfAutomatic(int32 DiamondIndex, const FString& SecretCode);
	void UpdateDiamondDebugOverlay() const;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
