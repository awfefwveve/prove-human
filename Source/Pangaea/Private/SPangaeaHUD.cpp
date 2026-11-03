#include "SPangaeaHUD.h"
#include "PangaeaGameMode.h"
#include "PangaeaData.h"

#include "Styling/CoreStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SSpacer.h"

#define LOCTEXT_NAMESPACE "Pangaea"

namespace
{
	const FLinearColor TextCol(0.92f, 0.92f, 0.9f);
	const FLinearColor Muted(0.6f, 0.62f, 0.65f);
	const FLinearColor GoodCol(0.45f, 0.85f, 0.5f);
	const FLinearColor BadCol(0.95f, 0.35f, 0.3f);
	const FLinearColor Accent(0.95f, 0.75f, 0.3f);

	FSlateFontInfo Font(int32 Size, bool bBold = false)
	{
		return FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size);
	}

	const FSlateBrush* White() { return FCoreStyle::Get().GetBrush("WhiteBrush"); }

	TSharedRef<SWidget> Label(const TAttribute<FText>& T, int32 Size = 16, bool bBold = false, const FLinearColor& Col = TextCol)
	{
		return SNew(STextBlock).Text(T).Font(Font(Size, bBold)).ColorAndOpacity(FSlateColor(Col)).AutoWrapText(true);
	}

	TSharedRef<SWidget> Button(const TAttribute<FText>& T, TFunction<void()> OnClick, const TAttribute<bool>& Enabled = true,
		const FLinearColor& Tint = FLinearColor(0.25f, 0.27f, 0.3f), int32 Size = 16)
	{
		return SNew(SButton)
			.IsFocusable(false)
			.ButtonColorAndOpacity(Tint)
			.IsEnabled(Enabled)
			.ContentPadding(FMargin(14.f, 8.f))
			.HAlign(HAlign_Center)
			.OnClicked_Lambda([OnClick]() { OnClick(); return FReply::Handled(); })
			[
				SNew(STextBlock).Text(T).Font(Font(Size)).Justification(ETextJustify::Center).AutoWrapText(true)
			];
	}

	TSharedRef<SWidget> Panel(const TSharedRef<SWidget>& Content, float Width = 0.f, float Alpha = 0.82f)
	{
		TSharedRef<SWidget> Inner = SNew(SBorder)
			.BorderImage(White())
			.BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.03f, Alpha))
			.Padding(18.f)
			[Content];
		if (Width > 0.f) return SNew(SBox).WidthOverride(Width)[Inner];
		return Inner;
	}
}

void SPangaeaHUD::Construct(const FArguments& InArgs)
{
	GameMode = InArgs._GameMode;
	Rebuild();
}

void SPangaeaHUD::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (bDirty)
	{
		bDirty = false;
		Rebuild();
	}
}

void SPangaeaHUD::Rebuild()
{
	TSharedRef<SWidget> Content = SNullWidget::NullWidget;
	if (APangaeaGameMode* GM = G())
	{
		switch (GM->GetPhase())
		{
		case EPhase::Title: Content = BuildTitle(); break;
		case EPhase::Briefing: Content = BuildBriefing(); break;
		case EPhase::Shift: Content = BuildShift(); break;
		case EPhase::NightReport: Content = BuildNight(); break;
		case EPhase::Facilities: Content = BuildFacilities(); break;
		case EPhase::Ending: Content = BuildEnding(); break;
		}
	}

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()[Content]
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(White())
			.Visibility(EVisibility::HitTestInvisible)
			.BorderBackgroundColor_Lambda([this]() { return FSlateColor(FLinearColor(0.f, 0.f, 0.f, G() ? G()->GetFade() : 0.f)); })
		]
	];
}

FText SPangaeaHUD::ResourceText() const
{
	const APangaeaGameMode* GM = G();
	if (!GM) return FText::GetEmpty();
	const FRunState& S = GM->State();
	return FText::Format(LOCTEXT("Res", "크레딧 {0}   식량 {1}   전력 {2}   사기 {3}   채혈 키트 {4}   섬 인구 {5}"),
		S.Credits, S.Food, S.Power, S.Morale, S.Kits, S.Population);
}

