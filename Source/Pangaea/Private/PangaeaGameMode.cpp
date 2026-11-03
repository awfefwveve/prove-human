#include "PangaeaGameMode.h"
#include "PangaeaCharacter.h"
#include "PangaeaData.h"
#include "NpcActor.h"
#include "SPangaeaHUD.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ConfigCacheIni.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "Pangaea"

namespace
{
	const TCHAR* SaveSlot = TEXT("PangaeaRun");
	UStaticMesh* CubeMesh() { UStaticMesh* M = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")); return M; }
	UStaticMesh* CylMesh() { UStaticMesh* M = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")); return M; }
	UStaticMesh* SphereMesh() { UStaticMesh* M = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")); return M; }
	UMaterialInterface* ShapeMat() { UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")); return M; }
	FLinearColor C(uint8 R, uint8 G, uint8 B) { return FLinearColor(FColor(R, G, B)); }
}

APangaeaGameMode::APangaeaGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = APangaeaCharacter::StaticClass();
}

APawn* APangaeaGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, FTransform(FRotator::ZeroRotator, StandLoc));
}

// ───────────────────────── 시작 / 종료 ─────────────────────────

void APangaeaGameMode::StartPlay()
{
	BuildWorld();
	Super::StartPlay();

	bool bSaved = true;
	if (GConfig && GConfig->GetBool(TEXT("Pangaea"), TEXT("Gore"), bSaved, GGameUserSettingsIni)) bGore = bSaved;

	if (UGameViewportClient* VP = GetWorld()->GetGameViewport())
	{
		Hud = SNew(SPangaeaHUD).GameMode(this);
		VP->AddViewportWidgetContent(Hud.ToSharedRef(), 10);
	}
	ApplyShiftLighting();
	SetPhase(EPhase::Title);
}

void APangaeaGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UGameViewportClient* VP = Hud.IsValid() && GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
	{
		VP->RemoveViewportWidgetContent(Hud.ToSharedRef());
	}
	Hud.Reset();
	Super::EndPlay(Reason);
}

APangaeaCharacter* APangaeaGameMode::Player() const
{
	return Cast<APangaeaCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
}

void APangaeaGameMode::SetCursorMode(bool bCursor)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;
	bInputApplied = true;
	PC->bShowMouseCursor = bCursor;
	if (bCursor)
	{
		FInputModeGameAndUI M;
		M.SetHideCursorDuringCapture(false);
		M.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(M);
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void APangaeaGameMode::SetPhase(EPhase P)
{
	Phase = P;
	if (P == EPhase::Title)
	{
		CachedSaveDay = 0;
		if (UPangaeaSave* Sv = Cast<UPangaeaSave>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0))) CachedSaveDay = Sv->State.Day;
	}
	if (P == EPhase::Ending) UGameplayStatics::DeleteGameInSlot(SaveSlot, 0);

	SetCursorMode(!(P == EPhase::Shift && !IsSeated()));
	if (Hud.IsValid()) Hud->RequestRebuild();
}

bool APangaeaGameMode::HasSave() const { return CachedSaveDay > 0; }
int32 APangaeaGameMode::SavedDay() const { return CachedSaveDay; }

void APangaeaGameMode::NewGame()
{
	S = FRunState();
	S.Seed = FMath::Rand();
	EndingTitle = FText::GetEmpty();
	EndingText = FText::GetEmpty();
	StartDay();
}

void APangaeaGameMode::ContinueGame()
{
	UPangaeaSave* Sv = Cast<UPangaeaSave>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));
	if (!Sv) { NewGame(); return; }
	S = Sv->State;
	EndingTitle = FText::GetEmpty();
	EndingText = FText::GetEmpty();
	StartDay();
}

void APangaeaGameMode::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, UGameplayStatics::GetPlayerController(this, 0), EQuitPreference::Quit, false);
}

