#include "HUD/HOLScriptScreen.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Match/HOLForumMessages.h"
#include "Sound/SoundBase.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "HUD/HOLCheatPanel.h"
#include "HUD/HOLGameHUD.h"
#include "HUD/HOLHUDLayoutRules.h"
#include "HUD/HOLHUDSpace.h"
#include "HUD/HOLScenarioPanel.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "HUD/HOLGameScreen.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLGameMenuRulesBridge.h"
#include "Player/HOLPlayerController.h"
#include "Screens/HOLGameMenuScreen.h"
#include "Screens/HOLStatsScreen.h"
#include "Style/HOLUIStyles.h"
#include "UObject/UObjectIterator.h"

FString UHOLScriptScreen::Describe_Implementation() const
{
	return FString();
}

bool UHOLScriptScreen::ClickWidget_Implementation(const FName Name)
{
	return false;
}

void UHOLScriptScreen::RefreshNow()
{
	RefreshLeft = RefreshSeconds;
	Refresh();
}

int32 UHOLScriptScreen::GetLocalSlot() const
{
	const AHOLPlayerState* PlayerState = GetOwningPlayerState<AHOLPlayerState>();
	return PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
}

int32 UHOLScriptScreen::GetShownSlot() const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	const AHOLPlayerState* PlayerState = GameScreen ? GameScreen->GetBoundPlayerState() : nullptr;
	return PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
}

UBorder* UHOLScriptScreen::GetPlayersPanel() const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	return GameScreen ? GameScreen->GetPlayersPanel() : nullptr;
}

UBorder* UHOLScriptScreen::GetGroupsPanel() const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	return GameScreen ? GameScreen->GetGroupsPanel() : nullptr;
}

int32 UHOLScriptScreen::GetNumControlGroups()
{
	return AHOLPlayerController::NumControlGroups;
}

bool UHOLScriptScreen::GetControlGroupsAvailable() const
{
	return Cast<AHOLPlayerController>(GetOwningPlayer()) != nullptr && GetWorld() != nullptr;
}

bool UHOLScriptScreen::GetControlGroupNow(const int32 Group, int32& OutCount, bool& bOutBuildings) const
{
	OutCount = 0;
	bOutBuildings = false;
	const AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer());
	if (!PlayerController || Group < 0 || Group >= AHOLPlayerController::NumControlGroups)
	{
		return false;
	}
	TArray<uint32> Units;
	TArray<AHOLBuilding*> Buildings;
	const bool bFull = PlayerController->GetControlGroup(Group, Units, Buildings);
	bOutBuildings = Units.IsEmpty();
	OutCount = Units.IsEmpty() ? Buildings.Num() : Units.Num();
	return bFull;
}

void UHOLScriptScreen::GetControlGroupTypes(const int32 Group, TArray<FString>& OutNames, TArray<int32>& OutCounts) const
{
	OutNames.Reset();
	OutCounts.Reset();
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->GetGroupTypes(Group, OutNames, OutCounts);
	}
}

int32 UHOLScriptScreen::GetLastSelectedGroup() const
{
	const AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer());
	double SelectTime = -1.0;
	return PlayerController ? PlayerController->GetLastSelectedGroup(SelectTime) : INDEX_NONE;
}

float UHOLScriptScreen::GetLastSelectedGroupAge() const
{
	const AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer());
	const UWorld* World = GetWorld();
	double SelectTime = -1.0;
	if (!PlayerController || !World || PlayerController->GetLastSelectedGroup(SelectTime) == INDEX_NONE)
	{
		return 1.0e9f;
	}
	return static_cast<float>(World->GetRealTimeSeconds() - SelectTime);
}

void UHOLScriptScreen::SelectControlGroup(const int32 Group)
{
	if (AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer()))
	{
		PlayerController->SelectControlGroup(Group);
	}
}

UBorder* UHOLScriptScreen::GetQueuePanel() const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	return GameScreen ? GameScreen->GetQueuePanel() : nullptr;
}