// ───────────────────────── 타이틀 ─────────────────────────

TSharedRef<SWidget> SPangaeaHUD::BuildTitle()
{
	return SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
	[
		Panel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[Label(LOCTEXT("GameTitle", "PROVE HUMAN"), 44, true, Accent)]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 6, 0, 24)
				[Label(LOCTEXT("GameSub", "세상은 AI에게 넘어갔다. 마지막 섬으로 들어오는 자들 중 누가 기계인가."), 16, false, Muted)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)[Button(LOCTEXT("TtNew", "새 게임"), [this]() { if (G()) G()->NewGame(); })]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
			[
				Button(TAttribute<FText>::CreateLambda([this]()
					{
						return G() && G()->HasSave() ? FText::Format(LOCTEXT("TtCont", "이어하기 (제 {0}일)"), G()->SavedDay()) : LOCTEXT("TtNoSave", "이어하기");
					}),
					[this]() { if (G()) G()->ContinueGame(); },
					TAttribute<bool>::CreateLambda([this]() { return G() && G()->HasSave(); }))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
			[
				Button(TAttribute<FText>::CreateLambda([this]()
					{
						return G() && G()->IsGoreOn() ? LOCTEXT("TtGoreOn", "피 표현: 켜짐") : LOCTEXT("TtGoreOff", "피 표현: 꺼짐");
					}),
					[this]() { if (G()) G()->ToggleGore(); })
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)[Button(LOCTEXT("TtQuit", "종료"), [this]() { if (G()) G()->QuitGame(); })],
			460.f)
	];
}

// ───────────────────────── 브리핑 ─────────────────────────

TSharedRef<SWidget> SPangaeaHUD::BuildBriefing()
{
	APangaeaGameMode* GM = G();
	const FDayRule& R = GM->Rule();
	TSharedRef<SVerticalBox> Lines = SNew(SVerticalBox);
	for (const FText& L : R.Briefing) Lines->AddSlot().AutoHeight().Padding(0, 4)[Label(FText::Format(LOCTEXT("Bullet", "· {0}"), L), 17)];

	const bool bWork = R.IsWorkDay();
	return SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
	[
		Panel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[Label(FText::Format(LOCTEXT("BrTitle", "제 {0}일 · {1}"), R.Day, GM->ShiftLabel()), 30, true, Accent)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 14)[Label(FText::Format(LOCTEXT("BrLeft", "교대까지 {0}일"), PangaeaRules::LastDay - R.Day), 15, false, Muted)]
			+ SVerticalBox::Slot().AutoHeight()[Lines]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 16, 0, 4)[Label(TAttribute<FText>::CreateSP(this, &SPangaeaHUD::ResourceText), 15, false, Muted)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
				[Label(bWork ? LOCTEXT("BrHint", "부스 의자로 가서 E로 착석하면 검사가 시작된다. 생존자 대기: 구조선 도착 완료.") : FText::GetEmpty(), 14, false, Muted)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[Button(bWork ? LOCTEXT("BrStart", "근무 시작") : LOCTEXT("BrRest", "하루 쉬기"), [this]() { if (G()) G()->BeginShift(); }, true, FLinearColor(0.3f, 0.4f, 0.3f), 18)],
			760.f)
	];
}

// ───────────────────────── 근무 ─────────────────────────

TSharedRef<SWidget> SPangaeaHUD::TopBar()
{
	return SNew(SBorder)
		.BorderImage(White())
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.6f))
		.Padding(FMargin(16.f, 8.f))
		[
			Label(TAttribute<FText>::CreateLambda([this]()
			{
				const APangaeaGameMode* GM = G();
				if (!GM) return FText::GetEmpty();
				return FText::Format(LOCTEXT("Top", "제 {0}일 {1}   {2}   |   대기 {3}명   |   {4}"),
					GM->Rule().Day, GM->ShiftLabel(), GM->ClockText(), GM->WaitingCount(), ResourceText());
			}), 16)
		];
}

