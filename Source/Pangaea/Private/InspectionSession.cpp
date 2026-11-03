#include "InspectionSession.h"

#define LOCTEXT_NAMESPACE "Pangaea"

FInspectionSession::FInspectionSession(const FNpcProfile& InNpc, const FDayRule& InRule, int32 InMaxQuestions, bool bKennel)
	: Npc(InNpc), Rule(InRule), QuestionsLeft(InMaxQuestions)
{
	Dialog.Add(FText::Format(LOCTEXT("DlgStart", "({0}이(가) 서류를 창구에 내민다)"), FText::FromString(Npc.Name)));
	if (bKennel)
	{
		ToolLog.Add(Npc.bDogGrowls
			? LOCTEXT("DogGrowl", "[경비견] 낮게 으르렁거린다.")
			: LOCTEXT("DogQuiet", "[경비견] 조용히 냄새만 맡는다."));
	}
}

FText FInspectionSession::BlockReason(ETool T, const FRunState& S) const
{
	switch (T)
	{
	case ETool::Thermo:
		return S.Power <= 0 ? LOCTEXT("BrPower", "전력 부족") : FText::GetEmpty();
	case ETool::Flashlight:
		return Rule.Flashlight() ? FText::GetEmpty() : LOCTEXT("BrDay3", "3일차 지급");
	case ETool::BloodKit:
		if (!Rule.Blood()) return LOCTEXT("BrDay7", "7일차 지급");
		if (bUsed[(int32)ETool::BloodKit]) return LOCTEXT("BrDone", "검사 완료");
		return S.Kits <= 0 ? LOCTEXT("BrNoKit", "키트 없음") : FText::GetEmpty();
	case ETool::XRay:
		if (!S.bXRayGate) return LOCTEXT("BrGate", "게이트 필요");
		return S.Power <= 0 ? LOCTEXT("BrPower", "전력 부족") : FText::GetEmpty();
	default:
		return FText::GetEmpty();
	}
}

void FInspectionSession::UseTool(ETool T, FRunState& S)
{
	if (!CanUse(T, S)) return;
	if (T == ETool::BloodKit) S.Kits--;

	const int32 I = (int32)T;
	if (bUsed[I]) return; // 결과 기록은 처음 한 번만
	bUsed[I] = true;

	switch (T)
	{
	case ETool::Thermo: ToolLog.Add(FormatThermo()); break;
	case ETool::Flashlight: ToolLog.Add(FText::Format(LOCTEXT("LogPupil", "[손전등] 동공 수축까지 {0} ms"), Npc.PupilMs)); break;
	case ETool::BloodKit: ToolLog.Add(FText::Format(LOCTEXT("LogBlood", "[채혈 키트]\n{0}"), Npc.BloodText)); break;
	case ETool::XRay: ToolLog.Add(FText::Format(LOCTEXT("LogXray", "[엑스레이] {0}"), XrayText())); break;
	default: break;
	}
}

void FInspectionSession::Ask(int32 Idx)
{
	if (Idx < 0 || Idx > 3 || bAsked[Idx] || QuestionsLeft <= 0) return;
	bAsked[Idx] = true;
	QuestionsLeft--;
	Dialog.Add(FText::Format(LOCTEXT("DlgMe", "나: {0}"), PangaeaText::Question(Rule, Idx)));
	Dialog.Add(FText::Format(LOCTEXT("DlgThem", "{0}: {1}"), FText::FromString(Npc.Name), Npc.Answers[Idx]));
}

FText FInspectionSession::ToolLogText() const
{
	return ToolLog.Num() ? FText::Join(FText::FromString(TEXT("\n\n")), ToolLog) : LOCTEXT("NoTool", "아직 측정한 결과가 없다.");
}

FText FInspectionSession::DialogText() const
{
	return FText::Join(FText::FromString(TEXT("\n")), Dialog);
}

FText FInspectionSession::FormatThermo() const
{
	FNumberFormattingOptions O2;
	O2.MinimumFractionalDigits = 2;
	O2.MaximumFractionalDigits = 2;
	FNumberFormattingOptions O0;
	O0.MaximumFractionalDigits = 0;

	float Sum = 0.f;
	for (float P : Npc.Pulse) Sum += P;
	const float Avg = Sum / UE_ARRAY_COUNT(Npc.Pulse);

	TArray<FText> Pulses;
	for (float P : Npc.Pulse) Pulses.Add(FText::AsNumber(P, &O2));

	return FText::Format(
		LOCTEXT("LogThermo", "[체온·맥박계]\n체온: {0} / {1} / {2} °C\n맥박 간격(초): {3}\n평균 맥박: {4} bpm"),
		FText::AsNumber(Npc.Temps[0], &O2), FText::AsNumber(Npc.Temps[1], &O2), FText::AsNumber(Npc.Temps[2], &O2),
		FText::Join(FText::FromString(TEXT("  ")), Pulses),
		FText::AsNumber(60.f / Avg, &O0));
}

FText FInspectionSession::XrayText() const
{
	switch (Npc.Xray)
	{
	case EXray::MetalFrame: return LOCTEXT("XrMetal", "전신 금속 프레임 감지");
	case EXray::Prosthetic: return LOCTEXT("XrProst", "골격 정상 / 왼팔 금속 의수");
	default: return LOCTEXT("XrBones", "정상 골격");
	}
}

#undef LOCTEXT_NAMESPACE