bool UHOLScriptScreen::GetQueueNow(TArray<FName>& OutTypes, TArray<bool>& OutResearch, TArray<FString>& OutTechNames, TArray<FString>& OutUnitNames,
	bool& bOutHeadStarted, float& OutHeadProgress, bool& bOutBlockedAtFull, bool& bOutPopulationShort) const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	if (!GameScreen)
	{
		OutTypes.Reset();
		OutResearch.Reset();
		OutTechNames.Reset();
		OutUnitNames.Reset();
		bOutHeadStarted = false;
		OutHeadProgress = 0.f;
		bOutBlockedAtFull = false;
		bOutPopulationShort = false;
		return false;
	}
	return GameScreen->ReadQueue(OutTypes, OutResearch, OutTechNames, OutUnitNames, bOutHeadStarted, OutHeadProgress, bOutBlockedAtFull, bOutPopulationShort);
}

void UHOLScriptScreen::SetQueueReport(const FString& Line)
{
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->SetQueueReportMirror(Line);
	}
}

FLinearColor UHOLScriptScreen::GetAccentColor()
{
	return HOLUIStyle::Accent;
}

bool UHOLScriptScreen::CancelProduction(const int32 Index)
{
	AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer());
	if (!PlayerController)
	{
		return false;
	}
	PlayerController->RequestCancelProduction(Index);
	return true;
}

void UHOLScriptScreen::SetQueueLive(const bool bLive)
{
	bQueueLive = bLive;
}

bool UHOLScriptScreen::GetPopulationNow(int32& OutUsed, int32& OutCap, float& OutRealSeconds) const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	const AHOLPlayerState* PlayerState = GameScreen ? GameScreen->GetBoundPlayerState() : nullptr;
	const UWorld* World = GetWorld();
	OutUsed = PlayerState ? PlayerState->GetPopulationUsed() : 0;
	OutCap = PlayerState ? PlayerState->GetPopulationCap() : 0;
	OutRealSeconds = World ? static_cast<float>(World->GetRealTimeSeconds()) : 0.f;
	return PlayerState != nullptr && World != nullptr;
}

bool UHOLScriptScreen::GetEconomyNow(FHOLResources& OutStock, int32& OutPopulationUsed, int32& OutPopulationCap, FHOLResources& OutGatherers, FHOLResources& OutIncome) const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	const AHOLPlayerState* PlayerState = GameScreen ? GameScreen->GetBoundPlayerState() : nullptr;
	OutStock = PlayerState ? PlayerState->GetResources() : FHOLResources();
	OutPopulationUsed = PlayerState ? PlayerState->GetPopulationUsed() : 0;
	OutPopulationCap = PlayerState ? PlayerState->GetPopulationCap() : 0;
	OutGatherers = PlayerState ? PlayerState->GetGatherers() : FHOLResources();
	OutIncome = PlayerState ? PlayerState->GetIncomePerMinute() : FHOLResources();
	return PlayerState != nullptr;
}

int32 UHOLScriptScreen::GetIdleWorkerCount() const
{
	const AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer());
	return PlayerController ? PlayerController->GetIdleWorkers().Num() : 0;
}

bool UHOLScriptScreen::GetStatsButtonShown() const
{
	const AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer());
	const AHOLGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AHOLGameState>() : nullptr;
	return PlayerController && UHOLGameMenuRulesBridge::NativeShowsStatsButton(PlayerController->IsSpectator(), GameState && GameState->IsMatchOver());
}

void UHOLScriptScreen::SelectNextIdleWorker()
{
	if (AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer()))
	{
		PlayerController->SelectNextIdleWorker();
	}
}

void UHOLScriptScreen::OpenGameMenu()
{
	UHOLGameMenuScreen::Open(GetOwningPlayer());
}

void UHOLScriptScreen::OpenStats()
{
	UHOLStatsScreen::Open(GetOwningPlayer());
}