FText SPangaeaHUD::DocText() const
{
	const APangaeaGameMode* GM = G();
	if (!GM) return FText::GetEmpty();
	const FInspectionSession* Ses = GM->Session();
	if (!Ses)
	{
		return GM->IsNpcArriving() ? LOCTEXT("DocWalking", "다음 생존자가 걸어오는 중...") : LOCTEXT("DocEmpty", "창구가 비어 있다.");
	}
	const FNpcProfile& N = Ses->Npc;
	const FText Damaged = LOCTEXT("DocDamaged", "■■■ (젖어서 판독 불가)");
	return FText::Format(LOCTEXT("DocBody", "이름: {0}\n나이: {1}세\n출신: {2}\n구조 지역: {3}\n구조 일자: {4}\n구조대 코드: {5}"),
		FText::FromString(N.Name), N.Age, PangaeaText::Region(N.HometownIdx),
		N.DamagedField == 0 ? Damaged : PangaeaText::Region(N.DocRegionIdx),
		N.DamagedField == 1 ? Damaged : FText::Format(LOCTEXT("DocDay", "제 {0}일"), N.DocDay),
		N.DamagedField == 2 ? Damaged : FText::FromString(N.DocCode));
}

FText SPangaeaHUD::RulesText() const
{
	const APangaeaGameMode* GM = G();
	if (!GM) return FText::GetEmpty();
	const FDayRule& R = GM->Rule();
	TArray<FText> Routes;
	for (int32 I : R.Routes) Routes.Add(PangaeaText::Region(I));
	TArray<FText> Codes;
	for (const FString& Cd : R.Codes) Codes.Add(FText::FromString(Cd));

	FText Days = R.MinValidDocDay() == R.Day
		? FText::Format(LOCTEXT("RlDay1", "제 {0}일"), R.Day)
		: FText::Format(LOCTEXT("RlDay2", "제 {0}일 ~ 제 {1}일"), R.MinValidDocDay(), R.Day);

	FText Out = FText::Format(LOCTEXT("Rules", "구조선 경로: {0}\n유효 구조대 코드: {1}\n유효 구조 일자: {2}"),
		FText::Join(FText::FromString(TEXT(", ")), Routes), FText::Join(FText::FromString(TEXT(", ")), Codes), Days);
	if (R.Password())
	{
		Out = FText::Format(LOCTEXT("RulesPw", "{0}\n암구호: '{1}' → '{2}'"), Out, PangaeaText::PwAsk(R.PwIndex), PangaeaText::PwAnswer(R.PwIndex));
	}
	return Out;
}

