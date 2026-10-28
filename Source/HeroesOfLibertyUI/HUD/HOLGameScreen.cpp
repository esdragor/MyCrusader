#include "HUD/HOLGameScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLGameNames.h"
#include "HUD/HOLRelationIcons.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLGameMenuRules.h"
#include "Player/HOLPlayerController.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"
#include "Sound/SoundBase.h"
#include "Screens/HOLGameMenu.h"
#include "Screens/HOLStatsScreen.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLGridCell.h"
#include "Widgets/HOLQueueItem.h"
#include "Widgets/HOLRootLayout.h"

#define LOCTEXT_NAMESPACE "HOLGameScreen"

namespace UE::HeroesOfLiberty::UI::GameScreen
{
	/** At its source or on its round trip (D307): the HUD says « Récolte » for both. */
	bool IsGatheringActivity(const EHOLUnitActivity Activity)
	{
		return Activity == EHOLUnitActivity::Gathering || Activity == EHOLUnitActivity::GatherTrip;
	}

	/** A resource's name, as the top bar writes it. */
	FText GetResourceName(const EHOLResource Resource)
	{
		switch (Resource)
		{
		case EHOLResource::Wood: return LOCTEXT("Wood", "Bois");
		case EHOLResource::Stone: return LOCTEXT("Stone", "Pierre");
		case EHOLResource::Gold: return LOCTEXT("Gold", "Or");
		default: return LOCTEXT("Food", "Nourriture");
		}
	}

	/** A resource's name inside a sentence (« Dépôt : pierre, or »). */
	FText GetResourceNameInText(const EHOLResource Resource)
	{
		switch (Resource)
		{
		case EHOLResource::Wood: return LOCTEXT("WoodInText", "bois");
		case EHOLResource::Stone: return LOCTEXT("StoneInText", "pierre");
		case EHOLResource::Gold: return LOCTEXT("GoldInText", "or");
		default: return LOCTEXT("FoodInText", "nourriture");
		}
	}

	/** What the stock lacks of Cost, resource by resource (0 where it covers it): shown in red (D313). */
	FHOLResources GetMissing(const FHOLResources& Stock, const FHOLResources& Cost)
	{
		return FHOLResources(FMath::Max(0, Cost.Food - Stock.Food), FMath::Max(0, Cost.Wood - Stock.Wood), FMath::Max(0, Cost.Stone - Stock.Stone),
			FMath::Max(0, Cost.Gold - Stock.Gold));
	}

	/** What a building gives, for its tooltip (D313): « +10 population », « Dépôt : pierre, or », « Produit : Lancier, Homme d'armes ». */
	FText GetBuildingEffect(const FHOLBuildingTypeRow* Type)
	{
		TArray<FText> Lines;
		if (Type && Type->PopulationProvided > 0)
		{
			Lines.Add(FText::Format(LOCTEXT("PopulationEffect", "+{0} population"), Type->PopulationProvided));
		}
		if (Type && !Type->Accepts.IsEmpty())
		{
			TArray<FText> Names;
			for (const EHOLResource Resource : Type->Accepts)
			{
				Names.Add(GetResourceNameInText(Resource));
			}
			Lines.Add(FText::Format(LOCTEXT("DepositEffect", "Dépôt : {0}"), FText::Join(LOCTEXT("ListSeparator", ", "), Names)));
		}
		if (Type && !Type->Produces.IsEmpty())
		{
			TArray<FText> Names;
			for (const FName Unit : Type->Produces)
			{
				Names.Add(HOLGameNames::GetUnitName(Unit, nullptr));
			}
			Lines.Add(FText::Format(LOCTEXT("ProducesEffect", "Produit : {0}"), FText::Join(LOCTEXT("ListSeparator", ", "), Names)));
		}
		return FText::Join(FText::AsCultureInvariant(TEXT("\n")), Lines);
	}

	/** What a unit does, for its tooltip. */
	FText GetUnitEffect(const FName TypeName)
	{
		static const FName Peasant(TEXT("Peasant"));
		return TypeName == Peasant ? LOCTEXT("PeasantEffect", "Récolte et bâtit") : FText::GetEmpty();
	}

	/** The key of a command's message, for the logs and Describe: "NotEnoughWood", "PopulationFull", "PlacementBuilding"... */
	FString GetFeedbackKey(const FHOLCommandFeedback& Feedback)
	{
		switch (Feedback.Kind)
		{
		case EHOLCommandFeedback::NotEnoughResources:
			return TEXT("NotEnough") + StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Feedback.Resource));
		case EHOLCommandFeedback::QueueFull: return TEXT("QueueFull");
		case EHOLCommandFeedback::PopulationFull: return TEXT("PopulationFull");
		case EHOLCommandFeedback::NoBuilding:
			return TEXT("No") + StaticEnum<EHOLBuildingCategory>()->GetNameStringByValue(static_cast<int64>(Feedback.Category)) + TEXT("Building");
		case EHOLCommandFeedback::PlacementRefused:
			switch (Feedback.Refusal)
			{
			case EHOLPlacementRefusal::OutOfMap: return TEXT("PlacementOutOfMap");
			case EHOLPlacementRefusal::Building: return TEXT("PlacementBuilding");
			case EHOLPlacementRefusal::Source: return TEXT("PlacementSource");
			case EHOLPlacementRefusal::EnemyCentre: return TEXT("PlacementEnemyCentre");
			case EHOLPlacementRefusal::NavMesh: return TEXT("PlacementNavMesh");
			default: return TEXT("Placement");
			}
		default: return FString();
		}
	}

	/** The short message of a command that did not go through (D313). */
	FText GetFeedbackText(const FHOLCommandFeedback& Feedback)
	{
		switch (Feedback.Kind)
		{
		case EHOLCommandFeedback::NotEnoughResources:
			switch (Feedback.Resource)
			{
			case EHOLResource::Wood: return LOCTEXT("NotEnoughWood", "Pas assez de bois");
			case EHOLResource::Stone: return LOCTEXT("NotEnoughStone", "Pas assez de pierre");
			case EHOLResource::Gold: return LOCTEXT("NotEnoughGold", "Pas assez d'or");
			default: return LOCTEXT("NotEnoughFood", "Pas assez de nourriture");
			}
		case EHOLCommandFeedback::QueueFull: return LOCTEXT("QueueFull", "File de production pleine");
		case EHOLCommandFeedback::PopulationFull: return LOCTEXT("PopulationMax", "Population maximale — construisez des maisons");
		case EHOLCommandFeedback::NoBuilding:
			switch (Feedback.Category)
			{
			case EHOLBuildingCategory::Military: return LOCTEXT("NoMilitaryBuilding", "Aucun bâtiment militaire");
			case EHOLBuildingCategory::Technological: return LOCTEXT("NoTechnologicalBuilding", "Aucun bâtiment technologique");
			default: return LOCTEXT("NoEconomicBuilding", "Aucun bâtiment économique");
			}
		case EHOLCommandFeedback::PlacementRefused:
			switch (Feedback.Refusal)
			{
			case EHOLPlacementRefusal::OutOfMap: return LOCTEXT("PlacementOutOfMap", "Hors de la carte");
			case EHOLPlacementRefusal::EnemyCentre: return LOCTEXT("PlacementEnemyCentre", "Trop près d'un centre ennemi");
			case EHOLPlacementRefusal::NavMesh: return LOCTEXT("PlacementNavMesh", "Terrain impraticable");
			default: return LOCTEXT("PlacementBlocked", "Emplacement occupé");
			}
		default: return FText::GetEmpty();
		}
	}
}

namespace UE::HeroesOfLiberty::UI::GameScreen
{
	/** Keys of the grid's cells, row by row, as they sit on an AZERTY keyboard (D30; IMC_Game, make_game_input.py). */
	const TCHAR* const GridKeys[AHOLPlayerController::NumGridCells] = {
		TEXT("A"), TEXT("Z"), TEXT("E"), TEXT("R"),
		TEXT("Q"), TEXT("S"), TEXT("D"), TEXT("F"),
		TEXT("W"), TEXT("X"), TEXT("C"), TEXT("V") };
	constexpr int32 GridColumns = 4;
	constexpr float CellSize = 76.f;
	constexpr float CellGap = 2.f;
	constexpr float QueueItemWidth = 56.f;
	constexpr float QueueItemHeight = 36.f;
	constexpr float ScreenMargin = 16.f;
	constexpr float MarkerSize = 12.f;
	constexpr float SwatchSize = 14.f;
	constexpr float IdleWorkersWidth = 130.f;
	/** The site's progress bar in the selection block (J6a.5). */
	constexpr float SiteBarWidth = 200.f;
	/** The grid panel's width: the short message above it wraps within it (D313). */
	constexpr float GridWidth = 4 * (CellSize + 2 * CellGap) + 12.f;
	/**
	 * The bottom bar (the user's 2nd trial of G.1, AoE4's way, D201), left to right at fixed places whatever the selection:
	 * the selection's block (a fixed width), the command panel (3 x 4), the builders' age panels; its height the panel's.
	 */
	constexpr float SelectionWidth = 340.f;
	constexpr float BarHeight = 3 * (CellSize + 2 * CellGap) + 12.f;
	constexpr float BarGap = 6.f;
	/** The minimap's place, bottom right (D201): no minimap yet, the place kept free. */
	constexpr float MinimapSize = 240.f;
	/** An age's medallion (AoE4's « (I) A »): its numeral's disc, its key's square, how far it rises over the panel's top edge. */
	constexpr float MedallionSize = 40.f;
	constexpr float KeyBadgeSize = 24.f;
	constexpr float MedallionRise = 26.f;
	/** The selection's chips (G.1e): how many (« Tous » and 11 types), their height, at most two on a line, a long name cut. */
	constexpr int32 NumChips = 12;
	constexpr float ChipHeight = 30.f;
	constexpr float ChipGap = 4.f;
	constexpr float ChipMaxWidth = (SelectionWidth - 24.f - ChipGap) / 2.f;
	constexpr int32 ChipNameChars = 12;
	/** A medallion's look: its age not reached (faded), reached, chosen (lit). */
	enum EMedallionLook : int32 { Locked, Reached, Chosen };
	/** The name the HUD's report gives the « Annuler » cell of a site. */
	const FName CancelName(TEXT("Cancel"));
	/** Seconds between two readings of a selected group's activities. */
	constexpr float GroupRefreshInterval = 0.25f;
	const FName IdleWorkersName(TEXT("IdleWorkers"));
	const FName MenuButtonName(TEXT("MenuButton"));
	const FName StatsButtonName(TEXT("StatsButton"));
	const FName AlertPanelName(TEXT("AlertPanel"));
	/** A control group's slot (J7a.5): its width, and how long it stays lit once its group is selected (s, real time). */
	constexpr float GroupSlotWidth = 64.f;
	constexpr double GroupLightSeconds = 0.5;

	FName GridCellName(const int32 Cell)
	{
		return *FString::Printf(TEXT("GridCell%02d"), Cell);
	}

	/** The ages' numerals (I to IV) and names (GDD § 11.1). */
	const TCHAR* const AgeNumerals[HOLCommandGrid::NumAges] = { TEXT("I"), TEXT("II"), TEXT("III"), TEXT("IV") };

	/** A chip's label (G.1e): « Paysan (5) », a long name cut short (« Homme d'arme. (3) »); Count 0: the name alone. */
	FText MakeChipLabel(const FText& Name, const int32 Count)
	{
		const FString Full = Name.ToString();
		const FText Shown = Full.Len() <= ChipNameChars ? Name : FText::AsCultureInvariant(Full.Left(ChipNameChars - 1).TrimEnd() + TEXT("."));
		return Count > 0 ? FText::Format(LOCTEXT("SelectionChip", "{0} ({1})"), Shown, Count) : Shown;
	}

	FText GetAgeName(const int32 Age)
	{
		switch (Age)
		{
		case 0: return LOCTEXT("Age0", "Fondation");
		case 1: return LOCTEXT("Age1", "Essor");
		case 2: return LOCTEXT("Age2", "Puissance");
		default: return LOCTEXT("Age3", "Légende");
		}
	}

	/** The building cells of an age's panel: the grid's 12 keys, one row per category (HOLCommandGrid::GetCategoryRow). */
	constexpr int32 NumAgeCells = HOLCommandGrid::NumCells;

	FName AgeMedallionName(const int32 Age)
	{
		return *FString::Printf(TEXT("AgeMedallion%d"), Age);
	}

	/** AgeCell<age><cell>: AgeCell000 the house (age 0, A), AgeCell004 the barracks (Q). */
	FName AgeCellName(const int32 Age, const int32 Cell)
	{
		return *FString::Printf(TEXT("AgeCell%d%02d"), Age, Cell);
	}

	FName QueueItemName(const int32 Index)
	{
		return *FString::Printf(TEXT("QueueItem%d"), Index);
	}

	/** Shows or collapses a widget, touching it only when that changes (the queue's head is updated every frame). */
	void SetShown(UWidget* Widget, const bool bShown, const ESlateVisibility ShownVisibility = ESlateVisibility::HitTestInvisible)
	{
		const ESlateVisibility Wanted = bShown ? ShownVisibility : ESlateVisibility::Collapsed;
		if (Widget && Widget->GetVisibility() != Wanted)
		{
			Widget->SetVisibility(Wanted);
		}
	}

	/** Amounts without thousands separators, as in AoE ("1200"). */
	FText FormatAmount(const int32 Value)
	{
		return FText::AsNumber(Value, &FNumberFormattingOptions::DefaultNoGrouping());
	}

	FText FormatRatio(const int32 Value, const int32 Max)
	{
		return FText::Format(INVTEXT("{0} / {1}"), FormatAmount(Value), FormatAmount(Max));
	}
}

UHOLGameScreen::UHOLGameScreen()
{
	bAutoActivate = true;
	// Escape belongs to the game (IA_Deselect): no Back here.
	bIsBackHandler = false;
	// Never the keyboard focus: activated without a focus target, the screen leaves it to the game viewport.
	SetIsFocusable(false);
	// A click on a panel bubbles up to the screen and stops there; the rest of the screen is hit-test invisible, so a
	// click there never reaches the screen and goes to the game.
	SetConsumePointerInput(true);
}

TOptional<FUIInputConfig> UHOLGameScreen::GetDesiredInputConfig() const
{
	// The game keeps its keys and its mouse (camera, selection, grid, Escape); the viewport holds the mouse while a
	// button is down, as the controller's input mode does (camera drag).
	return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::CaptureDuringMouseDown, /*bHideCursorDuringViewportCapture*/ false);
}

void UHOLGameScreen::NativeOnInitialized()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	if (!WidgetTree->RootWidget)
	{
		BuildLayout();
	}
	Super::NativeOnInitialized();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	for (int32 Cell = 0; Cell < AHOLPlayerController::NumGridCells; ++Cell)
	{
		UHOLGridCell* GridCell = Cast<UHOLGridCell>(WidgetTree->FindWidget(GridCellName(Cell)));
		GridCells.Add(GridCell);
		if (GridCell)
		{
			GridCell->SetKey(FText::AsCultureInvariant(FString(GridKeys[Cell])));
			GridCell->OnClicked().AddUObject(this, &UHOLGameScreen::HandleGridCellClicked, Cell);
		}
	}
	for (int32 Index = 0; Index < NumQueueItems; ++Index)
	{
		UHOLQueueItem* Item = Cast<UHOLQueueItem>(WidgetTree->FindWidget(QueueItemName(Index)));
		QueueItems.Add(Item);
		if (Item)
		{
			Item->OnClicked().AddUObject(this, &UHOLGameScreen::HandleQueueItemClicked, Index);
		}
	}
	if (IdleWorkers)
	{
		IdleWorkers->OnClicked().AddUObject(this, &UHOLGameScreen::HandleIdleWorkersClicked);
	}
	MenuButton = Cast<UHOLQueueItem>(WidgetTree->FindWidget(MenuButtonName));
	if (MenuButton)
	{
		MenuButton->SetLabel(LOCTEXT("Menu", "Menu"));
		MenuButton->OnClicked().AddUObject(this, &UHOLGameScreen::HandleMenuClicked);
	}
	StatsButton = Cast<UHOLQueueItem>(WidgetTree->FindWidget(StatsButtonName));
	if (StatsButton)
	{
		StatsButton->SetLabel(LOCTEXT("Stats", "Statistiques"));
		StatsButton->OnClicked().AddUObject(this, &UHOLGameScreen::HandleStatsClicked);
		StatsButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	for (int32 Group = 0; Group < AHOLPlayerController::NumControlGroups; ++Group)
	{
		UHOLQueueItem* GroupSlot = Cast<UHOLQueueItem>(WidgetTree->FindWidget(GroupSlotName(Group)));
		GroupSlots.Add(GroupSlot);
		if (GroupSlot)
		{
			GroupSlot->OnClicked().AddUObject(this, &UHOLGameScreen::HandleGroupSlotClicked, Group);
		}
	}
	SelectionChips = Cast<UWrapBox>(WidgetTree->FindWidget(TEXT("SelectionChips")));
	for (int32 Chip = 0; Chip < NumChips; ++Chip)
	{
		UHOLQueueItem* Item = Cast<UHOLQueueItem>(WidgetTree->FindWidget(*FString::Printf(TEXT("SelectionChip%d"), Chip)));
		ChipItems.Add(Item);
		ChipFrames.Add(Cast<UBorder>(WidgetTree->FindWidget(*FString::Printf(TEXT("SelectionChipFrame%d"), Chip))));
		if (Item)
		{
			Item->OnClicked().AddUObject(this, &UHOLGameScreen::HandleChipClicked, Chip);
		}
	}
	ShownGroups.Init(FString(), AHOLPlayerController::NumControlGroups);
	ShownGridTypes.Init(NAME_None, AHOLPlayerController::NumGridCells);
	ShownOrderTypes.Init(NAME_None, AHOLPlayerController::NumGridCells);
	for (int32 Age = 0; Age < HOLCommandGrid::NumAges; ++Age)
	{
		AgePanels.Add(Cast<UBorder>(WidgetTree->FindWidget(*FString::Printf(TEXT("AgePanel%d"), Age))));
		AgeMedallions.Add(Cast<UBorder>(WidgetTree->FindWidget(AgeMedallionName(Age))));
		for (int32 Index = 0; Index < NumAgeCells; ++Index)
		{
			const int32 Cell = Index;
			UHOLGridCell* AgeCell = Cast<UHOLGridCell>(WidgetTree->FindWidget(AgeCellName(Age, Cell)));
			AgeCells.Add(AgeCell);
			AgeCellBoxes.Add(AgeCell ? Cast<USizeBox>(AgeCell->GetParent()) : nullptr);
			if (AgeCell)
			{
				AgeCell->SetKey(FText::AsCultureInvariant(FString(GridKeys[Cell])));
				AgeCell->OnClicked().AddUObject(this, &UHOLGameScreen::HandleAgeClicked, Age, Cell);
			}
		}
	}
	ShownAgeTypes.Init(NAME_None, HOLCommandGrid::NumAges * NumAgeCells);
	ShownAgeReached.Init(false, HOLCommandGrid::NumAges);
	ShownAgeTooltips.Init(FString(), HOLCommandGrid::NumAges);
	ShownAgeLooks.Init(INDEX_NONE, HOLCommandGrid::NumAges);

	// For the scripted checks (hol.Test.Run): the captures of a match name the HUD, and the screen over it if any (C4c:
	// the statistics screen).
	const TWeakObjectPtr<UHOLGameScreen> WeakThis(this);
	HOLTestHooks::DescribeInterface() = [WeakThis]()
	{
		if (!WeakThis.IsValid())
		{
			return FString(TEXT("no interface"));
		}
		FString Description = FString::Printf(TEXT("hud=%s"), *WeakThis->GetClass()->GetName());
		const UHOLRootLayout* Layout = UHOLRootLayout::Get(WeakThis->GetOwningPlayer());
		if (const UCommonActivatableWidget* Menu = Layout ? Layout->GetActiveMenu() : nullptr)
		{
			Description += FString::Printf(TEXT(" menu=%s"), *Menu->GetClass()->GetName());
		}
		return Description;
	};
}

