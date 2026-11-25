#include "HUD/HOLGameScreen.h"
#include "HUD/HOLHUDSpace.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Simulation/HOLRageVisual.h"
#include "Simulation/HOLUnitAbilityRules.h"
#include "Simulation/HOLZoneEffectRules.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/ScaleBox.h"
#include "Components/ScaleBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLGameNames.h"
#include "Match/HOLPlayersTableReadLibrary.h"
#include "HUD/HOLScriptScreen.h"
#include "Match/HOLFactionRules.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMarketSubsystem.h"
#include "Siege/HOLSiegeRules.h"
#include "Siege/HOLDeployRules.h"
#include "Simulation/HOLUnitTierSubsystem.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLGameMenuRulesBridge.h"
#include "Player/HOLGridFaceBridge.h"
#include "Player/HOLPlayerController.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLHeroLibrary.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"
#include "Sound/SoundBase.h"
#include "Screens/HOLGameMenuScreen.h"
#include "Screens/HOLStatsScreen.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLGridCell.h"
#include "Widgets/HOLQueueItem.h"
#include "Widgets/HOLRootLayout.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "HOLGameScreen"

namespace UE::HeroesOfLiberty::UI::GameScreen
{
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
	// Q-barracks-tooltip-damned = A: with a world and a slot, « Produit : » lists only the unit types the slot's faction fields (the grid's filter).
	FText GetBuildingEffect(const FHOLBuildingTypeRow* Type, const FName FactionKey, const UWorld* World = nullptr, const int32 Slot = INDEX_NONE)
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
		const TArray<FName> Produced = !Type ? TArray<FName>()
			: (World && Slot != INDEX_NONE ? HOLFactionRules::FilterProduces(World, Type->Produces, Slot) : Type->Produces);
		if (!Produced.IsEmpty())
		{
			TArray<FText> Names;
			for (const FName Unit : Produced)
			{
				Names.Add(HOLGameNames::GetUnitName(Unit, nullptr, FactionKey));
			}
			Lines.Add(FText::Format(LOCTEXT("ProducesEffect", "Produit : {0}"), FText::Join(LOCTEXT("ListSeparator", ", "), Names)));
		}
		return FText::Join(FText::AsCultureInvariant(TEXT("\n")), Lines);
	}

	/** What a unit does, for its tooltip. */
	FText GetUnitEffect(const FName TypeName)
	{
		static const FName Peasant(TEXT("Peasant"));
		return (TypeName == Peasant || TypeName == FName(TEXT("Zombie"))) ? LOCTEXT("PeasantEffect", "Récolte et bâtit") : FText::GetEmpty();
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
		case EHOLCommandFeedback::ResearchRefused: return TEXT("ResearchRefused");
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

	/** Why a technology cannot be researched now (T4): the short message of a refused cell, and its tooltip's effect line. */
	FText GetResearchBlockText(const HOLTechnology::EBlock Block)
	{
		switch (Block)
		{
		case HOLTechnology::EBlock::NotImplemented: return LOCTEXT("ResearchSoon", "Bientôt");
		case HOLTechnology::EBlock::AlreadyResearched: return LOCTEXT("ResearchDone", "Déjà recherchée");
		case HOLTechnology::EBlock::AlreadyQueued: return LOCTEXT("ResearchQueued", "Déjà en cours de recherche");
		case HOLTechnology::EBlock::TierTooLow: return LOCTEXT("ResearchTier", "Palier supérieur requis");
		case HOLTechnology::EBlock::HeroLevelTooLow: return LOCTEXT("ResearchHeroLevel", "Niveau de héros insuffisant");
		case HOLTechnology::EBlock::RequiresMissing: return LOCTEXT("ResearchRequires", "Recherchez d'abord le niveau précédent");
		case HOLTechnology::EBlock::NoBuilding: return LOCTEXT("ResearchBuilding", "Bâtiment manquant");
		default: return FText::GetEmpty();
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
		case EHOLCommandFeedback::ResearchRefused: return GetResearchBlockText(Feedback.Block);
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
	constexpr float CellSize = 76.f;
	constexpr float CellGap = 2.f;
	constexpr float QueueItemHeight = 36.f;
	constexpr float ScreenMargin = 16.f;
	/** The grid panel's width: the short message above it wraps within it (D313). */
	constexpr float GridWidth = 4 * (CellSize + 2 * CellGap) + 12.f;
	/**
	 * The bottom bar (the user's 2nd trial of G.1, AoE4's way, D201), left to right at fixed places whatever the selection:
	 * the selection's block (a fixed width), the command panel (3 x 4), the builders' age panels; its height the panel's.
	 */
	constexpr float BarHeight = 3 * (CellSize + 2 * CellGap) + 12.f;
	constexpr float BarGap = 6.f;
	/** The minimap's place, bottom right (D201): no minimap yet, the place kept free. */
	constexpr float MinimapSize = 240.f;
	/** How far the script's age medallions rise over the bar's top edge (MedallionRoom keeps that room free). */
	constexpr float MedallionRise = 26.f;
	/** The name the HUD's report gives the « Annuler » cell of a site. */
	const FName CancelName(TEXT("Cancel"));
	/** Seconds between two readings of a selected group's activities. */
	constexpr float GroupRefreshInterval = 0.25f;


	/** The ages' numerals (I to IV) and names (GDD § 11.1). */
	const TCHAR* const AgeNumerals[HOLCommandGrid::NumAges] = { TEXT("I"), TEXT("II"), TEXT("III"), TEXT("IV") };

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

	/** AgeCell<age><cell>: AgeCell000 the house (age 0, A), AgeCell004 the barracks (Q). */
	FName AgeCellName(const int32 Age, const int32 Cell)
	{
		return *FString::Printf(TEXT("AgeCell%d%02d"), Age, Cell);
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

	// J8a.5: the part in script over the whole frame, last (on top); its panels alone take the clicks. None: nothing.
	UOverlay* Frame = Cast<UOverlay>(WidgetTree->RootWidget);
	UClass* ScriptClass = UHOLScriptScreen::FindScriptClass();
	if (Frame && ScriptClass)
	{
		ScriptScreen = CreateWidget<UHOLScriptScreen>(this, ScriptClass, TEXT("ScriptScreen"));
		if (ScriptScreen)
		{
			UOverlaySlot* ScriptSlot = Frame->AddChildToOverlay(ScriptScreen);
			ScriptSlot->SetHorizontalAlignment(HAlign_Fill);
			ScriptSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
	UE_LOG(LogHOLUI, Log, TEXT("Game screen: script part %s"), ScriptScreen ? *ScriptScreen->GetClass()->GetName() : TEXT("none"));

	// HUD H-10: the grid's cells are the script's (UHOLCommandGridPanel.Build, then SetGridCells): their keys and clicks are set there.
	// HUD H-9: the selection chips are the script's (UHOLSelectionChipsPanel): built and clicked there.
	ShownGridTypes.Init(NAME_None, AHOLPlayerController::NumGridCells);
	ShownOrderTypes.Init(NAME_None, AHOLPlayerController::NumGridCells);
	ShownAgeTypes.Init(NAME_None, HOLCommandGrid::NumAges * NumAgeCells);
	ShownAgeReached.Init(false, HOLCommandGrid::NumAges);
	ShownAgeTooltips.Init(FString(), HOLCommandGrid::NumAges);

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
		SinceTableRefresh = 0.f;
		// V2.3c: a selected main forum on the ground, its ruin's seconds and its rebuilding's percent move with time alone
		// (the forum's state changes only at its phases): its block is read at the same pace. Standing too (tiers, P4): its
		// lives follow the level of its player's hero, which moves without telling the forum's state.
		const AHOLBuilding* SelectedBuilding = BoundBuilding.Get();
		if (BoundForumState.IsValid() && SelectedBuilding && (SelectedBuilding->IsForumDown() || SelectedBuilding->IsPrimaryForum()))
		{
			Refresh();
		}
		// J8c.4: the hero's cells (units selected: no building, no group) follow his cool-downs and his level. L4: the Damned
		// Warriors' rage cell too (hol.Rage.Visual 3 or 4: its seconds of rage then of recharge move with time alone).
		// G6 f: the engine's D cell (« Monter » / « Démonter ») follows its phase, which the simulation moves without a delegate
		// (the deploy preflight saw « Monter » still shown once the engine was set up).
		// HALDIR-1125me: the Monk's Seal cell (LightSeal) too, its cool-down seconds under "Sceau" move with time alone (the cast changes no selection).
		else if (bHeroCellsShown || ShownGridTypes.Contains(FName(TEXT("NecroticRage"))) || ShownGridTypes.Contains(FName(TEXT("LightSeal")))
			|| ShownOrderTypes.ContainsByPredicate([](const FName& OrderType)
			{
				// The D cell's key is Deploy / Pack at rest and Mounting<n> / Dismounting<n> while it moves (ReadDeployFace): refresh in all four (D869).
				const FString Name = OrderType.ToString();
				return Name == TEXT("Deploy") || Name == TEXT("Pack") || Name.StartsWith(TEXT("Mounting")) || Name.StartsWith(TEXT("Dismounting"));
			}))
		{
			RefreshGrid(nullptr, false, BoundPlayerState.Get());
		}
		// P4 (tiers): the builders' age panels follow the tier of the player's hero (his level moves without a delegate).
		if (bAgesShown && ScriptScreen && BoundPlayerState.IsValid() && BoundTableState.IsValid()
			&& GetBoundTier() != ShownTier)
		{
			// The script draws the panels; the tier it was drawn at is kept here so that this check fires once per change.
			ShownTier = GetBoundTier();
			ScriptScreen->RefreshAges();
		}
	}
	// S7: the siege workshop's engine cells follow the tier of the hero (his level moves without a delegate): drawn again when it changes.
	if (bGridShown && ShownEngineTier != INDEX_NONE && BoundTableState.IsValid() && BoundPlayerState.IsValid()
		&& BoundTableState->GetTier(BoundPlayerState->GetPlayerIndex()) != ShownEngineTier)
	{
		Refresh();
	}
	// M5: a Market's trade cells quote the common book; another player's trade, the book's way back to rest or a client's
	// late replication moves it without telling this HUD: the cells are drawn again as soon as it differs from the quoted one.
	if (bGridShown && !ShownMarketPrices.IsEmpty() && BoundTableState.IsValid() && BoundTableState->HasMarketBook())
	{
		const HOLMarket::FBook Book = BoundTableState->GetMarketBook();
		for (int32 Resource = 0; Resource < HOLMarket::BookSize; ++Resource)
		{
			if (ShownMarketPrices.IsValidIndex(Resource) && ShownMarketPrices[Resource] != Book.Prices[Resource])
			{
				Refresh();
				break;
			}
		}
	}
	// A gatherer selected alone, a source's amount: the script follows them every frame while its block says so (UHOLScriptScreen::SelectionFrameTick, called by
	// the script screen's tick). A group of units' activities and a group of buildings (an F key), four times a second: the C++ aiguillage stays, the script draws.
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
	else if (NumSelectedUnits > 1 && SinceGroupRefresh >= GroupRefreshInterval)
	{
		SinceGroupRefresh = 0.f;
		if (ScriptScreen)
		{
			ScriptScreen->RequestRefresh();
		}
	}
	// The chosen age (a key, a placement, Escape) changes without a delegate: its header follows it (the 2nd check of G.1c).
	if (bAgesShown && ScriptScreen && Controller.IsValid() && Controller->GetChosenAge() != ShownChosenAge)
	{
		ScriptScreen->RefreshAges();
	}
	RefreshIdleWorkers();
	// HUD H-7: the queue's head bar moves each frame in the script (UHOLScriptScreen::QueueFrameTick, only while a queue is shown).
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
		Previous->OnForumEvent.Remove(TableForumEventHandle);
		Previous->OnHeroReadyMessage.Remove(TableHeroReadyHandle);
		Previous->OnTributeMessage.Remove(TableTributeHandle);
		Previous->OnSignalMessage.Remove(TableSignalHandle);
		Previous->OnHeroLevelUp.Remove(TableHeroLevelHandle);
	}
	TableForumEventHandle.Reset();
	TableHeroReadyHandle.Reset();
	TableHeroLevelHandle.Reset();
	BoundTableState = GameState;
	if (GameState)
	{
		// Every machine: the server at once, a client by its OnReps (D353: public).
		// V4.2: the events of the ruin, every machine (a reliable multicast).
		TableForumEventHandle = GameState->OnForumEvent.AddUObject(this, &UHOLGameScreen::HandleForumEvent);
		// J8a.4: the local player's hero ready to return (its owner alone, once per death).
		TableHeroReadyHandle = GameState->OnHeroReadyMessage.AddUObject(this, &UHOLGameScreen::HandleHeroReadyMessage);
		TableTributeHandle = GameState->OnTributeMessage.AddUObject(this, &UHOLGameScreen::HandleTributeMessage);
		// SG3 lot 2: a map signal received (its viewer alone).
		TableSignalHandle = GameState->OnSignalMessage.AddUObject(this, &UHOLGameScreen::HandleSignalMessage);
		// J8b.4: a hero gone up (every machine): the alerts for its owner alone.
		TableHeroLevelHandle = GameState->OnHeroLevelUp.AddUObject(this, &UHOLGameScreen::HandleHeroLevelUp);
	}
}

void UHOLGameScreen::HandleTributeMessage(const int32 ReceiverSlot, const FHOLForumMessage& Message)
{
	// Only the receiver's machine shows it (TB-Q1).
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	if (const UWorld* World = GetWorld(); World && PlayerState && PlayerState->GetPlayerIndex() == ReceiverSlot)
	{
		MessageThread.Add(Message, World->GetRealTimeSeconds());
		RebuildMessages();
	}
}

void UHOLGameScreen::HandleSignalMessage(const int32 ViewerSlot, const FHOLForumMessage& Message)
{
	// Only the viewer's machine shows it (a spectator: no slot).
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	const int32 OwnSlot = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
	if (const UWorld* World = GetWorld(); World && OwnSlot == ViewerSlot)
	{
		MessageThread.Add(Message, World->GetRealTimeSeconds());
		RebuildMessages();
	}
}

const AHOLPlayerState* UHOLGameScreen::GetBoundPlayerState() const
{
	return BoundPlayerState.Get();
}

void UHOLGameScreen::AddThreadLine(const FText& Line, const double Seconds)
{
	if (const UWorld* World = GetWorld())
	{
		MessageThread.AddText(Line, Seconds, World->GetRealTimeSeconds());
		RebuildMessages();
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

void UHOLGameScreen::HandleHeroLevelUp(const int32 HeroSlot, const int32 OldLevel, const int32 NewLevel)
{
	const UWorld* World = GetWorld();
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	if (!World || !PlayerState || PlayerState->GetPlayerIndex() != HeroSlot)
	{
		return;
	}
	const TArray<FHOLForumMessage> Messages = HOLForumMessages::GetHeroLevelMessages(OldLevel, NewLevel);
	for (const FHOLForumMessage& Message : Messages)
	{
		UE_LOG(LogHOLUI, Display, TEXT("%s (slot %d)"), *Message.Shown, HeroSlot);
		MessageThread.Add(Message, World->GetRealTimeSeconds());
	}
	RebuildMessages();
	// A minor alert with its sound (J8b-Q4), once for its lines: a fanfare (J8b.5), the tier's when a tier's line follows
	// the level's; synthesised until real sounds come.
	const bool bTier = Messages.Num() > 1;
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, bTier ? HOLForumMessages::HeroTierSoundPath : HOLForumMessages::HeroLevelSoundPath);
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
	UE_LOG(LogHOLUI, Display, TEXT("Hero level alert: slot %d level %d -> %d (sound%s)%s"), HeroSlot, OldLevel, NewLevel, Sound ? TEXT("") : TEXT(" missing"),
		bTier ? TEXT(" tier fanfare") : TEXT(""));
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
	const int32 ViewerSlot = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
	const FHOLForumMessage Message = HOLForumMessages::GetMessage(Event, GameState->GetLaunchConfig(), ViewerSlot, GameState->DoesViewerSeeForum(ViewerSlot, Event.Slot));
	MessageThread.Add(Message, World->GetRealTimeSeconds(), Event.Slot);
	RebuildMessages();
	// V4.3: an alert, the banner and its sound too.
	ShowAlert(Message, Event.Slot);
}

void UHOLGameScreen::ShowAlert(const FHOLForumMessage& Message, const int32 TouchedSlot)
{
	const UWorld* World = GetWorld();
	bool bMatchOverForMe = false;
	if (!World || !IsMatchOverForMe(bMatchOverForMe))
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
	bOver = UHOLGameMenuRulesBridge::NativeGetEndBanner(ViewerSlot, GameState->GetDefeatedMask(), GameState->IsMatchOver(),
		GameState->GetWinnerMask()) != EHOLEndBanner::None;
	return true;
}

void UHOLGameScreen::PresentAlert()
{
	// HUD H-5a: the banner, its sound and its journal line are the script's (HOLAlertBanner.as); it redraws when this number changes.
	++AlertPresentCount;
}

void UHOLGameScreen::TickAlert()
{
	const UWorld* World = GetWorld();
	if (!Alert.bShown || !World)
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
	// Its 4 s over: the script sees the state show none and hides the banner; the fade is its (Alert.GetOpacity).
	Alert.Expire(Now);
}

void UHOLGameScreen::FocusCameraOnForum(const int32 ForumSlot)
{
	if (AHOLPlayerController* PlayerController = Controller.Get())
	{
		PlayerController->FocusCameraOnForum(ForumSlot);
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
	// The last half second of a line: its fade is the script's (HOLMessageFeed.as reads FHOLForumThread::GetOpacity each frame).
}

void UHOLGameScreen::RebuildMessages()
{
	// HUD H-5b: the thread's lines are drawn by the script (HOLMessageFeed.as): it redraws when this number changes.
	++MessageRevision;
}

FString UHOLGameScreen::DescribeMessages() const
{
	return MessageThread.Describe();
}

void UHOLGameScreen::HandleSelectionChanged()
{
	BindBuilding(Controller.IsValid() ? Controller->GetSelection() : nullptr);
	// Audio B5: one finished building selected alone (not a unit, a source or a group): the script rings its family's selection sound.
	const AHOLPlayerController* SelectionController = Controller.Get();
	const AHOLBuilding* SelectedAlone = SelectionController ? SelectionController->GetSelection() : nullptr;
	if (ScriptScreen && SelectedAlone && SelectedAlone->IsFinished() && SelectionController->GetSelectedUnits().IsEmpty()
		&& SelectionController->GetSelectedSource() == INDEX_NONE && !SelectionController->HasBuildingGroup())
	{
		ScriptScreen->BuildingSelected(SelectedAlone->GetTypeName().ToString());
	}
	Refresh();
}

void UHOLGameScreen::Refresh()
{
	const AHOLPlayerController* PlayerController = Controller.Get();
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	// Only the owner gives a building orders, and only the owner receives its queue (D282).
	const AHOLBuilding* Own = PlayerController ? PlayerController->GetOwnSelection() : nullptr;

	RefreshTopBar(PlayerState);
	// HUD H-9: the selection block (the four paths, the chips) is the script's: RequestRefresh below makes it read the controller again the next frame.
	RefreshIdleWorkers();
	// D357: a spectator commands nothing, its grid hidden.
	const bool bSpectator = PlayerController && PlayerController->IsSpectator();
	RefreshGrid(bSpectator ? nullptr : Own, !bSpectator && PlayerController && PlayerController->HasBuildingGroup(), PlayerState);
	// HUD H-7: the queue is the script's (HOLQueuePanel.as): it reads it again the next frame (RequestRefresh), not 250 ms later.
	if (ScriptScreen)
	{
		ScriptScreen->RequestRefresh();
	}
}

UBorder* UHOLGameScreen::GetTopBar() const
{
	// H-3: the top bar is the script's (UHOLTopBarPanel, named "TopBar"): found by name in the script's tree, the anchor of the cheats' and scenario panels.
	return ScriptScreen && ScriptScreen->WidgetTree ? Cast<UBorder>(ScriptScreen->WidgetTree->FindWidget(FName(TEXT("TopBar")))) : nullptr;
}

void UHOLGameScreen::RefreshTopBar(const AHOLPlayerState* PlayerState)
{
	// H-3: the bar itself (widgets, tooltips) is the script's (UHOLTopBarPanel); the screen only keeps the fields Describe writes of it
	// (the "F.. W.. S.. G.., population U/C, gatherers .., idle N" lines of hol.HUD.Report, read by quick_check).
	bEconomyShown = PlayerState != nullptr;
	if (!PlayerState)
	{
		return;
	}
	ShownResources = PlayerState->GetResources();
	ShownPopulationUsed = PlayerState->GetPopulationUsed();
	ShownPopulationCap = PlayerState->GetPopulationCap();
	ShownGatherers = PlayerState->GetGatherers();
}

namespace
{
	TAutoConsoleVariable<int32> CVarUnitStats(TEXT("hol.HUD.UnitStats"), 1,
		TEXT("How the selected unit's statistics (attack with the technologies' bonus, armour, range, speed) are shown under its health: 0 hidden, 1 one line, 2 one line per statistic"));

	TAutoConsoleVariable<int32> CVarUnitStatsBuilding(TEXT("hol.HUD.UnitStatsBuilding"), 1,
		TEXT("The blow against buildings in the selected unit's statistics (Q-unit-stats-building): 0 never, 1 only for a unit with no ordinary attack (the Ram), 2 also next to an ordinary attack when it differs"));
}

namespace
{
	// HUD H-9: the script's selection block is built into the game screen's SelectionHost (HOLScriptScreen::GetSelectionHost), so its widgets are OWNED by the script screen's
	// tree but hang from the game screen's tree: UWidgetTree::FindWidget walks from the root, hence the game screen's tree is searched first, then the script screen's.
	UWidget* FindSelectionWidget(const UWidgetTree* GameTree, const UWidgetTree* ScriptTree, const FName Name)
	{
		UWidget* Found = GameTree ? GameTree->FindWidget(Name) : nullptr;
		return Found ? Found : ScriptTree ? ScriptTree->FindWidget(Name) : nullptr;
	}
}

FString UHOLGameScreen::DescribeStatsTip() const
{
	// One tooltip per shown statistic: only the chips that are visible AND carry a tooltip text are listed.
	// HUD H-9: the chips are the script's widgets "SelectionStats" and "SelectionStat<i>" (HOLSelectionUnit.as); ShownStatTipList is what SetStatsReport posed.
	TArray<FString> Parts;
	const UWidget* StatsBox = FindSelectionWidget(WidgetTree, ScriptScreen ? ScriptScreen->WidgetTree.Get() : nullptr, TEXT("SelectionStats"));
	if (StatsBox && StatsBox->GetVisibility() != ESlateVisibility::Collapsed)
	{
		for (int32 Index = 0; Index < 6 && Index < ShownStatTipList.Num(); ++Index)
		{
			const UCommonTextBlock* Chip = Cast<UCommonTextBlock>(FindSelectionWidget(WidgetTree, ScriptScreen->WidgetTree.Get(), *FString::Printf(TEXT("SelectionStat%d"), Index)));
			if (Chip && Chip->GetVisibility() == ESlateVisibility::Visible && !Chip->GetToolTipText().IsEmpty())
			{
				Parts.Add(ShownStatTipList[Index]);
			}
		}
	}
	return Parts.IsEmpty() ? FString(TEXT("none")) : FString::Join(Parts, TEXT(" | "));
}

FString UHOLGameScreen::HoverStat(const int32 Index) const
{
	// The hover as the mouse does it (user bug of unit_stats_attack: no tooltip on the other statistics): the centre of the chip is hit-tested
	// by Slate's own window hit test, and the tooltip the engine would show is the first non-empty one met from the widget found up to the window.
	// HUD H-9: the chip is the script's widget "SelectionStat<i>" (HOLSelectionUnit.as), found in the script's tree as HoverIncome finds FoodAmount.
	UCommonTextBlock* Chip = ScriptScreen && Index >= 0 && Index < 6
		? Cast<UCommonTextBlock>(FindSelectionWidget(WidgetTree, ScriptScreen->WidgetTree.Get(), *FString::Printf(TEXT("SelectionStat%d"), Index))) : nullptr;
	const TSharedPtr<SWidget> ChipWidget = Chip ? Chip->GetCachedWidget() : TSharedPtr<SWidget>();
	if (!Chip || !ChipWidget.IsValid() || Chip->GetVisibility() == ESlateVisibility::Collapsed || !FSlateApplication::IsInitialized())
	{
		return TEXT("no statistic shown");
	}
	FSlateApplication& Slate = FSlateApplication::Get();
	FWidgetPath ChipPath;
	if (!Slate.FindPathToWidget(ChipWidget.ToSharedRef(), ChipPath, EVisibility::All) || ChipPath.Widgets.Num() == 0)
	{
		return TEXT("no path to the statistic");
	}
	const FGeometry& Box = ChipPath.Widgets[ChipPath.Widgets.Num() - 1].Geometry;
	const FVector2f Position = Box.GetAbsolutePosition();
	const FVector2f Size = Box.GetAbsoluteSize();
	const FVector2f Centre = Position + Size * 0.5f;
	const FWidgetPath Hit = Slate.LocateWindowUnderMouse(Centre, Slate.GetInteractiveTopLevelWindows(), true);
	if (Hit.Widgets.Num() == 0)
	{
		return TEXT("nothing under the point");
	}
	for (int32 Level = Hit.Widgets.Num() - 1; Level >= 0; --Level)
	{
		const TSharedRef<SWidget> Widget = Hit.Widgets[Level].Widget;
		const TSharedPtr<IToolTip> Tip = Widget->GetToolTip();
		if (Tip.IsValid() && !Tip->IsEmpty())
		{
			if (&Widget.Get() == ChipWidget.Get())
			{
				return ShownStatTipList.IsValidIndex(Index) ? ShownStatTipList[Index] : FString(TEXT("tip"));
			}
			return FString::Printf(TEXT("the tip of another widget (%s)"), *Widget->GetTypeAsString());
		}
	}
	return FString::Printf(TEXT("none (hit %s)"), *Hit.Widgets[Hit.Widgets.Num() - 1].Widget->GetTypeAsString());
}

FString UHOLGameScreen::HoverIncome(const int32 Index, const bool bShow)
{
	// D788 (scenario income_tooltip): the hover of a resource's amount as the mouse does it, same hit test as HoverStat. The engine shows a
	// tooltip in a window of its own, which the capture of the viewport does not see: bShow draws the SAME tooltip widget in the viewport.
	UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
	if (IncomeTipOverlay.IsValid())
	{
		if (Viewport)
		{
			Viewport->RemoveViewportWidgetContent(IncomeTipOverlay.ToSharedRef());
		}
		IncomeTipOverlay.Reset();
	}
	const auto AmountNamed = [this](const TCHAR* Name) -> UCommonTextBlock*
	{
		return ScriptScreen && ScriptScreen->WidgetTree ? Cast<UCommonTextBlock>(ScriptScreen->WidgetTree->FindWidget(FName(Name))) : nullptr;
	};
	UCommonTextBlock* const Amounts[] = { AmountNamed(TEXT("FoodAmount")), AmountNamed(TEXT("WoodAmount")), AmountNamed(TEXT("StoneAmount")), AmountNamed(TEXT("GoldAmount")) };
	if (Index < 0 || Index >= UE_ARRAY_COUNT(Amounts))
	{
		return TEXT("closed");
	}
	UCommonTextBlock* Amount = Amounts[Index];
	const TSharedPtr<SWidget> AmountWidget = Amount ? Amount->GetCachedWidget() : TSharedPtr<SWidget>();
	if (!Amount || !AmountWidget.IsValid() || Amount->GetVisibility() == ESlateVisibility::Collapsed || !FSlateApplication::IsInitialized())
	{
		return TEXT("no amount shown");
	}
	FSlateApplication& Slate = FSlateApplication::Get();
	FWidgetPath AmountPath;
	if (!Slate.FindPathToWidget(AmountWidget.ToSharedRef(), AmountPath, EVisibility::All) || AmountPath.Widgets.Num() == 0)
	{
		return TEXT("no path to the amount");
	}
	const FGeometry& Box = AmountPath.Widgets[AmountPath.Widgets.Num() - 1].Geometry;
	const FVector2f Position = Box.GetAbsolutePosition();
	const FVector2f Size = Box.GetAbsoluteSize();
	const FWidgetPath Hit = Slate.LocateWindowUnderMouse(Position + Size * 0.5f, Slate.GetInteractiveTopLevelWindows(), true);
	if (Hit.Widgets.Num() == 0)
	{
		return TEXT("nothing under the point");
	}
	for (int32 Level = Hit.Widgets.Num() - 1; Level >= 0; --Level)
	{
		const TSharedRef<SWidget> Widget = Hit.Widgets[Level].Widget;
		const TSharedPtr<IToolTip> Tip = Widget->GetToolTip();
		if (Tip.IsValid() && !Tip->IsEmpty())
		{
			if (&Widget.Get() != AmountWidget.Get())
			{
				return FString::Printf(TEXT("the tip of another widget (%s)"), *Widget->GetTypeAsString());
			}
			if (bShow && Viewport)
			{
				const FVector2f Local = GetCachedGeometry().AbsoluteToLocal(Position + FVector2f(0.f, Size.Y));
				IncomeTipOverlay = SNew(SBox)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					.Padding(FMargin(Local.X, Local.Y + 4.f, 0.f, 0.f))
					[
						Tip->AsWidget()
					];
				Viewport->AddViewportWidgetContent(IncomeTipOverlay.ToSharedRef(), 1000);
			}
			return Amount->GetToolTipText().ToString();
		}
	}
	return FString::Printf(TEXT("none (hit %s)"), *Hit.Widgets[Hit.Widgets.Num() - 1].Widget->GetTypeAsString());
}

void UHOLGameScreen::RefreshIdleWorkers()
{
	// H-3: the counter button is the script's; Describe reads this number.
	ShownIdleWorkers = Controller.IsValid() ? Controller->GetIdleWorkers().Num() : 0;
}

void UHOLGameScreen::GetGroupTypes(const int32 Group, TArray<FString>& OutNames, TArray<int32>& OutCounts)
{
	OutNames.Reset();
	OutCounts.Reset();
	const AHOLPlayerController* PlayerController = Controller.Get();
	const UWorld* World = GetWorld();
	TArray<uint32> Units;
	TArray<AHOLBuilding*> Buildings;
	if (!PlayerController || !World || !PlayerController->GetControlGroup(Group, Units, Buildings))
	{
		return;
	}
	TArray<FName> Order;
	TMap<FName, int32> PerType;
	const auto AddType = [&Order, &PerType](const FName Type)
	{
		if (int32* Count = PerType.Find(Type))
		{
			++*Count;
		}
		else
		{
			PerType.Add(Type, 1);
			Order.Add(Type);
		}
	};
	if (!Units.IsEmpty())
	{
		TArray<FHOLUnitView> Views;
		HOLUnitView::Gather(*World, Views);
		for (const FHOLUnitView& View : Views)
		{
			if (Units.Contains(View.UnitId))
			{
				AddType(View.TypeName);
			}
		}
	}
	for (const AHOLBuilding* Building : Buildings)
	{
		AddType(Building->GetTypeName());
	}
	for (const FName Type : Order)
	{
		const FText Name = Units.IsEmpty() ? HOLGameNames::GetBuildingName(Type, UHOLBuildingSettings::FindType(Type))
			: HOLGameNames::GetUnitName(Type, FindUnitType(Type), LocalFactionKey());
		OutNames.Add(Name.ToString());
		OutCounts.Add(PerType[Type]);
	}
}

namespace
{
	/** D869b: while the selected engine mounts / dismounts, the case D's face (label + text bar) and the key that makes the cell redraw as the bar fills. */
	bool ReadDeployFace(const APlayerController* Owner, FText& OutFace, FName& OutKey)
	{
		int32 Phase = -1;
		int32 Permille = -1;
		const AHOLPlayerController* HOLController = Cast<AHOLPlayerController>(Owner);
		if (!HOLController || !HOLController->GetDeployProgress(Phase, Permille) || Permille < 0)
		{
			return false;
		}
		const bool bMounting = HOLDeploy::ButtonLabel(HOLDeploy::PhaseAt(Phase)) == 2;
		const int32 Filled = FMath::Clamp(Permille * 8 / 1000, 0, 8);
		const FString Bar = FString::ChrN(Filled, TEXT('#')) + FString::ChrN(8 - Filled, TEXT('-'));
		OutFace = FText::FromString(FString::Printf(TEXT("%s\n[%s]"), bMounting ? TEXT("Montage\u2026") : TEXT("D\u00E9montage\u2026"), *Bar));
		OutKey = FName(*FString::Printf(TEXT("%s%d"), bMounting ? TEXT("Mounting") : TEXT("Dismounting"), Filled));
		return true;
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
	bHeroCellsShown = Commands.ContainsByPredicate([](const FHOLGridCommand& Command) { return Command.Command == EHOLGridCommand::HeroAbility; });
	// The builders' age panels only (not the whole script Refresh: its population alert reads the HUD's real time once per timer step).
	if (ScriptScreen)
	{
		ScriptScreen->RefreshAges();
	}

	// Only a site of the player's own, selected alone (or alone in a group's subgroup), has « Annuler » (HOLCommandGrid::GetCommand).
	const FHOLResources Stock = PlayerState ? PlayerState->GetResources() : FHOLResources();
	ShownTradeLabels.Reset();
	ShownTradeLabels.SetNum(AHOLPlayerController::NumGridCells);
	GridFrameFill.Reset();
	GridFrameFill.SetNum(AHOLPlayerController::NumGridCells);
	ShownMarketPrices.Reset();
	ShownEngineTier = INDEX_NONE;
	for (int32 Cell = 0; Cell < AHOLPlayerController::NumGridCells && Cell < ShownGridTypes.Num(); ++Cell)
	{
		const FHOLGridCommand Command = bGridShown && Commands.IsValidIndex(Cell) ? Commands[Cell]
			: bOrdersShown && Orders.IsValidIndex(Cell) ? Orders[Cell] : FHOLGridCommand();
		ShownGridTypes[Cell] = !bGridShown ? NAME_None : Command.Command == EHOLGridCommand::CancelConstruction ? CancelName
			: Command.Command == EHOLGridCommand::NecroticRage ? FName(TEXT("NecroticRage"))
			: Command.Command == EHOLGridCommand::LightSeal ? FName(TEXT("LightSeal"))
			: Command.Command == EHOLGridCommand::Specialize || Command.Command == EHOLGridCommand::CancelSpecialization ? HOLCommandGrid::GetReportName(Command)
: Command.Command == EHOLGridCommand::HeroAbility ? FName(Command.Ability == EHOLHeroAbility::Banner ? TEXT("Banner") : TEXT("CloseRanks"))
			: Command.Command == EHOLGridCommand::Trade ? FName(*FString::Printf(TEXT("%s%s"), Command.bBuy ? TEXT("Buy") : TEXT("Sell"),
				Command.Resource == EHOLResource::Wood ? TEXT("Wood") : Command.Resource == EHOLResource::Stone ? TEXT("Stone") : TEXT("Food")))
			: Command.TypeName;
		if (ShownOrderTypes.IsValidIndex(Cell))
		{
			ShownOrderTypes[Cell] = Command.Command == EHOLGridCommand::AttackMove ? FName(TEXT("AttackMove"))
				: Command.Command == EHOLGridCommand::Stop ? FName(TEXT("Stop")) : Command.Command == EHOLGridCommand::Hold ? FName(TEXT("Hold"))
				: Command.Command == EHOLGridCommand::Deploy ? ([&]() { FText Face; FName Key; return ReadDeployFace(GetOwningPlayer(), Face, Key) ? Key : FName(Command.bPack ? TEXT("Pack") : TEXT("Deploy")); })() : NAME_None;
		}
		// HUD H-10: the cell is the script's; what it shows is written to the cache the script's panel applies (same method names, the switch is as it was).
		FHOLGridCellFill* GridCell = &GridFrameFill[Cell];
		switch (Command.Command)
		{
		case EHOLGridCommand::Produce:
		{
			// Greyed when the server would refuse it (D282): queue full (every queue of a group) or not enough resources;
			// still clickable, the click says why (D313). The population is only checked when the unit starts: a full
			// population makes it wait.
			const FHOLUnitTypeRow* UnitType = FindUnitTypeFor(Command.TypeName, PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE);
			const FHOLResources Cost = UnitType ? UnitType->Cost : FHOLResources();
			const FHOLResources Missing = GetMissing(Stock, Cost);
			const bool bQueueFull = !PlayerController || !PlayerController->FindProductionBuilding(Command.TypeName);
			const FText Name = HOLGameNames::GetUnitName(Command.TypeName, UnitType, LocalFactionKey());
			// S7 (siege engines, GDD 6.1): an engine whose tier the hero has not opened is greyed too, with the reason in its tooltip
			// (the server refuses it by the same rule, HOLSiege::Check; the workshop is there, it is its grid, the stock is above).
			// G1 (D711): the tier of every unit is read from its MinTier column.
			// MIG-CPP-UnitTier (D836 point 3): the rule is the script's alone, reached by the bridge UHOLUnitTierSubsystem (born in every game world, client included).
			const bool bTierBlocked = PlayerState && BoundTableState.IsValid() && PlayerController
				&& UHOLUnitTierSubsystem::CheckUnit(PlayerController->GetWorld(), Command.TypeName, BoundTableState->GetTier(PlayerState->GetPlayerIndex()), true, true) == HOLUnitTierReason::TierTooLow;
			if (PlayerState && BoundTableState.IsValid())
			{
				ShownEngineTier = BoundTableState->GetTier(PlayerState->GetPlayerIndex());
			}
			GridCell->SetCommand(Name, bQueueFull || bTierBlocked || !(Missing == FHOLResources()) ? EHOLGridCellState::Greyed : EHOLGridCellState::Ready);
			GridCell->SetTooltip(Name, Cost, Missing, bTierBlocked ? GetResearchBlockText(HOLTechnology::EBlock::TierTooLow) : GetUnitEffect(Command.TypeName));
			break;
		}
		case EHOLGridCommand::Research:
		{
			// A technology (T4, GDD 11): its name on the cell, its cost in the tooltip; greyed when the rules, the stock or the queue
			// would refuse it (still clickable, the click says why), the reason under its name in the tooltip.
			const FHOLTechnologyRow* Technology = UHOLTechnologySettings::FindTechnology(Command.TypeName);
			// BUG forge-tier1-grid: the tier the hero has opened decides the refusal of a technology (TierTooLow): the grid is drawn again when it changes (the tick compares ShownEngineTier).
			if (PlayerState && BoundTableState.IsValid())
			{
				ShownEngineTier = BoundTableState->GetTier(PlayerState->GetPlayerIndex());
			}
			const FHOLResources Cost = Technology ? Technology->Cost : FHOLResources();
			const FHOLResources Missing = GetMissing(Stock, Cost);
			const FText Name = HOLGameNames::GetTechnologyName(Command.TypeName, Technology);
			bool bAny = false;
			const AHOLBuilding* Researcher = PlayerController ? PlayerController->FindProductionBuilding(Command.TypeName, &bAny) : nullptr;
			FHOLCommandFeedback Feedback;
			if (Technology && !Technology->Implemented)
			{
				// T-Q10/T-Q11 = B (D601): no effect coded yet, greyed "Bientôt" whatever the building, the tier or the stock.
				Feedback.Kind = EHOLCommandFeedback::ResearchRefused;
				Feedback.Block = HOLTechnology::EBlock::NotImplemented;
			}
			else if (Researcher)
			{
				Feedback = PlayerController->CheckResearchAt(Command.TypeName, Researcher);
			}
			else if (bAny)
			{
				Feedback.Kind = EHOLCommandFeedback::QueueFull;
			}
			const FText Effect = Feedback.Kind == EHOLCommandFeedback::ResearchRefused ? GetResearchBlockText(Feedback.Block)
				: Technology ? HOLGameNames::GetTechnologyEffect(*Technology) : FText();
			GridCell->SetCommand(Name, Feedback.IsNone() ? EHOLGridCellState::Ready : EHOLGridCellState::Greyed);
			if (Feedback.Kind == EHOLCommandFeedback::ResearchRefused && Feedback.Block == HOLTechnology::EBlock::NotImplemented)
			{
				// GALADRIEL-1122g: the status is a line of its own at the bottom of the cell (same text as the tooltip's, three languages), the name keeps a bounded zone.
				GridCell->SetStatus(GetResearchBlockText(HOLTechnology::EBlock::NotImplemented));
			}
			GridCell->SetTooltip(Name, Cost, Missing, Effect);
			break;
		}
		case EHOLGridCommand::Specialize:
		case EHOLGridCommand::CancelSpecialization:
		{
			// SP3 (D332, D684): the main Forum's choice of the open tier, free; running, its cell shows the seconds left and cancels at no cost.
			// Provisional names (Placeholders.txt): the GDD 13 names of each faction's options are not in the text table yet.
			// The cell's text and the option's name come from the game's text table (HOLGameNames): running, the name is in the tooltip only.
			const FText Name = HOLGameNames::GetSpecializationName(Command.Tier, Command.Option);
			const bool bRunning = Command.Command == EHOLGridCommand::CancelSpecialization;
			GridCell->SetCommand(HOLGameNames::GetSpecializationCellText(Command), EHOLGridCellState::Ready);
			GridCell->SetTooltip(Name, FHOLResources(), FHOLResources(), bRunning
				? LOCTEXT("SpecializationRunningEffect", "Déblocage en cours au Forum. Cliquer annule sans coût ; le choix reste ouvert.")
				: LOCTEXT("SpecializationEffect", "Gratuit. Se débloque au Forum en 30 / 45 / 60 s (palier I / II / III), en parallèle de sa file. Définitif une fois appliqué."));
			break;
		}
		case EHOLGridCommand::Build:
		{
			// The builders' grid (D313): greyed when the stock does not cover the cost (in red in the tooltip).
			const FHOLBuildingTypeRow* BuildType = UHOLBuildingSettings::FindType(Command.TypeName);
			// The seat's own price (a Nomad's first forum costs the secondary centre's price, D783), as the server charges it.
			const FHOLResources Cost = UHOLBuildingSettings::GetBuildCost(GetWorld(), Command.TypeName,
				BoundPlayerState.Get() ? BoundPlayerState->GetPlayerIndex() : INDEX_NONE);
			const FHOLResources Missing = GetMissing(Stock, Cost);
			const FText Name = HOLGameNames::GetBuildingName(Command.TypeName, BuildType);
			GridCell->SetCommand(Name, Missing == FHOLResources() ? EHOLGridCellState::Ready : EHOLGridCellState::Greyed);
			GridCell->SetTooltip(Name, Cost, Missing, GetBuildingEffect(BuildType, LocalFactionKey(), GetWorld(),
					BoundPlayerState.Get() ? BoundPlayerState->GetPlayerIndex() : INDEX_NONE));
			break;
		}
		case EHOLGridCommand::Trade:
		{
			// M5 (Market, GDD 5.5): a lot of 100 goods sold or bought at the common book's price (the lot's quote on the cell, the
			// price after it in the tooltip); greyed when the stock would not pay it (still clickable, the click says why).
			if (!bMarketSettingsTried)
			{
				bMarketSettingsTried = true;
				bMarketSettingsLoaded = HOLMarket::LoadStandardSettings(MarketSettings);
			}
			const int32 Good = static_cast<int32>(Command.Resource);
			FText Label = GetResourceName(Command.Resource);
			FText Name = FText::Format(Command.bBuy ? LOCTEXT("MarketBuyName", "Acheter : {0}") : LOCTEXT("MarketSellName", "Vendre : {0}"),
				GetResourceNameInText(Command.Resource));
			FText Effect = LOCTEXT("MarketNoBook", "Le cours n'est pas encore connu");
			FHOLResources Cost;
			bool bGreyed = true;
			bool bQuoted = false;
			FHOLTradeGain Gain;
			if (bMarketSettingsLoaded && BoundTableState.IsValid() && BoundTableState->HasMarketBook() && Good >= 0 && Good < HOLMarket::TradableResources)
			{
				const HOLMarket::FBook Book = BoundTableState->GetMarketBook();
				ShownMarketPrices.Reset();
				ShownMarketPrices.Append(Book.Prices, HOLMarket::BookSize);
				// The rate is always for 100 of what is given up (the user's correction, 2026-11-01): buying, 100 gold against X of
				// the good; selling, 100 of the good against Y gold. The same lot the click sends, priced by the script's rule the
				// server applies (UHOLMarketSubsystem::Quote, D836 point 4); refused (no script board), the price stays unknown.
				const HOLMarket::FTrade Quote = UHOLMarketSubsystem::Quote(GetWorld(), MarketSettings, Book, Good, 1, Command.bBuy);
				if (Quote.bOk)
				{
					const int32 Received = Command.bBuy ? Quote.Units : Quote.Gold;
					if (Command.bBuy)
					{
						Cost.Gold = HOLMarket::Lot;
					}
					else
					{
						Cost[Command.Resource] = HOLMarket::Lot;
					}
					// D781 (the user's REGLAGE on the trial `market`, 2026-11-11): under the good's name, « +X » then the logo of what is
					// received (the good bought, gold for a sale); no « 100 or = X » any more (the rate stays in the tooltip).
					Gain = HOLCommandGrid::GetTradeGain(Command.Resource, Command.bBuy, Received);
					bQuoted = true;
					Effect = Command.bBuy
						? FText::Format(LOCTEXT("MarketBuyEffect", "100 or contre {0} {1}, taxe comprise ; le cours monte à {2}. Maj : 500 or d'un coup"),
							FText::AsNumber(Received), GetResourceNameInText(Command.Resource), FText::AsNumber(Quote.Book.Prices[Good]))
						: FText::Format(LOCTEXT("MarketSellEffect", "100 {0} contre {1} or, taxe déduite ; le cours baisse à {2}. Maj : 500 {0} d'un coup"),
							GetResourceNameInText(Command.Resource), FText::AsNumber(Received), FText::AsNumber(Quote.Book.Prices[Good]));
					bGreyed = !(GetMissing(Stock, Cost) == FHOLResources());
				}
			}
			GridCell->SetCommand(Label, bGreyed ? EHOLGridCellState::Greyed : EHOLGridCellState::Ready);
			if (bQuoted)
			{
				GridCell->SetGain(Gain);
			}
			GridCell->SetTooltip(Name, Cost, GetMissing(Stock, Cost), Effect);
			// What the cell itself shows (the name, the gain and its logo: "Bois +96 [Wood]") is read back from its widgets by ApplyGridCell into ShownTradeLabels.
			GridCell->bTrade = true;
			break;
		}
		case EHOLGridCommand::UngarrisonAll:
			GridCell->SetCommand(LOCTEXT("UngarrisonAll", "Sortir"), EHOLGridCellState::Ready);
			GridCell->SetTooltip(LOCTEXT("UngarrisonAllName", "Sortir tous"), FHOLResources(), FHOLResources(), LOCTEXT("UngarrisonAllEffect", "Toute la garnison sort du b\u00e2timent"));
			break;
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
		case EHOLGridCommand::Deploy:
		{
			// G6 d: PROPOSED labels (Retours/Q-palier3-deploy-button.md); no icon yet, the cell shows its text (Placeholders.txt).
			FText DeployFace; FName DeployKey; // D869b: the face while mounting / dismounting (label + bar), the settled label otherwise
			const bool bChanging = ReadDeployFace(GetOwningPlayer(), DeployFace, DeployKey);
			GridCell->SetCommand(bChanging ? DeployFace : Command.bPack ? LOCTEXT("Pack", "Démonter") : LOCTEXT("Deploy", "Monter"), EHOLGridCellState::Ready);
			GridCell->SetTooltip(Command.bPack ? LOCTEXT("PackName", "Démonter l'engin") : LOCTEXT("DeployName", "Monter l'engin"),
				FHOLResources(), FHOLResources(),
				Command.bPack ? LOCTEXT("PackEffect", "Replie l'engin : il ne tire plus, il peut marcher")
					: LOCTEXT("DeployEffect", "Installe l'engin : il tire, il ne marche plus"));
			break;
		}
			case EHOLGridCommand::HeroAbility:
			case EHOLGridCommand::LightSeal:
			case EHOLGridCommand::NecroticRage:
			{
				// G-5b (SAURON-1125md, D839): the FACE of these three cells (label, greyed or ready, tooltip) is the script's rule
				// HOLCommandGridFace::GetCellFace (Script/Player/HOLCommandGridFace.as, through UHOLGridFaceBridge); the C++ only reads the facts
				// the rule cannot see (cool-down seconds, hero level, the simulation's numbers) and keeps SetCommand / SetTooltip.
				int32 FaceCooldownSeconds = 0;
				int32 FaceUnlockLevel = 0;
				TArray<int32> FaceNumbers;
				if (Command.Command == EHOLGridCommand::HeroAbility)
				{
					// J8c.4: locked below its level (the level shows in its tooltip, J8-Q5: the owner's alone).
					const bool bLocked = !HOLHeroAbility::IsUnlocked(Command.Ability, PlayerState
						? UHOLHeroLibrary::GetHeroLevel(PlayerController, PlayerState->GetPlayerIndex()) : 0);
					FaceUnlockLevel = bLocked ? HOLHeroAbility::GetUnlockLevel(Command.Ability) : 0;
				}
				else if (Command.Command == EHOLGridCommand::LightSeal)
				{
					// S1-d: the seconds left of the Seal's cool-down (the slot's PowerCooldownEndMs 0, the one ServerCastUnitAbility writes).
					const AHOLGameState* SealState = GetWorld() ? GetWorld()->GetGameState<AHOLGameState>() : nullptr;
					if (SealState && PlayerState)
					{
						constexpr int32 SealCooldownIndex = 0; // HOLRpcPlayer.as ServerCastUnitAbility: SealCooldownIndex.
						const int32 NowMs = FMath::FloorToInt(GetWorld()->GetTimeSeconds() * 1000.0);
						const int32 SealLeftMs = SealState->GetPowerCooldownEndMs(PlayerState->GetPlayerIndex(), SealCooldownIndex) - NowMs;
						FaceCooldownSeconds = SealLeftMs > 0 ? FMath::CeilToInt(SealLeftMs / 1000.f) : 0;
					}
					FaceNumbers = { HOLZoneEffect::LightSealRadiusCm / 100, HOLZoneEffect::LightSealDurationMs / 1000, HOLZoneEffect::LightSealSlowPercent };
				}
				else
				{
					// L4 / hol.Rage.Visual 3 or 4 (default 4, D772): the seconds left of the rage, then of the recharge, of the first selected unit cooling down.
					if (HOLRageVisual::HasIcon(HOLRageVisual::GetMode()) && PlayerController)
					{
						for (const uint32 RageUnitId : PlayerController->GetOwnSelectedUnits())
						{
							int32 RageMs = 0;
							int32 RageCooldownMs = 0;
							if (HOLRageVisual::GetRageState(PlayerController->GetWorld(), RageUnitId, RageMs, RageCooldownMs) && RageCooldownMs > 0)
							{
								FaceCooldownSeconds = FMath::CeilToInt((RageMs > 0 ? RageMs : RageCooldownMs) / 1000.f);
								break;
							}
						}
					}
					// The simulation's own constants (HOLUnitAbilityRules.h, kept equal to UnitAbilities.csv by Combat.DamnedWarrior.RageConstants).
					FaceNumbers = { HOLUnitAbility::RageDurationMs / 1000, HOLUnitAbility::RageDamagePercent, HOLUnitAbility::RageRatePercent,
						HOLUnitAbility::RageHpCostPerHit, HOLUnitAbility::RageMinHp, HOLUnitAbility::RageStopHp, HOLUnitAbility::RageCooldownMs / 1000 };
				}
				FHOLGridFaceAnswer Face;
				if (UHOLGridFaceBridge::NativeGetCellFace(static_cast<int32>(Command.Command), static_cast<int32>(Command.Ability), Command.bGreyed,
					Command.CooldownSeconds, FaceCooldownSeconds, FaceUnlockLevel, FaceNumbers, Face))
				{
					GridCell->SetCommand(Face.Label, Face.State == HOLGridFaceState::Greyed ? EHOLGridCellState::Greyed : EHOLGridCellState::Ready);
					GridCell->SetTooltip(Face.TooltipName, FHOLResources(), FHOLResources(), Face.TooltipEffect);
				}
				else
				{
					GridCell->ClearUnit(); // no script class: no C++ twin of the rule, the cell stays empty
				}
				break;
			}
		default:
			// An empty cell stays, dark (disabled), at its key's place.
			GridCell->ClearUnit();
			break;
		}
	}
	// HUD H-10: the script's grid panel shows the frame just computed.
	if (ScriptScreen)
	{
		ScriptScreen->RefreshGridCells();
	}
}

void UHOLGameScreen::SetGridCells(const TArray<UHOLGridCell*>& Cells)
{
	GridCells.Reset();
	for (UHOLGridCell* Cell : Cells)
	{
		GridCells.Add(Cell);
	}
}

void UHOLGameScreen::ReadGridFrame(TArray<int32>& OutKinds, TArray<int32>& OutStates, TArray<FName>& OutOrderTypes, bool& bOutGrid, bool& bOutOrders) const
{
	OutKinds.Reset();
	OutStates.Reset();
	OutOrderTypes.Reset();
	for (int32 Cell = 0; Cell < GridFrameFill.Num(); ++Cell)
	{
		OutKinds.Add(GridFrameFill[Cell].bCommand ? 1 : 0);
		OutStates.Add(static_cast<int32>(GridFrameFill[Cell].State));
		OutOrderTypes.Add(ShownOrderTypes.IsValidIndex(Cell) ? ShownOrderTypes[Cell] : NAME_None);
	}
	bOutGrid = bGridShown;
	bOutOrders = bOrdersShown;
}

bool UHOLGameScreen::ApplyGridCell(const int32 Cell, UHOLGridCell* Target)
{
	if (!Target || !GridFrameFill.IsValidIndex(Cell) || !GridFrameFill[Cell].bCommand)
	{
		return false;
	}
	const FHOLGridCellFill& Fill = GridFrameFill[Cell];
	Target->SetCommand(Fill.Name, Fill.State);
	if (!Fill.Status.IsEmpty())
	{
		// After SetCommand, which clears the status (the cell is reused for the next command).
		Target->SetStatus(Fill.Status);
	}
	if (Fill.bGain)
	{
		Target->SetGain(Fill.Gain);
	}
	Target->SetTooltip(Fill.TooltipName, Fill.Cost, Fill.Missing, Fill.Effect);
	if (Fill.bTrade && ShownTradeLabels.IsValidIndex(Cell))
	{
		ShownTradeLabels[Cell] = Target->DescribeContent();
	}
	return true;
}

int32 UHOLGameScreen::GetBoundTier() const
{
	return BoundPlayerState.IsValid() && BoundTableState.IsValid() ? BoundTableState->GetTier(BoundPlayerState->GetPlayerIndex()) : INDEX_NONE;
}

void UHOLGameScreen::FillAgeCell(UHOLGridCell* Cell, const FName TypeName, const bool bGreyed)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	if (!Cell)
	{
		return;
	}
	// RefreshAges' body for a building (HUD H-8): greyed when its age is not reached, or when the stock does not cover the cost (in red in the tooltip, D313).
	const int32 PlayerIndex = BoundPlayerState.IsValid() ? BoundPlayerState->GetPlayerIndex() : INDEX_NONE;
	const FHOLResources Stock = BoundPlayerState.IsValid() ? BoundPlayerState->GetResources() : FHOLResources();
	const FHOLBuildingTypeRow* BuildType = UHOLBuildingSettings::FindType(TypeName);
	// The seat's own price (a Nomad's first forum costs the secondary centre's price, D783), as the server charges it.
	const FHOLResources Cost = UHOLBuildingSettings::GetBuildCost(GetWorld(), TypeName, PlayerIndex);
	const FHOLResources Missing = GetMissing(Stock, Cost);
	const FText Name = HOLGameNames::GetBuildingName(TypeName, BuildType);
	Cell->SetCommand(Name, !bGreyed && Missing == FHOLResources() ? EHOLGridCellState::Ready : EHOLGridCellState::Greyed);
	Cell->SetTooltip(Name, Cost, Missing, bGreyed ? LOCTEXT("AgeLocked", "Palier non atteint") : GetBuildingEffect(BuildType, LocalFactionKey(), GetWorld(), PlayerIndex));
}

void UHOLGameScreen::SetAgesMirror(const bool bShown, const int32 Chosen, const TArray<FName>& Types, const TArray<bool>& Reached, const TArray<FString>& Tooltips)
{
	bAgesShown = bShown;
	// The tier the script has just drawn the panels at (it read GetTier in the same call): kept at every draw, not only by
	// NativeTick's check. Left to that check alone it went stale when a draw came from elsewhere (a selection): a tier
	// lowered (hol.Cheat.Tier off) then back to the stale value was never drawn (quick_check "build tiers": II locked at level 3).
	ShownTier = GetBoundTier();
	ShownChosenAge = Chosen;
	ShownAgeTypes = Types;
	ShownAgeReached = Reached;
	ShownAgeTooltips = Tooltips;
}

bool UHOLGameScreen::ReadQueue(TArray<FName>& OutTypes, TArray<bool>& OutResearch, TArray<FString>& OutTechNames, TArray<FString>& OutUnitNames,
	bool& bOutHeadStarted, float& OutHeadProgress, bool& bOutBlockedAtFull, bool& bOutPopulationShort) const
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	OutTypes.Reset();
	OutResearch.Reset();
	OutTechNames.Reset();
	OutUnitNames.Reset();
	bOutHeadStarted = false;
	OutHeadProgress = 0.f;
	bOutBlockedAtFull = false;
	bOutPopulationShort = false;

	// RefreshQueue(Own): only the owner receives its building's queue (D282).
	const AHOLPlayerController* PlayerController = Controller.Get();
	const AHOLBuilding* Own = PlayerController ? PlayerController->GetOwnSelection() : nullptr;
	if (!Own)
	{
		return false;
	}
	for (const FHOLProductionItem& Item : Own->GetProductionQueue())
	{
		// A research waits in the queue as a unit does (T6): its name, not the type's key.
		const FHOLTechnologyRow* Research = UHOLTechnologySettings::FindTechnology(Item.TypeName);
		OutTypes.Add(Item.TypeName);
		OutResearch.Add(Research != nullptr);
		OutTechNames.Add(Research ? HOLGameNames::GetTechnologyName(Item.TypeName, Research).ToString() : FString());
		OutUnitNames.Add(Research ? FString() : HOLGameNames::GetUnitName(Item.TypeName, FindUnitType(Item.TypeName), LocalFactionKey()).ToString());
	}
	if (OutTypes.IsEmpty())
	{
		return true;
	}

	// UpdateHead: the queue shown is the selected building's (BoundBuilding), the head its first item.
	const AHOLBuilding* Building = BoundBuilding.Get();
	const TArray<FHOLProductionItem>* Queue = Building ? &Building->GetProductionQueue() : nullptr;
	if (!Queue || Queue->IsEmpty())
	{
		return true;
	}
	const FHOLProductionItem& Head = (*Queue)[0];
	const FHOLUnitTypeRow* UnitType = FindUnitType(Head.TypeName);
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	const int32 Places = UnitType ? UnitType->Population : 1;
	bOutHeadStarted = Head.IsStarted();
	if (bOutHeadStarted)
	{
		// Only the start step travels (D282): the progress is computed every frame, at the server's clock as this machine estimates it (D271).
		const int32 BuildSteps = UnitType
			? UHOLProductionSubsystem::GetBuildSteps(*UnitType, GetDefault<UHOLSimulationSettings>()->GetStepSeconds())
			: 1;
		OutHeadProgress = HOLProduction::GetProgress(Head, UHOLProductionSubsystem::GetDisplayStep(GetWorld()), BuildSteps);
		// D829: done but no place in the population (the script adds the 100 % test).
		bOutBlockedAtFull = PlayerState && HOLCommandGrid::IsPopulationBlocked(PlayerState->GetPopulationUsed(), PlayerState->GetPopulationCap(), Places);
	}
	else
	{
		// Not started: it waits for room in the population (as in AoE), or starts at the server's next step.
		bOutPopulationShort = PlayerState && PlayerState->GetPopulationUsed() + Places > PlayerState->GetPopulationCap();
	}
	return true;
}

void UHOLGameScreen::ClickGridCell(const int32 Cell)
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
		PlayerController->ActivateOrderCell(Cell);
		return;
	}
	PlayerController->ActivateGridCell(Cell);
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
	// H-2: the script's MessagePanel draws the message, fades it and writes the "HUD message: ..." line (HOLShortMessage.as); MessageKey is Describe's mirror.
	MessageKey = Key;
	if (ScriptScreen)
	{
		ScriptScreen->ShortMessage(Key, Text);
	}
	else
	{
		UE_LOG(LogHOLUI, Display, TEXT("HUD message: %s \"%s\""), *Key, *Text.ToString());
	}
}
void UHOLGameScreen::RefreshScriptNow()
{
	if (ScriptScreen)
	{
		ScriptScreen->RefreshNow();
	}
}

bool UHOLGameScreen::IsProductionHeldForRoom() const
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	// The player's buildings whose next unit waits for room (their queues reach their owner alone, D282).
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	const UWorld* World = GetWorld();
	bool bBlocked = false;
	if (PlayerState && World)
	{
		for (FHOLBuildingIterator It(World); It && !bBlocked; ++It)
		{
			const TArray<FHOLProductionItem>& Queue = It->GetProductionQueue();
			// D829: blocked = the head is done (100 %) and has no place to come out; a unit in production holds no place.
			const FHOLUnitTypeRow* HeadType = Queue.IsEmpty() ? nullptr : FindUnitType(Queue[0].TypeName);
			if (It->GetOwnerPlayer() != PlayerState->GetPlayerIndex() || !It->IsFinished() || !HeadType
				|| !HOLProduction::IsWaitingForRoom(Queue[0], UHOLProductionSubsystem::GetDisplayStep(World),
					UHOLProductionSubsystem::GetBuildSteps(*HeadType, GetDefault<UHOLSimulationSettings>()->GetStepSeconds())))
			{
				continue;
			}
			const FHOLUnitTypeRow* UnitType = FindUnitType(Queue[0].TypeName);
			bBlocked = HOLCommandGrid::IsPopulationBlocked(PlayerState->GetPopulationUsed(), PlayerState->GetPopulationCap(), UnitType ? UnitType->Population : 1);
		}
	}
	return bBlocked;
}

FString UHOLGameScreen::DescribeOverlaps() const
{
	return ScriptScreen ? ScriptScreen->DescribeOverlaps() : FString(TEXT("none"));
}

FString UHOLGameScreen::DescribeRects() const
{
	return ScriptScreen ? ScriptScreen->DescribeRects() : FString();
}

FString UHOLGameScreen::DescribeCheats() const
{
	return ScriptScreen ? ScriptScreen->GetCheatsReport() : FString();
}

void UHOLGameScreen::ApplyCheatsOpen(const bool bOpen)
{
	if (ScriptScreen)
	{
		ScriptScreen->ApplyCheatsOpen(bOpen);
	}
}

FString UHOLGameScreen::DescribeScript() const
{
	if (!ScriptScreen)
	{
		return TEXT("script: none");
	}
	ScriptScreen->RefreshNow();
	return ScriptScreen->Describe();
}

FString UHOLGameScreen::DescribeLayout() const
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;
	if (!BottomBar || !MinimapPlace)
	{
		return TEXT("none");
	}
	// The window's size in pixels and its DPI scale (the shortest side's curve): the HUD's widths are in Slate units.
	const FVector2D Viewport = UWidgetLayoutLibrary::GetViewportSize(this);
	const float Dpi = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), UE_KINDA_SMALL_NUMBER);
	const FGeometry& BarGeometry = BottomBar->GetCachedGeometry();
	const FGeometry& PlaceGeometry = MinimapPlace->GetCachedGeometry();
	const float BarDesired = BottomBar->GetDesiredSize().X;
	const float BarShown = BarGeometry.GetAbsoluteSize().X / Dpi;
	const float BarRight = (BarGeometry.GetAbsolutePosition().X + BarGeometry.GetAbsoluteSize().X) / Dpi;
	const float PlaceLeft = PlaceGeometry.GetAbsolutePosition().X / Dpi;
	// The age medallions rise over the bar's top edge: the first one's top against the scale box's top (the box clips its
	// content to its own area) and the window's top; "cut" = what of it is out of the one or the other (0 = all drawn);
	// "none" while the age panels are not shown.
	FString Medallion = TEXT("none");
	const UBorder* FirstMedallion = Cast<UBorder>(WidgetTree->FindWidget(TEXT("AgeMedallion0")));
	const UBorder* FirstPanel = Cast<UBorder>(WidgetTree->FindWidget(TEXT("AgePanel0")));
	if (FirstMedallion && FirstPanel && FirstPanel->GetVisibility() != ESlateVisibility::Collapsed && BottomScale)
	{
		const float MedallionTop = HOLHUDSpace::BoxOf(this, FirstMedallion->GetCachedGeometry()).Min.Y;
		const float ClipTop = HOLHUDSpace::BoxOf(this, BottomScale->GetCachedGeometry()).Min.Y;
		Medallion = FString::Printf(TEXT("top %d clip %d cut %d"), FMath::RoundToInt(MedallionTop), FMath::RoundToInt(ClipTop),
			FMath::Max(0, FMath::Max(FMath::RoundToInt(ClipTop) - FMath::RoundToInt(MedallionTop), -FMath::RoundToInt(MedallionTop))));
	}
	return FString::Printf(TEXT("viewport %dx%d, dpi %.3f, width %d, bar %d shown %d (scale %.2f) right %d, minimap %d left %d, gap %d, medallion %s"),
		FMath::RoundToInt(Viewport.X), FMath::RoundToInt(Viewport.Y), Dpi, FMath::RoundToInt(Viewport.X / Dpi),
		FMath::RoundToInt(BarDesired), FMath::RoundToInt(BarShown), BarDesired > 0.f ? BarShown / BarDesired : 1.f, FMath::RoundToInt(BarRight),
		FMath::RoundToInt(MinimapSize), FMath::RoundToInt(PlaceLeft), FMath::RoundToInt(PlaceLeft - BarRight), *Medallion);
}