TSharedRef<SWidget> SPangaeaHUD::BuildShift()
{
	auto Seated = [this]() { return G() && G()->IsSeated() ? EVisibility::Visible : EVisibility::Collapsed; };
	auto Standing = [this]() { return G() && !G()->IsSeated() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; };
	auto HasSes = [this]() { return G() && G()->Session() && !G()->IsBusy(); };

	auto ToolBtn = [this](ETool T, const FText& Key)
	{
		return Button(
			TAttribute<FText>::CreateLambda([this, T, Key]()
			{
				const APangaeaGameMode* GM = G();
				FText Name = FText::Format(LOCTEXT("ToolBtn", "{0}  {1}"), Key, PangaeaText::ToolName(T));
				if (T == ETool::BloodKit && GM) Name = FText::Format(LOCTEXT("ToolKits", "{0} ({1})"), Name, GM->State().Kits);
				if (GM && GM->Session())
				{
					const FText Why = GM->Session()->BlockReason(T, GM->State());
					if (!Why.IsEmpty()) return FText::Format(LOCTEXT("ToolWhy", "{0}\n{1}"), Name, Why);
				}
				return Name;
			}),
			[this, T]() { if (G()) G()->UseTool(T); },
			TAttribute<bool>::CreateLambda([this, T]()
			{
				const APangaeaGameMode* GM = G();
				return GM && GM->Session() && !GM->IsBusy() && GM->Session()->CanUse(T, GM->State());
			}), FLinearColor(0.22f, 0.3f, 0.38f), 15);
	};

	auto QBtn = [this](int32 Idx, const FText& Key)
	{
		return Button(
			TAttribute<FText>::CreateLambda([this, Idx, Key]()
			{
				const APangaeaGameMode* GM = G();
				return GM ? FText::Format(LOCTEXT("QBtn", "{0}   {1}"), Key, PangaeaText::Question(GM->Rule(), Idx)) : FText::GetEmpty();
			}),
			[this, Idx]() { if (G()) G()->Ask(Idx); },
			TAttribute<bool>::CreateLambda([this, Idx]()
			{
				const APangaeaGameMode* GM = G();
				const FInspectionSession* Ses = GM ? GM->Session() : nullptr;
				return Ses && !GM->IsBusy() && !Ses->Asked(Idx) && Ses->QuestionsLeft > 0;
			}), FLinearColor(0.28f, 0.27f, 0.32f), 15);
	};

	TSharedRef<SWidget> Left = Panel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[Label(LOCTEXT("DocTitle", "구조 기록"), 20, true, Accent)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 18)[Label(TAttribute<FText>::CreateSP(this, &SPangaeaHUD::DocText), 17)]
		+ SVerticalBox::Slot().AutoHeight()[Label(LOCTEXT("RuleTitle", "오늘 규정"), 20, true, Accent)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)[Label(TAttribute<FText>::CreateSP(this, &SPangaeaHUD::RulesText), 15)],
		400.f);

	TSharedRef<SWidget> Right = Panel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[Label(LOCTEXT("ToolTitle", "검사 도구"), 20, true, Accent)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 8)
		[
			SNew(SUniformGridPanel).SlotPadding(FMargin(4.f))
			+ SUniformGridPanel::Slot(0, 0)[ToolBtn(ETool::Thermo, LOCTEXT("K1", "1"))]
			+ SUniformGridPanel::Slot(1, 0)[ToolBtn(ETool::Flashlight, LOCTEXT("K2", "2"))]
			+ SUniformGridPanel::Slot(0, 1)[ToolBtn(ETool::BloodKit, LOCTEXT("K3", "3"))]
			+ SUniformGridPanel::Slot(1, 1)[ToolBtn(ETool::XRay, LOCTEXT("K4", "4"))]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBox).HeightOverride(190.f)
			[
				SNew(SScrollBox) + SScrollBox::Slot()
				[
					Label(TAttribute<FText>::CreateLambda([this]()
					{
						return G() && G()->Session() ? G()->Session()->ToolLogText() : FText::GetEmpty();
					}), 14, false, FLinearColor(0.75f, 0.9f, 0.95f))
				]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 6)
		[
			Label(TAttribute<FText>::CreateLambda([this]()
			{
				const FInspectionSession* Ses = G() ? G()->Session() : nullptr;
				return FText::Format(LOCTEXT("QTitle", "심문 (남은 질문 {0})"), Ses ? Ses->QuestionsLeft : 0);
			}), 20, true, Accent)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)[QBtn(0, LOCTEXT("KZ", "Z"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)[QBtn(1, LOCTEXT("KX", "X"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)[QBtn(2, LOCTEXT("KC", "C"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)[QBtn(3, LOCTEXT("KV", "V"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
		[
			SNew(SBox).HeightOverride(230.f)
			[
				SNew(SScrollBox) + SScrollBox::Slot()
				[
					Label(TAttribute<FText>::CreateLambda([this]()
					{
						return G() && G()->Session() ? G()->Session()->DialogText() : FText::GetEmpty();
					}), 15)
				]
			]
		],
		480.f);

	TSharedRef<SWidget> Decide = Panel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(4)
				[Button(LOCTEXT("Pass", "통과 (P)"), [this]() { if (G()) G()->DecidePass(); }, TAttribute<bool>::CreateLambda(HasSes), FLinearColor(0.2f, 0.45f, 0.25f), 20)]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(4)
			[
				Button(TAttribute<FText>::CreateLambda([this]()
					{
						return G() && G()->IsShootArmed() ? LOCTEXT("ShootArmed", "한 번 더 누르면 사살 (K)") : LOCTEXT("Shoot", "사살 (K)");
					}),
					[this]() { if (G()) G()->RequestShoot(); }, TAttribute<bool>::CreateLambda(HasSes), FLinearColor(0.55f, 0.15f, 0.12f), 20)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 6, 0, 0)
			[Label(LOCTEXT("StandHint", "E: 자리에서 일어나기"), 13, false, Muted)],
		560.f, 0.7f);

	TSharedRef<SWidget> WalkPrompt = Panel(
		Label(TAttribute<FText>::CreateLambda([this]()
		{
			const APangaeaGameMode* GM = G();
			if (!GM) return FText::GetEmpty();
			FText Base = GM->IsNearSeat() ? LOCTEXT("PromptSit", "E: 착석") : LOCTEXT("PromptGo", "부스 의자로 가서 E로 착석하라 (WASD 이동)");
			if (GM->Session()) Base = FText::Format(LOCTEXT("PromptWaiting", "{0}\n창구에 생존자가 기다리고 있다."), Base);
			return Base;
		}), 17), 0.f, 0.6f);

	return SNew(SOverlay)
		+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Top)[TopBar()]
		+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(20.f, 64.f, 0.f, 0.f)
			[SNew(SBox).Visibility_Lambda(Seated)[Left]]
		+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(0.f, 64.f, 20.f, 0.f)
			[SNew(SBox).Visibility_Lambda(Seated)[Right]]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 24.f)
			[SNew(SBox).Visibility_Lambda(Seated)[Decide]]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 60.f)
			[SNew(SBox).Visibility_Lambda(Standing)[WalkPrompt]];
}

// ───────────────────────── 밤 보고서 ─────────────────────────

TSharedRef<SWidget> SPangaeaHUD::BuildNight()
{
	APangaeaGameMode* GM = G();
	const FDayRule& R = GM->Rule();

	TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
	for (const FReportLine& L : GM->Report())
	{
		List->AddSlot().AutoHeight().Padding(0, 6)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[Label(L.Head, 17, true)]
			+ SVerticalBox::Slot().AutoHeight()[Label(L.Result, 16, false, L.bGood ? GoodCol : BadCol)]
			+ SVerticalBox::Slot().AutoHeight()[Label(L.Detail, 14, false, Muted)]
		];
	}
	if (GM->Report().Num() == 0)
	{
		List->AddSlot().AutoHeight()[Label(LOCTEXT("NoCases", "오늘 검사한 사람이 없다."), 16, false, Muted)];
	}

	TSharedRef<SVerticalBox> Sum = SNew(SVerticalBox);
	for (const FText& T : GM->NightLines()) Sum->AddSlot().AutoHeight().Padding(0, 2)[Label(FText::Format(LOCTEXT("Bullet", "· {0}"), T), 15)];

	const FText Title = R.IsWorkDay()
		? FText::Format(LOCTEXT("NtTitle", "제 {0}일 · 근무 보고서"), R.Day)
		: FText::Format(LOCTEXT("NtRest", "제 {0}일 · 휴무"), R.Day);

	return SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
	[
		Panel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[Label(Title, 28, true, Accent)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 10)
				[SNew(SBox).HeightOverride(430.f)[SNew(SScrollBox) + SScrollBox::Slot()[List]]]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)[Sum]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 10)[Label(TAttribute<FText>::CreateSP(this, &SPangaeaHUD::ResourceText), 15, false, Muted)]
			+ SVerticalBox::Slot().AutoHeight()
			[
				Button(GM->HasPendingEnding() ? LOCTEXT("NtEnd", "결말 보기") : LOCTEXT("NtNext", "시설 관리로"),
					[this]() { if (G()) G()->ToFacilities(); }, true, FLinearColor(0.3f, 0.35f, 0.4f), 18)
			],
			900.f)
	];
}

