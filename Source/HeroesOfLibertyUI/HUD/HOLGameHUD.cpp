#include "HUD/HOLGameHUD.h"

#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"
#include "HeroesOfLibertyUILog.h"
#include "Dev/HOLTestHooks.h"
#include "HUD/HOLCheatPanel.h"
#include "HUD/HOLGameScreen.h"
#include "HUD/HOLScenarioPanel.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLGameMenuRules.h"
#include "Screens/HOLEndBanner.h"
#include "TimerManager.h"
#include "Player/HOLPlayerController.h"
#include "Screens/HOLGameMenu.h"
#include "Screens/HOLStatsScreen.h"
#include "Widgets/HOLRootLayout.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLRootLayout.h"

namespace UE::HeroesOfLiberty::UI::GameHUD
{
	/** The selection box: a faint fill and a light outline. */
	const FLinearColor BoxFill(1.f, 1.f, 1.f, 0.08f);
	const FLinearColor BoxOutline(1.f, 1.f, 1.f, 0.85f);
	constexpr float BoxOutlineThickness = 1.5f;

	/**
	 * The « +10 » (J5c.3): starts this high above the ground where the Peasant unloaded (cm, over its head), rises this
	 * much while it shows, fully opaque for the first part of its life, then fades out.
	 */
	constexpr double PopupStartHeight = 200.0;
	constexpr double PopupRise = 120.0;
	constexpr float PopupOpaqueShare = 0.5f;
	/** Text size (points), the resource's marker (pixels), the gap between them and a stacked one's line height. */
	constexpr float PopupFontSize = 20.f;
	constexpr float PopupMarkerSize = 13.f;
	constexpr float PopupMarkerGap = 5.f;
	constexpr float PopupLineHeight = 24.f;
	const FLinearColor PopupText(1.f, 0.97f, 0.88f, 1.f);
	const FLinearColor PopupOutline(0.f, 0.f, 0.f, 0.85f);

#if !UE_BUILD_SHIPPING
	/** The HUD whose panel HOLTestHooks::OnScenarioChanged refreshes: only it clears the hook when it ends. */
	TWeakObjectPtr<const AHOLGameHUD> ScenarioHookOwner;
#endif

	/** A resource's colour, as its marker in the top bar. */
	const FLinearColor& GetResourceColor(const EHOLResource Resource)
	{
		switch (Resource)
		{
		case EHOLResource::Wood: return HOLUIStyle::Wood;
		case EHOLResource::Stone: return HOLUIStyle::Stone;
		case EHOLResource::Gold: return HOLUIStyle::Gold;
		default: return HOLUIStyle::Food;
		}
	}
}

void AHOLGameHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}
	PopupFont = LoadObject<UFont>(nullptr, TEXT("/Game/UI/Fonts/F_HOL.F_HOL"));
	Screen = CreateWidget<UHOLGameScreen>(PlayerController, UHOLGameScreen::StaticClass());
	if (Screen)
	{
		Screen->AddToViewport();
		UE_LOG(LogHOLUI, Display, TEXT("Game HUD: game screen shown"));
	}
#if !UE_BUILD_SHIPPING
	// The cheats' panel (D291), over the game screen, under its top bar.
	CheatPanel = CreateWidget<UHOLCheatPanel>(PlayerController, UHOLCheatPanel::StaticClass());
	if (CheatPanel)
	{
		CheatPanel->SetAnchorWidget(Screen ? Screen->GetTopBar() : nullptr);
		CheatPanel->AddToViewport(/*ZOrder*/ 1);
	}
	// The test scenario's panel (hol.Scenario), at the top centre, over both: what a scenario started before this map
	// opened shows at once, and what comes after (its notes, ready) through the hook.
	ScenarioPanel = CreateWidget<UHOLScenarioPanel>(PlayerController, UHOLScenarioPanel::StaticClass());
	if (ScenarioPanel)
	{
		ScenarioPanel->SetAnchorWidget(Screen ? Screen->GetTopBar() : nullptr);
		ScenarioPanel->AddToViewport(/*ZOrder*/ 2);
		const TWeakObjectPtr<UHOLScenarioPanel> WeakPanel(ScenarioPanel);
		HOLTestHooks::OnScenarioChanged() = [WeakPanel]()
		{
			if (UHOLScenarioPanel* Panel = WeakPanel.Get())
			{
				Panel->Refresh();
			}
		};
		UE::HeroesOfLiberty::UI::GameHUD::ScenarioHookOwner = this;
	}
#endif
	// D356: the in-game menu's layers, over everything; Escape with nothing to undo opens the menu.
	MenuLayout = CreateWidget<UHOLRootLayout>(PlayerController, UHOLRootLayout::StaticClass());
	if (MenuLayout)
	{
		MenuLayout->AddToViewport(/*ZOrder*/ 10);
	}
	if (AHOLPlayerController* HOLController = Cast<AHOLPlayerController>(PlayerController))
	{
		const TWeakObjectPtr<AHOLPlayerController> WeakController(HOLController);
		HOLController->OnGameMenuRequested.AddWeakLambda(this, [WeakController]()
		{
			UHOLGameMenu::Open(WeakController.Get());
		});
	}
	// D357, V1-Q2: the end banner, at each defeat or the match's end.
	ClosedEndBanner = EHOLEndBanner::None;
	WatchVictory();
}

