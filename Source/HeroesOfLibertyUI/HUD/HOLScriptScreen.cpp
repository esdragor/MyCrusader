#include "HUD/HOLScriptScreen.h"
#include "Components/HorizontalBox.h"
#include "Player/HOLCommandGrid.h"
#include "Widgets/HOLGridCell.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Match/HOLForumMessages.h"
#include "Sound/SoundBase.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "HUD/HOLGameHUD.h"
#include "HUD/HOLHUDLayoutRules.h"
#include "HUD/HOLHUDSpace.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "HUD/HOLGameScreen.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Buildings/HOLBuilding.h"
#include "Player/HOLGameMenuRulesBridge.h"
#include "Player/HOLPlayerController.h"
#include "Screens/HOLGameMenuScreen.h"
#include "Screens/HOLStatsScreen.h"
#include "Style/HOLUIStyles.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/Engine.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
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

// Unique namespace: unity builds merge the .cpp files.
namespace HOLScriptScreenAgesPrivate
{
	// The reverse of HOLCommandGridReadLibraryPrivate::ToRule (HOLCommandGridReadLibrary.cpp): the rule's context as the script reads it.
	FHOLGridContextView ToView(const FHOLGridContext& Rule)
	{
		FHOLGridContextView Out;
		Out.bOwnBuilding = Rule.bOwnBuilding;
		Out.bBuildingGroup = Rule.bBuildingGroup;
		Out.bBuildingFinished = Rule.bBuildingFinished;
		Out.bForumDown = Rule.bForumDown;
		Out.bMarket = Rule.bMarket;
		Out.Produces = Rule.Produces;
		Out.bBuilders = Rule.bBuilders;
		Out.bOwnUnits = Rule.bOwnUnits;
		Out.bSoldiers = Rule.bSoldiers;
		Out.bOnlyKings = Rule.bOnlyKings;
		Out.bHero = Rule.bHero;
		Out.HeroLevel = Rule.HeroLevel;
		Out.ReachedAge = Rule.ReachedAge;
		Out.Researches = Rule.Researches;
		Out.bDeployables = Rule.bDeployables;
		Out.bDeployedAny = Rule.bDeployedAny;
		Out.bGarrisonOccupied = Rule.bGarrisonOccupied;
		Out.bRageUnit = Rule.bRageUnit;
		Out.bPrimaryForum = Rule.bPrimaryForum;
		Out.SpecializationTier = Rule.SpecializationTier;
		Out.SpecializationPendingOption = Rule.SpecializationPendingOption;
		Out.SpecializationRemainingMs = Rule.SpecializationRemainingMs;
		for (int32 Ability = 0; Ability < 3; ++Ability)
		{
			Out.HeroCooldownSeconds.Add(Rule.HeroCooldownSeconds[Ability]);
		}
		for (const FHOLBuildable& Buildable : Rule.Buildable)
		{
			Out.BuildableTypes.Add(Buildable.TypeName);
			Out.BuildableCells.Add(Buildable.Cell);
			Out.BuildableAges.Add(Buildable.Age);
			Out.BuildableAlwaysOpen.Add(Buildable.bAlwaysOpen);
		}
		return Out;
	}
}

UHorizontalBox* UHOLScriptScreen::GetAgePanelsHost() const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	return GameScreen ? GameScreen->GetAgePanelsHost() : nullptr;
}

UVerticalBox* UHOLScriptScreen::GetGridPanelHost() const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	return GameScreen ? GameScreen->GetGridPanelHost() : nullptr;
}

void UHOLScriptScreen::GetGridFrame(TArray<FHOLGridCellInfoView>& OutCells, bool& bOutGrid, bool& bOutOrders) const
{
	OutCells.Reset();
	bOutGrid = false;
	bOutOrders = false;
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	if (!GameScreen)
	{
		return;
	}
	TArray<int32> Kinds;
	TArray<int32> States;
	TArray<FName> OrderTypes;
	GameScreen->ReadGridFrame(Kinds, States, OrderTypes, bOutGrid, bOutOrders);
	for (int32 Cell = 0; Cell < Kinds.Num(); ++Cell)
	{
		FHOLGridCellInfoView View;
		View.Kind = Kinds[Cell];
		View.State = States[Cell];
		View.OrderType = OrderTypes[Cell];
		OutCells.Add(View);
	}
}

bool UHOLScriptScreen::ApplyGridCell(const int32 Cell, UHOLGridCell* Target)
{
	UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	return GameScreen && GameScreen->ApplyGridCell(Cell, Target);
}

