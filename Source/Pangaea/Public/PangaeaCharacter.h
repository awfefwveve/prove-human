#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PangaeaCharacter.generated.h"

class UCameraComponent;
class USpotLightComponent;
class UInputAction;
class UInputMappingContext;
class APangaeaGameMode;
struct FInputActionValue;
struct FKey;

// 검사관 본인. 걸어 다니다가 부스 의자에서 E로 착석해 검사한다.
UCLASS()
class PANGAEA_API APangaeaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APangaeaCharacter();

	bool IsSeated() const { return bSeated; }
	void SetSeated(bool bSit);
	void StandAt(const FVector& Loc, float Yaw);

	// 손전등: 머리를 바라보며 확대
	void FocusFlashlight(const FVector& Target, float Duration);

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY() TObjectPtr<UCameraComponent> Camera;
	UPROPERTY() TObjectPtr<USpotLightComponent> Flashlight;

	UPROPERTY() TObjectPtr<UInputMappingContext> Mapping;
	UPROPERTY() TObjectPtr<UInputAction> MoveAction;
	UPROPERTY() TObjectPtr<UInputAction> LookAction;
	UPROPERTY() TArray<TObjectPtr<UInputAction>> KeyActions;

	bool bSeated = false;
	float FocusUntil = 0.f;
	FVector FocusTarget = FVector::ZeroVector;
	FRotator SeatRotation = FRotator::ZeroRotator;
	float DefaultFov = 80.f;

	APangaeaGameMode* GM() const;
	void BuildInput();
	UInputAction* MakeKey(const FKey& Key);

	void OnMove(const FInputActionValue& V);
	void OnLook(const FInputActionValue& V);
	void OnInteract();
	void OnTool1(); void OnTool2(); void OnTool3(); void OnTool4();
	void OnQ1(); void OnQ2(); void OnQ3(); void OnQ4();
	void OnPass(); void OnShoot();
};
