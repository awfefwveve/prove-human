#pragma once

#include "CoreMinimal.h"

// 엔진 독립적인 게임 데이터 타입. 정답(bAI)과 모든 검사 결과는 NPC 생성 시점에 확정된다.

enum class EPhase : uint8 { Title, Briefing, Shift, NightReport, Facilities, Ending };

enum class ETool : uint8 { Thermo, Flashlight, BloodKit, XRay, Count };

// 위장 AI가 드러내는 징후
enum class ESign : uint8
{
	TempFlat,       // 체온·맥박이 기계처럼 일정
	PupilDelay,     // 동공 반응 지연
	SyntheticBlood, // 합성 혈액
	MetalFrame,     // 전신 금속 골격
	DocError,       // 서류 오류
	Overprecise,    // 지나치게 정밀한 감각 묘사
	Contradiction,  // 구조 지역 진술 불일치
	WrongPassword   // 암구호 틀림
};

// 인간인데 의심을 사는 함정 요소
enum class ETrap : uint8 { None, Fever, Fatigue, Anticoagulant, Prosthetic, DamagedDoc, Trauma };

enum class EDecision : uint8 { None, Pass, Shoot, Timeout };
enum class EDocError : uint8 { None, Route, Date, Code };
enum class EViewMode : uint8 { Normal, Thermal, XRay };
enum class EXray : uint8 { Bones, MetalFrame, Prosthetic };
enum class EThermal : uint8 { Normal, Flat, Fever };

enum class EFacility : uint8 { Generator, Watchtower, Interrogation, XRayGate, Kennel, Count };

// 교대 근무: 주간 2일 → 휴무 → 야간 2일 → 휴무 (6일 주기)
enum class EShift : uint8 { Morning, Night, Off };

struct FDayRule
{
	int32 Day = 1;
	EShift Shift = EShift::Morning;
	int32 NpcCount = 6;
	float AiRatio = 0.3f;
	float ShiftSeconds = 600.f;      // 08:00~18:00 = 실제 10분
	TArray<int32> Routes;           // 오늘 구조선이 들른 지역 인덱스
	TArray<FString> Codes;          // 오늘 유효 구조대 코드
	int32 PwIndex = -1;             // 암구호 쌍 인덱스 (-1 미시행)
	int32 MinSigns = 2, MaxSigns = 3;
	float TrapChance = 0.15f;
	int32 FreeKits = 0;
	TArray<FText> Briefing;

	bool IsWorkDay() const { return Shift != EShift::Off; }
	int32 StartHour() const { return Shift == EShift::Night ? 18 : 8; } // 주간 08~18, 야간 18~04

	bool Flashlight() const { return Day >= 3; }
	bool Password() const { return PwIndex >= 0; }
	bool Blood() const { return Day >= 7; }
	bool AiFakesTemp() const { return Day >= 9; }
	bool DocClones() const { return Day >= 13; }
	bool AiKnowsPw() const { return Day >= 18; }
	bool AiFakesPupil() const { return Day >= 21; }
	int32 MinValidDocDay() const { return Day <= 1 ? 1 : Day - 1; }
	bool FacilityUnlocked(EFacility F) const;
};

struct FNpcProfile
{
	FString Name;
	int32 Age = 30;
	int32 JobIdx = 0;
	int32 HometownIdx = 0;

	bool bAI = false;
	TArray<ESign> Signs;
	ETrap Trap = ETrap::None;

	// 서류
	int32 DocRegionIdx = 0;
	int32 DocDay = 1;
	FString DocCode;
	EDocError DocError = EDocError::None;
	int32 DamagedField = -1; // 0 지역, 1 일자, 2 코드

	// 도구 결과
	float Temps[3] = { 36.5f, 36.5f, 36.5f };
	float Pulse[5] = { 0.8f, 0.8f, 0.8f, 0.8f, 0.8f };
	EThermal Thermal = EThermal::Normal;
	int32 PupilMs = 200;
	FText BloodText;
	EXray Xray = EXray::Bones;
	bool bDogGrowls = false;

	// 심문 답변 (질문 4개)
	FText Answers[4];

	// 외형 (메타휴먼 교체 전 임시)
	FLinearColor Clothes, Pants, Skin, Hair;
	float HeightScale = 1.f;

	bool Has(ESign S) const { return Signs.Contains(S); }
};

struct FDecisionRecord
{
	FNpcProfile Npc;
	EDecision Decision = EDecision::None;
};

struct FReportLine
{
	FText Head;
	FText Result;
	FText Detail;
	bool bGood = true;
};

namespace PangaeaText
{
	FText Region(int32 Idx);
	int32 RegionCount();
	FText Job(int32 Idx);
	int32 JobCount();
	FText PwAsk(int32 Idx);
	FText PwAnswer(int32 Idx);
	int32 PwCount();
	FText SignName(ESign S);
	FText TrapName(ETrap T);
	FText ToolName(ETool T);
	FText FacilityName(EFacility F);
	FText FacilityDesc(EFacility F);
	FText Question(const FDayRule& Rule, int32 Idx);
	FText Explanation(const FNpcProfile& P);
}