void UHOLScriptScreen::ClickGridCell(const int32 Cell)
{
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->ClickGridCell(Cell);
	}
}

void UHOLScriptScreen::RegisterGridCells(const TArray<UHOLGridCell*>& Cells)
{
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->SetGridCells(Cells);
	}
}

void UHOLScriptScreen::GetAgesContext(FHOLGridContextView& OutContext) const
{
	// Exactly RefreshAges' reading: the units' context, unless an own building or a building group is selected (then an empty context).
	OutContext = FHOLGridContextView();
	const AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer());
	if (PlayerController && !PlayerController->GetOwnSelection() && !PlayerController->HasBuildingGroup())
	{
		FHOLGridContext Context;
		PlayerController->GetUnitsContext(Context);
		OutContext = HOLScriptScreenAgesPrivate::ToView(Context);
	}
}

int32 UHOLScriptScreen::GetChosenAge() const
{
	const AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer());
	return PlayerController ? PlayerController->GetChosenAge() : INDEX_NONE;
}

int32 UHOLScriptScreen::GetTier() const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	return GameScreen ? GameScreen->GetBoundTier() : INDEX_NONE;
}

void UHOLScriptScreen::ClickAge(const int32 Age, const int32 Cell)
{
	if (AHOLPlayerController* PlayerController = Cast<AHOLPlayerController>(GetOwningPlayer()))
	{
		PlayerController->ClickAge(Age, Cell);
	}
}

bool UHOLScriptScreen::FillAgeCell(UHOLGridCell* Cell, const FHOLGridCommandView& Building)
{
	UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	if (!GameScreen || !Cell || Building.TypeName.IsNone())
	{
		return false;
	}
	GameScreen->FillAgeCell(Cell, Building.TypeName, Building.bGreyed);
	return true;
}

void UHOLScriptScreen::SetAgesReport(const bool bShown, const int32 Chosen, const TArray<FName>& Types, const TArray<bool>& Reached, const TArray<FString>& Tooltips)
{
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->SetAgesMirror(bShown, Chosen, Types, Reached, Tooltips);
	}
}

void UHOLScriptScreen::SetRoundedBrush(UBorder* Border, const FLinearColor Fill, const float Radius, const FLinearColor Outline, const float OutlineWidth) const
{
	if (Border)
	{
		Border->SetBrush(FSlateRoundedBoxBrush(Fill, Radius, Outline, OutlineWidth));
	}
}

FLinearColor UHOLScriptScreen::GetStyleColor(const int32 Which)
{
	switch (Which)
	{
	case 0: return HOLUIStyle::Accent;
	case 1: return HOLUIStyle::Text;
	case 2: return HOLUIStyle::TextMuted;
	case 3: return HOLUIStyle::HUDPanel;
	default: return HOLUIStyle::TextMuted;
	}
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

void UHOLScriptScreen::SetCheatsReport(const FString& Line)
{
	CheatsReport = Line;
}

void UHOLScriptScreen::ApplyCheatsOpen_Implementation(const bool bOpen)
{
}

void UHOLScriptScreen::SetQueueReport(const FString& Line)
{
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->SetQueueReportMirror(Line);
	}
}

void UHOLScriptScreen::SetSelectionReport(const FString& Kind, const FString& Activity, const FString& Health, const FString& Forum)
{
	SelectionReportKind = Kind;
	SelectionReportActivity = Activity;
	SelectionReportHealth = Health;
	SelectionReportForum = Forum;
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

UHorizontalBox* UHOLScriptScreen::GetSelectionHost() const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	return GameScreen ? GameScreen->GetSelectionHost() : nullptr;
}

void UHOLScriptScreen::SetStatsReport(const FString& Stats, const TArray<FString>& TipList)
{
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->SetStatsReportMirror(Stats, TipList);
	}
}

