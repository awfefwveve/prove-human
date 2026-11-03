#include "PangaeaTypes.h"
#include "PangaeaData.h"

#define LOCTEXT_NAMESPACE "Pangaea"

namespace
{
	const TArray<FText>& Regions()
	{
		static TArray<FText> A = {
			LOCTEXT("Rg0", "구 서울권"), LOCTEXT("Rg1", "인천 항구"), LOCTEXT("Rg2", "대전 폐허"), LOCTEXT("Rg3", "부산 외곽"),
			LOCTEXT("Rg4", "광주 지하"), LOCTEXT("Rg5", "강릉 해안"), LOCTEXT("Rg6", "청주 터널"), LOCTEXT("Rg7", "목포 섬")
		};
		return A;
	}

	const TArray<FText>& Jobs()
	{
		static TArray<FText> A = {
			LOCTEXT("Jb0", "어부"), LOCTEXT("Jb1", "간호사"), LOCTEXT("Jb2", "버스 기사"), LOCTEXT("Jb3", "초등학교 교사"),
			LOCTEXT("Jb4", "제빵사"), LOCTEXT("Jb5", "전기 기술자"), LOCTEXT("Jb6", "고등학생"), LOCTEXT("Jb7", "편의점 점주"),
			LOCTEXT("Jb8", "택배 기사"), LOCTEXT("Jb9", "목수"), LOCTEXT("Jb10", "요리사"), LOCTEXT("Jb11", "농부")
		};
		return A;
	}

	const TArray<TPair<FText, FText>>& Passwords()
	{
		static TArray<TPair<FText, FText>> A = {
			MakeTuple(LOCTEXT("PwA0", "등대"), LOCTEXT("PwB0", "파도")),
			MakeTuple(LOCTEXT("PwA1", "철새"), LOCTEXT("PwB1", "남쪽")),
			MakeTuple(LOCTEXT("PwA2", "새벽"), LOCTEXT("PwB2", "종소리")),
			MakeTuple(LOCTEXT("PwA3", "등불"), LOCTEXT("PwB3", "바람")),
			MakeTuple(LOCTEXT("PwA4", "바위"), LOCTEXT("PwB4", "이끼")),
			MakeTuple(LOCTEXT("PwA5", "겨울"), LOCTEXT("PwB5", "모닥불")),
			MakeTuple(LOCTEXT("PwA6", "그물"), LOCTEXT("PwB6", "고래")),
			MakeTuple(LOCTEXT("PwA7", "소금"), LOCTEXT("PwB7", "갈매기")),
			MakeTuple(LOCTEXT("PwA8", "닻"), LOCTEXT("PwB8", "밀물")),
			MakeTuple(LOCTEXT("PwA9", "판게아"), LOCTEXT("PwB9", "마지막 섬"))
		};
		return A;
	}
}

namespace PangaeaText
{
	FText Region(int32 Idx) { return Regions().IsValidIndex(Idx) ? Regions()[Idx] : FText::GetEmpty(); }
	int32 RegionCount() { return Regions().Num(); }
	FText Job(int32 Idx) { return Jobs().IsValidIndex(Idx) ? Jobs()[Idx] : FText::GetEmpty(); }
	int32 JobCount() { return Jobs().Num(); }
	FText PwAsk(int32 Idx) { return Passwords()[Idx % Passwords().Num()].Key; }
	FText PwAnswer(int32 Idx) { return Passwords()[Idx % Passwords().Num()].Value; }
	int32 PwCount() { return Passwords().Num(); }

	FText SignName(ESign S)
	{
		switch (S)
		{
		case ESign::TempFlat: return LOCTEXT("SgTemp", "체온·맥박이 기계처럼 일정");
		case ESign::PupilDelay: return LOCTEXT("SgPupil", "동공 반응 지연");
		case ESign::SyntheticBlood: return LOCTEXT("SgBlood", "합성 혈액");
		case ESign::MetalFrame: return LOCTEXT("SgMetal", "전신 금속 골격");
		case ESign::DocError: return LOCTEXT("SgDoc", "서류 오류");
		case ESign::Overprecise: return LOCTEXT("SgPrecise", "지나치게 정밀한 감각 묘사");
		case ESign::Contradiction: return LOCTEXT("SgContra", "구조 지역 진술 불일치");
		case ESign::WrongPassword: return LOCTEXT("SgPw", "암구호 틀림");
		}
		return FText::GetEmpty();
	}

