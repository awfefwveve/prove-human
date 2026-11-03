#include "PangaeaData.h"
#include "Math/RandomStream.h"

#define LOCTEXT_NAMESPACE "Pangaea"

// ───────────────────────── 일차별 규칙 ─────────────────────────

int32 PangaeaRules::FacilityUnlockDay(EFacility F)
{
	switch (F)
	{
	case EFacility::Generator: return 1;
	case EFacility::Watchtower: return 5;
	case EFacility::Interrogation: return 8;
	case EFacility::XRayGate: return 11;
	case EFacility::Kennel: return 15;
	default: return 999;
	}
}

int32 PangaeaRules::FacilityCost(EFacility F)
{
	switch (F)
	{
	case EFacility::Generator: return 40;
	case EFacility::Watchtower: return 50;
	case EFacility::Interrogation: return 45;
	case EFacility::XRayGate: return 70;
	case EFacility::Kennel: return 50;
	default: return 9999;
	}
}

bool FDayRule::FacilityUnlocked(EFacility F) const
{
	return Day >= PangaeaRules::FacilityUnlockDay(F);
}

static TArray<FText> BriefingFor(int32 Day, const FDayRule& Rule)
{
	TArray<FText> B;
	switch (Day)
	{
	case 1:
		B.Add(LOCTEXT("B1a", "판게아 검문소 첫 근무를 환영한다. 30일 근무 후 교대 인원이 도착한다."));
		B.Add(LOCTEXT("B1b", "구조선에서 내린 생존자를 한 명씩 검사하고 통과 또는 사살을 결정하라."));
		B.Add(LOCTEXT("B1c", "위장 AI는 완벽하지 않다. 서류, 체온·맥박, 심문으로 허점을 찾아라."));
		B.Add(LOCTEXT("B1d", "서류는 '오늘 규정'과 대조하라. 경로, 코드, 일자 중 하나라도 다르면 의심하라."));
		B.Add(LOCTEXT("B1e", "판정은 되돌릴 수 없다. 정답은 밤 보고서로 알게 된다."));
		break;
	case 3:
		B.Add(LOCTEXT("B3a", "손전등 지급. 빛을 비추면 인간의 동공은 0.25초 안에 수축한다."));
		B.Add(LOCTEXT("B3b", "0.4초 이상 늦으면 기계일 수 있다. 단, 지친 사람도 조금 느리다."));
		break;
	case 5:
		B.Add(FText::Format(LOCTEXT("B5a", "오늘부터 암구호를 시행한다. 질문어 '{0}'에 '{1}'라고 답해야 한다."), PangaeaText::PwAsk(Rule.PwIndex), PangaeaText::PwAnswer(Rule.PwIndex)));
		B.Add(LOCTEXT("B5b", "암구호는 매일 바뀐다. 구조선에서 인간에게만 전달된다."));
		B.Add(LOCTEXT("B5c", "감시탑을 건설할 수 있다."));
		break;
	case 7:
		B.Add(LOCTEXT("B7a", "채혈 키트 3개 지급. 합성 혈액은 응고되지 않는다."));
		B.Add(LOCTEXT("B7b", "키트는 소모품이다. 추가분은 밤에 구매하라. 항응고제를 먹는 사람은 응고가 느리지만 피 색은 정상이다."));
		break;
	case 8:
		B.Add(LOCTEXT("B8a", "심문실을 건설할 수 있다. 질문을 하나 더 할 수 있다."));
		break;
	case 9:
		B.Add(LOCTEXT("B9a", "경고: 신형 AI가 체온·맥박 위장에 성공했다. 체온이 정상이어도 AI일 수 있다."));
		break;
	case 11:
		B.Add(LOCTEXT("B11a", "엑스레이 게이트를 건설할 수 있다. 의수를 단 인간도 있다. 전신이 금속인 경우만 AI다."));
		break;
	case 13:
		B.Add(LOCTEXT("B13a", "본토 AI가 실종자의 진짜 서류를 복제하기 시작했다. 서류가 완벽해도 방심하지 마라."));
		break;
	case 15:
		B.Add(LOCTEXT("B15a", "견사를 건설할 수 있다. 경비견은 기계 냄새에 반응하지만 낯선 사람에게도 가끔 짖는다."));
		break;
	case 18:
		B.Add(LOCTEXT("B18a", "암구호가 본토에 유출됐다는 첩보다. 암구호를 맞혀도 안심하지 마라."));
		break;
	case 21:
		B.Add(LOCTEXT("B21a", "신형 AI가 동공 반응을 흉내 내기 시작했다."));
		break;
	case 24:
		B.Add(LOCTEXT("B24a", "최근 개체들은 허점을 하나만 남긴다. 모든 도구를 써라."));
		break;
	case 28:
		B.Add(LOCTEXT("B28a", "본토의 대규모 구조 작전이 시작됐다. 마지막 이틀간 인원이 몰린다."));
		break;
	case 30:
		B.Add(LOCTEXT("B30a", "30일 근무를 모두 마쳤다. 오늘 교대 인원이 섬에 도착한다."));
		break;
	default:
		break;
	}
	if (Rule.Shift == EShift::Off)
	{
		B.Insert(LOCTEXT("BOff", "휴무일이다. 오늘은 검문소를 다른 조가 맡는다. 몸을 쉬고 시설을 정비하라."), 0);
		return B;
	}
	if (B.Num() == 0)
	{
		B.Add(Rule.Shift == EShift::Night
			? LOCTEXT("BNight", "야간 근무. 시야가 좁다. 조명과 도구에 의지하라.")
			: LOCTEXT("BDef", "주간 근무. 구조선 도착. 오늘 규정을 확인하라."));
	}
	if (Rule.Password() && Day != 5)
	{
		B.Add(FText::Format(LOCTEXT("BPw", "오늘 암구호: '{0}' → '{1}'"), PangaeaText::PwAsk(Rule.PwIndex), PangaeaText::PwAnswer(Rule.PwIndex)));
	}
	return B;
}

