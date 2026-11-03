#pragma once

#include "CoreMinimal.h"
#include "PangaeaTypes.h"
#include "PangaeaSave.h"

// 창구 앞 NPC 한 명을 검사하는 동안의 상태 (엔진 비의존 로직)
class PANGAEA_API FInspectionSession
{
public:
	FInspectionSession(const FNpcProfile& InNpc, const FDayRule& InRule, int32 InMaxQuestions, bool bKennel);

	FNpcProfile Npc;
	FDayRule Rule;
	TArray<FText> ToolLog;
	TArray<FText> Dialog;
	int32 QuestionsLeft = 3;

	bool Asked(int32 Idx) const { return Idx >= 0 && Idx < 4 && bAsked[Idx]; }

	// 비어 있으면 사용 가능, 아니면 막힌 이유
	FText BlockReason(ETool T, const FRunState& S) const;
	bool CanUse(ETool T, const FRunState& S) const { return BlockReason(T, S).IsEmpty(); }

	void UseTool(ETool T, FRunState& S);
	void Ask(int32 Idx);

	FText ToolLogText() const;
	FText DialogText() const;

private:
	bool bUsed[(int32)ETool::Count] = { false, false, false, false };
	bool bAsked[4] = { false, false, false, false };

	FText FormatThermo() const;
	FText XrayText() const;
};
