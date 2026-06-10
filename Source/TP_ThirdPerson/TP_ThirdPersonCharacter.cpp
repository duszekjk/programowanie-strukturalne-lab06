// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_ThirdPersonCharacter.h"
#include "Engine/LocalPlayer.h"
#include "CommandReceiverComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "HttpModule.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TP_ThirdPerson.h"

ATP_ThirdPersonCharacter::ATP_ThirdPersonCharacter()
	: AutomaticDiamondCount(0)
	, ManualDiamondCount(0)
	, bUsedManualInputSinceLastReset(false)
	, SubmissionBaseUrl(TEXT("https://www.duszekjk.com/programowaniestrukturalne/api/submit_answer/"))
	, SubmissionStudentId(TEXT(""))
	, SubmissionStudentMail(TEXT(""))
	, SubmissionFirstName(TEXT(""))
	, SubmissionLastName(TEXT(""))
	, SubmissionGroup(TEXT(""))
	, SubmissionShareLink(TEXT(""))
{
	PrimaryActorTick.bCanEverTick = true;

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

	CommandReceiver = CreateDefaultSubobject<UCommandReceiverComponent>(TEXT("CommandReceiver"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ATP_ThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();
	LoadSubmissionConfig();
	ResetRunTracking();
}

void ATP_ThirdPersonCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateDiamondDebugOverlay();
}

void ATP_ThirdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATP_ThirdPersonCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ATP_ThirdPersonCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATP_ThirdPersonCharacter::Look);
	}
	else
	{
		UE_LOG(LogTP_ThirdPerson, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATP_ThirdPersonCharacter::Move(const FInputActionValue& Value)
{
	RegisterManualInputUsed();
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ATP_ThirdPersonCharacter::Look(const FInputActionValue& Value)
{
	RegisterManualInputUsed();
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ATP_ThirdPersonCharacter::DoMove(float Right, float Forward)
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

void ATP_ThirdPersonCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ATP_ThirdPersonCharacter::DoJumpStart()
{
	RegisterManualInputUsed();
	// signal the character to jump
	Jump();
}

void ATP_ThirdPersonCharacter::DoJumpEnd()
{
	RegisterManualInputUsed();
	// signal the character to stop jumping
	StopJumping();
}

void ATP_ThirdPersonCharacter::RegisterDiamondCollected(int32 DiamondIndex, const FString& SecretCode)
{
	if (bUsedManualInputSinceLastReset)
	{
		++ManualDiamondCount;
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("Diamond %d collected manually: %s"), DiamondIndex + 1, *SecretCode);
		return;
	}

	++AutomaticDiamondCount;
	UE_LOG(LogTP_ThirdPerson, Display, TEXT("Diamond %d collected automatically: %s"), DiamondIndex + 1, *SecretCode);
	SendFinalAnswerIfAutomatic(DiamondIndex, SecretCode);
}

void ATP_ThirdPersonCharacter::RegisterManualInputUsed()
{
	bUsedManualInputSinceLastReset = true;
}

void ATP_ThirdPersonCharacter::ResetRunTracking()
{
	bUsedManualInputSinceLastReset = false;
}

bool ATP_ThirdPersonCharacter::LoadSubmissionConfig()
{
	const FString ConfigPath = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("SubmissionConfig.json"));
	FString JsonText;

	if (!FFileHelper::LoadFileToString(JsonText, *ConfigPath))
	{
		UE_LOG(LogTP_ThirdPerson, Display, TEXT("Submission config not found at %s; using defaults"), *ConfigPath);
		return false;
	}

	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogTP_ThirdPerson, Warning, TEXT("Failed to parse submission config: %s"), *ConfigPath);
		return false;
	}

	auto ReadTextField = [RootObject](const TCHAR* FieldName, FString& OutValue) -> void
	{
		FString StringValue;
		if (RootObject->TryGetStringField(FieldName, StringValue))
		{
			OutValue = StringValue;
			return;
		}

		double NumericValue = 0.0;
		if (RootObject->TryGetNumberField(FieldName, NumericValue))
		{
			OutValue = FString::Printf(TEXT("%.0f"), NumericValue);
		}
	};

	ReadTextField(TEXT("base_url"), SubmissionBaseUrl);
	ReadTextField(TEXT("student_index"), SubmissionStudentId);
	ReadTextField(TEXT("student_mail"), SubmissionStudentMail);
	ReadTextField(TEXT("first_name"), SubmissionFirstName);
	ReadTextField(TEXT("last_name"), SubmissionLastName);
	ReadTextField(TEXT("group_number"), SubmissionGroup);
	ReadTextField(TEXT("share_link"), SubmissionShareLink);

	UE_LOG(LogTP_ThirdPerson, Display, TEXT("Loaded submission config from %s"), *ConfigPath);
	return true;
}