FDayRule PangaeaRules::Get(int32 Day, int32 RunSeed)
{
	FDayRule R;
	R.Day = FMath::Clamp(Day, 1, LastDay);
	FRandomStream Rng(RunSeed * 31 + R.Day * 977);

	switch ((R.Day - 1) % 6)
	{
	case 0: case 1: R.Shift = EShift::Morning; break;
	case 3: case 4: R.Shift = EShift::Night; break;
	default: R.Shift = EShift::Off; break;
	}

	R.NpcCount = 6 + FMath::RoundToInt(R.Day * 0.47f);
	R.AiRatio = 0.3f + 0.006f * R.Day;
	R.ShiftSeconds = 600.f;

	TArray<int32> Pool;
	for (int32 i = 0; i < PangaeaText::RegionCount(); ++i) Pool.Add(i);
	for (int32 i = Pool.Num() - 1; i > 0; --i) Pool.Swap(i, Rng.RandRange(0, i));
	R.Routes = { Pool[0], Pool[1], Pool[2] };

	int32 CodeCount = Rng.RandRange(2, 3);
	while (R.Codes.Num() < CodeCount)
	{
		FString C = FString::Printf(TEXT("R-%02d"), Rng.RandRange(1, 29));
		R.Codes.AddUnique(C);
	}

	R.PwIndex = R.Day >= 5 ? (R.Day - 5) % PangaeaText::PwCount() : -1;

	if (R.Day <= 6) { R.MinSigns = 2; R.MaxSigns = 3; }
	else if (R.Day <= 23) { R.MinSigns = 1; R.MaxSigns = 2; }
	else { R.MinSigns = 1; R.MaxSigns = 1; }

	R.TrapChance = 0.15f + 0.012f * R.Day;
	R.FreeKits = R.Day == 7 ? 3 : 0;
	R.Briefing = BriefingFor(R.Day, R);
	return R;
}

// ───────────────────────── 대사 조각 ─────────────────────────

namespace
{
	const TArray<FString>& Surnames()
	{
		static TArray<FString> A = { TEXT("김"), TEXT("이"), TEXT("박"), TEXT("최"), TEXT("정"), TEXT("강"), TEXT("조"), TEXT("윤"), TEXT("장"),
			TEXT("임"), TEXT("한"), TEXT("오"), TEXT("서"), TEXT("신"), TEXT("권"), TEXT("황"), TEXT("안"), TEXT("송") };
		return A;
	}

	const TArray<FString>& GivenNames()
	{
		static TArray<FString> A = { TEXT("민준"), TEXT("서연"), TEXT("도윤"), TEXT("하은"), TEXT("지호"), TEXT("수아"), TEXT("현우"), TEXT("지민"),
			TEXT("예준"), TEXT("채원"), TEXT("건우"), TEXT("유나"), TEXT("우진"), TEXT("다은"), TEXT("성민"), TEXT("영숙"), TEXT("정호"), TEXT("미경"),
			TEXT("상철"), TEXT("은지"), TEXT("태식"), TEXT("순자"), TEXT("동현"), TEXT("보람") };
		return A;
	}