FLinearColor UHOLScriptScreen::GetResourceColor(const int32 Resource)
{
	switch (Resource)
	{
	case 0: return HOLUIStyle::Food;
	case 1: return HOLUIStyle::Wood;
	case 2: return HOLUIStyle::Stone;
	case 3: return HOLUIStyle::Gold;
	default: return HOLUIStyle::TextMuted;
	}
}

void UHOLScriptScreen::AddThreadLine(const FText& Line)
{
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->AddThreadLine(Line);
	}
}

UBorder* UHOLScriptScreen::MakePanel(const FName Name)
{
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
	Panel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel, 4.f));
	Panel->SetPadding(FMargin(12.f, 8.f));
	Panel->SetVisibility(ESlateVisibility::Visible);
	return Panel;
}

UCommonTextBlock* UHOLScriptScreen::MakeText(const FName Name, const TSubclassOf<UCommonTextStyle> TextStyle)
{
	UCommonTextBlock* Block = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), Name);
	Block->SetStyle(TextStyle);
	return Block;
}

UButton* UHOLScriptScreen::MakeButton(const FName Name)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	SetButtonGreyed(Button, false);
	// Never focusable (as the HUD's other buttons): set before its Slate widget is built. InitIsFocusable is protected.
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	Button->IsFocusable = false;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	Button->SetVisibility(ESlateVisibility::Visible);
	return Button;
}

void UHOLScriptScreen::SetButtonGreyed(UButton* Button, const bool bGreyed)
{
	if (!Button)
	{
		return;
	}
	// UHOLHUDButtonStyle's and UHOLHUDButtonStyleGreyed's brushes (HOLUIStyles.cpp), for a UMG button the script fills.
	constexpr float Radius = 3.f;
	const FLinearColor Greyed(0.035f, 0.035f, 0.045f, 1.f);
	FButtonStyle Look;
	if (bGreyed)
	{
		Look.SetNormal(FSlateRoundedBoxBrush(Greyed, Radius));
		Look.SetHovered(FSlateRoundedBoxBrush(Greyed, Radius, HOLUIStyle::TextMuted, 1.f));
		Look.SetPressed(FSlateRoundedBoxBrush(Greyed, Radius, HOLUIStyle::TextMuted, 1.f));
	}
	else
	{
		Look.SetNormal(FSlateRoundedBoxBrush(FLinearColor(0.07f, 0.07f, 0.09f, 1.f), Radius));
		Look.SetHovered(FSlateRoundedBoxBrush(FLinearColor(0.16f, 0.12f, 0.05f, 1.f), Radius, HOLUIStyle::Accent, 1.f));
		Look.SetPressed(FSlateRoundedBoxBrush(FLinearColor(0.25f, 0.18f, 0.06f, 1.f), Radius, HOLUIStyle::Accent, 1.f));
	}
	Look.SetDisabled(FSlateRoundedBoxBrush(Greyed, Radius));
	Look.SetNormalPadding(FMargin(4.f));
	Look.SetPressedPadding(FMargin(4.f));
	Button->SetStyle(Look);
}

UProgressBar* UHOLScriptScreen::MakeBar(const FName Name, const FLinearColor FillColor)
{
	UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), Name);
	FProgressBarStyle Look;
	Look.SetBackgroundImage(FSlateRoundedBoxBrush(FLinearColor(0.08f, 0.08f, 0.1f, 1.f), 2.f));
	Look.SetFillImage(FSlateRoundedBoxBrush(FLinearColor::White, 2.f));
	Bar->SetWidgetStyle(Look);
	Bar->SetFillColorAndOpacity(FillColor);
	Bar->SetPercent(0.f);
	return Bar;
}

FLinearColor UHOLScriptScreen::GetLocalColor() const
{
	const AHOLPlayerState* PlayerState = GetOwningPlayerState<AHOLPlayerState>();
	if (!PlayerState || PlayerState->GetPlayerIndex() == INDEX_NONE)
	{
		return HOLUIStyle::TextMuted;
	}
	return FLinearColor(HOLPlayerColors::GetColor(PlayerState->GetColorIndex()));
}

FLinearColor UHOLScriptScreen::GetTextColor()
{
	return HOLUIStyle::Text;
}