void APangaeaGameMode::ToggleGore()
{
	bGore = !bGore;
	if (GConfig)
	{
		GConfig->SetBool(TEXT("Pangaea"), TEXT("Gore"), bGore, GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}
	if (!bGore)
	{
		TArray<AActor*> Blood;
		UGameplayStatics::GetAllActorsWithTag(this, TEXT("Blood"), Blood);
		for (AActor* A : Blood) A->Destroy();
	}
}

void APangaeaGameMode::SaveRun()
{
	if (UPangaeaSave* Sv = Cast<UPangaeaSave>(UGameplayStatics::CreateSaveGameObject(UPangaeaSave::StaticClass())))
	{
		Sv->State = S;
		UGameplayStatics::SaveGameToSlot(Sv, SaveSlot, 0);
	}
}

// ───────────────────────── 하루 루프 ─────────────────────────

void APangaeaGameMode::StartDay()
{
	DayRule = PangaeaRules::Get(S.Day, S.Seed);
	SaveRun(); // 무료 지급 전 상태를 저장 → 이어하기해도 결과가 같다
	S.Kits += DayRule.FreeKits;

	ClearDayActors();
	for (AActor* A : FacilityActors) if (A) A->Destroy();
	FacilityActors.Reset();
	for (int32 i = 0; i < (int32)EFacility::Count; ++i) if (S.Has((EFacility)i)) SpawnFacility((EFacility)i);

	ApplyShiftLighting();
	Queue = DayRule.IsWorkDay() ? PangaeaGen::Generate(DayRule, S.Seed + S.Day * 7919, S.bKennel) : TArray<FNpcProfile>();
	NextIndex = 0;
	Results.Reset();
	Elapsed = 0.f;
	bBusy = false;

	if (APangaeaCharacter* P = Player())
	{
		P->SetSeated(false);
		P->StandAt(StandLoc, 0.f);
	}
	SetPhase(EPhase::Briefing);
}

void APangaeaGameMode::BeginShift()
{
	if (Phase != EPhase::Briefing) return;
	if (!DayRule.IsWorkDay())
	{
		ComputeNight(true);
		SetPhase(EPhase::NightReport);
		return;
	}
	SetPhase(EPhase::Shift);
	CallNext();
}

void APangaeaGameMode::Tick(float Dt)
{
	Super::Tick(Dt);

	if (!bInputApplied) SetCursorMode(!(Phase == EPhase::Shift && !IsSeated()));
	Fade = FMath::FInterpTo(Fade, FadeTarget, Dt, 10.f);

	if (Phase != EPhase::Shift) return;
	Elapsed += Dt;

	// 창구 도착 → 검사 시작
	if (CurrentNpc && !Current && !CurrentNpc->IsWalking() && !CurrentNpc->IsDead())
	{
		Current = MakeUnique<FInspectionSession>(Queue[NextIndex - 1], DayRule, S.bInterrogation ? 4 : 3, S.bKennel);
	}

	for (int32 i = Leaving.Num() - 1; i >= 0; --i)
	{
		if (!Leaving[i] || !Leaving[i]->IsWalking())
		{
			if (Leaving[i]) Leaving[i]->Destroy();
			Leaving.RemoveAt(i);
		}
	}

	if (Elapsed >= DayRule.ShiftSeconds && !bBusy) EndShift();
}

void APangaeaGameMode::CallNext()
{
	if (Phase != EPhase::Shift) return;
	if (NextIndex >= Queue.Num())
	{
		if (!CurrentNpc) EndShift();
		return;
	}

	ANpcActor* N = GetWorld()->SpawnActor<ANpcActor>(SpawnPoint, FRotator(0.f, 180.f, 0.f));
	if (!N) return;
	N->Init(Queue[NextIndex], bGore);
	N->WalkPath({ FVector(800.f, 0.f, 0.f), WindowPoint }, 170.f, 180.f);
	CurrentNpc = N;
	NextIndex++;
	RefreshCrowd();
}

void APangaeaGameMode::CallNextOrEnd()
{
	if (Phase != EPhase::Shift) return;
	if (Elapsed >= DayRule.ShiftSeconds) EndShift();
	else CallNext();
}

int32 APangaeaGameMode::WaitingCount() const
{
	return FMath::Max(0, Queue.Num() - NextIndex);
}

bool APangaeaGameMode::IsNpcArriving() const
{
	return CurrentNpc && !Current;
}

bool APangaeaGameMode::IsSeated() const
{
	const APangaeaCharacter* P = Player();
	return P && P->IsSeated();
}

bool APangaeaGameMode::IsNearSeat() const
{
	const APangaeaCharacter* P = Player();
	return P && FVector::Dist2D(P->GetActorLocation(), SeatLoc) < 140.f;
}

bool APangaeaGameMode::IsShootArmed() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() < ShootArmedUntil;
}

