#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PangaeaTypes.h"
#include "PangaeaSave.h"
#include "InspectionSession.h"
#include "PangaeaGameMode.generated.h"

class ANpcActor;
class APangaeaCharacter;
class SPangaeaHUD;
class ADirectionalLight;
class APointLight;
class UStaticMesh;

// 게임 전체 진행 담당: 하루 루프, 검사 루프, 밤 정산, 시설, 엔딩, 저장, 월드 배치
UCLASS()
class PANGAEA_API APangaeaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APangaeaGameMode();

	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	// ── 조회 (UI용) ──
	EPhase GetPhase() const { return Phase; }
	const FRunState& State() const { return S; }
	const FDayRule& Rule() const { return DayRule; }
	const FInspectionSession* Session() const { return Current.Get(); }
	bool IsSeated() const;
	bool IsNearSeat() const;
	bool IsBusy() const { return bBusy; }
	bool CanWalk() const { return Phase == EPhase::Shift; }
	bool IsShootArmed() const;
	bool IsNpcArriving() const;
	int32 WaitingCount() const;
	FText ClockText() const;
	FText ShiftLabel() const;
	float GetFade() const { return Fade; }
	bool HasSave() const;
	int32 SavedDay() const;
	bool IsGoreOn() const { return bGore; }
	const TArray<FReportLine>& Report() const { return ReportLines; }
	const TArray<FText>& NightLines() const { return Summary; }
	bool HasPendingEnding() const { return !EndingTitle.IsEmpty(); }
	FText GetEndingTitle() const { return EndingTitle; }
	FText GetEndingText() const { return EndingText; }

	// ── 행동 ──
	void NewGame();
	void ContinueGame();
	void QuitGame();
	void ToggleGore();
	void BeginShift();     // 브리핑 → 근무 (휴무일이면 휴식 처리)
	void Interact();       // E: 착석/기립
	void UseTool(ETool T);
	void Ask(int32 Idx);
	void DecidePass();
	void RequestShoot();   // 두 번 눌러야 사살
	void ToFacilities();
	bool CanBuild(EFacility F) const;
	void Build(EFacility F);
	bool CanBuy(int32 Item) const;
	void Buy(int32 Item);
	static int32 ShopCount() { return 4; }
	static int32 ShopCost(int32 Item);
	FText ShopLabel(int32 Item) const;
	void NextDay();
	void ToTitle();

private:
	FRunState S;
	FDayRule DayRule;
	EPhase Phase = EPhase::Title;

	TArray<FNpcProfile> Queue;
	int32 NextIndex = 0;
	TUniquePtr<FInspectionSession> Current;
	TArray<FDecisionRecord> Results;
	TArray<FReportLine> ReportLines;
	TArray<FText> Summary;
	FText EndingTitle, EndingText;

	float Elapsed = 0.f;
	bool bBusy = false;
	float ShootArmedUntil = 0.f;
	float FlashReadyAt = 0.f;
	float Fade = 0.f;
	float FadeTarget = 0.f;
	bool bGore = true;

	UPROPERTY() TObjectPtr<ANpcActor> CurrentNpc;
	UPROPERTY() TArray<TObjectPtr<ANpcActor>> Leaving;
	UPROPERTY() TArray<TObjectPtr<AActor>> Corpses;
	UPROPERTY() TArray<TObjectPtr<AActor>> Crowd;
	UPROPERTY() TArray<TObjectPtr<AActor>> FacilityActors;
	UPROPERTY() TObjectPtr<ADirectionalLight> Sun;
	UPROPERTY() TObjectPtr<APointLight> WindowLamp;
	UPROPERTY() TObjectPtr<APointLight> BoothLamp;

	TSharedPtr<SPangaeaHUD> Hud;

	// 배치 좌표 (cm). 부스 정면 = +X
	FVector SeatLoc = FVector(60.f, 0.f, 92.f);
	FVector StandLoc = FVector(0.f, 70.f, 92.f);
	FVector WindowPoint = FVector(275.f, 0.f, 0.f);
	FVector SpawnPoint = FVector(1400.f, 0.f, 0.f);
	int32 CachedSaveDay = 0;
	bool bInputApplied = false;

	APangaeaCharacter* Player() const;
	void SetPhase(EPhase P);
	void SetCursorMode(bool bCursor);
	void StartDay();
	void SaveRun();
	void ApplyShiftLighting();

	void CallNext();
	void CallNextOrEnd();
	void Record(EDecision D);
	void Shoot();
	void EndShift();
	void ComputeNight(bool bRestDay);
	void SetEnding(const FText& Title, const FText& Text);
	void FinalEnding();
	void ClearDayActors();
	void RefreshCrowd();

	// 월드 배치
	void BuildWorld();
	AActor* Box(const FVector& Loc, const FVector& SizeCm, const FLinearColor& Color, bool bCollide = true, UStaticMesh* Mesh = nullptr);
	void SpawnFacility(EFacility F);
};
