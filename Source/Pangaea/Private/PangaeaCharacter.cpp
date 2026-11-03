#include "PangaeaCharacter.h"
#include "PangaeaGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "Engine/LocalPlayer.h"

APangaeaCharacter::APangaeaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(34.f, 90.f);

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->MaxWalkSpeed = 280.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	Camera->bUsePawnControlRotation = true;
	Camera->SetFieldOfView(DefaultFov);

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(Camera);
	Flashlight->SetMobility(EComponentMobility::Movable);
	Flashlight->SetIntensityUnits(ELightUnits::Candelas);
	Flashlight->SetIntensity(0.f);
	Flashlight->SetInnerConeAngle(5.f);
	Flashlight->SetOuterConeAngle(11.f);
	Flashlight->SetAttenuationRadius(1200.f);
	Flashlight->SetLightColor(FLinearColor(1.f, 0.96f, 0.88f));
}

void APangaeaCharacter::BeginPlay()
{
	Super::BeginPlay();
}

APangaeaGameMode* APangaeaCharacter::GM() const
{
	return GetWorld() ? Cast<APangaeaGameMode>(GetWorld()->GetAuthGameMode()) : nullptr;
}

UInputAction* APangaeaCharacter::MakeKey(const FKey& Key)
{
	UInputAction* A = NewObject<UInputAction>(this);
	A->ValueType = EInputActionValueType::Boolean;
	Mapping->MapKey(A, Key);
	KeyActions.Add(A);
	return A;
}

void APangaeaCharacter::BuildInput()
{
	if (Mapping) return;
	Mapping = NewObject<UInputMappingContext>(this);

	MoveAction = NewObject<UInputAction>(this);
	MoveAction->ValueType = EInputActionValueType::Axis2D;
	{
		FEnhancedActionKeyMapping& W = Mapping->MapKey(MoveAction, EKeys::W);
		W.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(this));
		FEnhancedActionKeyMapping& S = Mapping->MapKey(MoveAction, EKeys::S);
		S.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(this));
		S.Modifiers.Add(NewObject<UInputModifierNegate>(this));
		FEnhancedActionKeyMapping& A = Mapping->MapKey(MoveAction, EKeys::A);
		A.Modifiers.Add(NewObject<UInputModifierNegate>(this));
		Mapping->MapKey(MoveAction, EKeys::D);
	}

	LookAction = NewObject<UInputAction>(this);
	LookAction->ValueType = EInputActionValueType::Axis2D;
	Mapping->MapKey(LookAction, EKeys::Mouse2D);

	// 순서 고정: E, 1~4, Z X C V, P, K
	MakeKey(EKeys::E);
	MakeKey(EKeys::One); MakeKey(EKeys::Two); MakeKey(EKeys::Three); MakeKey(EKeys::Four);
	MakeKey(EKeys::Z); MakeKey(EKeys::X); MakeKey(EKeys::C); MakeKey(EKeys::V);
	MakeKey(EKeys::P); MakeKey(EKeys::K);
}

void APangaeaCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	BuildInput();
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Sub->AddMappingContext(Mapping, 0);
		}
	}
}

void APangaeaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	BuildInput();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC) return;

	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APangaeaCharacter::OnMove);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APangaeaCharacter::OnLook);

	int32 i = 0;
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnInteract);
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnTool1);
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnTool2);
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnTool3);
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnTool4);
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnQ1);
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnQ2);
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnQ3);
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnQ4);
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnPass);
	EIC->BindAction(KeyActions[i++], ETriggerEvent::Started, this, &APangaeaCharacter::OnShoot);
}

void APangaeaCharacter::OnMove(const FInputActionValue& V)
{
	APangaeaGameMode* G = GM();
	if (bSeated || !G || !G->CanWalk()) return;
	const FVector2D In = V.Get<FVector2D>();
	const FRotator Yaw(0.f, Controller ? Controller->GetControlRotation().Yaw : GetActorRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), In.Y);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), In.X);
}

void APangaeaCharacter::OnLook(const FInputActionValue& V)
{
	APangaeaGameMode* G = GM();
	if (bSeated || !G || !G->CanWalk()) return;
	const FVector2D In = V.Get<FVector2D>();
	AddControllerYawInput(In.X);
	AddControllerPitchInput(-In.Y);
}

void APangaeaCharacter::OnInteract() { if (APangaeaGameMode* G = GM()) G->Interact(); }
void APangaeaCharacter::OnTool1() { if (APangaeaGameMode* G = GM()) G->UseTool(ETool::Thermo); }
void APangaeaCharacter::OnTool2() { if (APangaeaGameMode* G = GM()) G->UseTool(ETool::Flashlight); }
void APangaeaCharacter::OnTool3() { if (APangaeaGameMode* G = GM()) G->UseTool(ETool::BloodKit); }
void APangaeaCharacter::OnTool4() { if (APangaeaGameMode* G = GM()) G->UseTool(ETool::XRay); }
void APangaeaCharacter::OnQ1() { if (APangaeaGameMode* G = GM()) G->Ask(0); }
void APangaeaCharacter::OnQ2() { if (APangaeaGameMode* G = GM()) G->Ask(1); }
void APangaeaCharacter::OnQ3() { if (APangaeaGameMode* G = GM()) G->Ask(2); }
void APangaeaCharacter::OnQ4() { if (APangaeaGameMode* G = GM()) G->Ask(3); }
void APangaeaCharacter::OnPass() { if (APangaeaGameMode* G = GM()) G->DecidePass(); }
void APangaeaCharacter::OnShoot() { if (APangaeaGameMode* G = GM()) G->RequestShoot(); }

void APangaeaCharacter::SetSeated(bool bSit)
{
	bSeated = bSit;
	if (bSit)
	{
		GetCharacterMovement()->DisableMovement();
		Camera->SetRelativeLocation(FVector(0.f, 0.f, 26.f));
		SeatRotation = FRotator(8.f, 0.f, 0.f); // 창구 너머 얼굴 높이
		if (Controller) Controller->SetControlRotation(SeatRotation);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
		FocusUntil = 0.f;
	}
}

void APangaeaCharacter::StandAt(const FVector& Loc, float Yaw)
{
	SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics);
	if (Controller) Controller->SetControlRotation(FRotator(0.f, Yaw, 0.f));
}

void APangaeaCharacter::FocusFlashlight(const FVector& Target, float Duration)
{
	FocusTarget = Target;
	FocusUntil = GetWorld()->GetTimeSeconds() + Duration;
}

void APangaeaCharacter::Tick(float Dt)
{
	Super::Tick(Dt);
	const float Now = GetWorld()->GetTimeSeconds();
	const bool bFocus = bSeated && Now < FocusUntil;

	if (bSeated && Controller)
	{
		const FRotator Want = bFocus ? (FocusTarget - Camera->GetComponentLocation()).Rotation() : SeatRotation;
		Controller->SetControlRotation(FMath::RInterpTo(Controller->GetControlRotation(), Want, Dt, 8.f));
	}

	Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView, bFocus ? 20.f : DefaultFov, Dt, 7.f));
	Flashlight->SetIntensity(FMath::FInterpTo(Flashlight->Intensity, bFocus ? 1500.f : 0.f, Dt, 14.f));
}