	FText TrapName(ETrap T)
	{
		switch (T)
		{
		case ETrap::Fever: return LOCTEXT("TpFever", "열병 (체온 높음)");
		case ETrap::Fatigue: return LOCTEXT("TpFatigue", "피로 (동공 반응 약간 느림)");
		case ETrap::Anticoagulant: return LOCTEXT("TpAnti", "항응고제 복용 (응고 지연)");
		case ETrap::Prosthetic: return LOCTEXT("TpProst", "금속 의수");
		case ETrap::DamagedDoc: return LOCTEXT("TpDoc", "젖은 서류 (판독 불가)");
		case ETrap::Trauma: return LOCTEXT("TpTrauma", "트라우마 (기억 혼란)");
		default: break;
		}
		return FText::GetEmpty();
	}

	FText ToolName(ETool T)
	{
		switch (T)
		{
		case ETool::Thermo: return LOCTEXT("TlThermo", "체온·맥박계");
		case ETool::Flashlight: return LOCTEXT("TlFlash", "손전등");
		case ETool::BloodKit: return LOCTEXT("TlBlood", "채혈 키트");
		case ETool::XRay: return LOCTEXT("TlXray", "엑스레이");
		default: break;
		}
		return FText::GetEmpty();
	}

	FText FacilityName(EFacility F)
	{
		switch (F)
		{
		case EFacility::Generator: return LOCTEXT("FcGen", "발전기");
		case EFacility::Watchtower: return LOCTEXT("FcTower", "감시탑");
		case EFacility::Interrogation: return LOCTEXT("FcInter", "심문실");
		case EFacility::XRayGate: return LOCTEXT("FcXray", "엑스레이 게이트");
		case EFacility::Kennel: return LOCTEXT("FcKennel", "견사");
		default: break;
		}
		return FText::GetEmpty();
	}

	FText FacilityDesc(EFacility F)
	{
		switch (F)
		{
		case EFacility::Generator: return LOCTEXT("FdGen", "매일 전력 +2");
		case EFacility::Watchtower: return LOCTEXT("FdTower", "침투 AI가 일으키는 사고 피해 절반");
		case EFacility::Interrogation: return LOCTEXT("FdInter", "심문 질문 4개 모두 가능");
		case EFacility::XRayGate: return LOCTEXT("FdXray", "엑스레이 사용 가능, 매일 전력 -1");
		case EFacility::Kennel: return LOCTEXT("FdKennel", "경비견이 기계 냄새에 반응 (가끔 오판), 매일 식량 -1");
		default: break;
		}
		return FText::GetEmpty();
	}

	FText Question(const FDayRule& Rule, int32 Idx)
	{
		switch (Idx)
		{
		case 0: return LOCTEXT("Q0", "어디서 구조됐지?");
		case 1: return LOCTEXT("Q1", "구조되기 전엔 뭘 했지?");
		case 2: return LOCTEXT("Q2", "마지막으로 먹은 음식, 맛이 어땠지?");
		default:
			return Rule.Password()
				? FText::Format(LOCTEXT("Q3Pw", "암구호. '{0}'"), PwAsk(Rule.PwIndex))
				: LOCTEXT("Q3Fam", "가족은 어떻게 됐지?");
		}
	}

	FText Explanation(const FNpcProfile& P)
	{
		if (P.bAI)
		{
			TArray<FText> Parts;
			for (ESign S : P.Signs) Parts.Add(SignName(S));
			return FText::Format(LOCTEXT("ExSign", "징후: {0}"), FText::Join(FText::FromString(TEXT(", ")), Parts));
		}
		if (P.Trap != ETrap::None) return FText::Format(LOCTEXT("ExTrap", "오해 요소: {0}"), TrapName(P.Trap));
		return LOCTEXT("ExNone", "특이사항 없음");
	}
}

#undef LOCTEXT_NAMESPACE