void UHOLScriptScreen::SetSelectionLive(const bool bLive)
{
	bSelectionLive = bLive;
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

bool UHOLScriptScreen::GetMinorEventsNow(int32& OutBuildingsFinished, int32& OutResearchesDone, FVector2D& OutLastBuildingLocation, FVector2D& OutLastResearchBuildingLocation) const
{
	// The shown player's finished buildings (their state replicates to every client: a build that ends here, or one that arrives finished) and the
	// technologies researched (replicated to the owner, OnRep_Technologies); the accumulation is ObserveMinorEvents'.
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	const AHOLPlayerState* PlayerState = GameScreen ? GameScreen->GetBoundPlayerState() : nullptr;
	const UWorld* World = GetWorld();
	if (PlayerState && World)
	{
		TArray<AHOLBuilding*> Finished;
		for (FHOLBuildingIterator It(World); It; ++It)
		{
			if (It->IsFinished() && It->GetOwnerPlayer() == PlayerState->GetPlayerIndex())
			{
				Finished.Add(*It);
			}
		}
		ObserveMinorEvents(MinorWatch, Finished, PlayerState->GetTechnologies().Done.Num());
	}
	OutBuildingsFinished = MinorWatch.BuildingsFinished;
	OutResearchesDone = MinorWatch.ResearchesDone;
	OutLastBuildingLocation = MinorWatch.LastBuildingLocation;
	OutLastResearchBuildingLocation = MinorWatch.LastResearchBuildingLocation;
	return PlayerState != nullptr && World != nullptr;
}

void UHOLScriptScreen::ObserveMinorEvents(FHOLMinorEventsWatch& Watch, const TArray<AHOLBuilding*>& FinishedBuildings, const int32 ResearchesDone)
{
	for (const AHOLBuilding* Building : FinishedBuildings)
	{
		if (!Building || Watch.SeenFinished.Contains(Building))
		{
			continue;
		}
		Watch.SeenFinished.Add(Building);
		// The baseline (the first observation) takes what is there without counting it.
		if (Watch.bPrimed)
		{
			++Watch.BuildingsFinished;
			Watch.LastBuildingLocation = FVector2D(Building->GetActorLocation().X, Building->GetActorLocation().Y);
		}
	}
	if (Watch.bPrimed && ResearchesDone > Watch.ResearchesSeen)
	{
		Watch.ResearchesDone += ResearchesDone - Watch.ResearchesSeen;
	}
	Watch.ResearchesSeen = ResearchesDone;
	Watch.bPrimed = true;
}

bool UHOLScriptScreen::GetHitsReceivedNow(int32& OutHits, FVector2D& OutLastHitLocation, double& OutLastHitSeconds) const
{
	// The shown player's buildings (every machine keeps each one's last hit time, set where DamageTaken grows, server or replicated); the
	// accumulation is ObserveHitsReceived's. A unit's hits are not read yet (no record of them on a client).
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	const AHOLPlayerState* PlayerState = GameScreen ? GameScreen->GetBoundPlayerState() : nullptr;
	const UWorld* World = GetWorld();
	if (PlayerState && World)
	{
		TArray<FVector2D> Places;
		TArray<double> Seconds;
		for (FHOLBuildingIterator It(World); It; ++It)
		{
			if (It->GetOwnerPlayer() == PlayerState->GetPlayerIndex())
			{
				Places.Add(FVector2D(It->GetActorLocation().X, It->GetActorLocation().Y));
				Seconds.Add(It->GetLastHitTime());
			}
		}
		ObserveHitsReceived(HitsWatch, Places, Seconds);
	}
	OutHits = HitsWatch.HitsReceived;
	OutLastHitLocation = HitsWatch.LastHitLocation;
	OutLastHitSeconds = HitsWatch.LastHitSeconds;
	return PlayerState != nullptr && World != nullptr;
}

void UHOLScriptScreen::ObserveHitsReceived(FHOLHitsWatch& Watch, const TArray<FVector2D>& HitPlaces, const TArray<double>& HitSeconds)
{
	const int32 Count = FMath::Min(HitPlaces.Num(), HitSeconds.Num());
	// The baseline (the first observation) takes the newest time there without counting it.
	const double Threshold = Watch.LastHitSeconds;
	double Newest = Threshold;
	FVector2D NewestPlace = Watch.LastHitLocation;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		if (HitSeconds[Index] < 0.0 || HitSeconds[Index] <= Threshold)
		{
			continue;
		}
		if (Watch.bPrimed)
		{
			++Watch.HitsReceived;
		}
		if (HitSeconds[Index] >= Newest)
		{
			Newest = HitSeconds[Index];
			NewestPlace = HitPlaces[Index];
		}
	}
	Watch.LastHitSeconds = Newest;
	if (Watch.bPrimed)
	{
		Watch.LastHitLocation = NewestPlace;
	}
	Watch.bPrimed = true;
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

void UHOLScriptScreen::AddThreadLine(const FText& Line, const double Seconds)
{
	if (UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>())
	{
		GameScreen->AddThreadLine(Line, Seconds);
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

bool UHOLScriptScreen::GetScenarioDisplay(FString& OutName, int32& OutState, FString& OutError, TArray<FString>& OutNotes, FString& OutWaitingForKey, bool& bOutClosed, bool& bOutHidden) const
{
	const HOLTestHooks::FScenarioDisplay& Display = HOLTestHooks::GetScenarioDisplay();
	OutName = Display.Name;
	OutState = static_cast<int32>(Display.State);
	OutError = Display.Error;
	OutNotes = Display.Notes;
	OutWaitingForKey = Display.WaitingForKey;
	bOutClosed = Display.bClosed;
	bOutHidden = Display.bHidden;
	return !Display.Name.IsEmpty();
}

void UHOLScriptScreen::CloseScenarioPanel()
{
	HOLTestHooks::GetScenarioDisplay().bClosed = true;
}

float UHOLScriptScreen::MeasureHeadingWidth(const FText& A, const FText& B) const
{
	// The scenario panel's heading measure (HOLScenarioPanelScript.as, state's minimum width): 0 with no renderer (-nullrhi, automation).
	if (!FSlateApplication::IsInitialized())
	{
		return 0.f;
	}
	const auto Renderer = FSlateApplication::Get().GetRenderer();
	if (!Renderer)
	{
		return 0.f;
	}
	FSlateFontInfo Font;
	GetDefault<UHOLTextStyleHUDHeading>()->GetFont(Font);
	const TSharedRef<FSlateFontMeasure> Measure = Renderer->GetFontMeasureService();
	return FMath::Max(Measure->Measure(A, Font).X, Measure->Measure(B, Font).X);
}

void UHOLScriptScreen::ExecConsole(const FString& Command)
{
	if (GEngine)
	{
		GEngine->Exec(GetWorld(), *Command);
	}
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
	// H-11a: the cheats' panel is the script's (UHOLCheatPanelScript, its Border "CheatPanel"): its box keeps the name "cheats" and its first place in the list.
	const UBorder* CheatBorder = WidgetTree ? Cast<UBorder>(WidgetTree->FindWidget(FName(TEXT("CheatPanel")))) : nullptr;
	if (CheatBorder && CheatBorder->GetVisibility() != ESlateVisibility::Collapsed && !CheatBorder->GetCachedGeometry().GetAbsoluteSize().IsNearlyZero())
	{
		OutBoxes.Add({ TEXT("cheats"), HOLHUDSpace::BoxOf(this, CheatBorder->GetCachedGeometry()) });
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
			if (!Border || !Border->IsVisible() || Border == CheatBorder)
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
			// L1-12b: the test scenario's own panel (top centre; a development build only) keeps its box name "scenario" (H-11b: the script's
			// ScenarioPanel border; Tools\Scenarios\HudRects.ps1 and the "HUD overlap:" lines name it so).
			const FString BoxName = Border->GetFName() == TEXT("ScenarioPanel") ? FString(TEXT("scenario")) : Border->GetName();
			OutBoxes.Add({ BoxName, HOLHUDSpace::BoxOf(this, Geometry) });
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

namespace
{
	/** The pieces of Message as the C++ FillMessageRow cuts them (the shown player's reading, the lobby's names when the game state is there), a colour per piece. */
	void FillMessagePieces(const FHOLForumMessage& Message, const UHOLGameScreen* GameScreen, const UWorld* World, TArray<FHOLForumPieceValue>& OutPieces, TArray<FLinearColor>& OutColors)
	{
		const AHOLGameState* GameState = World->GetGameState<AHOLGameState>();
		const AHOLPlayerState* PlayerState = GameScreen->GetBoundPlayerState();
		const int32 ViewerSlot = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
		for (const HOLForumMessages::FPiece& Part : GameState ? HOLForumMessages::SplitNames(Message, ViewerSlot, GameState->GetLaunchConfig())
			: HOLForumMessages::SplitNames(Message, ViewerSlot))
		{
			FHOLForumPieceValue Piece;
			Piece.Text = Part.Text;
			Piece.Slot = Part.Slot;
			OutPieces.Add(Piece);
			const bool bName = Part.Slot != INDEX_NONE && GameState && GameState->GetLaunchConfig().Slots.IsValidIndex(Part.Slot);
			OutColors.Add(bName ? FLinearColor(HOLPlayerColors::GetColor(GameState->GetLaunchConfig().Slots[Part.Slot].ColorIndex)) : FLinearColor::White);
		}
	}
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
	FillMessagePieces(Alert.Message, GameScreen, World, OutPieces, OutColors);

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

UVerticalBox* UHOLScriptScreen::GetMessagesList() const
{
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	return GameScreen ? GameScreen->GetMessagesList() : nullptr;
}

int32 UHOLScriptScreen::GetMessageState(int32& OutRevision, TArray<bool>& OutAlerts, TArray<int32>& OutSlots, TArray<float>& OutOpacities) const
{
	OutRevision = 0;
	OutAlerts.Reset();
	OutSlots.Reset();
	OutOpacities.Reset();
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	const UWorld* World = GetWorld();
	if (!GameScreen || !World)
	{
		return 0;
	}
	OutRevision = GameScreen->GetMessageRevision();
	const double Now = World->GetRealTimeSeconds();
	for (const FHOLForumThreadLine& Line : GameScreen->GetMessageThread().Lines)
	{
		OutAlerts.Add(Line.Message.bAlert);
		OutSlots.Add(Line.Slot);
		OutOpacities.Add(FHOLForumThread::GetOpacity(Line, Now));
	}
	return OutAlerts.Num();
}

bool UHOLScriptScreen::GetMessageLine(const int32 Index, TArray<FHOLForumPieceValue>& OutPieces, TArray<FLinearColor>& OutColors) const
{
	OutPieces.Reset();
	OutColors.Reset();
	const UHOLGameScreen* GameScreen = GetTypedOuter<UHOLGameScreen>();
	const UWorld* World = GetWorld();
	if (!GameScreen || !World || !GameScreen->GetMessageThread().Lines.IsValidIndex(Index))
	{
		return false;
	}
	FillMessagePieces(GameScreen->GetMessageThread().Lines[Index].Message, GameScreen, World, OutPieces, OutColors);
	return true;
}

void UHOLScriptScreen::BindMessageClick(UBorder* Panel)
{
	if (Panel)
	{
		Panel->OnMouseButtonDownEvent.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHOLScriptScreen, HandleMessageMouseDown));
	}
}

FEventReply UHOLScriptScreen::HandleMessageMouseDown(FGeometry Geometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FEventReply(false);
	}
	// The script's ClickWidget(MessageFeed): true only while a line carrying an alert is under the mouse, as the C++ handler's own loop was.
	return FEventReply(ClickWidget(FName(TEXT("MessageFeed"))));
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
	// The thread of messages' fades and end (HOLMessageFeed.as): called every frame while the C++ thread holds a line, and once more when it empties.
	if (const UHOLGameScreen* FeedScreen = GetTypedOuter<UHOLGameScreen>())
	{
		const bool bFeedHasLines = !FeedScreen->GetMessageThread().Lines.IsEmpty();
		if (bFeedHasLines || bFeedLive)
		{
			MessageFrameTick(InDeltaTime);
		}
		bFeedLive = bFeedHasLines;
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
	// HUD H-9: a source's remaining amount, one unit's health, activity and stats: every image, ONLY while the script's selection block says so.
	if (bSelectionLive)
	{
		SelectionFrameTick();
	}
	// The T key (the talent tree, TQ4 = A): the viewport's key state, as the Enter of a test scenario; the console open or a focused menu keeps it.
	if (const APlayerController* Controller = GetOwningPlayer())
	{
		if (Controller->IsLocalController() && Controller->WasInputKeyJustPressed(EKeys::T))
		{
			KeyPressed(TEXT("T"));
		}
		// Enter (D807, the chat's input screen): the same key state, for the CLOSED field (nothing has Slate focus then, so no field gets that Enter); an open field
		// has the keyboard focus, Slate gives it the Enter (OnTextCommitted) and the player input sees no key press: no double, as for the T key.
		if (Controller->IsLocalController() && Controller->WasInputKeyJustPressed(EKeys::Enter))
		{
			KeyPressed(TEXT("Enter"));
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

FReply UHOLScriptScreen::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// The Tab of the chat's field (RECETTE-1120sa: Tab Tab never gave "(Prive) <name>"): a text box with the keyboard focus gets the Tab through Slate's focus
	// navigation, which neither OnTextCommitted(OnUserMovedFocus) nor the tick's WasInputKeyJustPressed ever reports. The preview key goes down the focus path
	// from this widget BEFORE the text box and its navigation: the script gets the Tab here, once per press (no auto-repeat), and the Tab is consumed.
	if (bRelayTabToScript && InKeyEvent.GetKey() == EKeys::Tab && !InKeyEvent.IsRepeat() && HasFocusedDescendants())
	{
		KeyPressed(TEXT("Tab"));
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}