void AHOLGameHUD::WatchVictory()
{
	UWorld* World = GetWorld();
	AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (GameState && WatchedGameState.Get() != GameState)
	{
		GameState->OnVictoryChanged.AddUObject(this, &AHOLGameHUD::RefreshEndBanner);
		WatchedGameState = GameState;
	}
	const AHOLPlayerController* Controller = Cast<AHOLPlayerController>(GetOwningPlayerController());
	if (GameState && Controller && Controller->GetHOLPlayerState())
	{
		GetWorldTimerManager().ClearTimer(WatchVictoryTimer);
		RefreshEndBanner();
	}
	else if (World && !GetWorldTimerManager().IsTimerActive(WatchVictoryTimer))
	{
		GetWorldTimerManager().SetTimer(WatchVictoryTimer, this, &AHOLGameHUD::WatchVictory, 0.25f, /*bLoop*/ true);
	}
}

void AHOLGameHUD::ReopenEndBanner()
{
	UE_LOG(LogHOLUI, Display, TEXT("End banner: reopened"));
	ClosedEndBanner = EHOLEndBanner::None;
	RefreshEndBanner();
}

void AHOLGameHUD::RefreshEndBanner()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	const AHOLPlayerController* Controller = Cast<AHOLPlayerController>(PlayerController);
	const AHOLPlayerState* PlayerState = Controller ? Controller->GetHOLPlayerState() : nullptr;
	const AHOLGameState* GameState = WatchedGameState.Get();
	if (!PlayerState || !GameState)
	{
		return;
	}
	const EHOLEndBanner Due = HOLGameMenuRules::GetEndBanner(PlayerState->GetPlayerIndex(), GameState->GetDefeatedMask(),
		GameState->IsMatchOver(), GameState->GetWinnerMask());
	// The statistics screen opened from the banner (C4) covers it, the banner still there under it.
	const UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	const bool bShown = EndBanner && (EndBanner->IsActivated() || (Layout && Cast<UHOLStatsScreen>(Layout->GetActiveMenu())));
	if (!HOLGameMenuRules::ShouldOpenEndBanner(Due, bShown ? EndBanner->GetResult() : EHOLEndBanner::None, ClosedEndBanner))
	{
		return;
	}
	if (bShown)
	{
		EndBanner->SetResult(Due);
		return;
	}
	EndBanner = UHOLEndBanner::Open(PlayerController, Due);
	if (EndBanner && Due == EHOLEndBanner::MatchOver)
	{
		// M6-Q10: the spectator's banner names the winners (the lobby's names, an AI's « IA Difficile 3 », else Joueur N).
		TArray<FText> Names;
		for (const int32 Slot : GameState->GetWinners())
		{
			Names.Add(HOLForumMessages::GetPlayerName(GameState->GetLaunchConfig(), Slot, INDEX_NONE));
		}
		EndBanner->SetDetail(Names.IsEmpty() ? NSLOCTEXT("HOLGameHUD", "NoWinner", "Aucun vainqueur")
			: FText::Format(Names.Num() == 1 ? NSLOCTEXT("HOLGameHUD", "Winner", "Vainqueur : {0}") : NSLOCTEXT("HOLGameHUD", "Winners", "Vainqueurs : {0}"),
				FText::Join(FText::AsCultureInvariant(TEXT(", ")), Names)));
	}
	if (EndBanner)
	{
		EndBanner->OnClosedByPlayer.AddWeakLambda(this, [this](const EHOLEndBanner Closed)
		{
			ClosedEndBanner = Closed;
		});
	}
}

void AHOLGameHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if !UE_BUILD_SHIPPING
	// Back in the menu, a scenario's error before its map opens falls back to the debug line instead of a gone panel.
	if (UE::HeroesOfLiberty::UI::GameHUD::ScenarioHookOwner.Get() == this)
	{
		HOLTestHooks::OnScenarioChanged() = nullptr;
		UE::HeroesOfLiberty::UI::GameHUD::ScenarioHookOwner.Reset();
	}
#endif
	if (MenuLayout)
	{
		MenuLayout->RemoveFromParent();
		MenuLayout = nullptr;
	}
	if (ScenarioPanel)
	{
		ScenarioPanel->RemoveFromParent();
		ScenarioPanel = nullptr;
	}
	if (CheatPanel)
	{
		CheatPanel->RemoveFromParent();
		CheatPanel = nullptr;
	}
	if (Screen)
	{
		Screen->RemoveFromParent();
		Screen = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AHOLGameHUD::DrawHUD()
{
	Super::DrawHUD();
	const AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayerController());
	if (!Canvas || !PlayerController)
	{
		return;
	}
	DrawDepositPopups(*PlayerController);
	DrawSelectionBox(*PlayerController);
}