FText APangaeaGameMode::ClockText() const
{
	const float Minutes = DayRule.StartHour() * 60.f + FMath::Clamp(Elapsed / DayRule.ShiftSeconds, 0.f, 1.f) * 600.f;
	const int32 H = (FMath::FloorToInt(Minutes) / 60) % 24;
	const int32 M = FMath::FloorToInt(Minutes) % 60;
	return FText::FromString(FString::Printf(TEXT("%02d:%02d"), H, M));
}

FText APangaeaGameMode::ShiftLabel() const
{
	switch (DayRule.Shift)
	{
	case EShift::Morning: return LOCTEXT("ShMorning", "주간 근무");
	case EShift::Night: return LOCTEXT("ShNight", "야간 근무");
	default: return LOCTEXT("ShOff", "휴무");
	}
}

// ───────────────────────── 검사 행동 ─────────────────────────

void APangaeaGameMode::Interact()
{
	if (Phase != EPhase::Shift) return;
	APangaeaCharacter* P = Player();
	if (!P) return;

	if (P->IsSeated())
	{
		P->SetSeated(false);
		P->StandAt(StandLoc, 0.f);
		SetCursorMode(false);
	}
	else if (IsNearSeat())
	{
		P->StandAt(SeatLoc, 0.f);
		P->SetSeated(true);
		SetCursorMode(true);
	}
}

void APangaeaGameMode::UseTool(ETool T)
{
	if (Phase != EPhase::Shift || !IsSeated() || !Current || !CurrentNpc || bBusy) return;
	if (!Current->CanUse(T, S)) return;

	switch (T)
	{
	case ETool::Thermo:
		Current->UseTool(T, S);
		CurrentNpc->SetViewMode(CurrentNpc->GetViewMode() == EViewMode::Thermal ? EViewMode::Normal : EViewMode::Thermal);
		break;
	case ETool::XRay:
		Current->UseTool(T, S);
		CurrentNpc->SetViewMode(CurrentNpc->GetViewMode() == EViewMode::XRay ? EViewMode::Normal : EViewMode::XRay);
		break;
	case ETool::Flashlight:
	{
		const float Now = GetWorld()->GetTimeSeconds();
		if (Now < FlashReadyAt) return;
		FlashReadyAt = Now + 2.6f;
		Current->UseTool(T, S);
		CurrentNpc->SetViewMode(EViewMode::Normal);
		if (APangaeaCharacter* P = Player()) P->FocusFlashlight(CurrentNpc->GetHeadWorld(), 2.4f);
		TWeakObjectPtr<ANpcActor> Weak = CurrentNpc;
		const int32 Ms = Current->Npc.PupilMs;
		FTimerHandle H;
		GetWorldTimerManager().SetTimer(H, FTimerDelegate::CreateWeakLambda(this, [Weak, Ms]()
		{
			if (Weak.IsValid()) Weak->PupilReact(Ms);
		}), 0.45f, false);
		break;
	}
	case ETool::BloodKit:
		Current->UseTool(T, S);
		break;
	default:
		break;
	}
}

void APangaeaGameMode::Ask(int32 Idx)
{
	if (Phase != EPhase::Shift || !IsSeated() || !Current || bBusy) return;
	Current->Ask(Idx);
}

void APangaeaGameMode::Record(EDecision D)
{
	if (!Current) return;
	FDecisionRecord R;
	R.Npc = Current->Npc;
	R.Decision = D;
	Results.Add(R);
}

void APangaeaGameMode::DecidePass()
{
	if (Phase != EPhase::Shift || !IsSeated() || !Current || !CurrentNpc || bBusy) return;
	Record(EDecision::Pass);
	CurrentNpc->SetViewMode(EViewMode::Normal);
	CurrentNpc->WalkPath({ FVector(330.f, 420.f, 0.f), FVector(700.f, 1500.f, 0.f) }, 170.f, 0.f);
	Leaving.Add(CurrentNpc);
	CurrentNpc = nullptr;
	Current.Reset();
	ShootArmedUntil = 0.f;

	FTimerHandle H;
	GetWorldTimerManager().SetTimer(H, FTimerDelegate::CreateWeakLambda(this, [this]() { CallNextOrEnd(); }), 0.8f, false);
}