// ───────────────────────── 시설 / 상점 ─────────────────────────

TSharedRef<SWidget> SPangaeaHUD::BuildFacilities()
{
	TSharedRef<SVerticalBox> Fac = SNew(SVerticalBox);
	for (int32 i = 0; i < (int32)EFacility::Count; ++i)
	{
		const EFacility F = (EFacility)i;
		Fac->AddSlot().AutoHeight().Padding(0, 4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[Label(PangaeaText::FacilityName(F), 17, true)]
				+ SVerticalBox::Slot().AutoHeight()[Label(PangaeaText::FacilityDesc(F), 14, false, Muted)]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0)
			[
				SNew(SBox).WidthOverride(200.f)
				[
					Button(TAttribute<FText>::CreateLambda([this, F]()
						{
							const APangaeaGameMode* GM = G();
							if (!GM) return FText::GetEmpty();
							if (GM->State().Has(F)) return LOCTEXT("FcBuilt", "운영 중");
							if (!GM->Rule().FacilityUnlocked(F)) return FText::Format(LOCTEXT("FcLocked", "{0}일차 해금"), PangaeaRules::FacilityUnlockDay(F));
							return FText::Format(LOCTEXT("FcBuild", "건설 ({0} 크레딧)"), PangaeaRules::FacilityCost(F));
						}),
						[this, F]() { if (G()) G()->Build(F); },
						TAttribute<bool>::CreateLambda([this, F]() { return G() && G()->CanBuild(F); }),
						FLinearColor(0.3f, 0.35f, 0.25f), 15)
				]
			]
		];
	}

	TSharedRef<SVerticalBox> Shop = SNew(SVerticalBox);
	for (int32 i = 0; i < APangaeaGameMode::ShopCount(); ++i)
	{
		Shop->AddSlot().AutoHeight().Padding(0, 4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[Label(G()->ShopLabel(i), 16)]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0)
			[
				SNew(SBox).WidthOverride(200.f)
				[
					Button(FText::Format(LOCTEXT("ShBuy", "구매 ({0} 크레딧)"), APangaeaGameMode::ShopCost(i)),
						[this, i]() { if (G()) G()->Buy(i); },
						TAttribute<bool>::CreateLambda([this, i]() { return G() && G()->CanBuy(i); }),
						FLinearColor(0.25f, 0.3f, 0.38f), 15)
				]
			]
		];
	}

	return SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
	[
		Panel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[Label(LOCTEXT("FcTitle", "검문소 시설 관리"), 28, true, Accent)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 12)[Label(TAttribute<FText>::CreateSP(this, &SPangaeaHUD::ResourceText), 15, false, Muted)]
			+ SVerticalBox::Slot().AutoHeight()[Fac]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 18, 0, 6)[Label(LOCTEXT("ShTitle", "보급품"), 22, true, Accent)]
			+ SVerticalBox::Slot().AutoHeight()[Shop]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 18, 0, 0)
				[Button(LOCTEXT("NextDay", "다음 날로"), [this]() { if (G()) G()->NextDay(); }, true, FLinearColor(0.3f, 0.4f, 0.3f), 18)],
			820.f)
	];
}

// ───────────────────────── 엔딩 ─────────────────────────

TSharedRef<SWidget> SPangaeaHUD::BuildEnding()
{
	APangaeaGameMode* GM = G();
	const FRunState& S = GM->State();
	const FText Stats = FText::Format(
		LOCTEXT("EdStats", "버틴 날: {0}일\n정확한 판정: {1} / {2}\n섬에 들어간 AI: {3}\n사살한 인간: {4}\n사살한 AI: {5}"),
		FMath::Min(S.Day, PangaeaRules::LastDay), S.Correct, S.Total, S.Infiltrated, S.HumansKilled, S.AiKilled);

	return SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
	[
		Panel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[Label(GM->GetEndingTitle(), 40, true, Accent)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 12)[Label(GM->GetEndingText(), 18)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 8)[Label(Stats, 16, false, Muted)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)[Button(LOCTEXT("EdTitle", "타이틀로"), [this]() { if (G()) G()->ToTitle(); })],
			700.f, 0.9f)
	];
}

#undef LOCTEXT_NAMESPACE