void AHOLGameHUD::DrawDepositPopups(const AHOLPlayerController& PlayerController)
{
	using namespace UE::HeroesOfLiberty::UI::GameHUD;

	const TConstArrayView<AHOLPlayerController::FShownDepositPopup> Popups = PlayerController.GetDepositPopups();
	const UWorld* World = GetWorld();
	if (Popups.IsEmpty() || !World)
	{
		return;
	}
	const UFont* Font = PopupFont ? PopupFont.Get() : GEngine->GetLargeFont();
	const FSlateFontInfo FontInfo(Font, FMath::RoundToInt(PopupFontSize), TEXT("Bold"));
	const float Now = World->GetTimeSeconds();
	for (const AHOLPlayerController::FShownDepositPopup& Shown : Popups)
	{
		const float Life = (Now - Shown.BornSeconds) / AHOLPlayerController::DepositPopupSeconds;
		if (Life < 0.f || Life >= 1.f)
		{
			continue;
		}
		const FVector Anchor = FVector(Shown.Popup.Location) + FVector(0., 0., PopupStartHeight + PopupRise * Life);
		const FVector OnScreen = Project(Anchor);
		if (OnScreen.Z <= 0. || OnScreen.X < -50. || OnScreen.Y < -50. || OnScreen.X > Canvas->ClipX + 50. || OnScreen.Y > Canvas->ClipY + 50.)
		{
			continue;
		}
		const float Alpha = Life < PopupOpaqueShare ? 1.f : 1.f - (Life - PopupOpaqueShare) / (1.f - PopupOpaqueShare);
		const FVector2D Position(OnScreen.X, OnScreen.Y - Shown.Stack * PopupLineHeight);

		// The amount, its left edge just right of the anchor; the marker left of it, centred on its height.
		FCanvasTextItem Text(FVector2D(Position.X + PopupMarkerGap * 0.5, Position.Y), FText::AsCultureInvariant(FString::Printf(TEXT("+%d"), Shown.Popup.Amount)),
			FontInfo, PopupText.CopyWithNewOpacity(Alpha));
		Text.bOutlined = true;
		Text.OutlineColor = PopupOutline.CopyWithNewOpacity(PopupOutline.A * Alpha);
		Canvas->DrawItem(Text);
		const double TextHeight = Text.DrawnSize.Y > 0. ? Text.DrawnSize.Y : PopupFontSize;
		const double MarkerX = Position.X - PopupMarkerGap * 0.5 - PopupMarkerSize;
		const double MarkerY = Position.Y + (TextHeight - PopupMarkerSize) * 0.5;
		DrawRect(PopupOutline.CopyWithNewOpacity(PopupOutline.A * Alpha), MarkerX - 1., MarkerY - 1., PopupMarkerSize + 2., PopupMarkerSize + 2.);
		DrawRect(GetResourceColor(Shown.Popup.Resource).CopyWithNewOpacity(Alpha), MarkerX, MarkerY, PopupMarkerSize, PopupMarkerSize);
	}
}

void AHOLGameHUD::DrawSelectionBox(const AHOLPlayerController& PlayerController)
{
	using namespace UE::HeroesOfLiberty::UI::GameHUD;

	FVector2D Min, Max;
	if (!PlayerController.GetSelectionBox(Min, Max))
	{
		return;
	}
	const FVector2D Size(Canvas->ClipX, Canvas->ClipY);
	const FVector2D TopLeft = Min * Size;
	const FVector2D BottomRight = Max * Size;
	DrawRect(BoxFill, TopLeft.X, TopLeft.Y, BottomRight.X - TopLeft.X, BottomRight.Y - TopLeft.Y);
	DrawLine(TopLeft.X, TopLeft.Y, BottomRight.X, TopLeft.Y, BoxOutline, BoxOutlineThickness);
	DrawLine(BottomRight.X, TopLeft.Y, BottomRight.X, BottomRight.Y, BoxOutline, BoxOutlineThickness);
	DrawLine(BottomRight.X, BottomRight.Y, TopLeft.X, BottomRight.Y, BoxOutline, BoxOutlineThickness);
	DrawLine(TopLeft.X, BottomRight.Y, TopLeft.X, TopLeft.Y, BoxOutline, BoxOutlineThickness);
}

UHOLGameScreen* AHOLGameHUD::FindScreen(const APlayerController* PlayerController)
{
	const AHOLGameHUD* GameHUD = PlayerController ? Cast<AHOLGameHUD>(PlayerController->GetHUD()) : nullptr;
	return GameHUD ? GameHUD->GetScreen() : nullptr;
}

UHOLCheatPanel* AHOLGameHUD::FindCheatPanel(const APlayerController* PlayerController)
{
	const AHOLGameHUD* GameHUD = PlayerController ? Cast<AHOLGameHUD>(PlayerController->GetHUD()) : nullptr;
	return GameHUD ? GameHUD->GetCheatPanel() : nullptr;
}

UHOLScenarioPanel* AHOLGameHUD::FindScenarioPanel(const APlayerController* PlayerController)
{
	const AHOLGameHUD* GameHUD = PlayerController ? Cast<AHOLGameHUD>(PlayerController->GetHUD()) : nullptr;
	return GameHUD ? GameHUD->GetScenarioPanel() : nullptr;
}