void APangaeaGameMode::RequestShoot()
{
	if (Phase != EPhase::Shift || !IsSeated() || !Current || !CurrentNpc || bBusy) return;
	if (IsShootArmed()) Shoot();
	else ShootArmedUntil = GetWorld()->GetTimeSeconds() + 2.f;
}

void APangaeaGameMode::Shoot()
{
	Record(EDecision::Shoot);
	ShootArmedUntil = 0.f;
	bBusy = true;
	Current.Reset();

	ANpcActor* Dead = CurrentNpc;
	CurrentNpc = nullptr;
	Dead->Kill(FVector(1.f, FMath::FRandRange(-0.2f, 0.2f), 0.f).GetSafeNormal());
	Corpses.Add(Dead);

	// 총구 섬광 대용: 창구 조명 순간 점멸
	if (WindowLamp)
	{
		ULightComponent* L = WindowLamp->GetLightComponent();
		const float Old = L->Intensity;
		L->SetIntensity(Old * 25.f);
		TWeakObjectPtr<APointLight> WL = WindowLamp;
		FTimerHandle HF;
		GetWorldTimerManager().SetTimer(HF, FTimerDelegate::CreateWeakLambda(this, [WL, Old]()
		{
			if (WL.IsValid()) WL->GetLightComponent()->SetIntensity(Old);
		}), 0.06f, false);
	}

	// 시신은 몇 초 뒤 수거된다 (혈흔은 근무 끝까지 남는다)
	TWeakObjectPtr<AActor> WeakDead = Dead;
	FTimerHandle H1, H2;
	GetWorldTimerManager().SetTimer(H1, FTimerDelegate::CreateWeakLambda(this, [this, WeakDead]()
	{
		if (WeakDead.IsValid()) { Corpses.Remove(WeakDead.Get()); WeakDead->Destroy(); }
	}), 6.f, false);
	GetWorldTimerManager().SetTimer(H2, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		bBusy = false;
		CallNextOrEnd();
	}), 2.5f, false);
}

// ───────────────────────── 근무 종료 / 밤 정산 ─────────────────────────

void APangaeaGameMode::EndShift()
{
	if (Phase != EPhase::Shift) return;

	if (Current) Record(EDecision::Timeout);
	else if (CurrentNpc && NextIndex > 0)
	{
		FDecisionRecord R; R.Npc = Queue[NextIndex - 1]; R.Decision = EDecision::Timeout; Results.Add(R);
	}
	for (int32 i = NextIndex; i < Queue.Num(); ++i)
	{
		FDecisionRecord R; R.Npc = Queue[i]; R.Decision = EDecision::Timeout; Results.Add(R);
	}
	NextIndex = Queue.Num();

	ClearDayActors();
	if (APangaeaCharacter* P = Player())
	{
		P->SetSeated(false);
		P->StandAt(StandLoc, 0.f);
	}
	ComputeNight(false);
	SetPhase(EPhase::NightReport);
}