FLinearColor UHOLScriptScreen::GetMutedColor()
{
	return HOLUIStyle::TextMuted;
}

FLinearColor UHOLScriptScreen::GetMissingColor()
{
	return HOLUIStyle::Missing;
}

float UHOLScriptScreen::GetCheatPanelBottom() const
{
	const UHOLCheatPanel* CheatPanel = AHOLGameHUD::FindCheatPanel(GetOwningPlayer());
	return CheatPanel ? CheatPanel->GetBottom() : 0.f;
}

void UHOLScriptScreen::SetCheatsHeroBlockShown(const bool bShown) const
{
	if (UHOLCheatPanel* CheatPanel = AHOLGameHUD::FindCheatPanel(GetOwningPlayer()))
	{
		CheatPanel->SetHeroBlockShown(bShown);
	}
}

FVector2D UHOLScriptScreen::GetViewportSlateSize() const
{
	const float Dpi = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), UE_KINDA_SMALL_NUMBER);
	return UWidgetLayoutLibrary::GetViewportSize(this) / Dpi;
}

bool UHOLScriptScreen::GetWidgetBox(UWidget* Widget, FVector2D& OutMin, FVector2D& OutMax) const
{
	if (!Widget || !Widget->IsVisible())
	{
		return false;
	}
	const FGeometry& Geometry = Widget->GetCachedGeometry();
	if (Geometry.GetAbsoluteSize().IsNearlyZero())
	{
		return false;
	}
	// Desktop space is not the HUD's (a windowed game): HOLHUDSpace.
	const FBox2D Box = HOLHUDSpace::BoxOf(this, Geometry);
	OutMin = Box.Min;
	OutMax = Box.Max;
	return true;
}

bool UHOLScriptScreen::GetGameScreenBox(const FName WidgetName, FVector2D& OutMin, FVector2D& OutMax) const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	UWidget* Widget = GameScreen && GameScreen->WidgetTree ? GameScreen->WidgetTree->FindWidget(WidgetName) : nullptr;
	return GetWidgetBox(Widget, OutMin, OutMax);
}

void UHOLScriptScreen::GatherPanelBoxes(TArray<FHOLHUDNamedBox>& OutBoxes) const
{
	if (const UHOLCheatPanel* CheatPanel = AHOLGameHUD::FindCheatPanel(GetOwningPlayer()))
	{
		FBox2D CheatBox;
		if (CheatPanel->GetBox(CheatBox))
		{
			OutBoxes.Add({ TEXT("cheats"), CheatBox });
		}
	}
	// L1-12b: the test scenario's own panel (top centre; a development build only): a panel of the script (the duel window) under it is seen.
	if (const UHOLScenarioPanel* ScenarioPanel = AHOLGameHUD::FindScenarioPanel(GetOwningPlayer()))
	{
		FBox2D ScenarioBox;
		if (ScenarioPanel->GetBox(ScenarioBox))
		{
			OutBoxes.Add({ TEXT("scenario"), ScenarioBox });
		}
	}
	// The C++ blocks a panel of the script must not cover (the Diplomacy panel's trial): the players' table, the menu's buttons.
	// MenuPanel is the script's since H-3 (UHOLTopBarPanel): its box comes with the script's outermost panels below.
	const TCHAR* const GameNames[] = { TEXT("PlayersPanel") };
	for (const TCHAR* GameName : GameNames)
	{
		FVector2D Low;
		FVector2D High;
		if (GetGameScreenBox(FName(GameName), Low, High))
		{
			OutBoxes.Add({ GameName, FBox2D(Low, High) });
		}
	}
	if (WidgetTree)
	{
		TArray<UWidget*> Widgets;
		WidgetTree->GetAllWidgets(Widgets);
		for (UWidget* Widget : Widgets)
		{
			const UBorder* Border = Cast<UBorder>(Widget);
			if (!Border || !Border->IsVisible())
			{
				continue;
			}
			// A border inside another one (the hero's portrait in the hero's block, its ability boxes) is part of that panel, not a panel
			// covering it: only the outermost borders are compared (the hero's block read "HeroPanel x HeroPortraitFill" in every scenario).
			bool bNested = false;
			for (const UWidget* Parent = Border->GetParent(); Parent && !bNested; Parent = Parent->GetParent())
			{
				bNested = Parent->IsA<UBorder>();
			}
			if (bNested)
			{
				continue;
			}
			const FGeometry& Geometry = Border->GetCachedGeometry();
			if (Geometry.GetAbsoluteSize().IsNearlyZero())
			{
				continue;
			}
			OutBoxes.Add({ Border->GetName(), HOLHUDSpace::BoxOf(this, Geometry) });
		}
	}
}