void UHOLGameScreen::NativeConstruct()
{
	Super::NativeConstruct();

	Controller = GetOwningPlayer<AHOLPlayerController>();
	AHOLPlayerController* PlayerController = Controller.Get();
	if (PlayerController)
	{
		SelectionHandle = PlayerController->OnSelectionChanged.AddUObject(this, &UHOLGameScreen::HandleSelectionChanged);
		FeedbackHandle = PlayerController->OnCommandFeedback.AddUObject(this, &UHOLGameScreen::HandleCommandFeedback);
	}
	else
	{
		UE_LOG(LogHOLUI, Warning, TEXT("Game screen: the owning player is not a match controller (AHOLPlayerController), nothing to show"));
	}
	BindPlayerState(PlayerController ? PlayerController->GetHOLPlayerState() : nullptr);
	BindBuilding(PlayerController ? PlayerController->GetSelection() : nullptr);
	BindTableState(GetWorld() ? GetWorld()->GetGameState<AHOLGameState>() : nullptr);
	Refresh();
	RefreshPlayersTable();
}

void UHOLGameScreen::NativeDestruct()
{
	if (AHOLPlayerController* PlayerController = Controller.Get())
	{
		PlayerController->OnSelectionChanged.Remove(SelectionHandle);
		PlayerController->OnCommandFeedback.Remove(FeedbackHandle);
	}
	SelectionHandle.Reset();
	FeedbackHandle.Reset();
	Controller.Reset();
	BindPlayerState(nullptr);
	BindBuilding(nullptr);
	BindTableState(nullptr);
	Super::NativeDestruct();
}

void UHOLGameScreen::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// The refresh paces and the message's age in the world's real time: Slate's in a game, the fixed step's in quick_check
	// (Docs/Archive/TestSpeedInProgress.md: the game's seconds go by faster than the clock's, the HUD keeps up with them).
	const UWorld* TickWorld = GetWorld();
	const float FrameSeconds = TickWorld ? TickWorld->DeltaRealTimeSeconds : InDeltaTime;

	TickMessages();
	TickAlert();
	// The player state reaches a client after the HUD is up.
	AHOLPlayerState* PlayerState = Controller.IsValid() ? Controller->GetHOLPlayerState() : nullptr;
	if (PlayerState != BoundPlayerState.Get())
	{
		BindPlayerState(PlayerState);
		Refresh();
	}
	// The players' table (V2.4): its game state reaches a client after the HUD is up; a rebuilding's percent moves without
	// telling the server's HUD, the launch configuration reaches a client late: read four times a second.
	SinceTableRefresh += FrameSeconds;
	if (SinceTableRefresh >= UE::HeroesOfLiberty::UI::GameScreen::GroupRefreshInterval)
	{
		BindTableState(GetWorld() ? GetWorld()->GetGameState<AHOLGameState>() : nullptr);
		RefreshPlayersTable();
		RefreshWaitBanner();
		RefreshAbsenceBanner();
		RefreshStatsButton();
		// V2.3c: a selected main forum on the ground, its ruin's seconds and its rebuilding's percent move with time alone
		// (the forum's state changes only at its phases): its block is read at the same pace.
		const AHOLBuilding* SelectedBuilding = BoundBuilding.Get();
		if (BoundForumState.IsValid() && SelectedBuilding && SelectedBuilding->IsForumDown())
		{
			Refresh();
		}
	}
	// A gatherer selected alone: its line follows it at every change (D303); a group's activities four times a second; a
	// source's amount at every change (J5b); a group of buildings (an F key) four times a second, its sites and queues
	// followed without their delegates. The other blocks follow their delegates.
	using namespace UE::HeroesOfLiberty::UI::GameScreen;
	const int32 NumSelectedUnits = Controller.IsValid() ? Controller->GetSelectedUnits().Num() : 0;
	SinceGroupRefresh += FrameSeconds;
	if (Controller.IsValid() && Controller->HasBuildingGroup())
	{
		if (SinceGroupRefresh >= GroupRefreshInterval)
		{
			SinceGroupRefresh = 0.f;
			Refresh();
		}
	}
	else if (NumSelectedUnits == 1)
	{
		FHOLUnitView Unit;
		if (GetWorld() && HOLUnitView::Find(*GetWorld(), Controller->GetSelectedUnits()[0], Unit))
		{
			// A soldier's line shows while it strikes alone (J7b.4), a gatherer's always; its health as it changes (J7b.5).
			RefreshUnitActivity(Unit.CarryCapacity > 0 || Unit.Activity == EHOLUnitActivity::Attacking ? &Unit : nullptr);
			RefreshUnitHealth(&Unit);
		}
	}
	else if (NumSelectedUnits > 1 && SinceGroupRefresh >= GroupRefreshInterval)
	{
		SinceGroupRefresh = 0.f;
		RefreshUnitSelection(Controller->GetSelectedUnits());
	}
	else if (Controller.IsValid() && Controller->GetSelectedSource() != INDEX_NONE)
	{
		RefreshSourceSelection(Controller->GetSelectedSource());
	}
	// The chosen age (a key, a placement, Escape) changes without a delegate: its header follows it (the 2nd check of G.1c).
	if (bAgesShown && Controller.IsValid() && Controller->GetChosenAge() != ShownChosenAge)
	{
		RefreshAges(BoundPlayerState.Get());
	}
	RefreshIdleWorkers();
	RefreshControlGroups(FrameSeconds);
	UpdateHead();
	CheckPopulationBlocked();
	UpdateMessage(FrameSeconds);
}

void UHOLGameScreen::BindPlayerState(AHOLPlayerState* PlayerState)
{
	if (BoundPlayerState.Get() == PlayerState)
	{
		return;
	}
	if (AHOLPlayerState* Previous = BoundPlayerState.Get())
	{
		Previous->OnEconomyChanged.Remove(EconomyHandle);
	}
	EconomyHandle.Reset();
	BoundPlayerState = PlayerState;
	if (PlayerState)
	{
		EconomyHandle = PlayerState->OnEconomyChanged.AddUObject(this, &UHOLGameScreen::Refresh);
	}
}

void UHOLGameScreen::BindBuilding(AHOLBuilding* Building)
{
	if (BoundBuilding.Get() == Building)
	{
		return;
	}
	if (AHOLBuilding* Previous = BoundBuilding.Get())
	{
		Previous->OnProductionChanged.Remove(ProductionHandle);
		Previous->OnConstructionChanged.Remove(ConstructionHandle);
	}
	if (AHOLGameState* GameState = BoundForumState.Get())
	{
		GameState->OnForumsChanged.Remove(ForumsHandle);
	}
	ProductionHandle.Reset();
	ConstructionHandle.Reset();
	ForumsHandle.Reset();
	BoundForumState.Reset();
	BoundBuilding = Building;
	if (Building)
	{
		ProductionHandle = Building->OnProductionChanged.AddUObject(this, &UHOLGameScreen::Refresh);
		// A foundation's state and work (J6a.3, D312): its name, its health.
		ConstructionHandle = Building->OnConstructionChanged.AddUObject(this, &UHOLGameScreen::Refresh);
		// A main forum's phase (V2.3b): its ruin counting down, its rebuilding rising, its lives, on a client too (replicated).
		AHOLGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AHOLGameState>() : nullptr;
		if (Building->IsPrimaryForum() && GameState)
		{
			BoundForumState = GameState;
			ForumsHandle = GameState->OnForumsChanged.AddUObject(this, &UHOLGameScreen::Refresh);
		}
	}
}

void UHOLGameScreen::BindTableState(AHOLGameState* GameState)
{
	if (BoundTableState.Get() == GameState)
	{
		return;
	}
	if (AHOLGameState* Previous = BoundTableState.Get())
	{
		Previous->OnForumsChanged.Remove(TableForumsHandle);
		Previous->OnVictoryChanged.Remove(TableVictoryHandle);
		Previous->OnForumEvent.Remove(TableForumEventHandle);
		Previous->OnHeroReadyMessage.Remove(TableHeroReadyHandle);
	}
	TableForumsHandle.Reset();
	TableVictoryHandle.Reset();
	TableForumEventHandle.Reset();
	TableHeroReadyHandle.Reset();
	BoundTableState = GameState;
	if (GameState)
	{
		// Every machine: the server at once, a client by its OnReps (D353: public).
		TableForumsHandle = GameState->OnForumsChanged.AddUObject(this, &UHOLGameScreen::RefreshPlayersTable);
		TableVictoryHandle = GameState->OnVictoryChanged.AddUObject(this, &UHOLGameScreen::RefreshPlayersTable);
		// V4.2: the events of the ruin, every machine (a reliable multicast).
		TableForumEventHandle = GameState->OnForumEvent.AddUObject(this, &UHOLGameScreen::HandleForumEvent);
		// J8a.4: the local player's hero ready to return (its owner alone, once per death).
		TableHeroReadyHandle = GameState->OnHeroReadyMessage.AddUObject(this, &UHOLGameScreen::HandleHeroReadyMessage);
	}
}

void UHOLGameScreen::HandleHeroReadyMessage(const FHOLForumMessage& Message)
{
	if (const UWorld* World = GetWorld())
	{
		MessageThread.Add(Message, World->GetRealTimeSeconds());
		RebuildMessages();
	}
}

void UHOLGameScreen::HandleForumEvent(const FHOLForumEvent& Event)
{
	const AHOLGameState* GameState = BoundTableState.Get();
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	const UWorld* World = GetWorld();
	if (!GameState || !World)
	{
		return;
	}
	// As the players' table: the local player's slot, INDEX_NONE for a spectator.
	const FHOLForumMessage Message = HOLForumMessages::GetMessage(Event, GameState->GetLaunchConfig(), PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE);
	MessageThread.Add(Message, World->GetRealTimeSeconds(), Event.Slot);
	RebuildMessages();
	// V4.3: an alert, the banner and its sound too.
	ShowAlert(Message, Event.Slot);
}

namespace UE::HeroesOfLiberty::UI::GameScreen
{
	/** V4.2, V4.3: Row filled with Message's pieces, the words in WordsStyle, each name in NamesStyle and its player's colour. */
	void FillMessageRow(UWidgetTree& Tree, UHorizontalBox& Row, const FHOLForumMessage& Message, const int32 ViewerSlot, const AHOLGameState* GameState,
		const TSubclassOf<UCommonTextStyle> WordsStyle, const TSubclassOf<UCommonTextStyle> NamesStyle)
	{
		Row.ClearChildren();
		for (const HOLForumMessages::FPiece& Part : HOLForumMessages::SplitNames(Message, ViewerSlot))
		{
			const bool bName = Part.Slot != INDEX_NONE;
			UCommonTextBlock* Text = Tree.ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
			Text->SetStyle(bName ? NamesStyle : WordsStyle);
			Text->SetText(FText::AsCultureInvariant(Part.Text));
			UHorizontalBoxSlot* TextSlot = Row.AddChildToHorizontalBox(Text);
			TextSlot->SetVerticalAlignment(VAlign_Center);
			// After it joins the row: its style, applied when its widget is built, would put back the style's colour.
			if (bName && GameState && GameState->GetLaunchConfig().Slots.IsValidIndex(Part.Slot))
			{
				Text->SetColorAndOpacity(FSlateColor(FLinearColor(HOLPlayerColors::GetColor(GameState->GetLaunchConfig().Slots[Part.Slot].ColorIndex))));
			}
		}
	}
}

void UHOLGameScreen::ShowAlert(const FHOLForumMessage& Message, const int32 TouchedSlot)
{
	const UWorld* World = GetWorld();
	bool bMatchOverForMe = false;
	if (!World || !AlertPanel || !AlertRow || !IsMatchOverForMe(bMatchOverForMe))
	{
		return;
	}
	// V4.5b: shown now, or waiting for the shown banner's 1.5 s (TickAlert).
	if (Alert.Show(Message, TouchedSlot, bMatchOverForMe, World->GetRealTimeSeconds()))
	{
		PresentAlert();
	}
}

bool UHOLGameScreen::IsMatchOverForMe(bool& bOver) const
{
	const AHOLGameState* GameState = BoundTableState.Get();
	if (!GameState)
	{
		return false;
	}
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	const int32 ViewerSlot = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
	// D202: the match over for me, my Victory / Defeat banner holds the screen; the thread's line is enough.
	bOver = HOLGameMenuRules::GetEndBanner(ViewerSlot, GameState->GetDefeatedMask(), GameState->IsMatchOver(),
		GameState->GetWinnerMask()) != EHOLEndBanner::None;
	return true;
}

void UHOLGameScreen::PresentAlert()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	const AHOLGameState* GameState = BoundTableState.Get();
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	const int32 ViewerSlot = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
	FillMessageRow(*WidgetTree, *AlertRow, Alert.Message, ViewerSlot, GameState, UHOLTextStyleHUDHeading::StaticClass(), UHOLTextStyleHUDHeading::StaticClass());
	AlertPanel->SetRenderOpacity(1.f);
	// Visible: a click on it centres the camera on the touched forum.
	SetShown(AlertPanel, true, ESlateVisibility::Visible);
	if (!Alert.bSound)
	{
		// V4.5b: one sound for a burst of alerts (less than 1 s apart), played with its first banner.
		UE_LOG(LogHOLUI, Display, TEXT("Alert: %s (no sound, its burst's played)"), *Alert.Message.Text.ToString());
		return;
	}
	// Provisional, 2D, the alert's only (the thread is silent): the herald's voice will replace it (FHOLForumAlert::SoundPath).
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, FHOLForumAlert::SoundPath);
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
	UE_LOG(LogHOLUI, Display, TEXT("Alert: %s (sound %s%s)"), *Alert.Message.Text.ToString(), FHOLForumAlert::SoundPath, Sound ? TEXT("") : TEXT(", missing"));
}

void UHOLGameScreen::TickAlert()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	const UWorld* World = GetWorld();
	if (!Alert.bShown || !World || !AlertPanel || !AlertRow)
	{
		return;
	}
	const double Now = World->GetRealTimeSeconds();
	// V4.5b: the next waiting alert once the shown one had its 1.5 s.
	bool bMatchOverForMe = false;
	if (!Alert.Waiting.IsEmpty() && IsMatchOverForMe(bMatchOverForMe) && Alert.Advance(Now, bMatchOverForMe))
	{
		PresentAlert();
		return;
	}
	if (Alert.Expire(Now))
	{
		SetShown(AlertPanel, false);
		return;
	}
	AlertPanel->SetRenderOpacity(Alert.GetOpacity(Now));
}

FEventReply UHOLGameScreen::HandleAlertMouseDown(FGeometry Geometry, const FPointerEvent& MouseEvent)
{
	if (!Alert.bShown || MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FEventReply(false);
	}
	FocusAlertSlot(Alert.Slot);
	return FEventReply(true);
}

FEventReply UHOLGameScreen::HandleMessageLineMouseDown(FGeometry Geometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FEventReply(false);
	}
	// The line under the mouse: only the lines carrying an alert take the clicks (RebuildMessages).
	for (int32 Index = 0; Index < MessageLines.Num() && MessageThread.Lines.IsValidIndex(Index); ++Index)
	{
		if (MessageLines[Index]->IsHovered() && MessageThread.Lines[Index].Message.bAlert)
		{
			FocusAlertSlot(MessageThread.Lines[Index].Slot);
			return FEventReply(true);
		}
	}
	return FEventReply(false);
}

void UHOLGameScreen::FocusAlertSlot(const int32 TouchedSlot)
{
	if (AHOLPlayerController* PlayerController = Controller.Get())
	{
		UE_LOG(LogHOLUI, Display, TEXT("HUD: alert of slot %d clicked (camera on its forum)"), TouchedSlot);
		PlayerController->FocusCameraOnForum(TouchedSlot);
	}
}

FString UHOLGameScreen::DescribeAlert() const
{
	return Alert.Describe();
}

void UHOLGameScreen::TickMessages()
{
	const UWorld* World = GetWorld();
	if (MessageThread.Lines.IsEmpty() || !World)
	{
		return;
	}
	const double Now = World->GetRealTimeSeconds();
	if (MessageThread.Expire(Now))
	{
		RebuildMessages();
	}
	// The last half second of a line: its fade.
	for (int32 Index = 0; Index < MessageThread.Lines.Num() && MessageLines.IsValidIndex(Index); ++Index)
	{
		MessageLines[Index]->SetRenderOpacity(FHOLForumThread::GetOpacity(MessageThread.Lines[Index], Now));
	}
}