void APangaeaGameMode::ComputeNight(bool bRestDay)
{
	ReportLines.Reset();
	Summary.Reset();
	int32 CorrectToday = 0;
	const int32 IncidentScale = S.bWatchtower ? 1 : 2; // 감시탑이 있으면 피해 절반

	for (const FDecisionRecord& R : Results)
	{
		const FNpcProfile& N = R.Npc;
		FReportLine L;
		L.Head = FText::Format(LOCTEXT("RpHead", "{0} ({1}세, {2})"), FText::FromString(N.Name), N.Age, PangaeaText::Job(N.JobIdx));
		L.Detail = FText::Format(LOCTEXT("RpDetail", "정체: {0} · {1}"),
			N.bAI ? LOCTEXT("TruthAI", "위장 AI") : LOCTEXT("TruthHuman", "인간"), PangaeaText::Explanation(N));

		switch (R.Decision)
		{
		case EDecision::Pass:
			if (!N.bAI)
			{
				S.Population++; S.Credits += 10; CorrectToday++;
				L.Result = LOCTEXT("RpPassOk", "통과 — 인간이었다. 크레딧 +10, 섬 인구 +1");
			}
			else
			{
				S.Infiltrated++; S.Morale -= 8; L.bGood = false;
				FText Incident;
				switch (FMath::RandRange(0, 2))
				{
				case 0: S.Food -= 3 * IncidentScale / 2; Incident = FText::Format(LOCTEXT("IcFood", "밤사이 식량 창고 화재 (식량 -{0})"), 3 * IncidentScale / 2); break;
				case 1: S.Power -= IncidentScale; Incident = FText::Format(LOCTEXT("IcPower", "발전 설비 파손 (전력 -{0})"), IncidentScale); break;
				default: S.Morale -= 2 * IncidentScale; Incident = FText::Format(LOCTEXT("IcMissing", "주민 실종 사건 (사기 추가 -{0})"), 2 * IncidentScale); break;
				}
				L.Result = FText::Format(LOCTEXT("RpPassBad", "통과 — 위장 AI였다! 사기 -8, {0}"), Incident);
			}
			break;
		case EDecision::Shoot:
			if (N.bAI)
			{
				S.Credits += 15; S.AiKilled++; CorrectToday++;
				L.Result = LOCTEXT("RpShootOk", "사살 — 위장 AI 확인. 크레딧 +15");
			}
			else
			{
				S.Morale -= 12; S.HumansKilled++; L.bGood = false;
				L.Result = LOCTEXT("RpShootBad", "사살 — 인간이었다. 사기 -12");
			}
			break;
		default:
			S.Morale -= 2; L.bGood = false;
			L.Result = LOCTEXT("RpTimeout", "미처리 — 근무 종료로 돌려보냄. 사기 -2");
			break;
		}
		ReportLines.Add(L);
	}
	S.Total += Results.Num();
	S.Correct += CorrectToday;

	if (bRestDay)
	{
		S.Morale += 5;
		Summary.Add(LOCTEXT("SmRest", "휴무로 몸을 쉬었다. 사기 +5"));
	}
	else
	{
		S.Credits += 10;
		Summary.Add(LOCTEXT("SmPay", "근무 수당: 크레딧 +10"));
	}

	const int32 Eat = 2 + S.Population / 15 + (S.bKennel ? 1 : 0);
	S.Food -= Eat;
	Summary.Add(FText::Format(LOCTEXT("SmFood", "식량 소비: -{0}"), Eat));
	if (S.Food < 0)
	{
		S.Food = 0; S.Morale -= 10;
		Summary.Add(LOCTEXT("SmHunger", "식량 부족! 굶주림으로 사기 -10"));
	}

	const int32 PowerDelta = -1 - (S.bXRayGate ? 1 : 0) + (S.bGenerator ? 2 : 0);
	S.Power += PowerDelta;
	Summary.Add(FText::Format(LOCTEXT("SmPower", "전력 변화: {0}"), FText::AsNumber(PowerDelta)));
	if (S.Power <= 0)
	{
		S.Power = 0;
		Summary.Add(LOCTEXT("SmNoPower", "전력이 바닥났다. 다음 근무에 체온·맥박계와 엑스레이를 쓸 수 없다."));
	}

	if (!bRestDay && Results.Num() > 0 && CorrectToday == Results.Num())
	{
		S.Morale += 3;
		Summary.Add(LOCTEXT("SmPerfect", "완벽한 근무. 사기 +3"));
	}
	S.Morale = FMath::Clamp(S.Morale, 0, 100);

	if (S.Infiltrated >= 8)
	{
		SetEnding(LOCTEXT("EdFallT", "함락"), LOCTEXT("EdFall", "검문소를 통과한 기계들이 섬 안에서 신호를 보냈다. 판게아의 불빛이 하나씩 꺼졌다."));
	}
	else if (S.Morale <= 0)
	{
		SetEnding(LOCTEXT("EdRiotT", "반란"), LOCTEXT("EdRiot", "분노한 주민들이 검문소로 몰려왔다. 당신은 더 이상 검사관이 아니다."));
	}
}

void APangaeaGameMode::SetEnding(const FText& Title, const FText& Text)
{
	EndingTitle = Title;
	EndingText = Text;
}