FString UHOLScriptScreen::DescribeRects() const
{
	// L1-12b: every box DescribeOverlaps compares (the cheats' panel, PlayersPanel, MenuPanel, the script's outermost visible panels such as
	// AILevelPanel and DuelPanel), Slate units of the HUD; Tools\Scenarios\HudRects.ps1 reads the line.
	TArray<FHOLHUDNamedBox> Boxes;
	GatherPanelBoxes(Boxes);
	FString Result;
	for (const FHOLHUDNamedBox& Named : Boxes)
	{
		Result += (Result.IsEmpty() ? TEXT("") : TEXT("; ")) + FString::Printf(TEXT("%s=%.1f,%.1f,%.1f,%.1f"), *Named.Name, Named.Box.Min.X, Named.Box.Min.Y, Named.Box.Max.X, Named.Box.Max.Y);
	}
	return Result;
}

FString UHOLScriptScreen::DescribeOverlaps() const
{
	using FNamedBox = FHOLHUDNamedBox;
	TArray<FNamedBox> Boxes;
	GatherPanelBoxes(Boxes);
	FString Result;
	// Placement (2026-11-03, E-diplomacy-compare): a panel must lie inside the window (a box off it was read in another space than the
	// HUD's, as the windowed game's desktop coordinates were), and the Diplomacy button sits left of the menu's buttons at their height.
	const FVector2D Room = GetViewportSlateSize();
	const FBox2D* MenuBox = nullptr;
	const FBox2D* ButtonBox = nullptr;
	for (const FNamedBox& Named : Boxes)
	{
		const float Slack = 2.f;
		if (Named.Box.Min.X < -Slack || Named.Box.Min.Y < -Slack || Named.Box.Max.X > Room.X + Slack || Named.Box.Max.Y > Room.Y + Slack)
		{
			Result += (Result.IsEmpty() ? TEXT("") : TEXT("; ")) + FString::Printf(TEXT("%s outside window (%d,%d to %d,%d in %dx%d)"), *Named.Name,
				FMath::RoundToInt(Named.Box.Min.X), FMath::RoundToInt(Named.Box.Min.Y), FMath::RoundToInt(Named.Box.Max.X), FMath::RoundToInt(Named.Box.Max.Y),
				FMath::RoundToInt(Room.X), FMath::RoundToInt(Room.Y));
		}
		if (Named.Name == TEXT("MenuPanel"))
		{
			MenuBox = &Named.Box;
		}
		else if (Named.Name == TEXT("DiplomacyButtonPanel"))
		{
			ButtonBox = &Named.Box;
		}
	}
	if (MenuBox && ButtonBox && (FMath::Abs(ButtonBox->Min.Y - MenuBox->Min.Y) > 3.f || ButtonBox->Max.X > MenuBox->Min.X + 3.f))
	{
		Result += (Result.IsEmpty() ? TEXT("") : TEXT("; ")) + FString::Printf(TEXT("DiplomacyButtonPanel misplaced against MenuPanel (button %d,%d to %d,%d; menu %d,%d to %d,%d)"),
			FMath::RoundToInt(ButtonBox->Min.X), FMath::RoundToInt(ButtonBox->Min.Y), FMath::RoundToInt(ButtonBox->Max.X), FMath::RoundToInt(ButtonBox->Max.Y),
			FMath::RoundToInt(MenuBox->Min.X), FMath::RoundToInt(MenuBox->Min.Y), FMath::RoundToInt(MenuBox->Max.X), FMath::RoundToInt(MenuBox->Max.Y));
	}
	for (int32 First = 0; First < Boxes.Num(); ++First)
	{
		for (int32 Second = First + 1; Second < Boxes.Num(); ++Second)
		{
			if (HOLHUDLayout::Overlaps(Boxes[First].Box, Boxes[Second].Box))
			{
				Result += (Result.IsEmpty() ? TEXT("") : TEXT("; ")) + FString::Printf(TEXT("%s x %s"), *Boxes[First].Name, *Boxes[Second].Name);
			}
		}
	}
	return Result.IsEmpty() ? FString(TEXT("none")) : Result;
}