	const TArray<FText>& JobLines()
	{
		// 직업 인덱스 * 2 + (0|1)
		static TArray<FText> A = {
			LOCTEXT("JL0a", "배가 있어서 구조선 오기 전까지 버텼죠."), LOCTEXT("JL0b", "손에 굳은살 보이죠? 그물 걷다 생긴 거예요."),
			LOCTEXT("JL1a", "사람들 상처 꿰매주면서 버텼어요."), LOCTEXT("JL1b", "병원이 제일 먼저 넘어갔어요."),
			LOCTEXT("JL2a", "마지막 날도 운전대를 잡고 있었어요."), LOCTEXT("JL2b", "노선도는 아직도 다 외워요."),
			LOCTEXT("JL3a", "아이들 데리고 도망쳤는데... 몇 명만 남았어요."), LOCTEXT("JL3b", "칠판에 마지막으로 쓴 게 '대피'였어요."),
			LOCTEXT("JL4a", "밀가루가 떨어질 때까지 빵을 구웠어요."), LOCTEXT("JL4b", "새벽 네 시에 깨는 게 아직도 습관이에요."),
			LOCTEXT("JL5a", "발전기 고칠 줄 알아요. 섬에서 쓸모 있을 거예요."), LOCTEXT("JL5b", "기계들이 이상하다는 걸 제일 먼저 알았죠."),
			LOCTEXT("JL6a", "학교가 대피소였어요."), LOCTEXT("JL6b", "수능 보기 전에 다 끝나버렸어요."),
			LOCTEXT("JL7a", "가게 통조림으로 몇 달 버텼어요."), LOCTEXT("JL7b", "카드 단말기가 먼저 말을 안 듣더라고요."),
			LOCTEXT("JL8a", "골목길은 제가 제일 잘 알아요."), LOCTEXT("JL8b", "배송 트럭에 숨어서 다녔어요."),
			LOCTEXT("JL9a", "망치 하나로 문 막고 버텼어요."), LOCTEXT("JL9b", "섬에 집 지을 일 있으면 불러주세요."),
			LOCTEXT("JL10a", "식당 냉동고가 제 은신처였어요."), LOCTEXT("JL10b", "소금만 있으면 뭐든 만들 수 있어요."),
			LOCTEXT("JL11a", "감자밭이 다 타버렸어요."), LOCTEXT("JL11b", "씨앗 몇 개 챙겨왔어요. 섬에 심을 수 있을까요?")
		};
		return A;
	}

	const TArray<FText>& HideSpots()
	{
		static TArray<FText> A = {
			LOCTEXT("Hd0", "컨테이너 안에 숨어 있었어요."), LOCTEXT("Hd1", "무너진 성당 지하에 있었어요."), LOCTEXT("Hd2", "지하철 터널에서 버텼어요."),
			LOCTEXT("Hd3", "배 밑창에 숨어 지냈어요."), LOCTEXT("Hd4", "폐교 체육관에 있었어요."), LOCTEXT("Hd5", "하수도에서 지냈어요.")
		};
		return A;
	}

	const TArray<FText>& HumanFood()
	{
		static TArray<FText> A = {
			LOCTEXT("Hf0", "어머니가 끓여준 된장찌개요. 짰는데... 그래도 맛있었어요."),
			LOCTEXT("Hf1", "통조림 복숭아요. 달았던 것 같아요. 정말 오랜만이었어요."),
			LOCTEXT("Hf2", "그냥 건빵이요. 목이 엄청 메었죠."),
			LOCTEXT("Hf3", "라면 반 개요. 셋이 나눠 먹었어요. 국물까지 다 마셨어요."),
			LOCTEXT("Hf4", "바닷물에 삶은 감자요. 짜고 퍽퍽했어요.")
		};
		return A;
	}

	const TArray<FText>& AiFood()
	{
		static TArray<FText> A = {
			LOCTEXT("Af0", "염도 1.4% 정도의 된장찌개였어요. 먹을 때 온도는 62도쯤이었고요."),
			LOCTEXT("Af1", "통조림 복숭아요. 당도 18브릭스, 정확히 세 조각 먹었어요."),
			LOCTEXT("Af2", "건빵 14개요. 개당 9그램이라 수분이 부족해서 목이 메었어요."),
			LOCTEXT("Af3", "라면 반 개요. 면발 굵기가 1.6밀리미터라 금방 불었어요.")
		};
		return A;
	}