void ATP_ThirdPersonCharacter::SendFinalAnswerIfAutomatic(int32 DiamondIndex, const FString& SecretCode)
{
	if (SubmissionBaseUrl.IsEmpty())
	{
		UE_LOG(LogTP_ThirdPerson, Warning, TEXT("SubmissionBaseUrl is empty; not sending answer for diamond %d"), DiamondIndex + 1);
		return;
	}

	const FString TaskId = FString::Printf(TEXT("PS_Diamond_%02d"), DiamondIndex + 1);
	const FString FinalAnswer = SecretCode;
	const FString FinalAnswerSend = FString::Printf(TEXT("%s\n%d znaków"), *FinalAnswer.Left(300), FinalAnswer.Len());

	TSharedPtr<FJsonObject> BodyObject = MakeShared<FJsonObject>();
	BodyObject->SetStringField(TEXT("student_id"), SubmissionStudentId);
	BodyObject->SetStringField(TEXT("student_mail"), SubmissionStudentMail);
	BodyObject->SetStringField(TEXT("first_name"), SubmissionFirstName);
	BodyObject->SetStringField(TEXT("last_name"), SubmissionLastName);
	BodyObject->SetStringField(TEXT("task"), TaskId);
	BodyObject->SetStringField(TEXT("grupa"), SubmissionGroup);
	BodyObject->SetStringField(TEXT("answer"), FinalAnswerSend);
	BodyObject->SetStringField(TEXT("share_link"), SubmissionShareLink);

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	if (!FJsonSerializer::Serialize(BodyObject.ToSharedRef(), Writer))
	{
		UE_LOG(LogTP_ThirdPerson, Warning, TEXT("Failed to serialize submission payload for %s"), *TaskId);
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(SubmissionBaseUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	Request->SetContentAsString(Body);
	Request->OnProcessRequestComplete().BindLambda(
		[TaskId, FinalAnswerSend](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
		{
			if (!bSucceeded || !HttpResponse.IsValid())
			{
				UE_LOG(LogTP_ThirdPerson, Warning, TEXT("Failed to submit %s"), *TaskId);
				return;
			}

			UE_LOG(LogTP_ThirdPerson, Display, TEXT("Submitted %s. Status: %d"), *TaskId, HttpResponse->GetResponseCode());
			UE_LOG(LogTP_ThirdPerson, Display, TEXT("Answer: %s"), *FinalAnswerSend);
		});

	Request->ProcessRequest();
}

void ATP_ThirdPersonCharacter::UpdateDiamondDebugOverlay() const
{
	if (!GEngine)
	{
		return;
	}

	const FString Message = FString::Printf(
		TEXT("Diamonds auto=%d manual=%d manual_input=%s"),
		AutomaticDiamondCount,
		ManualDiamondCount,
		bUsedManualInputSinceLastReset ? TEXT("yes") : TEXT("no"));

	GEngine->AddOnScreenDebugMessage(reinterpret_cast<uint64>(this) + 1, 0.0f, FColor::Cyan, Message);
}