void UHOLGameScreen::RebuildMessages()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	const AHOLGameState* GameState = BoundTableState.Get();
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	const int32 ViewerSlot = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
	// The list lets the clicks through but to the lines carrying an alert (V4.3: a click centres the camera on the forum).
	SetShown(MessagesList, !MessageThread.Lines.IsEmpty(), ESlateVisibility::SelfHitTestInvisible);
	for (int32 Index = 0; Index < MessageLines.Num(); ++Index)
	{
		const FHOLForumThreadLine* Line = MessageThread.Lines.IsValidIndex(Index) ? &MessageThread.Lines[Index] : nullptr;
		SetShown(MessageLines[Index], Line != nullptr, Line && Line->Message.bAlert ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
		UHorizontalBox* Row = MessageRows[Index];
		if (!Line)
		{
			Row->ClearChildren();
			continue;
		}
		MessageLines[Index]->SetRenderOpacity(1.f);
		// The plain words in the label's style, each name in its player's colour (the table's style).
		FillMessageRow(*WidgetTree, *Row, Line->Message, ViewerSlot, GameState, UHOLTextStyleHUDLabel::StaticClass(), UHOLTextStyleHUDPlayerName::StaticClass());
	}
}

FString UHOLGameScreen::DescribeMessages() const
{
	return MessageThread.Describe();
}

void UHOLGameScreen::RefreshPlayersTable()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	SinceTableRefresh = 0.f;
	const AHOLGameState* GameState = BoundTableState.Get();
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	TArray<FHOLPlayersTableRow> Rows;
	if (GameState)
	{
		TArray<FHOLForumState> Forums;
		for (int32 Seat = 0; Seat < FHOLMatchConfig::MaxSlots; ++Seat)
		{
			Forums.Add(GameState->GetForumState(Seat));
		}
		Rows = HOLForumRules::GetPlayersTable(GameState->GetLaunchConfig(), Forums, GameState->GetDefeatedMask(),
			PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE, FHOLForumTuning::FromSettings());
	}
	const FString Shown = FString::JoinBy(Rows, TEXT(" | "), [](const FHOLPlayersTableRow& Row) { return Row.Shown; });
	if (Shown == ShownPlayersTable)
	{
		return;
	}
	// M6-Q12: the names shown, logged when they change (the lobby's names reach the client with the launch configuration).
	const FString Names = FString::JoinBy(Rows, TEXT(", "), [](const FHOLPlayersTableRow& Row) { return Row.Name.ToString(); });
	if (Names != ShownPlayerNames)
	{
		ShownPlayerNames = Names;
		UE_LOG(LogHOLUI, Display, TEXT("Players table: %s"), *Names);
	}
	ShownPlayersTable = Shown;
	SetShown(PlayersPanel, !Rows.IsEmpty(), ESlateVisibility::HitTestInvisible);
	for (int32 Index = 0; Index < PlayerRows.Num(); ++Index)
	{
		const FHOLPlayersTableRow* Row = Rows.IsValidIndex(Index) ? &Rows[Index] : nullptr;
		SetShown(PlayerRows[Index], Row != nullptr, ESlateVisibility::SelfHitTestInvisible);
		if (!Row)
		{
			continue;
		}
		// A new team: a gap above its first row.
		if (UVerticalBoxSlot* RowSlot = Cast<UVerticalBoxSlot>(PlayerRows[Index]->Slot))
		{
			RowSlot->SetPadding(FMargin(0.f, Row->bNewGroup ? 6.f : 0.f, 0.f, 0.f));
		}
		// The name in the player's colour, the whole row faded once defeated.
		PlayerRows[Index]->SetRenderOpacity(Row->bGreyed ? 0.45f : 1.f);
		PlayerNames[Index]->SetText(Row->Name);
		const FLinearColor NameColor(HOLPlayerColors::GetColor(Row->ColorIndex));
		PlayerNames[Index]->SetColorAndOpacity(FSlateColor(NameColor));
		// E4-2: a defeated player's name struck through, in the name's colour (faded with the row).
		PlayerStrikes[Index]->SetColorAndOpacity(NameColor);
		SetShown(PlayerStrikes[Index], Row->bStruck, ESlateVisibility::HitTestInvisible);
		// E4-1: an ally's green shield, an enemy's red swords (faded with a defeated row). The viewer's own row keeps the
		// icon's room empty, so that the names line up; a spectator's table (all None) has no room for it.
		UTexture2D* Icon = Row->Relation == EHOLPlayerRelation::Ally ? AllyIcon.Get()
			: Row->Relation == EHOLPlayerRelation::Enemy ? EnemyIcon.Get() : nullptr;
		if (Icon)
		{
			PlayerRelations[Index]->SetBrushFromTexture(Icon);
			PlayerRelations[Index]->SetDesiredSizeOverride(FVector2D(16.f, 16.f));
		}
		const ESlateVisibility RelationVisibility = Icon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
		if (PlayerRelations[Index]->GetVisibility() != RelationVisibility)
		{
			PlayerRelations[Index]->SetVisibility(RelationVisibility);
		}
		// The room itself: on every row of a player's table, none in a spectator's (no relation at all).
		SetShown(PlayerRelations[Index]->GetParent(), Row->Relation != EHOLPlayerRelation::None, ESlateVisibility::HitTestInvisible);
		PlayerLives[Index]->SetText(Row->Lives);
		PlayerStates[Index]->SetText(Row->State);
	}
}

void UHOLGameScreen::RefreshWaitBanner()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	const AHOLGameState* GameState = BoundTableState.Get();
	const bool bWaiting = GameState && GameState->IsWaitingForPlayers() && !GameState->GetMissingPlayers().IsEmpty();
	if (!bWaiting)
	{
		if (!ShownWaitPlayers.IsEmpty())
		{
			ShownWaitPlayers.Reset();
			SetShown(WaitPanel, false);
			UE_LOG(LogHOLUI, Display, TEXT("Wait banner: hidden"));
		}
		return;
	}
	const FString Players = FString::Join(GameState->GetMissingPlayers(), TEXT(", "));
	const int32 Left = FMath::Max(0, FMath::CeilToInt(GameState->GetWaitEndSeconds() - GameState->GetServerWorldTimeSeconds()));
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("Players"), FText::AsCultureInvariant(Players));
	Arguments.Add(TEXT("Time"), FText::AsCultureInvariant(FString::Printf(TEXT("%d:%02d"), Left / 60, Left % 60)));
	const FText Text = FText::Format(LOCTEXT("WaitingFor", "En attente de {Players} ({Time})"), Arguments);
	if (WaitText)
	{
		WaitText->SetText(Text);
	}
	if (Players != ShownWaitPlayers)
	{
		ShownWaitPlayers = Players;
		SetShown(WaitPanel, true, ESlateVisibility::HitTestInvisible);
		UE_LOG(LogHOLUI, Display, TEXT("Wait banner: shown, %s"), *Text.ToString());
	}
}

void UHOLGameScreen::RefreshAbsenceBanner()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	const AHOLGameState* GameState = BoundTableState.Get();
	const TArray<FText> Lines = GameState ? GameState->GetAbsenceBannerLines(GameState->GetServerWorldTimeSeconds()) : TArray<FText>();
	if (Lines.IsEmpty())
	{
		if (!ShownAbsence.IsEmpty())
		{
			ShownAbsence.Reset();
			SetShown(AbsencePanel, false);
			UE_LOG(LogHOLUI, Display, TEXT("Absence banner: hidden"));
		}
		return;
	}
	const FText Text = FText::Join(FText::AsCultureInvariant(TEXT("\n")), Lines);
	if (AbsenceText)
	{
		AbsenceText->SetText(Text);
	}
	// Logged when who is gone or back changes, not at each second of the countdown.
	FString Shown = FString::FromInt(Lines.Num());
	for (const FHOLAbsentNotice& Notice : GameState->GetAbsentNotices())
	{
		Shown += FString::Printf(TEXT(";%s%s"), *Notice.Name, Notice.BackAt > 0. ? TEXT("+") : TEXT("-"));
	}
	if (Shown != ShownAbsence)
	{
		ShownAbsence = Shown;
		SetShown(AbsencePanel, true, ESlateVisibility::HitTestInvisible);
		UE_LOG(LogHOLUI, Display, TEXT("Absence banner: shown, %s"), *Text.ToString().Replace(TEXT("\n"), TEXT(" | ")));
	}
}

void UHOLGameScreen::HandleSelectionChanged()
{
	BindBuilding(Controller.IsValid() ? Controller->GetSelection() : nullptr);
	Refresh();
}

void UHOLGameScreen::Refresh()
{
	const AHOLPlayerController* PlayerController = Controller.Get();
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	const AHOLBuilding* Selected = PlayerController ? PlayerController->GetSelection() : nullptr;
	// Only the owner gives a building orders, and only the owner receives its queue (D282).
	const AHOLBuilding* Own = PlayerController ? PlayerController->GetOwnSelection() : nullptr;

	RefreshTopBar(PlayerState);
	ChipLabels.Reset();
	ActiveChip = INDEX_NONE;
	// A main forum's lives row (V2.3d): RefreshSelection alone shows it, for a single building.
	const bool bSingleBuilding = !PlayerController || (PlayerController->GetSelectedUnits().IsEmpty()
		&& PlayerController->GetSelectedSource() == INDEX_NONE && !PlayerController->HasBuildingGroup());
	if (!bSingleBuilding)
	{
		UE::HeroesOfLiberty::UI::GameScreen::SetShown(SelectionLivesRow, false);
	}
	if (PlayerController && !PlayerController->GetSelectedUnits().IsEmpty())
	{
		RefreshUnitSelection(PlayerController->GetSelectedUnits());
	}
	else if (PlayerController && PlayerController->GetSelectedSource() != INDEX_NONE)
	{
		RefreshSourceSelection(PlayerController->GetSelectedSource());
	}
	else if (PlayerController && PlayerController->HasBuildingGroup())
	{
		TArray<AHOLBuilding*> Group;
		PlayerController->GetSelectedBuildings(Group);
		RefreshBuildingGroup(Group);
	}
	else
	{
		RefreshSelection(Selected, Selected && Selected == Own);
	}
	RefreshChips();
	RefreshIdleWorkers();
	// D357: a spectator commands nothing, its grid hidden.
	const bool bSpectator = PlayerController && PlayerController->IsSpectator();
	RefreshGrid(bSpectator ? nullptr : Own, !bSpectator && PlayerController && PlayerController->HasBuildingGroup(), PlayerState);
	RefreshQueue(Own);
	UpdateHead();
}

void UHOLGameScreen::RefreshTopBar(const AHOLPlayerState* PlayerState)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// Nothing to show before the player state has reached this machine.
	bEconomyShown = PlayerState != nullptr;
	SetShown(TopBar, bEconomyShown, ESlateVisibility::Visible);
	if (!PlayerState)
	{
		return;
	}
	ShownResources = PlayerState->GetResources();
	ShownPopulationUsed = PlayerState->GetPopulationUsed();
	ShownPopulationCap = PlayerState->GetPopulationCap();
	ShownGatherers = PlayerState->GetGatherers();

	const auto SetAmount = [](UCommonTextBlock* Block, const int32 Value)
	{
		if (Block)
		{
			Block->SetText(FormatAmount(Value));
		}
	};
	SetAmount(FoodAmount, ShownResources.Food);
	SetAmount(WoodAmount, ShownResources.Wood);
	SetAmount(StoneAmount, ShownResources.Stone);
	SetAmount(GoldAmount, ShownResources.Gold);
	const auto SetGatherers = [](UCommonTextBlock* Block, const int32 Value)
	{
		if (Block)
		{
			Block->SetText(FText::Format(LOCTEXT("GathererCount", "({0})"), FormatAmount(Value)));
		}
	};
	SetGatherers(FoodGatherers, ShownGatherers.Food);
	SetGatherers(WoodGatherers, ShownGatherers.Wood);
	SetGatherers(StoneGatherers, ShownGatherers.Stone);
	SetGatherers(GoldGatherers, ShownGatherers.Gold);
	if (PopulationAmount)
	{
		PopulationAmount->SetText(FormatRatio(ShownPopulationUsed, ShownPopulationCap));
	}
}

void UHOLGameScreen::RefreshSelection(const AHOLBuilding* Building, const bool bOwn)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	SetShown(SelectionPanel, Building != nullptr, ESlateVisibility::Visible);
	const bool bSite = Building && !Building->IsFinished();
	SetShown(SelectionProgressRow, bSite);
	if (!Building)
	{
		ShownSelection = TEXT("none");
		ShownHealth.Reset();
		RefreshUnitActivity(nullptr);
		return;
	}
	const FHOLBuildingTypeRow* Type = Building->GetType();
	const int32 Percent = FMath::FloorToInt(Building->GetProgress() * 100.f);
	ShownSelection = FString::Printf(TEXT("%s (%s)%s"), *Building->GetTypeName().ToString(), bOwn ? TEXT("own") : TEXT("other"),
		Building->IsFoundation() ? TEXT(", foundation") : bSite ? *FString::Printf(TEXT(", under construction %d%%"), Percent) : TEXT(""));

	if (SelectionName)
	{
		// A building not finished: its foundation (J6a.3), then its site with its progress (J6a.5, D313).
		const FText Name = HOLGameNames::GetBuildingName(Building->GetTypeName(), Type);
		SelectionName->SetText(!bSite ? Name : Building->IsFoundation() ? FText::Format(LOCTEXT("FoundationName", "Fondation : {0}"), Name)
			: FText::Format(LOCTEXT("SiteName", "Chantier : {0}"), Name));
	}
	if (bSite && SelectionProgress)
	{
		SelectionProgress->SetPercent(Building->GetProgress());
	}
	if (bSite && SelectionProgressText)
	{
		SelectionProgressText->SetText(FText::Format(LOCTEXT("ProgressPercent", "{0} %"), FText::AsNumber(Percent)));
	}
	SetShown(SelectionOwnerRow, true);
	if (SelectionSwatch)
	{
		SelectionSwatch->SetColorAndOpacity(FLinearColor(HOLPlayerColors::GetColor(Building->GetColorIndex())));
	}
	if (SelectionOwner)
	{
		SelectionOwner->SetText(bOwn ? LOCTEXT("You", "Vous") : FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), Building->GetOwnerPlayer() + 1));
	}
	ShownHealth.Reset();
	// A main forum (V2.3b): its lives to anyone (D353); down, its ruin or its rebuilding in place of its health (0 until up).
	const FHOLForumBlock Forum = Building->GetForumBlock(bOwn);
	// Reported on the line "HUD forum", not in Describe (the checks read the report's line to its end).
	ShownForum = Forum.bShown ? Forum.Shown : FString();
	SetShown(SelectionHealthRow, !Forum.bHideHealth);
	// Its lives on their own row (V2.3d), styled as the health's.
	SetShown(SelectionLivesRow, Forum.bShown);
	if (Forum.bShown)
	{
		SetDetailLine(Forum.Shown, Forum.Text);
		// Standing: no line of its own, its lives alone (the description keeps them for the reports).
		SetShown(SelectionActivity, !Forum.Text.IsEmpty());
		if (SelectionLives && Forum.Lives != ShownForumLives)
		{
			ShownForumLives = Forum.Lives;
			SelectionLives->SetText(FText::AsNumber(Forum.Lives));
		}
	}
	else if (!bSite || !bOwn)
	{
		RefreshUnitActivity(nullptr);
	}
	if (SelectionHealth)
	{
		// The health of its work (D312), whole once finished, less the damage it took (D345.2a).
		SelectionHealth->SetText(FormatRatio(Building->GetHealth(), Building->GetMaxHealth()));
	}
	if (bSite && bOwn)
	{
		// Its builders at work (D313), which only its owner receives.
		const int32 Builders = Building->GetBuildersAtWork();
		SetDetailLine(FString::Printf(TEXT(", builders %d"), Builders), FText::Format(LOCTEXT("Builders", "Bâtisseurs : {0}"), Builders));
	}
}