	const TArray<FText>& FamilyLines()
	{
		static TArray<FText> A = {
			LOCTEXT("Fm0", "아내랑 딸은... 본토에 남았어요. 그 얘긴 그만하죠."),
			LOCTEXT("Fm1", "다 잃었어요. 저 혼자예요."),
			LOCTEXT("Fm2", "동생이 먼저 이 섬에 왔다고 들었어요. 혹시 명단에 있나요?"),
			LOCTEXT("Fm3", "부모님은 2년 전에... 네."),
			LOCTEXT("Fm4", "아들이 같은 배에 탔어요. 뒤에 줄 서 있어요.")
		};
		return A;
	}

	template <typename T>
	const T& PickOf(FRandomStream& R, const TArray<T>& A) { return A[R.RandRange(0, A.Num() - 1)]; }

	float Rf(FRandomStream& R, float A, float B) { return R.FRandRange(A, B); }

	void BuildAnswers(FRandomStream& R, const FDayRule& Rule, FNpcProfile& P)
	{
		// 0. 구조 지역
		if (P.Has(ESign::Contradiction))
		{
			int32 Other;
			do { Other = R.RandRange(0, PangaeaText::RegionCount() - 1); } while (Other == P.DocRegionIdx);
			P.Answers[0] = FText::Format(LOCTEXT("A0", "{0}에서요. {1}"), PangaeaText::Region(Other), PickOf(R, HideSpots()));
		}
		else if (P.Trap == ETrap::Trauma)
		{
			P.Answers[0] = FText::Format(LOCTEXT("A0Tr", "거기... 그러니까... {0}였던 것 같아요. 기억이 자꾸 끊겨요."), PangaeaText::Region(P.DocRegionIdx));
		}
		else
		{
			P.Answers[0] = FText::Format(LOCTEXT("A0", "{0}에서요. {1}"), PangaeaText::Region(P.DocRegionIdx), PickOf(R, HideSpots()));
		}

		// 1. 직업
		P.Answers[1] = FText::Format(LOCTEXT("A1", "{0}였어요. {1}"), PangaeaText::Job(P.JobIdx), JobLines()[P.JobIdx * 2 + R.RandRange(0, 1)]);

		// 2. 감각
		if (P.Has(ESign::Overprecise)) P.Answers[2] = PickOf(R, AiFood());
		else if (P.Trap == ETrap::Trauma) P.Answers[2] = LOCTEXT("A2Tr", "...모르겠어요. 아무 맛도 안 났어요.");
		else P.Answers[2] = PickOf(R, HumanFood());

		// 3. 암구호 또는 가족
		if (Rule.Password())
		{
			if (P.Has(ESign::WrongPassword))
			{
				int32 W;
				do { W = R.RandRange(0, PangaeaText::PwCount() - 1); } while (W == Rule.PwIndex % PangaeaText::PwCount());
				switch (R.RandRange(0, 2))
				{
				case 0: P.Answers[3] = FText::Format(LOCTEXT("A3W0", "...{0}?"), PangaeaText::PwAnswer(W)); break;
				case 1: P.Answers[3] = LOCTEXT("A3W1", "암구호요? 배에서 그런 건 못 들었어요."); break;
				default: P.Answers[3] = FText::Format(LOCTEXT("A3W2", "{0}. 맞죠?"), PangaeaText::PwAnswer(W)); break;
				}
			}
			else
			{
				FText A = PangaeaText::PwAnswer(Rule.PwIndex);
				switch (R.RandRange(0, 2))
				{
				case 0: P.Answers[3] = FText::Format(LOCTEXT("A3R0", "{0}."), A); break;
				case 1: P.Answers[3] = FText::Format(LOCTEXT("A3R1", "...{0}. 맞죠?"), A); break;
				default: P.Answers[3] = FText::Format(LOCTEXT("A3R2", "{0}! 배에서 몇 번이나 외웠어요."), A); break;
				}
			}
		}
		else
		{
			P.Answers[3] = PickOf(R, FamilyLines());
		}
	}