float UHOLGameScreen::GetBottomScale() const
{
	if (!BottomBar)
	{
		return 1.f;
	}
	// The bar's layout scale over the window's DPI scale: the scale box's share; 1 before the first layout.
	const FGeometry& BarGeometry = BottomBar->GetCachedGeometry();
	const float Dpi = UWidgetLayoutLibrary::GetViewportScale(this);
	const float Scale = BarGeometry.GetAccumulatedLayoutTransform().GetScale();
	return BarGeometry.GetLocalSize().X > 0.f && Dpi > UE_KINDA_SMALL_NUMBER && Scale > UE_KINDA_SMALL_NUMBER ? FMath::Min(Scale / Dpi, 1.f) : 1.f;
}

FMargin UHOLGameScreen::GetBottomLeftPlace() const
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;
	// The control groups' bar: its slots (QueueItemHeight * 0.8) and its panel's padding (4 above and below); the column
	// (bar, gap, groups) at the scale it is drawn at (EtapeDefini T).
	constexpr float GroupsHeight = QueueItemHeight * 0.8f + 2.f * 4.f;
	return FMargin(ScreenMargin, 0.f, 0.f, ScreenMargin + GetBottomScale() * (BarHeight + BarGap + GroupsHeight) + BarGap);
}

bool UHOLGameScreen::ClickWidget(const FName Name)
{
	if (ScriptScreen && ScriptScreen->ClickWidget(Name))
	{
		return true;
	}
	// HUD H-10: GridCell00..11 are the script's too (UHOLCommandGridPanel.ClickName, same path); the journal lines are the script's (MenuLog).
	// HUD H-7: QueueItem0..4 are the script's (UHOLProductionQueuePanel.ClickName, through UHOLScriptScreen::ClickWidget above).
	// HUD H-8: the age panels' buildings (AgeCell<age><cell>) are the script's too (UHOLAgePanelsPanel.ClickName, same path).
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
					const UHOLGridCell* AgeCell = Cast<UHOLGridCell>(WidgetTree->FindWidget(AgeCellName(AgeIndex / NumAgeCells, AgeIndex % NumAgeCells)));
					const EHOLGridCellState State = AgeCell ? AgeCell->GetState() : EHOLGridCellState::Disabled;
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

		// HUD H-7: the queue part is the script's (UHOLProductionQueuePanel.Describe: "N [Type state, ...]", written by SetQueueReport at each refresh).
		Queue = QueueReport;
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
	// M5: the text drawn on a Market's trade cells, last on the line ("trades A=Nourriture +96 [Food] | Z=...", D781).
	TArray<FString> Trades;
	for (int32 Cell = 0; bGridShown && Cell < ShownTradeLabels.Num(); ++Cell)
	{
		if (!ShownTradeLabels[Cell].IsEmpty())
		{
			Trades.Add(FString::Printf(TEXT("%s=%s"), GridKeys[Cell], *ShownTradeLabels[Cell]));
		}
	}
	const FString TradeText = Trades.IsEmpty() ? FString() : FString::Printf(TEXT(", trades %s"), *FString::Join(Trades, TEXT(" | ")));
	// A main forum's lives and state (V2.3b) and the players' table (V2.4): on their own line (DescribeForum).
	// HUD H-9: the selection part is the script's (UHOLSelectionPanel, the only writer, by SetSelectionReport): the kind ("King (own)", "none"), the activity, the health.
	const FString& ReportKind = ScriptScreen ? ScriptScreen->GetSelectionReportKind() : ShownSelection;
	const FString& ReportActivity = ScriptScreen ? ScriptScreen->GetSelectionReportActivity() : ShownActivity;
	const FString& ReportHealth = ScriptScreen ? ScriptScreen->GetSelectionReportHealth() : ShownHealth;
	const FString& ReportForum = ScriptScreen ? ScriptScreen->GetSelectionReportForum() : ShownForum;
	const FString Activity = !ReportForum.IsEmpty() && ReportActivity == ReportForum ? FString() : ReportActivity;
	return FString::Printf(TEXT("resources %s, selection %s%s%s, grid %s, queue %s%s%s%s"), *Economy, *ReportKind, *Activity, *ReportHealth, *Grid, *Queue,
		*Message, *Workers, *TradeText);
}