void UHOLGameScreen::RefreshUnitSelection(const TConstArrayView<uint32> UnitIds)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// The selected units as this machine knows them: type and owner.
	TArray<FHOLUnitView> Views;
	if (const UWorld* World = GetWorld())
	{
		FHOLUnitView One;
		if (UnitIds.Num() != 1)
		{
			HOLUnitView::Gather(*World, Views);
		}
		else if (HOLUnitView::Find(*World, UnitIds[0], One))
		{
			Views.Add(One);
		}
	}
	Views.RemoveAll([UnitIds](const FHOLUnitView& View) { return !UnitIds.Contains(View.UnitId); });
	SetShown(SelectionPanel, !Views.IsEmpty(), ESlateVisibility::Visible);
	SetShown(SelectionProgressRow, false);
	if (Views.IsEmpty())
	{
		ShownSelection = TEXT("none");
		ShownHealth.Reset();
		RefreshUnitActivity(nullptr);
		return;
	}
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	const int32 Owner = Views[0].OwnerPlayer;
	const bool bOwn = PlayerState && PlayerState->GetPlayerIndex() == Owner;

	// Types in a stable order, each with its number.
	TMap<FName, int32> PerType;
	for (const FHOLUnitView& View : Views)
	{
		++PerType.FindOrAdd(View.TypeName);
	}
	// Soldiers first, workers (the Peasants) last (J7a.5), each part by name.
	TSet<FName> WorkerTypes;
	for (const FHOLUnitView& View : Views)
	{
		if (View.CarryCapacity > 0)
		{
			WorkerTypes.Add(View.TypeName);
		}
	}
	PerType.KeySort([&WorkerTypes](const FName A, const FName B)
	{
		const bool bWorkerA = WorkerTypes.Contains(A);
		return bWorkerA != WorkerTypes.Contains(B) ? !bWorkerA : A.LexicalLess(B);
	});
	TArray<FString> ShownTypes;
	for (const TPair<FName, int32>& Type : PerType)
	{
		ShownTypes.Add(Views.Num() == 1 ? Type.Key.ToString() : FString::Printf(TEXT("%s x%d"), *Type.Key.ToString(), Type.Value));
	}
	ShownSelection = FString::Printf(TEXT("%s (%s)"), *FString::Join(ShownTypes, TEXT(", ")), bOwn ? TEXT("own") : TEXT("other"));
	if (SelectionName && Views.Num() == 1)
	{
		SelectionName->SetText(HOLGameNames::GetUnitName(Views[0].TypeName, FindUnitType(Views[0].TypeName)));
	}
	// A group (G.1e, AoE4's way): chips, the player's units of several types by their subgroups (Tab's, « Tous (8) » first,
	// the active one lit, the soldiers first, the Peasants last); else « Tous » (several types) then one per type.
	TArray<FHOLSubgroup> Subgroups;
	FHOLSubgroup Subgroup;
	const AHOLPlayerController* PlayerController = Controller.Get();
	if (Views.Num() > 1 && bOwn && PlayerController && PlayerController->GetActiveUnitSubgroup(Subgroup))
	{
		PlayerController->GetUnitSubgroups(Subgroups);
		for (const FHOLSubgroup& Each : Subgroups)
		{
			ChipLabels.Add(Each.bAll ? FText::Format(LOCTEXT("SubgroupAll", "Tous ({0})"), Each.Members.Num())
				: MakeChipLabel(HOLGameNames::GetUnitName(Each.TypeName, FindUnitType(Each.TypeName)), Each.Members.Num()));
		}
		ActiveChip = Subgroups.IndexOfByKey(Subgroup);
	}
	else if (Views.Num() > 1)
	{
		if (PerType.Num() > 1)
		{
			ChipLabels.Add(FText::Format(LOCTEXT("SubgroupAll", "Tous ({0})"), Views.Num()));
		}
		for (const TPair<FName, int32>& Type : PerType)
		{
			ChipLabels.Add(MakeChipLabel(HOLGameNames::GetUnitName(Type.Key, FindUnitType(Type.Key)), Type.Value));
		}
		ActiveChip = bOwn ? 0 : INDEX_NONE;
	}
	SetShown(SelectionOwnerRow, true);
	if (SelectionSwatch)
	{
		SelectionSwatch->SetColorAndOpacity(FLinearColor(HOLPlayerColors::GetColor(AHOLPlayerState::FindColorIndex(*GetWorld(), Owner))));
	}
	if (SelectionOwner)
	{
		SelectionOwner->SetText(bOwn ? LOCTEXT("You", "Vous") : FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), Owner + 1));
	}
	// One unit shows its health, a group does not (D288).
	RefreshUnitHealth(Views.Num() == 1 ? &Views[0] : nullptr);
	if (Views.Num() == 1)
	{
		// One gatherer shows what it does and what it carries (J5, D298); any unit striking, its attack (J7b.4).
		RefreshUnitActivity(Views[0].CarryCapacity > 0 || Views[0].Activity == EHOLUnitActivity::Attacking ? &Views[0] : nullptr);
		return;
	}
	// A group, its workers by activity (J5b, D298): the round trip counts as gathering (D307); building (J6a.4) on its own;
	// fighting on its own too, never idle (J7b trial feedback).
	int32 Gathering = 0, Moving = 0, Idle = 0, Building = 0, Fighting = 0;
	for (const FHOLUnitView& View : Views)
	{
		if (View.CarryCapacity > 0)
		{
			++(IsGatheringActivity(View.Activity) ? Gathering : View.Activity == EHOLUnitActivity::Building ? Building
				: View.Activity == EHOLUnitActivity::Attacking ? Fighting
				: View.Activity == EHOLUnitActivity::Moving ? Moving : Idle);
		}
	}
	TArray<FText> Parts;
	if (Gathering > 0)
	{
		Parts.Add(FText::Format(LOCTEXT("GroupGathering", "{0} en récolte"), Gathering));
	}
	if (Building > 0)
	{
		Parts.Add(FText::Format(LOCTEXT("GroupBuilding", "{0} en construction"), Building));
	}
	if (Fighting > 0)
	{
		Parts.Add(FText::Format(LOCTEXT("GroupFighting", "{0} au combat"), Fighting));
	}
	if (Moving > 0)
	{
		Parts.Add(FText::Format(LOCTEXT("GroupMoving", "{0} en marche"), Moving));
	}
	if (Idle > 0)
	{
		Parts.Add(FText::Format(LOCTEXT("GroupIdle", "{0} {0}|plural(one=inactif,other=inactifs)"), Idle));
	}
	SetDetailLine(Parts.IsEmpty() ? FString() : FString::Printf(TEXT(", gathering %d building %d fighting %d moving %d idle %d"), Gathering, Building, Fighting, Moving, Idle),
		FText::Join(FText::AsCultureInvariant(TEXT(" · ")), Parts));
}

void UHOLGameScreen::RefreshBuildingGroup(const TConstArrayView<AHOLBuilding*> Buildings)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// Several of the player's buildings (an F key, D30): by type and number, as a group of units; the sites among them.
	SetShown(SelectionPanel, !Buildings.IsEmpty(), ESlateVisibility::Visible);
	SetShown(SelectionProgressRow, false);
	ShownHealth.Reset();
	SetShown(SelectionHealthRow, false);
	if (Buildings.IsEmpty())
	{
		ShownSelection = TEXT("none");
		ShownHealth.Reset();
		SetDetailLine(FString(), FText::GetEmpty());
		return;
	}
	// In the group's order (the centre first), its ghosts and sites counted as « en construction ».
	TMap<FName, int32> PerType;
	int32 Sites = 0;
	for (const AHOLBuilding* Building : Buildings)
	{
		++PerType.FindOrAdd(Building->GetTypeName());
		Sites += Building->IsFinished() ? 0 : 1;
	}
	TArray<FString> ShownTypes;
	for (const TPair<FName, int32>& Type : PerType)
	{
		ShownTypes.Add(FString::Printf(TEXT("%s x%d"), *Type.Key.ToString(), Type.Value));
	}
	// Chips (G.1e, as a group of units): the subgroups Tab goes through, « Tous (7) », « Maison (3) », « Centre principal »
	// (each main centre one alone), the active one lit.
	FHOLSubgroup Subgroup;
	TArray<FHOLSubgroup> Subgroups;
	const AHOLPlayerController* PlayerController = Controller.Get();
	if (PlayerController && PlayerController->GetActiveSubgroup(Subgroup))
	{
		PlayerController->GetSubgroups(Subgroups);
		for (const FHOLSubgroup& Each : Subgroups)
		{
			ChipLabels.Add(Each.bAll ? FText::Format(LOCTEXT("SubgroupAll", "Tous ({0})"), Each.Members.Num())
				: MakeChipLabel(HOLGameNames::GetBuildingName(Each.TypeName, UHOLBuildingSettings::FindType(Each.TypeName)),
					HOLCommandGrid::IsAloneInGroup(Each.TypeName) ? 0 : Each.Members.Num()));
		}
		ActiveChip = Subgroups.IndexOfByKey(Subgroup);
	}
	ShownSelection = FString::Printf(TEXT("%d buildings (own): %s, subgroup %s"), Buildings.Num(), *FString::Join(ShownTypes, TEXT(" ")),
		PlayerController ? *PlayerController->DescribeSubgroup() : TEXT(""));
	SetShown(SelectionOwnerRow, true);
	if (SelectionSwatch)
	{
		SelectionSwatch->SetColorAndOpacity(FLinearColor(HOLPlayerColors::GetColor(Buildings[0]->GetColorIndex())));
	}
	if (SelectionOwner)
	{
		SelectionOwner->SetText(LOCTEXT("You", "Vous"));
	}
	SetDetailLine(Sites > 0 ? FString::Printf(TEXT(", sites %d"), Sites) : FString(), FText::Format(LOCTEXT("GroupBuilding", "{0} en construction"), Sites));
}

void UHOLGameScreen::RefreshChips()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// The chips in place of the name (G.1e), each written when it changed; the active one's frame lit as the chosen age.
	const bool bChips = !ChipLabels.IsEmpty();
	SetShown(SelectionName, !bChips);
	SetShown(SelectionChips, bChips, ESlateVisibility::SelfHitTestInvisible);
	ShownChips.SetNum(ChipItems.Num());
	for (int32 Chip = 0; Chip < ChipItems.Num(); ++Chip)
	{
		const FText Label = ChipLabels.IsValidIndex(Chip) ? ChipLabels[Chip] : FText::GetEmpty();
		const FString Shown = Label.ToString();
		UBorder* Frame = ChipFrames.IsValidIndex(Chip) ? ChipFrames[Chip].Get() : nullptr;
		if (ShownChips[Chip] != Shown)
		{
			ShownChips[Chip] = Shown;
			if (ChipItems[Chip])
			{
				ChipItems[Chip]->SetLabel(Label);
			}
			SetShown(Frame, !Shown.IsEmpty(), ESlateVisibility::SelfHitTestInvisible);
		}
		if (Frame && (ShownActiveChip == Chip) != (ActiveChip == Chip))
		{
			Frame->SetBrushColor(ActiveChip == Chip ? HOLUIStyle::Accent : FLinearColor::Transparent);
		}
	}
	ShownActiveChip = ActiveChip;
}

void UHOLGameScreen::HandleChipClicked(const int32 Chip)
{
	// Tab as many times as it takes from the active subgroup to the chip's (the same path and logs as the key).
	AHOLPlayerController* PlayerController = Controller.Get();
	if (!PlayerController)
	{
		return;
	}
	TArray<FHOLSubgroup> Subgroups;
	FHOLSubgroup Active;
	const bool bUnits = !PlayerController->GetSelectedUnits().IsEmpty();
	const bool bFound = bUnits ? PlayerController->GetActiveUnitSubgroup(Active) : PlayerController->GetActiveSubgroup(Active);
	if (bUnits)
	{
		PlayerController->GetUnitSubgroups(Subgroups);
	}
	else
	{
		PlayerController->GetSubgroups(Subgroups);
	}
	const int32 From = bFound ? Subgroups.IndexOfByKey(Active) : INDEX_NONE;
	if (From == INDEX_NONE || !Subgroups.IsValidIndex(Chip))
	{
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("HUD: chip %d clicked"), Chip);
	for (int32 Step = (Chip - From + Subgroups.Num()) % Subgroups.Num(); Step > 0; --Step)
	{
		PlayerController->CycleSubgroup();
	}
	Refresh();
}

void UHOLGameScreen::RefreshSourceSelection(const int32 SourceIndex)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	const UHOLSourceSubsystem* Sources = GetWorld() ? GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	const FHOLSource* Source = Sources && Sources->GetSources().IsValidIndex(SourceIndex) ? &Sources->GetSources()[SourceIndex] : nullptr;
	const FName TypeName = Source && Sources->GetTypeNames().IsValidIndex(Source->TypeIndex) ? Sources->GetTypeNames()[Source->TypeIndex] : NAME_None;
	SetShown(SelectionPanel, !TypeName.IsNone(), ESlateVisibility::Visible);
	SetShown(SelectionProgressRow, false);
	if (TypeName.IsNone())
	{
		ShownSelection = TEXT("none");
		ShownHealth.Reset();
		SetDetailLine(FString(), FText::GetEmpty());
		return;
	}
	// Nobody's: no owner, no health; its name, and what it still holds (« Filon d'or — 1840 », D298).
	SetShown(SelectionOwnerRow, false);
	ShownHealth.Reset();
	SetShown(SelectionHealthRow, false);
	const FString Shown = FString::Printf(TEXT("%s source %d"), *TypeName.ToString(), SourceIndex);
	if (Shown != ShownSelection)
	{
		ShownSelection = Shown;
		if (SelectionName)
		{
			SelectionName->SetText(HOLGameNames::GetSourceName(TypeName, Sources->GetType(Source->TypeIndex)));
		}
	}
	SetDetailLine(FString::Printf(TEXT(", %d left"), Source->Remaining),
		FText::Format(LOCTEXT("SourceRemaining", "Reste : {0}"), FormatAmount(Source->Remaining)));
}

void UHOLGameScreen::RefreshUnitActivity(const FHOLUnitView* Unit)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// The amount is known in solo, and on a client once the server has sent it (D303): until then, the activity alone.
	const bool bLoad = Unit && Unit->bLoadAmountKnown && (Unit->LoadAmount > 0 || IsGatheringActivity(Unit->Activity));
	const FString Shown = !Unit ? FString()
		: bLoad ? FString::Printf(TEXT(", %s %s %d/%d"), *UEnum::GetValueAsString(Unit->Activity),
			*UEnum::GetValueAsString(Unit->LoadResource), Unit->LoadAmount, Unit->CarryCapacity)
		: FString::Printf(TEXT(", %s"), *UEnum::GetValueAsString(Unit->Activity));
	if (Shown == ShownActivity || !Unit)
	{
		SetDetailLine(Shown, FText::GetEmpty());
		return;
	}
	const FText Activity = IsGatheringActivity(Unit->Activity) ? LOCTEXT("ActivityGathering", "Récolte")
		: Unit->Activity == EHOLUnitActivity::Building ? LOCTEXT("ActivityBuilding", "Construction")
		: Unit->Activity == EHOLUnitActivity::Attacking ? LOCTEXT("ActivityAttacking", "Attaque")
		: Unit->Activity == EHOLUnitActivity::Moving ? LOCTEXT("ActivityMoving", "En marche") : LOCTEXT("ActivityIdle", "Inactif");
	SetDetailLine(Shown, bLoad
		? FText::Format(LOCTEXT("ActivityWithLoad", "{0} — {1} {2}/{3}"), Activity, GetResourceName(Unit->LoadResource),
			Unit->LoadAmount, Unit->CarryCapacity)
		: Activity);
}

void UHOLGameScreen::RefreshUnitHealth(const FHOLUnitView* Unit)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// Exact for the troops (J7-Q5 revised: a client receives a wounded unit's exact health); a client's hero (J8a.3) is
	// exact from his public sheet, before it arrives known by a share of his maximum, drawn back to health points within 2
	// of his own: " (share)" marks it.
	const bool bShown = Unit && Unit->MaxHealth > 0;
	const FString Shown = bShown ? FString::Printf(TEXT(", hp %d/%d%s"), Unit->Health, Unit->MaxHealth, Unit->bHealthExact ? TEXT("") : TEXT(" (share)"))
		: FString();
	// Shown or hidden whatever was shown last (the review of J7b): a building selected before left the row shown with an
	// empty ShownHealth, which a group of units or a unit of unknown type also has.
	SetShown(SelectionHealthRow, bShown);
	if (Shown == ShownHealth)
	{
		return;
	}
	ShownHealth = Shown;
	if (SelectionHealth && bShown)
	{
		SelectionHealth->SetText(FormatRatio(Unit->Health, Unit->MaxHealth));
	}
}

void UHOLGameScreen::SetDetailLine(const FString& Shown, const FText& Text)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	if (Shown == ShownActivity)
	{
		return;
	}
	ShownActivity = Shown;
	SetShown(SelectionActivity, !Shown.IsEmpty());
	if (SelectionActivity && !Shown.IsEmpty())
	{
		SelectionActivity->SetText(Text);
	}
}

void UHOLGameScreen::RefreshIdleWorkers()
{
	const int32 Count = Controller.IsValid() ? Controller->GetIdleWorkers().Num() : 0;
	if (Count == ShownIdleWorkers)
	{
		return;
	}
	ShownIdleWorkers = Count;
	if (IdleWorkers)
	{
		IdleWorkers->SetLabel(FText::Format(LOCTEXT("IdleWorkers", "Inactifs : {0}"), Count));
		IdleWorkers->SetIsEnabled(Count > 0);
	}
}

void UHOLGameScreen::HandleIdleWorkersClicked()
{
	if (AHOLPlayerController* PlayerController = Controller.Get())
	{
		UE_LOG(LogHOLUI, Display, TEXT("HUD: idle workers clicked"));
		PlayerController->SelectNextIdleWorker();
	}
}

FName UHOLGameScreen::GroupSlotName(const int32 Group)
{
	return *FString::Printf(TEXT("ControlGroup%d"), Group);
}

void UHOLGameScreen::RefreshControlGroups(const float DeltaSeconds)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	const AHOLPlayerController* PlayerController = Controller.Get();
	const UWorld* World = GetWorld();
	if (!PlayerController || !World || GroupSlots.Num() != AHOLPlayerController::NumControlGroups)
	{
		return;
	}
	// The group just selected lit for a moment (its key, its slot).
	double SelectTime = -1.0;
	const int32 Selected = PlayerController->GetLastSelectedGroup(SelectTime);
	const int32 Lit = Selected != INDEX_NONE && World->GetRealTimeSeconds() - SelectTime < GroupLightSeconds ? Selected : INDEX_NONE;
	if (Lit != LitGroup)
	{
		for (int32 Group = 0; Group < GroupSlots.Num(); ++Group)
		{
			if (UHOLQueueItem* GroupSlot = GroupSlots[Group])
			{
				GroupSlot->SetRenderScale(Group == Lit ? FVector2D(1.15f, 1.15f) : FVector2D(1.f, 1.f));
				GroupSlot->SetRenderOpacity(Lit == INDEX_NONE || Group == Lit ? 1.f : 0.7f);
			}
		}
		LitGroup = Lit;
	}

	SinceGroupsRefresh += DeltaSeconds;
	if (SinceGroupsRefresh < GroupRefreshInterval)
	{
		return;
	}
	SinceGroupsRefresh = 0.f;

	TArray<FHOLUnitView> Views;
	bool bViewsRead = false;
	TArray<FString> LineParts;
	bool bAny = false;
	// 1 to 9, then 0, as the keys.
	for (int32 Index = 1; Index <= AHOLPlayerController::NumControlGroups; ++Index)
	{
		const int32 Group = Index % AHOLPlayerController::NumControlGroups;
		UHOLQueueItem* GroupSlot = GroupSlots[Group];
		TArray<uint32> Units;
		TArray<AHOLBuilding*> Buildings;
		const bool bFull = PlayerController->GetControlGroup(Group, Units, Buildings);
		const FString Shown = !bFull ? FString() : Units.IsEmpty() ? FString::Printf(TEXT("b%d"), Buildings.Num()) : FString::FromInt(Units.Num());
		if (bFull)
		{
			bAny = true;
			LineParts.Add(FString::Printf(TEXT("%d=%s"), Group, *Shown));
		}
		if (Shown == ShownGroups[Group])
		{
			continue;
		}
		ShownGroups[Group] = Shown;
		// The slot's box, which keeps its width even with the slot collapsed.
		SetShown(GroupSlot ? GroupSlot->GetParent() : nullptr, bFull, ESlateVisibility::SelfHitTestInvisible);
		if (!bFull || !GroupSlot)
		{
			continue;
		}
		const int32 Count = Units.IsEmpty() ? Buildings.Num() : Units.Num();
		GroupSlot->SetLabel(FText::Format(LOCTEXT("ControlGroupSlot", "{0} · {1}"), FText::AsCultureInvariant(FString::FromInt(Group)), Count));
		// The tooltip: « Groupe 1 », then its types by number, the most first (soldiers before Peasants when even).
		TMap<FName, int32> PerType;
		if (!Units.IsEmpty())
		{
			if (!bViewsRead)
			{
				HOLUnitView::Gather(*World, Views);
				bViewsRead = true;
			}
			for (const FHOLUnitView& View : Views)
			{
				if (Units.Contains(View.UnitId))
				{
					++PerType.FindOrAdd(View.TypeName);
				}
			}
		}
		for (const AHOLBuilding* Building : Buildings)
		{
			++PerType.FindOrAdd(Building->GetTypeName());
		}
		PerType.ValueStableSort([](const int32 A, const int32 B) { return A > B; });
		TArray<FText> Lines;
		Lines.Add(FText::Format(LOCTEXT("ControlGroupTooltip", "Groupe {0}"), FText::AsCultureInvariant(FString::FromInt(Group))));
		for (const TPair<FName, int32>& Type : PerType)
		{
			const FText Name = Units.IsEmpty() ? HOLGameNames::GetBuildingName(Type.Key, UHOLBuildingSettings::FindType(Type.Key))
				: HOLGameNames::GetUnitName(Type.Key, FindUnitType(Type.Key));
			Lines.Add(FText::Format(LOCTEXT("UnitCount", "{0} × {1}"), Name, Type.Value));
		}
		GroupSlot->SetToolTipText(FText::Join(FText::AsCultureInvariant(TEXT("\n")), Lines));
	}
	SetShown(GroupsPanel, bAny, ESlateVisibility::SelfHitTestInvisible);
	const FString Line = bAny ? FString::Join(LineParts, TEXT(" ")) : FString(TEXT("none"));
	if (Line != ShownGroupsLine)
	{
		ShownGroupsLine = Line;
		UE_LOG(LogHOLUI, Display, TEXT("HUD groups: %s"), *Line);
	}
}

