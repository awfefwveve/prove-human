#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class APangaeaGameMode;

// 게임 전체 UI (Slate, 에셋 없이 코드로 구성). 단계가 바뀔 때만 다시 만들고, 수치는 람다로 실시간 갱신한다.
class PANGAEA_API SPangaeaHUD : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPangaeaHUD) {}
		SLATE_ARGUMENT(TWeakObjectPtr<APangaeaGameMode>, GameMode)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void Rebuild();
	// 버튼 클릭 처리 도중에 위젯을 갈아끼우지 않도록 다음 틱에 다시 만든다
	void RequestRebuild() { bDirty = true; }
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TWeakObjectPtr<APangaeaGameMode> GameMode;
	bool bDirty = false;
	APangaeaGameMode* G() const { return GameMode.Get(); }

	TSharedRef<SWidget> BuildTitle();
	TSharedRef<SWidget> BuildBriefing();
	TSharedRef<SWidget> BuildShift();
	TSharedRef<SWidget> BuildNight();
	TSharedRef<SWidget> BuildFacilities();
	TSharedRef<SWidget> BuildEnding();
	TSharedRef<SWidget> TopBar();

	FText ResourceText() const;
	FText DocText() const;
	FText RulesText() const;
};