FString UHOLGameScreen::DescribeForum() const
{
	// "selection, in ruin 27 s, lives 1, players [slot 0 you colour 0 lives 1 ruin | ...]" ("selection none" without a main forum selected).
	// HUD H-9: the forum line and the activity are the script's copies (SetSelectionReport).
	const FString ReportForum = ScriptScreen ? ScriptScreen->GetSelectionReportForum() : ShownForum;
	const FString ReportActivity = ScriptScreen ? ScriptScreen->GetSelectionReportActivity() : ShownActivity;
	const bool bForum = !ReportForum.IsEmpty() && ReportActivity == ReportForum;
	const TArray<FHOLPlayersTableRow> TableRows = UHOLPlayersTableReadLibrary::BuildRows(BoundTableState.Get(), BoundPlayerState.IsValid() ? BoundPlayerState->GetPlayerIndex() : INDEX_NONE);
	// HUD H-4: the table is the script's now; its rows are read again here (HOLForumRules::GetPlayersTable, one source) for the report line.
	const FString ShownPlayersTable = FString::JoinBy(TableRows, TEXT(" | "), [](const FHOLPlayersTableRow& Row) { return Row.Shown; });
	return FString::Printf(TEXT("selection%s, players [%s]"), bForum ? *ReportForum : TEXT(" none"), *ShownPlayersTable);
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
	for (int32 AgeIndex = 0; bAgesShown && AgeIndex < HOLCommandGrid::NumAges * NumAgeCells; ++AgeIndex)
	{
		const UHOLGridCell* Cell = ShownAgeTypes.IsValidIndex(AgeIndex) && !ShownAgeTypes[AgeIndex].IsNone() ? Cast<UHOLGridCell>(WidgetTree->FindWidget(AgeCellName(AgeIndex / NumAgeCells, AgeIndex % NumAgeCells))) : nullptr;
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
			const UHOLGridCell* Cell = Index != INDEX_NONE && ShownAgeTypes.IsValidIndex(AgeIndex)
				&& !ShownAgeTypes[AgeIndex].IsNone() ? Cast<UHOLGridCell>(WidgetTree->FindWidget(AgeCellName(AgeIndex / NumAgeCells, AgeIndex % NumAgeCells))) : nullptr;
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

FText UHOLGameScreen::UnitNameFor(const FName TypeName, const int32 OwnerPlayer) const
{
	const FName FactionKey = OwnerPlayer >= 0 ? HOLFactionRules::GetSlotFactionKey(GetWorld(), OwnerPlayer) : NAME_None;
	return HOLGameNames::GetUnitName(TypeName, FindUnitTypeFor(TypeName, OwnerPlayer), FactionKey);
}

FName UHOLGameScreen::LocalFactionKey() const
{
	const AHOLPlayerState* PlayerState = BoundPlayerState.Get();
	return PlayerState ? HOLFactionRules::GetSlotFactionKey(GetWorld(), PlayerState->GetPlayerIndex()) : NAME_None;
}

FString UHOLGameScreen::DescribeSelectionName() const
{
	// HUD H-9: the name is the script's widget "SelectionName" (HOLSelection.as), found in the script's tree as HoverIncome finds FoodAmount.
	const UCommonTextBlock* Name = Cast<UCommonTextBlock>(FindSelectionWidget(WidgetTree, ScriptScreen ? ScriptScreen->WidgetTree.Get() : nullptr, TEXT("SelectionName")));
	return Name && Name->GetVisibility() != ESlateVisibility::Collapsed && !Name->GetText().IsEmpty()
		? Name->GetText().ToString() : FString(TEXT("none"));
}

const FHOLUnitTypeRow* UHOLGameScreen::FindUnitTypeFor(const FName TypeName, const int32 OwnerPlayer) const
{
	const UWorld* World = GetWorld();
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	// The row of the player's faction (the cost it pays, the name it reads, the statistics it fields); the base row for nobody.
	return Units ? (OwnerPlayer >= 0 ? Units->FindTypeRowFor(TypeName, OwnerPlayer) : Units->FindTypeRow(TypeName)) : nullptr;
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

	// H-3: the top bar (TopBar) and the Menu / Statistiques buttons (MenuPanel) are the script's (UHOLTopBarPanel).

	// H-5a: the major alert's banner (AlertPanel, AlertRow) is the script's (UHOLAlertBannerPanel).

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
	// H-5b: the thread's lines (MessageLine<i>, MessageRow<i>) are the script's (UHOLMessageFeedPanel), drawn into MessagesList (its host, kept here).

	// The control groups' bar (J7a.5, D30), bottom left over the selection's block (the cheats' panel holds the top left):
	// a slot per group not empty, « 1 · 9 », in the keys' order.
	// EtapeDefini T: the groups' bar and the bottom bar under it, one column in a scale box given the width left of the
	// minimap's place: too wide for the window (1500 px at 1080 high, a 4:3 window), the column is drawn smaller, as one,
	// down only (never larger); from 1920x1080, and at 1600x900, its scale is 1 (the look validated at G.1). The box and
	// the column let the clicks through where they are empty.
	BottomScale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("BottomScale"));
	BottomScale->SetStretch(EStretch::ScaleToFitX);
	BottomScale->SetStretchDirection(EStretchDirection::DownOnly);
	BottomScale->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	UOverlaySlot* BottomScaleSlot = Frame->AddChildToOverlay(BottomScale);
	BottomScaleSlot->SetHorizontalAlignment(HAlign_Fill);
	BottomScaleSlot->SetVerticalAlignment(VAlign_Bottom);
	BottomScaleSlot->SetPadding(FMargin(ScreenMargin, 0.f, ScreenMargin + MinimapSize + BarGap, ScreenMargin));
	UVerticalBox* BottomLeft = MakeVerticalBox(TEXT("BottomLeft"));
	BottomLeft->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (UScaleBoxSlot* ColumnSlot = Cast<UScaleBoxSlot>(BottomScale->AddChild(BottomLeft)))
	{
		ColumnSlot->SetHorizontalAlignment(HAlign_Left);
		ColumnSlot->SetVerticalAlignment(VAlign_Bottom);
	}
	// The age medallions rise over the bar's top edge (MedallionRise), out of the column's own area; the scale box clips its
	// content to its allotted area (ScaleToFitX), which cut them in half: the room above the column, a spacer in the box
	// (the slot's padding is not used by a scale box).
	USizeBox* MedallionRoom = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MedallionRoom"));
	MedallionRoom->SetHeightOverride(MedallionRise);
	MedallionRoom->SetVisibility(ESlateVisibility::HitTestInvisible);
	BottomLeft->AddChildToVerticalBox(MedallionRoom);

	GroupsPanel = MakePanel(TEXT("GroupsPanel"));
	GroupsPanel->SetPadding(FMargin(6.f, 4.f));
	GroupsPanel->SetVisibility(ESlateVisibility::Collapsed);
	UVerticalBoxSlot* GroupsSlot = BottomLeft->AddChildToVerticalBox(GroupsPanel);
	GroupsSlot->SetHorizontalAlignment(HAlign_Left);
	GroupsSlot->SetPadding(FMargin(0.f, 0.f, 0.f, BarGap));
	// The row of chips (GroupsRow, ControlGroupBox0-9, ControlGroup0-9) is built by Script\UI\HOLGroupBar.as into this panel (HUD H-6).

	// The bottom bar, bottom left (the user's 2nd trial of G.1, AoE4's way, D201): the selection's block, the command
	// panel, the builders' age panels, each at a fixed place whatever the selection (a block hidden keeps its place).
	BottomBar = MakeHorizontalBox(TEXT("BottomBar"));
	BottomLeft->AddChildToVerticalBox(BottomBar)->SetHorizontalAlignment(HAlign_Left);
	const auto AddToBar = [Bar = BottomBar.Get()](UWidget* Widget, const float LeftPadding)
	{
		UHorizontalBoxSlot* WidgetSlot = Bar->AddChildToHorizontalBox(Widget);
		WidgetSlot->SetVerticalAlignment(VAlign_Bottom);
		WidgetSlot->SetPadding(FMargin(LeftPadding, 0.f, 0.f, 0.f));
	};

	// HUD H-9: the selection block (SelectionPlace, SelectionPanel, name, chips, owner, health, stats, site progress, activity, lives) is the script's (Script\UI\HOLSelection*.as),
	// built into this empty host, at the bar's first place (it keeps the bar's order and the bar's bottom alignment).
	SelectionHost = MakeHorizontalBox(TEXT("SelectionHost"));
	AddToBar(SelectionHost, 0.f);

	// Right of the selection's block, the command panel's width: the short message, the production queue, then the
	// command panel under them (the column grows upwards, the panel stays at the bar's bottom).
	USizeBox* CommandBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CommandBox"));
	CommandBox->SetWidthOverride(GridWidth);
	AddToBar(CommandBox, BarGap);
	UVerticalBox* CommandColumn = MakeVerticalBox(TEXT("CommandColumn"));
	CommandBox->SetContent(CommandColumn);

	// Production queue, as wide as the grid (the panel only: HUD H-7, the script draws what is in it).
	QueuePanel = MakePanel(TEXT("QueuePanel"));
	UVerticalBoxSlot* QueueSlot = CommandColumn->AddChildToVerticalBox(QueuePanel);
	QueueSlot->SetHorizontalAlignment(HAlign_Fill);
	QueueSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
	// HUD H-7: the script's queue panel (HOLQueuePanel.as) fills QueuePanel (QueueColumn, QueueRow, QueueItem0..4, QueueStatus, QueueProgress, QueueNote, QueueHint).

	// The command panel (D30, the user's 2nd trial of G.1): 3 rows of 4 square cells, A Z E R / Q S D F / W X C V, always
	// all there (an empty one dark): the orders of the player's units or the grid of its building.
	// HUD H-10: the host the script's command grid (GridPanel, Grid, GridCell00..11) is put in, in the same place (under the queue panel).
	GridPanelHost = MakeVerticalBox(TEXT("GridPanelHost"));
	CommandColumn->AddChildToVerticalBox(GridPanelHost)->SetHorizontalAlignment(HAlign_Left);

	// HUD H-8: the host the script's age panels are put in (empty here; it takes no room until they are).
	AgePanelsHost = MakeHorizontalBox(TEXT("AgePanelsHost"));
	AddToBar(AgePanelsHost, 0.f);

	// The minimap's place, bottom right (D201): no minimap yet; nothing else goes there.
	MinimapPlace = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MinimapPlace"));
	MinimapPlace->SetWidthOverride(MinimapSize);
	MinimapPlace->SetHeightOverride(MinimapSize);
	MinimapPlace->SetVisibility(ESlateVisibility::HitTestInvisible);
	UOverlaySlot* MinimapSlot = Frame->AddChildToOverlay(MinimapPlace);
	MinimapSlot->SetHorizontalAlignment(HAlign_Right);
	MinimapSlot->SetVerticalAlignment(VAlign_Bottom);
	MinimapSlot->SetPadding(FMargin(ScreenMargin));

	// Nothing selected yet.
	QueuePanel->SetVisibility(ESlateVisibility::Collapsed);
}

#undef LOCTEXT_NAMESPACE