void UHOLGameScreen::HandleGroupSlotClicked(const int32 Group)
{
	if (AHOLPlayerController* PlayerController = Controller.Get())
	{
		UE_LOG(LogHOLUI, Display, TEXT("HUD: group %d clicked"), Group);
		PlayerController->SelectControlGroup(Group);
	}
}

void UHOLGameScreen::RefreshGrid(const AHOLBuilding* OwnBuilding, const bool bOwnGroup, const AHOLPlayerState* PlayerState)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// What each cell does with the selection (D30, D313): the building's production or its site's « Annuler », what a
	// group of buildings all produce, or the builders' grid. A cell shows its name only (the user's trial of J6a.5); its
	// cost, what is missing in red, is its tooltip's.
	const AHOLPlayerController* PlayerController = Controller.Get();
	TArray<FHOLGridCommand> Commands;
	if (PlayerController)
	{
		PlayerController->GetGridCommands(Commands);
	}
	bGridShown = OwnBuilding != nullptr || bOwnGroup || Commands.ContainsByPredicate([](const FHOLGridCommand& Command) { return !Command.IsNone(); });
	// The player's units (never with a building): their orders in the same panel, Q attack-move (soldiers), S stop, V hold.
	TArray<FHOLGridCommand> Orders;
	if (PlayerController && !bGridShown)
	{
		PlayerController->GetOrderCommands(Orders);
	}
	bOrdersShown = Orders.ContainsByPredicate([](const FHOLGridCommand& Command) { return !Command.IsNone(); });
	// Hidden, never collapsed: its place in the bottom bar kept (nothing moves from a selection to the next).
	if (GridPanel)
	{
		const ESlateVisibility Wanted = bGridShown || bOrdersShown ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
		if (GridPanel->GetVisibility() != Wanted)
		{
			GridPanel->SetVisibility(Wanted);
		}
	}
	RefreshAges(PlayerState);

	// Only a site of the player's own, selected alone (or alone in a group's subgroup), has « Annuler » (HOLCommandGrid::GetCommand).
	const FHOLResources Stock = PlayerState ? PlayerState->GetResources() : FHOLResources();
	for (int32 Cell = 0; Cell < GridCells.Num() && Cell < ShownGridTypes.Num(); ++Cell)
	{
		const FHOLGridCommand Command = bGridShown && Commands.IsValidIndex(Cell) ? Commands[Cell]
			: bOrdersShown && Orders.IsValidIndex(Cell) ? Orders[Cell] : FHOLGridCommand();
		ShownGridTypes[Cell] = !bGridShown ? NAME_None : Command.Command == EHOLGridCommand::CancelConstruction ? CancelName : Command.TypeName;
		if (ShownOrderTypes.IsValidIndex(Cell))
		{
			ShownOrderTypes[Cell] = Command.Command == EHOLGridCommand::AttackMove ? FName(TEXT("AttackMove"))
				: Command.Command == EHOLGridCommand::Stop ? FName(TEXT("Stop")) : Command.Command == EHOLGridCommand::Hold ? FName(TEXT("Hold")) : NAME_None;
		}
		UHOLGridCell* GridCell = GridCells[Cell];
		if (!GridCell)
		{
			continue;
		}
		switch (Command.Command)
		{
		case EHOLGridCommand::Produce:
		{
			// Greyed when the server would refuse it (D282): queue full (every queue of a group) or not enough resources;
			// still clickable, the click says why (D313). The population is only checked when the unit starts: a full
			// population makes it wait.
			const FHOLUnitTypeRow* UnitType = FindUnitType(Command.TypeName);
			const FHOLResources Cost = UnitType ? UnitType->Cost : FHOLResources();
			const FHOLResources Missing = GetMissing(Stock, Cost);
			const bool bQueueFull = !PlayerController || !PlayerController->FindProductionBuilding(Command.TypeName);
			const FText Name = HOLGameNames::GetUnitName(Command.TypeName, UnitType);
			GridCell->SetCommand(Name, bQueueFull || !(Missing == FHOLResources()) ? EHOLGridCellState::Greyed : EHOLGridCellState::Ready);
			GridCell->SetTooltip(Name, Cost, Missing, GetUnitEffect(Command.TypeName));
			break;
		}
		case EHOLGridCommand::Build:
		{
			// The builders' grid (D313): greyed when the stock does not cover the cost (in red in the tooltip).
			const FHOLBuildingTypeRow* BuildType = UHOLBuildingSettings::FindType(Command.TypeName);
			const FHOLResources Cost = BuildType ? BuildType->Cost : FHOLResources();
			const FHOLResources Missing = GetMissing(Stock, Cost);
			const FText Name = HOLGameNames::GetBuildingName(Command.TypeName, BuildType);
			GridCell->SetCommand(Name, Missing == FHOLResources() ? EHOLGridCellState::Ready : EHOLGridCellState::Greyed);
			GridCell->SetTooltip(Name, Cost, Missing, GetBuildingEffect(BuildType));
			break;
		}
		case EHOLGridCommand::CancelConstruction:
		{
			// The tooltip is its name alone: no amount, no effect line (the user's choice; the cancel refunds the share of
			// the cost not built, D345).
			GridCell->SetCommand(LOCTEXT("Cancel", "Annuler"), EHOLGridCellState::Ready);
			GridCell->SetTooltip(LOCTEXT("CancelSite", "Annuler la construction"), FHOLResources(), FHOLResources(), FText());
			break;
		}
		case EHOLGridCommand::AttackMove:
			GridCell->SetCommand(LOCTEXT("AttackMove", "Attaque"), EHOLGridCellState::Ready);
			GridCell->SetTooltip(LOCTEXT("AttackMoveName", "Attaque-déplacement"), FHOLResources(), FHOLResources(),
				LOCTEXT("AttackMoveEffect", "Puis clic au sol : attaque les ennemis en chemin"));
			break;
		case EHOLGridCommand::Stop:
			GridCell->SetCommand(LOCTEXT("Stop", "Arrêt"), EHOLGridCellState::Ready);
			GridCell->SetTooltip(LOCTEXT("StopName", "Arrêt"), FHOLResources(), FHOLResources(), LOCTEXT("StopEffect", "Annule les ordres et le combat"));
			break;
		case EHOLGridCommand::Hold:
			GridCell->SetCommand(LOCTEXT("Hold", "Tenir"), EHOLGridCellState::Ready);
			GridCell->SetTooltip(LOCTEXT("HoldName", "Tenir la position"), FHOLResources(), FHOLResources(),
				LOCTEXT("HoldEffect", "N'attaque qu'à portée, ne poursuit pas"));
			break;
		default:
			// An empty cell stays, dark (disabled), at its key's place.
			GridCell->ClearUnit();
			break;
		}
	}
}

void UHOLGameScreen::RefreshAges(const AHOLPlayerState* PlayerState)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// The builders' age panels (the user's trials of G.1, AoE4's way), right of the command panel: I to IV side by side,
	// each its medallion (numeral, key; no click) on its top edge over its buildings at their keys' places; only the
	// columns and rows its buildings use (an empty age: its medallion alone); an age not reached faded, medallion and
	// buildings; the chosen one's medallion and edge lit.
	const AHOLPlayerController* PlayerController = Controller.Get();
	FHOLGridContext Context;
	if (PlayerController && !PlayerController->GetOwnSelection() && !PlayerController->HasBuildingGroup())
	{
		PlayerController->GetUnitsContext(Context);
	}
	bAgesShown = HOLCommandGrid::IsBuildGrid(Context);
	ShownChosenAge = bAgesShown && PlayerController ? PlayerController->GetChosenAge() : INDEX_NONE;
	const FHOLResources Stock = PlayerState ? PlayerState->GetResources() : FHOLResources();
	for (int32 Age = 0; Age < HOLCommandGrid::NumAges; ++Age)
	{
		UBorder* AgePanel = AgePanels.IsValidIndex(Age) ? AgePanels[Age].Get() : nullptr;
		SetShown(AgePanel, bAgesShown, ESlateVisibility::Visible);
		const bool bReached = HOLCommandGrid::IsAgeReached(Context, Age);
		ShownAgeReached[Age] = bReached;
		// The medallion's look, written when it changed: its disc (lit when chosen), the panel's edge, the panel faded.
		const int32 Look = !bReached ? EMedallionLook::Locked : Age == ShownChosenAge ? EMedallionLook::Chosen : EMedallionLook::Reached;
		UBorder* Medallion = AgeMedallions.IsValidIndex(Age) ? AgeMedallions[Age].Get() : nullptr;
		if (ShownAgeLooks.IsValidIndex(Age) && ShownAgeLooks[Age] != Look)
		{
			ShownAgeLooks[Age] = Look;
			const bool bChosen = Look == EMedallionLook::Chosen;
			if (Medallion)
			{
				Medallion->SetBrush(FSlateRoundedBoxBrush(bChosen ? HOLUIStyle::Accent : FLinearColor(0.1f, 0.11f, 0.14f, 1.f), MedallionSize / 2.f,
					bChosen ? HOLUIStyle::Text : HOLUIStyle::TextMuted, 1.5f));
			}
			if (AgePanel)
			{
				AgePanel->SetBrush(bChosen ? FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel, 4.f, HOLUIStyle::Accent, 2.f) : FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel, 4.f));
				AgePanel->SetRenderOpacity(Look == EMedallionLook::Locked ? 0.55f : 1.f);
			}
		}
		// « Palier II — Essor », and « Palier non atteint » under it while not reached; read on every refresh (the language may
		// change), written when it changed.
		const FText AgeName = FText::Format(LOCTEXT("AgeHeaderName", "Palier {0} — {1}"), FText::AsCultureInvariant(FString(AgeNumerals[Age])), GetAgeName(Age));
		const FText Locked = LOCTEXT("AgeLocked", "Palier non atteint");
		const FString Tooltip = bReached ? AgeName.ToString() : FString::Printf(TEXT("%s | %s"), *AgeName.ToString(), *Locked.ToString());
		if (ShownAgeTooltips.IsValidIndex(Age) && ShownAgeTooltips[Age] != Tooltip)
		{
			ShownAgeTooltips[Age] = Tooltip;
			if (Medallion)
			{
				Medallion->SetToolTipText(bReached ? AgeName : FText::Format(INVTEXT("{0}\n{1}"), AgeName, Locked));
			}
		}
		// The columns its buildings use, packed (the panel the width of its content; an empty age one column); its 3 rows
		// always kept, empty or not (the user's 3rd trial of G.1): a category always at the same height.
		bool bUsedColumns[GridColumns] = {};
		TArray<FHOLGridCommand> Buildings;
		for (int32 Index = 0; Index < NumAgeCells; ++Index)
		{
			const FHOLGridCommand& Building = Buildings.Add_GetRef(HOLCommandGrid::GetAgeBuilding(Context, Age, Index));
			bUsedColumns[Index % GridColumns] |= !Building.IsNone();
		}
		bUsedColumns[0] |= !bUsedColumns[1] && !bUsedColumns[2] && !bUsedColumns[3];
		for (int32 Index = 0; Index < NumAgeCells; ++Index)
		{
			const int32 AgeIndex = Age * NumAgeCells + Index;
			const FHOLGridCommand& Building = Buildings[Index];
			ShownAgeTypes[AgeIndex] = bAgesShown ? Building.TypeName : NAME_None;
			UHOLGridCell* AgeCell = AgeCells.IsValidIndex(AgeIndex) ? AgeCells[AgeIndex].Get() : nullptr;
			USizeBox* Box = AgeCellBoxes.IsValidIndex(AgeIndex) ? AgeCellBoxes[AgeIndex].Get() : nullptr;
			const int32 Column = Index % GridColumns;
			const int32 Row = Index / GridColumns;
			if (Box)
			{
				// Unused columns collapsed; a cell of a used one without a building shown dark (disabled), as the commands'.
				Box->SetVisibility(bUsedColumns[Column] ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
				if (UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(Box->Slot))
				{
					int32 PackedColumn = 0;
					for (int32 Before = 0; Before < Column; ++Before)
					{
						PackedColumn += bUsedColumns[Before] ? 1 : 0;
					}
					GridSlot->SetColumn(PackedColumn);
					GridSlot->SetRow(Row);
				}
			}
			if (!AgeCell)
			{
				continue;
			}
			if (Building.IsNone())
			{
				AgeCell->ClearUnit();
				continue;
			}
			// Greyed when its age is not reached, or when the stock does not cover the cost (in red in the tooltip, D313).
			const FHOLBuildingTypeRow* BuildType = UHOLBuildingSettings::FindType(Building.TypeName);
			const FHOLResources Cost = BuildType ? BuildType->Cost : FHOLResources();
			const FHOLResources Missing = GetMissing(Stock, Cost);
			const FText Name = HOLGameNames::GetBuildingName(Building.TypeName, BuildType);
			AgeCell->SetCommand(Name, !Building.bGreyed && Missing == FHOLResources() ? EHOLGridCellState::Ready : EHOLGridCellState::Greyed);
			AgeCell->SetTooltip(Name, Cost, Missing, Building.bGreyed ? LOCTEXT("AgeLocked", "Palier non atteint") : GetBuildingEffect(BuildType));
		}
	}
}

void UHOLGameScreen::RefreshQueue(const AHOLBuilding* OwnBuilding)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	ShownQueueTypes.Reset();
	if (OwnBuilding)
	{
		for (const FHOLProductionItem& Item : OwnBuilding->GetProductionQueue())
		{
			ShownQueueTypes.Add(Item.TypeName);
		}
	}
	bQueueShown = !ShownQueueTypes.IsEmpty();
	SetShown(QueuePanel, bQueueShown, ESlateVisibility::Visible);

	for (int32 Index = 0; Index < QueueItems.Num(); ++Index)
	{
		UHOLQueueItem* Item = QueueItems[Index];
		const bool bShown = ShownQueueTypes.IsValidIndex(Index);
		SetShown(Item, bShown, ESlateVisibility::Visible);
		if (Item && bShown)
		{
			Item->SetLabel(HOLGameNames::GetUnitName(ShownQueueTypes[Index], FindUnitType(ShownQueueTypes[Index])));
		}
	}
}

void UHOLGameScreen::UpdateHead()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// The queue shown is the selected building's, which is then the player's own.
	const AHOLBuilding* Building = bQueueShown ? BoundBuilding.Get() : nullptr;
	const TArray<FHOLProductionItem>* Queue = Building ? &Building->GetProductionQueue() : nullptr;
	if (!Queue || Queue->IsEmpty())
	{
		HeadState = EHeadState::None;
		HeadProgress = 0.f;
		return;
	}

	const FHOLProductionItem& Head = (*Queue)[0];
	const FHOLUnitTypeRow* UnitType = FindUnitType(Head.TypeName);
	if (Head.IsStarted())
	{
		// Only the start step travels (D282): the progress is computed here every frame, at the server's clock as this
		// machine estimates it (D271).
		const int32 BuildSteps = UnitType
			? UHOLProductionSubsystem::GetBuildSteps(*UnitType, GetDefault<UHOLSimulationSettings>()->GetStepSeconds())
			: 1;
		HeadProgress = HOLProduction::GetProgress(Head, UHOLProductionSubsystem::GetDisplayStep(GetWorld()), BuildSteps);
		HeadState = EHeadState::InProgress;
	}
	else
	{
		// Not started: it waits for room in the population (as in AoE), or starts at the server's next step.
		const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
		const int32 Places = UnitType ? UnitType->Population : 1;
		HeadState = PlayerState && PlayerState->GetPopulationUsed() + Places > PlayerState->GetPopulationCap()
			? EHeadState::PopulationFull
			: EHeadState::Starting;
		HeadProgress = 0.f;
	}

	if (QueueProgress && QueueProgress->GetPercent() != HeadProgress)
	{
		QueueProgress->SetPercent(HeadProgress);
	}
	SetShown(QueueProgress, HeadState != EHeadState::PopulationFull);
	SetShown(QueueNote, HeadState == EHeadState::PopulationFull);
}

