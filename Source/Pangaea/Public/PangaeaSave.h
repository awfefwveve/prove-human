#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PangaeaTypes.h"
#include "PangaeaSave.generated.h"

// 한 판(런)의 진행 상태. 매일 아침 근무 시작 전에 자동 저장된다 (되돌리기 없음).
USTRUCT()
struct FRunState
{
	GENERATED_BODY()

	UPROPERTY() int32 Day = 1;
	UPROPERTY() int32 Seed = 0;
	UPROPERTY() int32 Credits = 30;
	UPROPERTY() int32 Food = 8;
	UPROPERTY() int32 Power = 4;
	UPROPERTY() int32 Morale = 70;
	UPROPERTY() int32 Kits = 0;
	UPROPERTY() int32 Population = 20;
	UPROPERTY() int32 Infiltrated = 0;
	UPROPERTY() int32 HumansKilled = 0;
	UPROPERTY() int32 AiKilled = 0;
	UPROPERTY() int32 Correct = 0;
	UPROPERTY() int32 Total = 0;

	UPROPERTY() bool bGenerator = false;
	UPROPERTY() bool bWatchtower = false;
	UPROPERTY() bool bInterrogation = false;
	UPROPERTY() bool bXRayGate = false;
	UPROPERTY() bool bKennel = false;

	bool Has(EFacility F) const
	{
		switch (F)
		{
		case EFacility::Generator: return bGenerator;
		case EFacility::Watchtower: return bWatchtower;
		case EFacility::Interrogation: return bInterrogation;
		case EFacility::XRayGate: return bXRayGate;
		case EFacility::Kennel: return bKennel;
		default: return false;
		}
	}

	void SetBuilt(EFacility F)
	{
		switch (F)
		{
		case EFacility::Generator: bGenerator = true; break;
		case EFacility::Watchtower: bWatchtower = true; break;
		case EFacility::Interrogation: bInterrogation = true; break;
		case EFacility::XRayGate: bXRayGate = true; break;
		case EFacility::Kennel: bKennel = true; break;
		default: break;
		}
	}
};

UCLASS()
class PANGAEA_API UPangaeaSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() FRunState State;
};