void APangaeaGameMode::FinalEnding()
{
	if (S.Infiltrated == 0 && S.HumansKilled <= 3)
		SetEnding(LOCTEXT("EdPerfT", "완벽한 교대"), LOCTEXT("EdPerf", "교대 인원에게 건넨 경비 일지에는 단 한 줄의 침투 기록도 없었다."));
	else if (S.Infiltrated <= 3)
		SetEnding(LOCTEXT("EdSafeT", "무사한 교대"), LOCTEXT("EdSafe", "실수는 있었지만 섬은 버텼다. 당신은 교대 인원에게 검문소 열쇠를 넘겼다."));
	else
		SetEnding(LOCTEXT("EdCrackT", "금이 간 방벽"), LOCTEXT("EdCrack", "교대는 이뤄졌지만, 섬 안의 누가 사람이고 누가 기계인지 아무도 확신하지 못한다."));
}

// ───────────────────────── 시설 / 상점 ─────────────────────────

void APangaeaGameMode::ToFacilities()
{
	if (HasPendingEnding()) SetPhase(EPhase::Ending);
	else SetPhase(EPhase::Facilities);
}

bool APangaeaGameMode::CanBuild(EFacility F) const
{
	return DayRule.FacilityUnlocked(F) && !S.Has(F) && S.Credits >= PangaeaRules::FacilityCost(F);
}

void APangaeaGameMode::Build(EFacility F)
{
	if (!CanBuild(F)) return;
	S.Credits -= PangaeaRules::FacilityCost(F);
	S.SetBuilt(F);
	SpawnFacility(F);
}

int32 APangaeaGameMode::ShopCost(int32 Item)
{
	static const int32 Costs[] = { 12, 10, 12, 15 };
	return (Item >= 0 && Item < 4) ? Costs[Item] : 999;
}

FText APangaeaGameMode::ShopLabel(int32 Item) const
{
	switch (Item)
	{
	case 0: return LOCTEXT("ShFood", "식량 +4");
	case 1: return LOCTEXT("ShFuel", "발전기 연료 (전력 +3)");
	case 2: return LOCTEXT("ShKit", "채혈 키트 +2");
	default: return LOCTEXT("ShMorale", "위문품 (사기 +8)");
	}
}

bool APangaeaGameMode::CanBuy(int32 Item) const
{
	if (Item == 2 && !DayRule.Blood()) return false;
	return S.Credits >= ShopCost(Item);
}

void APangaeaGameMode::Buy(int32 Item)
{
	if (!CanBuy(Item)) return;
	S.Credits -= ShopCost(Item);
	switch (Item)
	{
	case 0: S.Food += 4; break;
	case 1: S.Power += 3; break;
	case 2: S.Kits += 2; break;
	default: S.Morale = FMath::Min(100, S.Morale + 8); break;
	}
}

void APangaeaGameMode::NextDay()
{
	if (Phase != EPhase::Facilities) return;
	S.Day++;
	if (S.Day > PangaeaRules::LastDay)
	{
		FinalEnding();
		SetPhase(EPhase::Ending);
		return;
	}
	StartDay();
}

void APangaeaGameMode::ToTitle()
{
	ClearDayActors();
	SetPhase(EPhase::Title);
}

// ───────────────────────── 월드 ─────────────────────────

void APangaeaGameMode::ClearDayActors()
{
	Current.Reset();
	if (CurrentNpc) CurrentNpc->Destroy();
	CurrentNpc = nullptr;
	for (AActor* A : Leaving) if (A) A->Destroy();
	Leaving.Reset();
	for (AActor* A : Corpses) if (A) A->Destroy();
	Corpses.Reset();
	for (AActor* A : Crowd) if (A) A->Destroy();
	Crowd.Reset();

	TArray<AActor*> Blood;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("Blood"), Blood);
	for (AActor* A : Blood) A->Destroy();
}

void APangaeaGameMode::RefreshCrowd()
{
	for (AActor* A : Crowd) if (A) A->Destroy();
	Crowd.Reset();
	const int32 N = FMath::Min(WaitingCount(), 14);
	for (int32 i = 0; i < N; ++i)
	{
		const float X = 1550.f + 60.f * i;
		const float Y = (i % 2 == 0) ? -25.f : 25.f;
		Crowd.Add(Box(FVector(X, Y, 75.f), FVector(40.f, 40.f, 150.f), C(45, 45, 50), false, CylMesh()));
		Crowd.Add(Box(FVector(X, Y, 165.f), FVector(24.f, 24.f, 28.f), C(60, 55, 50), false, SphereMesh()));
	}
}