void UHOLGameScreen::HandleGridCellClicked(const int32 Cell)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	AHOLPlayerController* PlayerController = Controller.Get();
	if (!PlayerController || Cell < 0 || Cell >= AHOLPlayerController::NumGridCells)
	{
		return;
	}
	// The player's units: the panel shows their orders (never with a building's grid).
	if (!bGridShown && bOrdersShown && ShownOrderTypes.IsValidIndex(Cell) && !ShownOrderTypes[Cell].IsNone())
	{
		UE_LOG(LogHOLUI, Display, TEXT("HUD: order cell %s clicked"), GridKeys[Cell]);
		PlayerController->ActivateOrderCell(Cell);
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("HUD: grid cell %s clicked"), GridKeys[Cell]);
	PlayerController->ActivateGridCell(Cell);
}

void UHOLGameScreen::HandleAgeClicked(const int32 Age, const int32 Cell)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	AHOLPlayerController* PlayerController = Controller.Get();
	if (PlayerController && Age >= 0 && Age < HOLCommandGrid::NumAges)
	{
		UE_LOG(LogHOLUI, Display, TEXT("HUD: age %s %s clicked"), AgeNumerals[Age], Cell >= 0 && Cell < AHOLPlayerController::NumGridCells ? GridKeys[Cell] : TEXT("?"));
		PlayerController->ClickAge(Age, Cell);
	}
}

void UHOLGameScreen::HandleCommandFeedback(const FHOLCommandFeedback& Feedback)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	if (!Feedback.IsNone())
	{
		ShowMessage(GetFeedbackKey(Feedback), GetFeedbackText(Feedback));
	}
}

void UHOLGameScreen::ShowMessage(const FString& Key, const FText& Text)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	MessageKey = Key;
	MessageAge = 0.f;
	if (MessageText)
	{
		MessageText->SetText(Text);
	}
	if (MessagePanel)
	{
		MessagePanel->SetRenderOpacity(1.f);
	}
	SetShown(MessagePanel, true);
	UE_LOG(LogHOLUI, Display, TEXT("HUD message: %s \"%s\""), *Key, *Text.ToString());
}

void UHOLGameScreen::UpdateMessage(const float DeltaSeconds)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	if (MessageKey.IsEmpty())
	{
		return;
	}
	MessageAge += DeltaSeconds;
	if (MessageAge >= MessageSeconds)
	{
		MessageKey.Reset();
		SetShown(MessagePanel, false);
		return;
	}
	const float Left = MessageSeconds - MessageAge;
	if (MessagePanel)
	{
		MessagePanel->SetRenderOpacity(Left < MessageFadeSeconds ? Left / MessageFadeSeconds : 1.f);
	}
}

void UHOLGameScreen::CheckPopulationBlocked()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// The player's buildings whose next unit waits for room (their queues reach their owner alone, D282).
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	const UWorld* World = GetWorld();
	bool bBlocked = false;
	if (PlayerState && World)
	{
		for (TActorIterator<AHOLBuilding> It(World); It && !bBlocked; ++It)
		{
			const TArray<FHOLProductionItem>& Queue = It->GetProductionQueue();
			if (It->GetOwnerPlayer() != PlayerState->GetPlayerIndex() || !It->IsFinished() || Queue.IsEmpty() || Queue[0].IsStarted())
			{
				continue;
			}
			const FHOLUnitTypeRow* UnitType = FindUnitType(Queue[0].TypeName);
			bBlocked = HOLCommandGrid::IsPopulationBlocked(PlayerState->GetPopulationUsed(), PlayerState->GetPopulationCap(), UnitType ? UnitType->Population : 1);
		}
	}
	if (bBlocked && !bPopulationBlocked)
	{
		// The production stops (D313): the player's first wall.
		FHOLCommandFeedback Feedback;
		Feedback.Kind = EHOLCommandFeedback::PopulationFull;
		ShowMessage(GetFeedbackKey(Feedback), GetFeedbackText(Feedback));
	}
	bPopulationBlocked = bBlocked;
}

void UHOLGameScreen::HandleQueueItemClicked(const int32 Index)
{
	if (AHOLPlayerController* PlayerController = Controller.Get())
	{
		UE_LOG(LogHOLUI, Display, TEXT("HUD: queue item %d clicked (cancel)"), Index);
		PlayerController->RequestCancelProduction(Index);
	}
}

void UHOLGameScreen::HandleMenuClicked()
{
	UHOLGameMenu::Open(GetOwningPlayer());
}

void UHOLGameScreen::HandleStatsClicked()
{
	if (StatsButton && StatsButton->GetVisibility() != ESlateVisibility::Collapsed)
	{
		UHOLStatsScreen::Open(GetOwningPlayer());
	}
}

void UHOLGameScreen::RefreshStatsButton()
{
	const AHOLPlayerController* PlayerController = Controller.Get();
	const AHOLGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AHOLGameState>() : nullptr;
	const bool bShown = PlayerController && HOLGameMenuRules::ShowsStatsButton(PlayerController->IsSpectator(),
		GameState && GameState->IsMatchOver());
	UE::HeroesOfLiberty::UI::GameScreen::SetShown(StatsButton, bShown, ESlateVisibility::Visible);
	// Its box too: collapsed, « Menu » keeps its place alone.
	UE::HeroesOfLiberty::UI::GameScreen::SetShown(StatsButton ? StatsButton->GetParent() : nullptr, bShown, ESlateVisibility::SelfHitTestInvisible);
}

bool UHOLGameScreen::ClickWidget(const FName Name)
{
	if (AlertPanel && Name == UE::HeroesOfLiberty::UI::GameScreen::AlertPanelName)
	{
		if (!Alert.bShown)
		{
			return false;
		}
		FocusAlertSlot(Alert.Slot);
		return true;
	}
	if (MenuButton && Name == UE::HeroesOfLiberty::UI::GameScreen::MenuButtonName)
	{
		HandleMenuClicked();
		return true;
	}
	if (StatsButton && Name == UE::HeroesOfLiberty::UI::GameScreen::StatsButtonName)
	{
		if (StatsButton->GetVisibility() == ESlateVisibility::Collapsed)
		{
			return false;
		}
		HandleStatsClicked();
		return true;
	}
	if (IdleWorkers && Name == UE::HeroesOfLiberty::UI::GameScreen::IdleWorkersName)
	{
		if (!IdleWorkers->GetIsEnabled())
		{
			return false;
		}
		HandleIdleWorkersClicked();
		return true;
	}
	for (int32 Group = 0; Group < GroupSlots.Num(); ++Group)
	{
		const UHOLQueueItem* GroupSlot = GroupSlots[Group];
		if (GroupSlot && GroupSlot->GetFName() == Name)
		{
			if (!ShownGroups.IsValidIndex(Group) || ShownGroups[Group].IsEmpty())
			{
				return false;
			}
			HandleGroupSlotClicked(Group);
			return true;
		}
	}
	for (int32 Cell = 0; Cell < GridCells.Num(); ++Cell)
	{
		const UHOLGridCell* GridCell = GridCells[Cell];
		if (GridCell && GridCell->GetFName() == Name)
		{
			if ((!bGridShown && !bOrdersShown) || !GridCell->GetIsEnabled())
			{
				return false;
			}
			HandleGridCellClicked(Cell);
			return true;
		}
	}
	// The builders' age panels' buildings, AgeCell<age><cell> (AgeCell000 the house, AgeCell004 the barracks); an age not reached takes the click
	// and does nothing, as a greyed cell. Their medallions take no click.
	for (int32 AgeIndex = 0; AgeIndex < AgeCells.Num(); ++AgeIndex)
	{
		const UHOLGridCell* AgeCell = AgeCells[AgeIndex];
		if (AgeCell && AgeCell->GetFName() == Name)
		{
			if (!bAgesShown || !ShownAgeTypes.IsValidIndex(AgeIndex) || ShownAgeTypes[AgeIndex].IsNone())
			{
				return false;
			}
			HandleAgeClicked(AgeIndex / UE::HeroesOfLiberty::UI::GameScreen::NumAgeCells, AgeIndex % UE::HeroesOfLiberty::UI::GameScreen::NumAgeCells);
			return true;
		}
	}
	for (int32 Index = 0; Index < QueueItems.Num(); ++Index)
	{
		const UHOLQueueItem* Item = QueueItems[Index];
		if (Item && Item->GetFName() == Name)
		{
			if (!bQueueShown || !Item->IsVisible())
			{
				return false;
			}
			HandleQueueItemClicked(Index);
			return true;
		}
	}
	return false;
}

FString UHOLGameScreen::Describe() const
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	const FString Economy = bEconomyShown
		? FString::Printf(TEXT("%s, population %d/%d"), *ShownResources.ToString(), ShownPopulationUsed, ShownPopulationCap)
		: FString(TEXT("none"));

	FString Grid = TEXT("hidden");
	FString Queue = TEXT("hidden");
	if (bAgesShown)
	{
		// The builders' age panels: "ages I=open [A=House enabled Z=LumberCamp greyed] II=locked [] ... (chosen I)".
		TArray<FString> Ages;
		for (int32 Age = 0; Age < HOLCommandGrid::NumAges; ++Age)
		{
			TArray<FString> Cells;
			for (int32 Index = 0; Index < NumAgeCells; ++Index)
			{
				const int32 AgeIndex = Age * NumAgeCells + Index;
				if (ShownAgeTypes.IsValidIndex(AgeIndex) && !ShownAgeTypes[AgeIndex].IsNone())
				{
					const EHOLGridCellState State = AgeCells.IsValidIndex(AgeIndex) && AgeCells[AgeIndex] ? AgeCells[AgeIndex]->GetState() : EHOLGridCellState::Disabled;
					Cells.Add(FString::Printf(TEXT("%s=%s %s"), GridKeys[Index], *ShownAgeTypes[AgeIndex].ToString(),
						State == EHOLGridCellState::Ready ? TEXT("enabled") : State == EHOLGridCellState::Greyed ? TEXT("greyed") : TEXT("disabled")));
				}
			}
			Ages.Add(FString::Printf(TEXT("%s=%s [%s]"), AgeNumerals[Age], ShownAgeReached.IsValidIndex(Age) && ShownAgeReached[Age] ? TEXT("open") : TEXT("locked"),
				*FString::Join(Cells, TEXT(" "))));
		}
		Grid = FString::Printf(TEXT("ages %s (chosen %s)"), *FString::Join(Ages, TEXT(" ")),
			ShownChosenAge >= 0 && ShownChosenAge < HOLCommandGrid::NumAges ? AgeNumerals[ShownChosenAge] : TEXT("none"));
	}
	if (bGridShown)
	{
		TArray<FString> Cells;
		for (int32 Cell = 0; Cell < ShownGridTypes.Num(); ++Cell)
		{
			if (!ShownGridTypes[Cell].IsNone())
			{
				const EHOLGridCellState State = GridCells.IsValidIndex(Cell) && GridCells[Cell] ? GridCells[Cell]->GetState() : EHOLGridCellState::Disabled;
				Cells.Add(FString::Printf(TEXT("%s=%s %s"), GridKeys[Cell], *ShownGridTypes[Cell].ToString(), State == EHOLGridCellState::Ready ? TEXT("enabled")
					: State == EHOLGridCellState::Greyed ? TEXT("greyed") : TEXT("disabled")));
			}
		}
		Grid = Cells.IsEmpty() ? FString(TEXT("empty")) : FString::Join(Cells, TEXT(" "));

		TArray<FString> Items;
		for (int32 Index = 0; Index < ShownQueueTypes.Num(); ++Index)
		{
			FString State = TEXT("waiting");
			if (Index == 0)
			{
				State = HeadState == EHeadState::InProgress ? FString::Printf(TEXT("%.2f"), HeadProgress)
					: HeadState == EHeadState::PopulationFull ? FString(TEXT("population full"))
					: FString(TEXT("starting"));
			}
			Items.Add(FString::Printf(TEXT("%s %s"), *ShownQueueTypes[Index].ToString(), *State));
		}
		Queue = FString::Printf(TEXT("%d [%s]"), Items.Num(), *FString::Join(Items, TEXT(", ")));
	}
	// The orders' panel (the user's choice of 2026-10-21), after the grid: Q=AttackMove S=Stop V=Hold.
	TArray<FString> Orders;
	for (int32 Cell = 0; bOrdersShown && Cell < ShownOrderTypes.Num(); ++Cell)
	{
		if (!ShownOrderTypes[Cell].IsNone())
		{
			Orders.Add(FString::Printf(TEXT("%s=%s"), GridKeys[Cell], *ShownOrderTypes[Cell].ToString()));
		}
	}
	Grid += FString::Printf(TEXT(", orders %s"), Orders.IsEmpty() ? TEXT("none") : *FString::Join(Orders, TEXT(" ")));
	// The gatherers and the idle workers last (J5b): the checks of J3 read the line from its start.
	const FString Workers = bEconomyShown
		? FString::Printf(TEXT(", gatherers %s, idle %d"), *ShownGatherers.ToString(), FMath::Max(ShownIdleWorkers, 0))
		: FString();
	// The short message shown (D313), after the queue: the checks of J3 read the line from its start.
	const FString Message = MessageKey.IsEmpty() ? FString() : FString::Printf(TEXT(", message %s"), *MessageKey);
	// A main forum's lives and state (V2.3b) and the players' table (V2.4): on their own line (DescribeForum).
	const FString Activity = !ShownForum.IsEmpty() && ShownActivity == ShownForum ? FString() : ShownActivity;
	return FString::Printf(TEXT("resources %s, selection %s%s%s, grid %s, queue %s%s%s"), *Economy, *ShownSelection, *Activity, *ShownHealth, *Grid, *Queue,
		*Message, *Workers);
}

FString UHOLGameScreen::DescribeForum() const
{
	// "selection, in ruin 27 s, lives 1, players [slot 0 you colour 0 lives 1 ruin | ...]" ("selection none" without a main forum selected).
	const bool bForum = !ShownForum.IsEmpty() && ShownActivity == ShownForum;
	return FString::Printf(TEXT("selection%s, players [%s]"), bForum ? *ShownForum : TEXT(" none"), *ShownPlayersTable);
}

FString UHOLGameScreen::DescribeCells() const
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	TArray<FString> Parts;
	for (int32 Cell = 0; Cell < GridCells.Num(); ++Cell)
	{
		const FString Content = (bGridShown || bOrdersShown) && GridCells[Cell] ? GridCells[Cell]->DescribeContent() : FString();
		if (!Content.IsEmpty())
		{
			Parts.Add(FString::Printf(TEXT("%s: %s"), GridKeys[Cell], *Content.Replace(TEXT("\n"), TEXT(" "))));
		}
	}
	// The builders' age panels' buildings (their medallions are no cells): "I A: Maison; I Z: Camp de bûcherons; ...".
	for (int32 AgeIndex = 0; bAgesShown && AgeIndex < AgeCells.Num(); ++AgeIndex)
	{
		const UHOLGridCell* Cell = ShownAgeTypes.IsValidIndex(AgeIndex) && !ShownAgeTypes[AgeIndex].IsNone() ? AgeCells[AgeIndex].Get() : nullptr;
		const FString Content = Cell ? Cell->DescribeContent() : FString();
		if (!Content.IsEmpty())
		{
			Parts.Add(FString::Printf(TEXT("%s %s: %s"), AgeNumerals[AgeIndex / NumAgeCells], GridKeys[AgeIndex % NumAgeCells],
				*Content.Replace(TEXT("\n"), TEXT(" "))));
		}
	}
	return Parts.IsEmpty() ? FString(TEXT("none")) : FString::Join(Parts, TEXT("; "));
}

FString UHOLGameScreen::DescribeTooltips() const
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	TArray<FString> Parts;
	for (int32 Cell = 0; Cell < GridCells.Num(); ++Cell)
	{
		const FString Tooltip = (bGridShown || bOrdersShown) && GridCells[Cell] ? GridCells[Cell]->DescribeTooltip() : FString();
		if (!Tooltip.IsEmpty())
		{
			Parts.Add(FString::Printf(TEXT("%s: %s"), GridKeys[Cell], *Tooltip));
		}
	}
	// The builders' age panels: each medallion's, then its buildings': "I A: Palier I — Fondation; I A: Maison | W 50 | ...".
	for (int32 Age = 0; bAgesShown && Age < HOLCommandGrid::NumAges; ++Age)
	{
		for (int32 Index = INDEX_NONE; Index < NumAgeCells; ++Index)
		{
			const int32 AgeIndex = Age * NumAgeCells + Index;
			const UHOLGridCell* Cell = Index != INDEX_NONE && AgeCells.IsValidIndex(AgeIndex) && ShownAgeTypes.IsValidIndex(AgeIndex)
				&& !ShownAgeTypes[AgeIndex].IsNone() ? AgeCells[AgeIndex].Get() : nullptr;
			const FString Tooltip = Index == INDEX_NONE ? (ShownAgeTooltips.IsValidIndex(Age) ? ShownAgeTooltips[Age] : FString()) : Cell ? Cell->DescribeTooltip() : FString();
			if (!Tooltip.IsEmpty())
			{
				Parts.Add(FString::Printf(TEXT("%s %s: %s"), AgeNumerals[Age], GridKeys[Index == INDEX_NONE ? Age : Index],
					*Tooltip.Replace(TEXT("\n"), TEXT(" "))));
			}
		}
	}
	return Parts.IsEmpty() ? FString(TEXT("none")) : FString::Join(Parts, TEXT("; "));
}

const FHOLUnitTypeRow* UHOLGameScreen::FindUnitType(const FName TypeName) const
{
	const UWorld* World = GetWorld();
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	return Units ? Units->FindTypeRow(TypeName) : nullptr;
}