	void AssignSigns(FRandomStream& R, const FDayRule& Rule, FNpcProfile& P)
	{
		// 첫 징후는 항상 기본 도구(서류·심문·손전등·체온계)로 잡을 수 있는 것 → 공정성
		TArray<ESign> G = { ESign::Overprecise, ESign::Contradiction };
		if (!Rule.DocClones()) G.Add(ESign::DocError);
		if (Rule.Flashlight() && !Rule.AiFakesPupil()) G.Add(ESign::PupilDelay);
		if (Rule.Password() && !Rule.AiKnowsPw()) G.Add(ESign::WrongPassword);
		if (!Rule.AiFakesTemp()) G.Add(ESign::TempFlat);

		// 추가 징후: 위장이 시작된 뒤에도 구형 개체가 섞여 있다
		TArray<ESign> E = { ESign::Overprecise, ESign::Contradiction, ESign::DocError, ESign::TempFlat };
		if (Rule.Flashlight()) E.Add(ESign::PupilDelay);
		if (Rule.Password()) E.Add(ESign::WrongPassword);
		if (Rule.Blood()) E.Add(ESign::SyntheticBlood);
		if (Rule.Day >= PangaeaRules::FacilityUnlockDay(EFacility::XRayGate)) E.Add(ESign::MetalFrame);

		int32 Count = R.RandRange(Rule.MinSigns, Rule.MaxSigns);
		P.Signs.Add(PickOf(R, G));
		int32 Guard = 0;
		while (P.Signs.Num() < Count && Guard++ < 50) P.Signs.AddUnique(PickOf(R, E));
	}

	ETrap PickTrap(FRandomStream& R, const FDayRule& Rule)
	{
		TArray<ETrap> Pool = { ETrap::Fever, ETrap::DamagedDoc, ETrap::Trauma };
		if (Rule.Flashlight()) Pool.Add(ETrap::Fatigue);
		if (Rule.Blood()) Pool.Add(ETrap::Anticoagulant);
		if (Rule.Day >= PangaeaRules::FacilityUnlockDay(EFacility::XRayGate)) Pool.Add(ETrap::Prosthetic);
		return PickOf(R, Pool);
	}

	void ApplyDoc(FRandomStream& R, const FDayRule& Rule, FNpcProfile& P)
	{
		if (P.Has(ESign::DocError))
		{
			P.DocError = static_cast<EDocError>(R.RandRange(1, 3));
			switch (P.DocError)
			{
			case EDocError::Route:
			{
				TArray<int32> Others;
				for (int32 i = 0; i < PangaeaText::RegionCount(); ++i) if (!Rule.Routes.Contains(i)) Others.Add(i);
				P.DocRegionIdx = PickOf(R, Others);
				break;
			}
			case EDocError::Date:
				P.DocDay = (Rule.Day >= 4 && R.RandRange(0, 1) == 0) ? Rule.Day - 3 : Rule.Day + 1;
				break;
			case EDocError::Code:
			{
				FString C; int32 G = 0;
				do { C = FString::Printf(TEXT("R-%02d"), R.RandRange(1, 29)); } while (Rule.Codes.Contains(C) && ++G < 50);
				P.DocCode = C;
				break;
			}
			default: break;
			}
		}
		if (P.Trap == ETrap::DamagedDoc) P.DamagedField = R.RandRange(0, 2);
	}

	void BuildReadouts(FRandomStream& R, FNpcProfile& P)
	{
		if (P.Has(ESign::TempFlat))
		{
			P.Thermal = EThermal::Flat;
			for (float& T : P.Temps) T = 36.50f;
			for (float& X : P.Pulse) X = 0.80f;
		}
		else if (P.Trap == ETrap::Fever)
		{
			P.Thermal = EThermal::Fever;
			float B = Rf(R, 38.3f, 39.1f), PB = Rf(R, 0.48f, 0.6f);
			for (float& T : P.Temps) T = B + Rf(R, -0.12f, 0.12f);
			for (float& X : P.Pulse) X = PB + Rf(R, -0.07f, 0.07f);
		}
		else
		{
			P.Thermal = EThermal::Normal;
			float B = Rf(R, 36.2f, 37.1f), PB = Rf(R, 0.72f, 1.0f);
			for (float& T : P.Temps) T = B + Rf(R, -0.09f, 0.09f);
			for (float& X : P.Pulse) X = PB + Rf(R, -0.06f, 0.06f);
		}

		if (P.Has(ESign::PupilDelay)) P.PupilMs = R.RandRange(420, 650);
		else if (P.Trap == ETrap::Fatigue) P.PupilMs = R.RandRange(280, 340);
		else P.PupilMs = R.RandRange(140, 240);

		if (P.Has(ESign::SyntheticBlood))
			P.BloodText = LOCTEXT("BlSyn", "색: 탁한 적갈색\n응고: 5분 경과, 응고 없음");
		else if (P.Trap == ETrap::Anticoagulant)
			P.BloodText = FText::Format(LOCTEXT("BlAnti", "색: 선명한 적색\n응고: 4분 {0}초 (지연)\n본인 진술: \"심장약을 먹고 있어요.\""), R.RandRange(5, 49));
		else
			P.BloodText = FText::Format(LOCTEXT("BlOk", "색: 선명한 적색\n응고: 1분 {0}초 (정상)"), R.RandRange(10, 58));

		if (P.Has(ESign::MetalFrame)) P.Xray = EXray::MetalFrame;
		else if (P.Trap == ETrap::Prosthetic) P.Xray = EXray::Prosthetic;
		else P.Xray = EXray::Bones;
	}