FMargin UHOLScriptScreen::GetBottomLeftPlace() const
{
	// The game screen made this widget (CreateWidget: its widget tree is the outer); EtapeDefini T: the place follows the
	// bar's scale, so the script reads it on every refresh. Without it: the places at scale 1.
	if (const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		return GameScreen->GetBottomLeftPlace();
	}
	return GetDefault<UHOLGameScreen>()->GetBottomLeftPlace();
}

bool UHOLScriptScreen::IsProductionHeldForRoom() const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	return GameScreen && GameScreen->IsProductionHeldForRoom();
}

void UHOLScriptScreen::SetShownMessageKey(const FString& Key)
{
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->SetMessageKeyMirror(Key);
	}
	bMessageLive = !Key.IsEmpty();
}

void UHOLScriptScreen::SetTextWrapWidth(UCommonTextBlock* Text, const float Width) const
{
	if (Text)
	{
		Text->SetWrapTextAt(Width);
	}
}

UClass* UHOLScriptScreen::FindScriptClass()
{
	// Angelscript's hot reload keeps the older classes, flagged CLASS_NewerVersionExists.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
		{
			return *It;
		}
	}
	return nullptr;
}

bool UHOLScriptScreen::GetAlertBanner(bool& bOutShown, int32& OutSerial, int32& OutSlot, float& OutOpacity, bool& bOutSound, FString& OutText,
	TArray<FHOLForumPieceValue>& OutPieces, TArray<FLinearColor>& OutColors) const
{
	bOutShown = false;
	OutSerial = 0;
	OutSlot = INDEX_NONE;
	OutOpacity = 0.f;
	bOutSound = false;
	OutText.Reset();
	OutPieces.Reset();
	OutColors.Reset();
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	const UWorld* World = GetWorld();
	if (!GameScreen || !World)
	{
		return false;
	}
	const FHOLForumAlert& Alert = GameScreen->GetAlert();
	OutSerial = GameScreen->GetAlertPresentCount();
	bOutShown = Alert.bShown;
	if (!Alert.bShown)
	{
		return true;
	}
	OutSlot = Alert.Slot;
	OutOpacity = Alert.GetOpacity(World->GetRealTimeSeconds());
	bOutSound = Alert.bSound;
	OutText = Alert.Message.Text.ToString();
	// The pieces as the C++ FillMessageRow cuts them (the shown player's reading, the lobby's names when the game state is there).
	const AHOLGameState* GameState = World->GetGameState<AHOLGameState>();
	const AHOLPlayerState* PlayerState = GameScreen->GetBoundPlayerState();
	const int32 ViewerSlot = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
	for (const HOLForumMessages::FPiece& Part : GameState ? HOLForumMessages::SplitNames(Alert.Message, ViewerSlot, GameState->GetLaunchConfig())
		: HOLForumMessages::SplitNames(Alert.Message, ViewerSlot))
	{
		FHOLForumPieceValue Piece;
		Piece.Text = Part.Text;
		Piece.Slot = Part.Slot;
		OutPieces.Add(Piece);
		const bool bName = Part.Slot != INDEX_NONE && GameState && GameState->GetLaunchConfig().Slots.IsValidIndex(Part.Slot);
		OutColors.Add(bName ? FLinearColor(HOLPlayerColors::GetColor(GameState->GetLaunchConfig().Slots[Part.Slot].ColorIndex)) : FLinearColor::White);
	}
	return true;
}