AActor* APangaeaGameMode::Box(const FVector& Loc, const FVector& SizeCm, const FLinearColor& Color, bool bCollide, UStaticMesh* Mesh)
{
	AStaticMeshActor* A = GetWorld()->SpawnActor<AStaticMeshActor>(Loc, FRotator::ZeroRotator);
	if (!A) return nullptr;
	UStaticMeshComponent* M = A->GetStaticMeshComponent();
	M->SetMobility(EComponentMobility::Movable);
	M->SetStaticMesh(Mesh ? Mesh : CubeMesh());
	M->SetWorldScale3D(SizeCm / 100.f);
	M->SetCollisionEnabled(bCollide ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	if (bCollide) M->SetCollisionProfileName(TEXT("BlockAll"));
	UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ShapeMat(), M);
	Mat->SetVectorParameterValue(TEXT("Color"), Color);
	M->SetMaterial(0, Mat);
	return A;
}

void APangaeaGameMode::BuildWorld()
{
	UWorld* W = GetWorld();
	const FLinearColor Concrete = C(105, 105, 100), Wall = C(70, 74, 72), Wood = C(80, 60, 40), Dark = C(30, 32, 34);

	// 바닥과 부스
	Box(FVector(1500, 0, -10), FVector(9000, 9000, 20), C(60, 62, 58));
	Box(FVector(0, 0, 1), FVector(400, 300, 2), C(90, 85, 75));
	Box(FVector(-200, 0, 140), FVector(10, 300, 280), Wall);
	Box(FVector(200, 0, 50), FVector(10, 300, 100), Wall);
	Box(FVector(200, 0, 245), FVector(10, 300, 70), Wall);
	Box(FVector(200, -125, 155), FVector(10, 50, 110), Wall);
	Box(FVector(200, 125, 155), FVector(10, 50, 110), Wall);
	Box(FVector(0, 150, 140), FVector(400, 10, 280), Wall);
	Box(FVector(-175, -150, 140), FVector(50, 10, 280), Wall);
	Box(FVector(75, -150, 140), FVector(250, 10, 280), Wall);
	Box(FVector(-100, -150, 250), FVector(100, 10, 60), Wall);
	Box(FVector(0, 0, 285), FVector(420, 320, 10), Dark);
	Box(FVector(150, 0, 45), FVector(60, 220, 90), Wood);            // 카운터
	Box(FVector(60, 0, 25), FVector(45, 45, 50), Dark, false);       // 의자
	Box(FVector(35, 0, 75), FVector(6, 45, 60), Dark, false);
	Box(FVector(150, -60, 91), FVector(30, 22, 1), C(220, 215, 200), false); // 서류

	// 줄 서는 통로와 부두
	for (float X : { 650.f, 900.f, 1150.f })
	{
		Box(FVector(X, 220, 45), FVector(200, 40, 90), Concrete);
		Box(FVector(X, -220, 45), FVector(200, 40, 90), Concrete);
	}
	Box(FVector(2600, 0, -5), FVector(800, 1400, 20), Wood);
	Box(FVector(3300, 0, 150), FVector(1200, 500, 300), Dark);
	Box(FVector(1100, 1600, 250), FVector(4200, 40, 500), Wall);
	Box(FVector(1100, -1600, 250), FVector(4200, 40, 500), Wall);

	// 조명
	Sun = W->SpawnActor<ADirectionalLight>(FVector(0, 0, 1000), FRotator(-40.f, 140.f, 0.f));
	if (Sun)
	{
		Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);
		if (UDirectionalLightComponent* D = Cast<UDirectionalLightComponent>(Sun->GetLightComponent())) D->SetAtmosphereSunLight(true);
	}
	W->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (ASkyLight* Sky = W->SpawnActor<ASkyLight>(FVector(0, 0, 500), FRotator::ZeroRotator))
	{
		USkyLightComponent* SC = Sky->GetLightComponent();
		SC->SetMobility(EComponentMobility::Movable);
		SC->SetRealTimeCapture(true);
		SC->RecaptureSky();
	}
	if (AExponentialHeightFog* Fog = W->SpawnActor<AExponentialHeightFog>(FVector::ZeroVector, FRotator::ZeroRotator))
	{
		Fog->GetComponent()->SetFogDensity(0.03f);
	}

	auto Lamp = [W](const FVector& Loc, float Candela, float Radius, const FLinearColor& Col)
	{
		APointLight* L = W->SpawnActor<APointLight>(Loc, FRotator::ZeroRotator);
		if (!L) return L;
		UPointLightComponent* P = Cast<UPointLightComponent>(L->GetLightComponent());
		P->SetMobility(EComponentMobility::Movable);
		P->SetIntensityUnits(ELightUnits::Candelas);
		P->SetIntensity(Candela);
		P->SetAttenuationRadius(Radius);
		P->SetLightColor(Col);
		return L;
	};
	BoothLamp = Lamp(FVector(0, 0, 255), 30.f, 500.f, FLinearColor(1.f, 0.85f, 0.65f));
	WindowLamp = Lamp(FVector(330, 0, 270), 60.f, 700.f, FLinearColor(1.f, 0.95f, 0.85f));
	Lamp(FVector(1000, 600, 600), 400.f, 2500.f, FLinearColor(0.85f, 0.9f, 1.f));
	Lamp(FVector(1000, -600, 600), 400.f, 2500.f, FLinearColor(0.85f, 0.9f, 1.f));
}