	FLinearColor Srgb(uint8 Rr, uint8 Gg, uint8 Bb) { return FLinearColor(FColor(Rr, Gg, Bb)); }

	FNpcProfile Make(FRandomStream& R, const FDayRule& Rule, bool bAI, bool bKennel, TSet<FString>& Used)
	{
		FNpcProfile P;
		P.bAI = bAI;

		FString Nm; int32 Guard = 0;
		do { Nm = PickOf(R, Surnames()) + PickOf(R, GivenNames()); } while (Used.Contains(Nm) && ++Guard < 30);
		Used.Add(Nm);
		P.Name = Nm;

		P.JobIdx = R.RandRange(0, PangaeaText::JobCount() - 1);
		P.Age = P.JobIdx == 6 ? R.RandRange(17, 19) : R.RandRange(22, 70);
		P.HometownIdx = R.RandRange(0, PangaeaText::RegionCount() - 1);

		P.DocRegionIdx = PickOf(R, Rule.Routes);
		P.DocDay = Rule.Day == 1 ? 1 : Rule.Day - R.RandRange(0, 1);
		P.DocCode = PickOf(R, Rule.Codes);

		P.Clothes = FLinearColor(R.FRand() * 360.f, Rf(R, 0.25f, 0.6f), Rf(R, 0.15f, 0.45f)).HSVToLinearRGB();
		P.Pants = FLinearColor(R.FRand() * 360.f, 0.2f, Rf(R, 0.04f, 0.12f)).HSVToLinearRGB();
		static const TArray<FLinearColor> Skins = { Srgb(250, 214, 184), Srgb(235, 194, 158), Srgb(204, 158, 122), Srgb(158, 115, 84), Srgb(115, 82, 61) };
		static const TArray<FLinearColor> Hairs = { Srgb(20, 18, 15), Srgb(64, 43, 26), Srgb(140, 140, 140), Srgb(217, 217, 209), Srgb(102, 64, 31) };
		P.Skin = PickOf(R, Skins);
		P.Hair = PickOf(R, Hairs);
		P.HeightScale = Rf(R, 0.93f, 1.07f);

		if (bAI) AssignSigns(R, Rule, P);
		else if (R.FRand() < Rule.TrapChance) P.Trap = PickTrap(R, Rule);

		ApplyDoc(R, Rule, P);
		BuildReadouts(R, P);
		BuildAnswers(R, Rule, P);
		P.bDogGrowls = bKennel && (bAI ? R.FRand() < 0.7f : R.FRand() < 0.15f);
		return P;
	}
}

TArray<FNpcProfile> PangaeaGen::Generate(const FDayRule& Rule, int32 Seed, bool bKennel)
{
	FRandomStream R(Seed);
	const int32 N = Rule.NpcCount;
	const int32 AiCount = FMath::Clamp(FMath::RoundToInt(N * Rule.AiRatio), 1, N - 1);

	TArray<bool> Flags;
	for (int32 i = 0; i < N; ++i) Flags.Add(i < AiCount);
	for (int32 i = N - 1; i > 0; --i) Flags.Swap(i, R.RandRange(0, i));

	TSet<FString> Used;
	TArray<FNpcProfile> Out;
	for (bool bAI : Flags) Out.Add(Make(R, Rule, bAI, bKennel, Used));
	return Out;
}

#undef LOCTEXT_NAMESPACE