bool UHOLScriptScreen::PlayAlertSound()
{
	// Provisional, 2D, the alert's only (FHOLForumAlert::SoundPath): the herald's voice will replace it.
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, FHOLForumAlert::SoundPath);
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
	return Sound != nullptr;
}

void UHOLScriptScreen::BindAlertClick(UBorder* Panel)
{
	if (Panel)
	{
		Panel->OnMouseButtonDownEvent.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHOLScriptScreen, HandleAlertMouseDown));
	}
}

FEventReply UHOLScriptScreen::HandleAlertMouseDown(FGeometry Geometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FEventReply(false);
	}
	// The script's ClickWidget(AlertPanel): true only while a banner is shown, as the C++ handler's own guard was.
	return FEventReply(ClickWidget(FName(TEXT("AlertPanel"))));
}

void UHOLScriptScreen::FocusCameraOnForum(const int32 ForumSlot)
{
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->FocusCameraOnForum(ForumSlot);
	}
}

void UHOLScriptScreen::NativeOnInitialized()
{
	// The script's OnInitialized builds the tree.
	Super::NativeOnInitialized();
	// As the C++ frame: the clicks go through, only the panels take them.
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UWidget* Root = GetRootWidget())
	{
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UHOLScriptScreen::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshNow();
}

void UHOLScriptScreen::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// The short message's fade and end (HOLShortMessage.as): called only while a message with a key is shown.
	if (bMessageLive)
	{
		FrameTick(InDeltaTime);
	}
	// The major alert banner's fade and end (HOLAlertBanner.as): called only while the C++ state shows one, and once more when it ends.
	if (const UHOLGameScreen* AlertScreen = GetTypedOuter<UHOLGameScreen>())
	{
		const bool bAlertShown = AlertScreen->GetAlert().bShown;
		if (bAlertShown || bAlertLive)
		{
			AlertFrameTick(InDeltaTime);
		}
		bAlertLive = bAlertShown;
	}
	// The control group lit (the chip of the group just recalled, HOLGroupBar.as LightSeconds = 0.5 s): the script is called only when it changes, on the very frame.
	{
		const int32 SelectedGroup = GetLastSelectedGroup();
		const int32 LitNow = (SelectedGroup != INDEX_NONE && GetLastSelectedGroupAge() < 0.5f) ? SelectedGroup : INDEX_NONE;
		if (LitNow != LitGroupSeen)
		{
			LitGroupSeen = LitNow;
			GroupLitChanged();
		}
	}
	// The queue's head bar, every frame, ONLY while a queue is shown (HUD H-7; the C++ UpdateHead ran every frame).
	if (bQueueLive)
	{
		QueueFrameTick();
	}
	// The T key (the talent tree, TQ4 = A): the viewport's key state, as the Enter of a test scenario; the console open or a focused menu keeps it.
	if (const APlayerController* Controller = GetOwningPlayer())
	{
		if (Controller->IsLocalController() && Controller->WasInputKeyJustPressed(EKeys::T))
		{
			KeyPressed(TEXT("T"));
		}
		// Ctrl + D: the Diplomacy panel (D387, D650). Not F11 (the window's full screen); D alone is a cell of the command grid (D30), so the
		// chord keeps clear of it. A provisional key (DiplomacyInProgress.md), reconfigurable with the others.
		if (Controller->IsLocalController() && Controller->WasInputKeyJustPressed(EKeys::D)
			&& (Controller->IsInputKeyDown(EKeys::LeftControl) || Controller->IsInputKeyDown(EKeys::RightControl)))
		{
			KeyPressed(TEXT("CtrlD"));
		}
	}
	RefreshLeft -= InDeltaTime;
	if (RefreshLeft <= 0.f)
	{
		RefreshNow();
	}
}