void UHOLGameScreen::BuildLayout()
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// Only the panels are hit-test visible: the frame and the boxes that place them let the clicks through to the game.
	UOverlay* Frame = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Frame"));
	Frame->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = Frame;

	const auto MakePanel = [this](const FName Name)
	{
		UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		Panel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel, 4.f));
		Panel->SetPadding(FMargin(12.f, 8.f));
		Panel->SetVisibility(ESlateVisibility::Visible);
		return Panel;
	};
	const auto MakeText = [this](const FName Name, const TSubclassOf<UCommonTextStyle> TextStyle, const FText& Text)
	{
		UCommonTextBlock* Block = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), Name);
		Block->SetStyle(TextStyle);
		Block->SetText(Text);
		return Block;
	};
	const auto MakeSquare = [this](const FName Name, const FLinearColor& Color, const float Size)
	{
		UImage* Square = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
		Square->SetColorAndOpacity(Color);
		Square->SetDesiredSizeOverride(FVector2D(Size, Size));
		return Square;
	};
	const auto MakeHorizontalBox = [this](const FName Name)
	{
		return WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), Name);
	};
	const auto MakeVerticalBox = [this](const FName Name)
	{
		return WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), Name);
	};
	const auto AddToRow = [](UHorizontalBox* Row, UWidget* Widget, const float RightPadding)
	{
		UHorizontalBoxSlot* WidgetSlot = Row->AddChildToHorizontalBox(Widget);
		WidgetSlot->SetVerticalAlignment(VAlign_Center);
		WidgetSlot->SetPadding(FMargin(0.f, 0.f, RightPadding, 0.f));
	};

	// Top bar, top left: each resource's marker, name and amount, then the population.
	TopBar = MakePanel(TEXT("TopBar"));
	UOverlaySlot* TopBarSlot = Frame->AddChildToOverlay(TopBar);
	TopBarSlot->SetHorizontalAlignment(HAlign_Left);
	TopBarSlot->SetVerticalAlignment(VAlign_Top);
	TopBarSlot->SetPadding(FMargin(ScreenMargin));
	UHorizontalBox* Resources = MakeHorizontalBox(TEXT("Resources"));
	TopBar->SetContent(Resources);
	// A resource's gatherers follow its amount (J5b), « (3) »; the population has none.
	const auto AddResource = [&](const FString& Name, const FLinearColor* Color, const FText& Label, UCommonTextBlock** OutGatherers)
	{
		if (Color)
		{
			AddToRow(Resources, MakeSquare(*(Name + TEXT("Marker")), *Color, MarkerSize), 6.f);
		}
		AddToRow(Resources, MakeText(*(Name + TEXT("Label")), UHOLTextStyleHUDLabel::StaticClass(), Label), 6.f);
		UCommonTextBlock* Value = MakeText(*(Name + TEXT("Amount")), UHOLTextStyleHUD::StaticClass(), FText::GetEmpty());
		AddToRow(Resources, Value, OutGatherers ? 4.f : 20.f);
		if (OutGatherers)
		{
			*OutGatherers = MakeText(*(Name + TEXT("Gatherers")), UHOLTextStyleHUDLabel::StaticClass(), FText::GetEmpty());
			AddToRow(Resources, *OutGatherers, 20.f);
		}
		return Value;
	};
	UCommonTextBlock* Gatherers[4] = {};
	FoodAmount = AddResource(TEXT("Food"), &HOLUIStyle::Food, LOCTEXT("Food", "Nourriture"), &Gatherers[0]);
	WoodAmount = AddResource(TEXT("Wood"), &HOLUIStyle::Wood, LOCTEXT("Wood", "Bois"), &Gatherers[1]);
	StoneAmount = AddResource(TEXT("Stone"), &HOLUIStyle::Stone, LOCTEXT("Stone", "Pierre"), &Gatherers[2]);
	GoldAmount = AddResource(TEXT("Gold"), &HOLUIStyle::Gold, LOCTEXT("Gold", "Or"), &Gatherers[3]);
	FoodGatherers = Gatherers[0];
	WoodGatherers = Gatherers[1];
	StoneGatherers = Gatherers[2];
	GoldGatherers = Gatherers[3];
	PopulationAmount = AddResource(TEXT("Population"), nullptr, LOCTEXT("Population", "Population"), nullptr);

	// The idle workers' counter, a button (J5b, D298): a click selects the next one, as ".".
	USizeBox* IdleBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("IdleWorkersBox"));
	IdleBox->SetWidthOverride(IdleWorkersWidth);
	IdleBox->SetHeightOverride(QueueItemHeight * 0.8f);
	IdleWorkers = WidgetTree->ConstructWidget<UHOLQueueItem>(UHOLQueueItem::StaticClass(), IdleWorkersName);
	IdleBox->SetContent(IdleWorkers);
	AddToRow(Resources, IdleBox, 0.f);

	// The in-game menu's button (D356), top right: opens the menu, as Escape with nothing selected.
	UBorder* MenuPanel = MakePanel(TEXT("MenuPanel"));
	MenuPanel->SetPadding(FMargin(6.f, 4.f));
	UOverlaySlot* MenuPanelSlot = Frame->AddChildToOverlay(MenuPanel);
	MenuPanelSlot->SetHorizontalAlignment(HAlign_Right);
	MenuPanelSlot->SetVerticalAlignment(VAlign_Top);
	MenuPanelSlot->SetPadding(FMargin(ScreenMargin));
	USizeBox* MenuBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MenuButtonBox"));
	MenuBox->SetWidthOverride(IdleWorkersWidth * 0.75f);
	MenuBox->SetHeightOverride(QueueItemHeight * 0.8f);
	UHOLQueueItem* Menu = WidgetTree->ConstructWidget<UHOLQueueItem>(UHOLQueueItem::StaticClass(), MenuButtonName);
	MenuBox->SetContent(Menu);
	// M6-Q10 (C6): « Statistiques » left of « Menu », a spectator's alone during the match (collapsed otherwise).
	UHorizontalBox* MenuRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MenuRow"));
	USizeBox* StatsBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("StatsButtonBox"));
	StatsBox->SetWidthOverride(IdleWorkersWidth * 1.25f);
	StatsBox->SetHeightOverride(QueueItemHeight * 0.8f);
	UHOLQueueItem* Stats = WidgetTree->ConstructWidget<UHOLQueueItem>(UHOLQueueItem::StaticClass(), StatsButtonName);
	StatsBox->SetContent(Stats);
	UHorizontalBoxSlot* StatsSlot = MenuRow->AddChildToHorizontalBox(StatsBox);
	StatsSlot->SetPadding(FMargin(0.f, 0.f, 6.f, 0.f));
	MenuRow->AddChildToHorizontalBox(MenuBox);
	MenuPanel->SetContent(MenuRow);

	// R3.10: the wait banner, top centre under the top bar, while the match waits for its named players.
	WaitPanel = MakePanel(TEXT("WaitPanel"));
	WaitPanel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel.CopyWithNewOpacity(0.8f), 4.f));
	WaitPanel->SetVisibility(ESlateVisibility::Collapsed);
	UOverlaySlot* WaitSlot = Frame->AddChildToOverlay(WaitPanel);
	WaitSlot->SetHorizontalAlignment(HAlign_Center);
	WaitSlot->SetVerticalAlignment(VAlign_Top);
	WaitSlot->SetPadding(FMargin(ScreenMargin, ScreenMargin + QueueItemHeight * 2.f, ScreenMargin, ScreenMargin));
	WaitText = MakeText(TEXT("WaitText"), UHOLTextStyleHUDHeading::StaticClass(), FText::GetEmpty());
	WaitPanel->SetContent(WaitText);

	// R7.5: the disconnection banner, top centre under the alert's place (both can show during the match), as the wait
	// banner; lets the clicks through.
	AbsencePanel = MakePanel(TEXT("AbsencePanel"));
	AbsencePanel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel.CopyWithNewOpacity(0.8f), 4.f));
	AbsencePanel->SetVisibility(ESlateVisibility::Collapsed);
	UOverlaySlot* AbsenceSlot = Frame->AddChildToOverlay(AbsencePanel);
	AbsenceSlot->SetHorizontalAlignment(HAlign_Center);
	AbsenceSlot->SetVerticalAlignment(VAlign_Top);
	AbsenceSlot->SetPadding(FMargin(ScreenMargin, ScreenMargin + QueueItemHeight * 2.f + 52.f, ScreenMargin, ScreenMargin));
	AbsenceText = MakeText(TEXT("AbsenceText"), UHOLTextStyleHUDHeading::StaticClass(), FText::GetEmpty());
	AbsenceText->SetJustification(ETextJustify::Center);
	AbsencePanel->SetContent(AbsenceText);

	// V4.3: the major alert's banner, top centre under the top bar (the wait banner's place, before the match), larger than
	// the thread's lines; a click centres the camera on the touched forum.
	AlertPanel = MakePanel(AlertPanelName);
	AlertPanel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel.CopyWithNewOpacity(0.85f), 4.f));
	AlertPanel->SetPadding(FMargin(16.f, 6.f));
	AlertPanel->SetVisibility(ESlateVisibility::Collapsed);
	AlertPanel->OnMouseButtonDownEvent.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHOLGameScreen, HandleAlertMouseDown));
	UOverlaySlot* AlertSlot = Frame->AddChildToOverlay(AlertPanel);
	AlertSlot->SetHorizontalAlignment(HAlign_Center);
	AlertSlot->SetVerticalAlignment(VAlign_Top);
	AlertSlot->SetPadding(FMargin(ScreenMargin, ScreenMargin + QueueItemHeight * 2.f, ScreenMargin, ScreenMargin));
	AlertRow = MakeHorizontalBox(TEXT("AlertRow"));
	AlertPanel->SetContent(AlertRow);

	// The players' table (V2.4, V2-Q4: AoE2's score), top right under the menu's button (the minimap holds the bottom
	// right): a row per occupied slot, its name in its colour, its lives, its forum's phase. Small, it lets the clicks through.
	PlayersPanel = MakePanel(TEXT("PlayersPanel"));
	// Darker than the other panels (V2.3d): the names in a dark colour (blue, green, violet) read on it.
	PlayersPanel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel.CopyWithNewOpacity(0.8f), 4.f));
	PlayersPanel->SetPadding(FMargin(8.f, 4.f));
	PlayersPanel->SetVisibility(ESlateVisibility::Collapsed);
	// V4.2: the table and, under it, the thread of messages, in one column right aligned (above the minimap's place).
	UVerticalBox* RightColumn = MakeVerticalBox(TEXT("RightColumn"));
	RightColumn->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	UOverlaySlot* PlayersSlot = Frame->AddChildToOverlay(RightColumn);
	PlayersSlot->SetHorizontalAlignment(HAlign_Right);
	PlayersSlot->SetVerticalAlignment(VAlign_Top);
	PlayersSlot->SetPadding(FMargin(ScreenMargin, ScreenMargin + QueueItemHeight * 0.8f + 8.f + 6.f, ScreenMargin, ScreenMargin));
	RightColumn->AddChildToVerticalBox(PlayersPanel)->SetHorizontalAlignment(HAlign_Right);
	MessagesList = MakeVerticalBox(TEXT("MessagesList"));
	MessagesList->SetVisibility(ESlateVisibility::Collapsed);
	UVerticalBoxSlot* MessagesSlot = RightColumn->AddChildToVerticalBox(MessagesList);
	MessagesSlot->SetHorizontalAlignment(HAlign_Right);
	MessagesSlot->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
	MessageLines.Reset();
	MessageRows.Reset();
	for (int32 Index = 0; Index < FHOLForumThread::MaxLines; ++Index)
	{
		// A line's own panel, as dark as the table's (the names in a dark colour read on it).
		UBorder* Line = MakePanel(*FString::Printf(TEXT("MessageLine%d"), Index));
		Line->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel.CopyWithNewOpacity(0.8f), 4.f));
		Line->SetPadding(FMargin(8.f, 2.f));
		Line->SetVisibility(ESlateVisibility::Collapsed);
		Line->OnMouseButtonDownEvent.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHOLGameScreen, HandleMessageLineMouseDown));
		UHorizontalBox* Row = MakeHorizontalBox(*FString::Printf(TEXT("MessageRow%d"), Index));
		Line->SetContent(Row);
		UVerticalBoxSlot* LineSlot = MessagesList->AddChildToVerticalBox(Line);
		LineSlot->SetHorizontalAlignment(HAlign_Right);
		LineSlot->SetPadding(FMargin(0.f, Index > 0 ? 2.f : 0.f, 0.f, 0.f));
		MessageLines.Add(Line);
		MessageRows.Add(Row);
	}
	UVerticalBox* PlayersList = MakeVerticalBox(TEXT("PlayersList"));
	PlayersPanel->SetContent(PlayersList);
	PlayerRows.Reset();
	PlayerNames.Reset();
	PlayerRelations.Reset();
	AllyIcon = LoadObject<UTexture2D>(nullptr, HOLRelationIcons::AllyPath);
	EnemyIcon = LoadObject<UTexture2D>(nullptr, HOLRelationIcons::EnemyPath);
	PlayerStrikes.Reset();
	PlayerLives.Reset();
	PlayerStates.Reset();
	for (int32 Seat = 0; Seat < FHOLMatchConfig::MaxSlots; ++Seat)
	{
		UHorizontalBox* Row = MakeHorizontalBox(*FString::Printf(TEXT("PlayerRow%d"), Seat));
		Row->SetVisibility(ESlateVisibility::Collapsed);
		PlayersList->AddChildToVerticalBox(Row);
		const auto AddCell = [&](const TCHAR* Name, const TSubclassOf<UCommonTextStyle> TextStyle, const float Width, const float RightPadding)
		{
			UCommonTextBlock* Text = MakeText(*FString::Printf(TEXT("Player%s%d"), Name, Seat), TextStyle, FText::GetEmpty());
			USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("Player%sBox%d"), Name, Seat));
			Box->SetMinDesiredWidth(Width);
			Box->SetContent(Text);
			AddToRow(Row, Box, RightPadding);
			return Text;
		};
		PlayerRows.Add(Row);
		// E4-1: the relation's icon before the name (an ally's shield, an enemy's swords), set by RefreshPlayersTable.
		UImage* Relation = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *FString::Printf(TEXT("PlayerRelation%d"), Seat));
		Relation->SetDesiredSizeOverride(FVector2D(16.f, 16.f));
		Relation->SetVisibility(ESlateVisibility::Hidden);
		// A fixed 16 px room, kept on the viewer's own row so that the names line up (a hidden image alone gave none).
		USizeBox* RelationBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("PlayerRelationBox%d"), Seat));
		RelationBox->SetWidthOverride(16.f);
		RelationBox->SetHeightOverride(16.f);
		RelationBox->SetContent(Relation);
		RelationBox->SetVisibility(ESlateVisibility::Collapsed);
		AddToRow(Row, RelationBox, 4.f);
		PlayerRelations.Add(Relation);
		// E4-2: the name in an overlay with a thin bar across it, its width, shown once defeated (a text has no strike).
		UCommonTextBlock* Name = MakeText(*FString::Printf(TEXT("PlayerName%d"), Seat), UHOLTextStyleHUDPlayerName::StaticClass(), FText::GetEmpty());
		UOverlay* NameOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), *FString::Printf(TEXT("PlayerNameOverlay%d"), Seat));
		NameOverlay->AddChildToOverlay(Name);
		UImage* Strike = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *FString::Printf(TEXT("PlayerNameStrike%d"), Seat));
		Strike->SetVisibility(ESlateVisibility::Collapsed);
		// A size box holds the bar to 2 px: the image alone took the whole line's height (seen in a capture).
		USizeBox* StrikeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("PlayerNameStrikeBox%d"), Seat));
		StrikeBox->SetHeightOverride(2.f);
		StrikeBox->SetContent(Strike);
		UOverlaySlot* StrikeSlot = NameOverlay->AddChildToOverlay(StrikeBox);
		StrikeSlot->SetHorizontalAlignment(HAlign_Fill);
		StrikeSlot->SetVerticalAlignment(VAlign_Center);
		PlayerStrikes.Add(Strike);
		USizeBox* NameBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("PlayerNameBox%d"), Seat));
		NameBox->SetMinDesiredWidth(64.f);
		NameBox->SetContent(NameOverlay);
		// The overlay as wide as the name, not the box (the bar the name's width).
		if (USizeBoxSlot* NameSlot = Cast<USizeBoxSlot>(NameOverlay->Slot))
		{
			NameSlot->SetHorizontalAlignment(HAlign_Left);
		}
		AddToRow(Row, NameBox, 10.f);
		PlayerNames.Add(Name);
		PlayerLives.Add(AddCell(TEXT("Lives"), UHOLTextStyleHUDLabel::StaticClass(), 44.f, 10.f));
		PlayerStates.Add(AddCell(TEXT("State"), UHOLTextStyleHUDLabel::StaticClass(), 0.f, 0.f));
	}

	// The control groups' bar (J7a.5, D30), bottom left over the selection's block (the cheats' panel holds the top left):
	// a slot per group not empty, « 1 · 9 », in the keys' order.
	GroupsPanel = MakePanel(TEXT("GroupsPanel"));
	GroupsPanel->SetPadding(FMargin(6.f, 4.f));
	GroupsPanel->SetVisibility(ESlateVisibility::Collapsed);
	UOverlaySlot* GroupsSlot = Frame->AddChildToOverlay(GroupsPanel);
	GroupsSlot->SetHorizontalAlignment(HAlign_Left);
	GroupsSlot->SetVerticalAlignment(VAlign_Bottom);
	GroupsSlot->SetPadding(FMargin(ScreenMargin, 0.f, 0.f, ScreenMargin + BarHeight + BarGap));
	UHorizontalBox* GroupsRow = MakeHorizontalBox(TEXT("GroupsRow"));
	GroupsPanel->SetContent(GroupsRow);
	for (int32 Index = 1; Index <= AHOLPlayerController::NumControlGroups; ++Index)
	{
		const int32 Group = Index % AHOLPlayerController::NumControlGroups;
		USizeBox* GroupBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("ControlGroupBox%d"), Group));
		GroupBox->SetWidthOverride(GroupSlotWidth);
		GroupBox->SetHeightOverride(QueueItemHeight * 0.8f);
		UHOLQueueItem* GroupSlot = WidgetTree->ConstructWidget<UHOLQueueItem>(UHOLQueueItem::StaticClass(), GroupSlotName(Group));
		GroupBox->SetVisibility(ESlateVisibility::Collapsed);
		GroupSlot->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		GroupBox->SetContent(GroupSlot);
		AddToRow(GroupsRow, GroupBox, 4.f);
	}

	// The bottom bar, bottom left (the user's 2nd trial of G.1, AoE4's way, D201): the selection's block, the command
	// panel, the builders' age panels, each at a fixed place whatever the selection (a block hidden keeps its place).
	UHorizontalBox* BottomBar = MakeHorizontalBox(TEXT("BottomBar"));
	UOverlaySlot* BottomBarSlot = Frame->AddChildToOverlay(BottomBar);
	BottomBarSlot->SetHorizontalAlignment(HAlign_Left);
	BottomBarSlot->SetVerticalAlignment(VAlign_Bottom);
	BottomBarSlot->SetPadding(FMargin(ScreenMargin));
	const auto AddToBar = [BottomBar](UWidget* Widget, const float LeftPadding)
	{
		UHorizontalBoxSlot* WidgetSlot = BottomBar->AddChildToHorizontalBox(Widget);
		WidgetSlot->SetVerticalAlignment(VAlign_Bottom);
		WidgetSlot->SetPadding(FMargin(LeftPadding, 0.f, 0.f, 0.f));
	};

	// Selection block, a fixed width and the bar's height (a spacer holds its place while it is collapsed): name, owner
	// (with its colour), health.
	UOverlay* SelectionPlace = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("SelectionPlace"));
	SelectionPlace->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	AddToBar(SelectionPlace, 0.f);
	USpacer* SelectionSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("SelectionSpacer"));
	SelectionSpacer->SetSize(FVector2D(SelectionWidth, BarHeight));
	SelectionPlace->AddChildToOverlay(SelectionSpacer);
	SelectionPanel = MakePanel(TEXT("SelectionPanel"));
	UOverlaySlot* SelectionSlot = SelectionPlace->AddChildToOverlay(SelectionPanel);
	SelectionSlot->SetHorizontalAlignment(HAlign_Fill);
	SelectionSlot->SetVerticalAlignment(VAlign_Fill);
	USizeBox* SelectionBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SelectionBox"));
	SelectionBox->SetWidthOverride(SelectionWidth - 24.f);
	SelectionPanel->SetContent(SelectionBox);
	UVerticalBox* SelectionColumn = MakeVerticalBox(TEXT("SelectionColumn"));
	SelectionBox->SetContent(SelectionColumn);

	SelectionName = MakeText(TEXT("SelectionName"), UHOLTextStyleHUDHeading::StaticClass(), FText::GetEmpty());
	SelectionColumn->AddChildToVerticalBox(SelectionName)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
	// A group's chips (G.1e, AoE4's way), in place of the name: wrapping in the block's width, each at most half of it (a
	// long name cut short), in a frame lit for the active subgroup; a click makes it active, as Tab.
	SelectionChips = WidgetTree->ConstructWidget<UWrapBox>(UWrapBox::StaticClass(), TEXT("SelectionChips"));
	SelectionChips->SetInnerSlotPadding(FVector2D(ChipGap, ChipGap));
	SelectionChips->SetExplicitWrapSize(true);
	SelectionChips->SetWrapSize(SelectionWidth - 24.f);
	SelectionChips->SetVisibility(ESlateVisibility::Collapsed);
	SelectionColumn->AddChildToVerticalBox(SelectionChips)->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
	for (int32 Chip = 0; Chip < NumChips; ++Chip)
	{
		UBorder* ChipFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("SelectionChipFrame%d"), Chip));
		ChipFrame->SetBrush(FSlateRoundedBoxBrush(FLinearColor::White, 5.f));
		ChipFrame->SetBrushColor(FLinearColor::Transparent);
		ChipFrame->SetPadding(FMargin(2.f));
		ChipFrame->SetVisibility(ESlateVisibility::Collapsed);
		USizeBox* ChipBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ChipBox->SetHeightOverride(ChipHeight);
		ChipBox->SetMaxDesiredWidth(ChipMaxWidth - 4.f);
		ChipBox->SetClipping(EWidgetClipping::ClipToBounds);
		ChipBox->SetContent(WidgetTree->ConstructWidget<UHOLQueueItem>(UHOLQueueItem::StaticClass(), *FString::Printf(TEXT("SelectionChip%d"), Chip)));
		ChipFrame->SetContent(ChipBox);
		SelectionChips->AddChildToWrapBox(ChipFrame);
	}
	UHorizontalBox* OwnerRow = MakeHorizontalBox(TEXT("SelectionOwnerRow"));
	SelectionOwnerRow = OwnerRow;
	SelectionColumn->AddChildToVerticalBox(OwnerRow)->SetPadding(FMargin(0.f, 0.f, 0.f, 2.f));
	SelectionSwatch = MakeSquare(TEXT("SelectionSwatch"), FLinearColor::White, SwatchSize);
	AddToRow(OwnerRow, SelectionSwatch, 8.f);
	SelectionOwner = MakeText(TEXT("SelectionOwner"), UHOLTextStyleHUD::StaticClass(), FText::GetEmpty());
	AddToRow(OwnerRow, SelectionOwner, 0.f);
	UHorizontalBox* HealthRow = MakeHorizontalBox(TEXT("SelectionHealthRow"));
	SelectionHealthRow = HealthRow;
	SelectionColumn->AddChildToVerticalBox(HealthRow);
	AddToRow(HealthRow, MakeText(TEXT("SelectionHealthLabel"), UHOLTextStyleHUDLabel::StaticClass(), LOCTEXT("Health", "Points de vie")), 8.f);
	SelectionHealth = MakeText(TEXT("SelectionHealth"), UHOLTextStyleHUD::StaticClass(), FText::GetEmpty());
	AddToRow(HealthRow, SelectionHealth, 0.f);
	// A site's progress (J6a.5, D313): a bar and its percentage.
	UHorizontalBox* ProgressRow = MakeHorizontalBox(TEXT("SelectionProgressRow"));
	SelectionProgressRow = ProgressRow;
	SelectionColumn->AddChildToVerticalBox(ProgressRow)->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));
	SelectionProgress = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("SelectionProgress"));
	FProgressBarStyle SiteBarStyle;
	SiteBarStyle.SetBackgroundImage(FSlateRoundedBoxBrush(FLinearColor(0.08f, 0.08f, 0.1f, 1.f), 2.f));
	SiteBarStyle.SetFillImage(FSlateRoundedBoxBrush(HOLUIStyle::Accent, 2.f));
	SelectionProgress->SetWidgetStyle(SiteBarStyle);
	USizeBox* SiteBarBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SelectionProgressBox"));
	SiteBarBox->SetWidthOverride(SiteBarWidth);
	SiteBarBox->SetHeightOverride(8.f);
	SiteBarBox->SetContent(SelectionProgress);
	AddToRow(ProgressRow, SiteBarBox, 8.f);
	SelectionProgressText = MakeText(TEXT("SelectionProgressText"), UHOLTextStyleHUD::StaticClass(), FText::GetEmpty());
	AddToRow(ProgressRow, SelectionProgressText, 0.f);
	ProgressRow->SetVisibility(ESlateVisibility::Collapsed);
	SelectionActivity = MakeText(TEXT("SelectionActivity"), UHOLTextStyleHUD::StaticClass(), FText::GetEmpty());
	SelectionActivity->SetAutoWrapText(true);
	SelectionColumn->AddChildToVerticalBox(SelectionActivity)->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));
	// A main forum's lives (V2.3d): a row as the health's, under the ruin's or the rebuilding's lines.
	UHorizontalBox* LivesRow = MakeHorizontalBox(TEXT("SelectionLivesRow"));
	SelectionLivesRow = LivesRow;
	SelectionColumn->AddChildToVerticalBox(LivesRow)->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));
	AddToRow(LivesRow, MakeText(TEXT("SelectionLivesLabel"), UHOLTextStyleHUDLabel::StaticClass(), LOCTEXT("ForumLivesLabel", "Vies")), 8.f);
	SelectionLives = MakeText(TEXT("SelectionLives"), UHOLTextStyleHUD::StaticClass(), FText::GetEmpty());
	AddToRow(LivesRow, SelectionLives, 0.f);
	LivesRow->SetVisibility(ESlateVisibility::Collapsed);

	// Right of the selection's block, the command panel's width: the short message, the production queue, then the
	// command panel under them (the column grows upwards, the panel stays at the bar's bottom).
	USizeBox* CommandBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CommandBox"));
	CommandBox->SetWidthOverride(GridWidth);
	AddToBar(CommandBox, BarGap);
	UVerticalBox* CommandColumn = MakeVerticalBox(TEXT("CommandColumn"));
	CommandBox->SetContent(CommandColumn);

	// The short message (D313), above the queue and the grid, wrapped within the grid's width; it lets the clicks through.
	MessagePanel = MakePanel(TEXT("MessagePanel"));
	UVerticalBoxSlot* MessageSlot = CommandColumn->AddChildToVerticalBox(MessagePanel);
	MessageSlot->SetHorizontalAlignment(HAlign_Left);
	MessageSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
	USizeBox* MessageBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MessageBox"));
	MessageBox->SetMaxDesiredWidth(GridWidth);
	MessagePanel->SetContent(MessageBox);
	MessageText = MakeText(TEXT("MessageText"), UHOLTextStyleHUDAlert::StaticClass(), FText::GetEmpty());
	// Wrapped at a fixed width: a short message keeps its own width, a long one goes on two lines.
	MessageText->SetWrapTextAt(GridWidth - 24.f);
	MessageBox->SetContent(MessageText);
	MessagePanel->SetVisibility(ESlateVisibility::Collapsed);

	// Production queue, as wide as the grid: its items, the head's progress (or the population note), the hint.
	QueuePanel = MakePanel(TEXT("QueuePanel"));
	UVerticalBoxSlot* QueueSlot = CommandColumn->AddChildToVerticalBox(QueuePanel);
	QueueSlot->SetHorizontalAlignment(HAlign_Fill);
	QueueSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
	UVerticalBox* QueueColumn = MakeVerticalBox(TEXT("QueueColumn"));
	QueuePanel->SetContent(QueueColumn);
	UHorizontalBox* QueueRow = MakeHorizontalBox(TEXT("QueueRow"));
	QueueColumn->AddChildToVerticalBox(QueueRow);
	for (int32 Index = 0; Index < NumQueueItems; ++Index)
	{
		USizeBox* ItemBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ItemBox->SetWidthOverride(QueueItemWidth);
		ItemBox->SetHeightOverride(QueueItemHeight);
		ItemBox->SetContent(WidgetTree->ConstructWidget<UHOLQueueItem>(UHOLQueueItem::StaticClass(), QueueItemName(Index)));
		AddToRow(QueueRow, ItemBox, 4.f);
	}

	// The bar and the note share one line of a fixed height, so that the panel does not move when one replaces the other.
	USizeBox* StatusBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("QueueStatus"));
	StatusBox->SetHeightOverride(18.f);
	QueueColumn->AddChildToVerticalBox(StatusBox)->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
	UOverlay* Status = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("QueueStatusLine"));
	StatusBox->SetContent(Status);

	QueueProgress = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("QueueProgress"));
	FProgressBarStyle BarStyle;
	BarStyle.SetBackgroundImage(FSlateRoundedBoxBrush(FLinearColor(0.08f, 0.08f, 0.1f, 1.f), 2.f));
	BarStyle.SetFillImage(FSlateRoundedBoxBrush(HOLUIStyle::Accent, 2.f));
	QueueProgress->SetWidgetStyle(BarStyle);
	USizeBox* BarBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("QueueProgressBox"));
	BarBox->SetHeightOverride(6.f);
	BarBox->SetContent(QueueProgress);
	UOverlaySlot* BarSlot = Status->AddChildToOverlay(BarBox);
	BarSlot->SetHorizontalAlignment(HAlign_Fill);
	BarSlot->SetVerticalAlignment(VAlign_Center);

	QueueNote = MakeText(TEXT("QueueNote"), UHOLTextStyleHUDAlert::StaticClass(), LOCTEXT("PopulationFull", "Population pleine"));
	QueueNote->SetVisibility(ESlateVisibility::Collapsed);
	UOverlaySlot* NoteSlot = Status->AddChildToOverlay(QueueNote);
	NoteSlot->SetHorizontalAlignment(HAlign_Left);
	NoteSlot->SetVerticalAlignment(VAlign_Center);

	UCommonTextBlock* Hint = MakeText(TEXT("QueueHint"), UHOLTextStyleHUDLabel::StaticClass(), LOCTEXT("CancelHint", "Clic : annuler"));
	UVerticalBoxSlot* HintSlot = QueueColumn->AddChildToVerticalBox(Hint);
	HintSlot->SetHorizontalAlignment(HAlign_Right);
	HintSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));

	// The command panel (D30, the user's 2nd trial of G.1): 3 rows of 4 square cells, A Z E R / Q S D F / W X C V, always
	// all there (an empty one dark): the orders of the player's units or the grid of its building.
	GridPanel = MakePanel(TEXT("GridPanel"));
	GridPanel->SetPadding(FMargin(6.f));
	CommandColumn->AddChildToVerticalBox(GridPanel)->SetHorizontalAlignment(HAlign_Left);
	UUniformGridPanel* Grid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("Grid"));
	Grid->SetSlotPadding(FMargin(CellGap));
	GridPanel->SetContent(Grid);
	for (int32 Cell = 0; Cell < AHOLPlayerController::NumGridCells; ++Cell)
	{
		USizeBox* CellBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		CellBox->SetWidthOverride(CellSize);
		CellBox->SetHeightOverride(CellSize);
		CellBox->SetContent(WidgetTree->ConstructWidget<UHOLGridCell>(UHOLGridCell::StaticClass(), GridCellName(Cell)));
		Grid->AddChildToUniformGrid(CellBox, Cell / GridColumns, Cell % GridColumns);
	}

	// The secondary panel, right of the command panel: the builders' age panels (the user's trials of G.1, AoE4's way), I
	// to IV side by side, each a medallion (numeral in a disc, its key A Z E R in a square beside; no click) on its top
	// edge over its buildings, Q S D F / W X C V (RefreshAges packs the used ones). Absent otherwise, nothing moves.
	for (int32 Age = 0; Age < HOLCommandGrid::NumAges; ++Age)
	{
		UBorder* AgePanel = MakePanel(*FString::Printf(TEXT("AgePanel%d"), Age));
		AgePanel->SetPadding(FMargin(6.f));
		AddToBar(AgePanel, Age == 0 ? BarGap : 4.f);
		UVerticalBox* AgeColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		AgePanel->SetContent(AgeColumn);
		// The medallion rises over the panel's top edge (drawn higher, its place in the panel mostly given back).
		UHorizontalBox* MedallionRow = MakeHorizontalBox(*FString::Printf(TEXT("AgeMedallionRow%d"), Age));
		MedallionRow->SetRenderTranslation(FVector2D(0.f, -MedallionRise));
		UVerticalBoxSlot* MedallionSlot = AgeColumn->AddChildToVerticalBox(MedallionRow);
		MedallionSlot->SetHorizontalAlignment(HAlign_Center);
		MedallionSlot->SetPadding(FMargin(0.f, 0.f, 0.f, -MedallionRise + 4.f));
		USizeBox* MedallionBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		MedallionBox->SetWidthOverride(MedallionSize);
		MedallionBox->SetHeightOverride(MedallionSize);
		UBorder* Medallion = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), AgeMedallionName(Age));
		Medallion->SetBrush(FSlateRoundedBoxBrush(FLinearColor(0.1f, 0.11f, 0.14f, 1.f), MedallionSize / 2.f, HOLUIStyle::TextMuted, 1.5f));
		Medallion->SetPadding(FMargin(0.f));
		Medallion->SetHorizontalAlignment(HAlign_Center);
		Medallion->SetVerticalAlignment(VAlign_Center);
		Medallion->SetContent(MakeText(*FString::Printf(TEXT("AgeNumeral%d"), Age), UHOLTextStyleHUDHeading::StaticClass(), FText::AsCultureInvariant(FString(AgeNumerals[Age]))));
		MedallionBox->SetContent(Medallion);
		AddToRow(MedallionRow, MedallionBox, 3.f);
		USizeBox* KeyBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		KeyBox->SetWidthOverride(KeyBadgeSize);
		KeyBox->SetHeightOverride(KeyBadgeSize);
		UBorder* KeyBadge = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("AgeKey%d"), Age));
		KeyBadge->SetBrush(FSlateRoundedBoxBrush(FLinearColor(0.06f, 0.065f, 0.08f, 1.f), 3.f, HOLUIStyle::TextMuted, 1.f));
		KeyBadge->SetPadding(FMargin(0.f));
		KeyBadge->SetHorizontalAlignment(HAlign_Center);
		KeyBadge->SetVerticalAlignment(VAlign_Center);
		KeyBadge->SetContent(MakeText(NAME_None, UHOLTextStyleHUDSmall::StaticClass(), FText::AsCultureInvariant(FString(GridKeys[Age]))));
		KeyBox->SetContent(KeyBadge);
		AddToRow(MedallionRow, KeyBox, 0.f);
		UUniformGridPanel* AgeGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass());
		AgeGrid->SetSlotPadding(FMargin(CellGap));
		AgeColumn->AddChildToVerticalBox(AgeGrid)->SetHorizontalAlignment(HAlign_Left);
		for (int32 Index = 0; Index < NumAgeCells; ++Index)
		{
			USizeBox* CellBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
			CellBox->SetWidthOverride(CellSize);
			CellBox->SetHeightOverride(CellSize);
			CellBox->SetContent(WidgetTree->ConstructWidget<UHOLGridCell>(UHOLGridCell::StaticClass(), AgeCellName(Age, Index)));
			AgeGrid->AddChildToUniformGrid(CellBox, Index / GridColumns, Index % GridColumns);
		}
		AgePanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	// The minimap's place, bottom right (D201): no minimap yet; nothing else goes there.
	USizeBox* MinimapPlace = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MinimapPlace"));
	MinimapPlace->SetWidthOverride(MinimapSize);
	MinimapPlace->SetHeightOverride(MinimapSize);
	MinimapPlace->SetVisibility(ESlateVisibility::HitTestInvisible);
	UOverlaySlot* MinimapSlot = Frame->AddChildToOverlay(MinimapPlace);
	MinimapSlot->SetHorizontalAlignment(HAlign_Right);
	MinimapSlot->SetVerticalAlignment(VAlign_Bottom);
	MinimapSlot->SetPadding(FMargin(ScreenMargin));

	// Nothing selected yet.
	SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
	QueuePanel->SetVisibility(ESlateVisibility::Collapsed);
	GridPanel->SetVisibility(ESlateVisibility::Hidden);
}

#undef LOCTEXT_NAMESPACE