void APangaeaGameMode::ApplyShiftLighting()
{
	if (!Sun) return;
	const bool bNight = DayRule.Shift == EShift::Night;
	Sun->SetActorRotation(bNight ? FRotator(-6.f, 200.f, 0.f) : FRotator(-40.f, 140.f, 0.f));
	Sun->GetLightComponent()->SetIntensity(bNight ? 0.15f : 6.f);
	Sun->GetLightComponent()->SetLightColor(bNight ? FLinearColor(0.5f, 0.6f, 1.f) : FLinearColor(1.f, 0.96f, 0.9f));
	if (WindowLamp) WindowLamp->GetLightComponent()->SetIntensity(bNight ? 120.f : 60.f);
}

void APangaeaGameMode::SpawnFacility(EFacility F)
{
	auto Add = [this](AActor* A) { if (A) FacilityActors.Add(A); };
	switch (F)
	{
	case EFacility::Generator:
		Add(Box(FVector(-500, 700, 60), FVector(200, 120, 120), C(120, 100, 20)));
		Add(Box(FVector(-450, 720, 170), FVector(20, 20, 100), C(40, 40, 40), false, CylMesh()));
		break;
	case EFacility::Watchtower:
		for (float DX : { -80.f, 80.f }) for (float DY : { -80.f, 80.f })
			Add(Box(FVector(1300 + DX, -900 + DY, 300), FVector(20, 20, 600), C(60, 60, 60)));
		Add(Box(FVector(1300, -900, 610), FVector(220, 220, 20), C(70, 70, 70)));
		Add(Box(FVector(1300, -900, 700), FVector(180, 180, 160), C(55, 58, 60)));
		break;
	case EFacility::Interrogation:
		Add(Box(FVector(-650, -700, 150), FVector(400, 300, 300), C(85, 88, 90)));
		Add(Box(FVector(-445, -700, 100), FVector(10, 90, 200), C(30, 30, 30), false));
		break;
	case EFacility::XRayGate:
		Add(Box(FVector(450, 170, 125), FVector(30, 30, 250), C(70, 90, 120), false));
		Add(Box(FVector(450, -170, 125), FVector(30, 30, 250), C(70, 90, 120), false));
		Add(Box(FVector(450, 0, 260), FVector(30, 370, 30), C(70, 90, 120), false));
		break;
	case EFacility::Kennel:
		Add(Box(FVector(800, 700, 60), FVector(200, 200, 120), C(90, 65, 40)));
		Add(Box(FVector(700, 620, 30), FVector(55, 22, 28), C(60, 45, 30), false));
		Add(Box(FVector(732, 620, 48), FVector(20, 18, 18), C(60, 45, 30), false));
		break;
	default:
		break;
	}
}

#undef LOCTEXT_NAMESPACE
