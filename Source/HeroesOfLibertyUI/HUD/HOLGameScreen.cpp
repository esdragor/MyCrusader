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
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLGameNames.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLPlayerController.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLGridCell.h"
#include "Widgets/HOLQueueItem.h"

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
	constexpr float QueueItemWidth = 58.f;
	constexpr float QueueItemHeight = 36.f;
	constexpr float ScreenMargin = 16.f;
	constexpr float MarkerSize = 12.f;
	constexpr float SwatchSize = 14.f;
	constexpr float SelectionMinWidth = 320.f;
	constexpr float IdleWorkersWidth = 130.f;
	/** The site's progress bar in the selection block (J6a.5). */
	constexpr float SiteBarWidth = 200.f;
	/** The grid panel's width: the short message above it wraps within it (D313). */
	constexpr float GridWidth = 4 * (CellSize + 2 * CellGap) + 12.f;
	/** The name the HUD's report gives the « Annuler » cell of a site. */
	const FName CancelName(TEXT("Cancel"));
	/** Seconds between two readings of a selected group's activities. */
	constexpr float GroupRefreshInterval = 0.25f;
	const FName IdleWorkersName(TEXT("IdleWorkers"));
	/** A control group's slot (J7a.5): its width, and how long it stays lit once its group is selected (s, real time). */
	constexpr float GroupSlotWidth = 64.f;
	constexpr double GroupLightSeconds = 0.5;

	FName GridCellName(const int32 Cell)
	{
		return *FString::Printf(TEXT("GridCell%02d"), Cell);
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
	for (int32 Group = 0; Group < AHOLPlayerController::NumControlGroups; ++Group)
	{
		UHOLQueueItem* GroupSlot = Cast<UHOLQueueItem>(WidgetTree->FindWidget(GroupSlotName(Group)));
		GroupSlots.Add(GroupSlot);
		if (GroupSlot)
		{
			GroupSlot->OnClicked().AddUObject(this, &UHOLGameScreen::HandleGroupSlotClicked, Group);
		}
	}
	ShownGroups.Init(FString(), AHOLPlayerController::NumControlGroups);
	ShownGridTypes.Init(NAME_None, AHOLPlayerController::NumGridCells);

	// For the scripted checks (hol.Test.Run): the captures of a match name the HUD.
	const TWeakObjectPtr<UHOLGameScreen> WeakThis(this);
	HOLTestHooks::DescribeInterface() = [WeakThis]()
	{
		return WeakThis.IsValid() ? FString::Printf(TEXT("hud=%s"), *WeakThis->GetClass()->GetName()) : FString(TEXT("no interface"));
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
	Super::NativeDestruct();
}

void UHOLGameScreen::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// The player state reaches a client after the HUD is up.
	AHOLPlayerState* PlayerState = Controller.IsValid() ? Controller->GetHOLPlayerState() : nullptr;
	if (PlayerState != BoundPlayerState.Get())
	{
		BindPlayerState(PlayerState);
		Refresh();
	}
	// A gatherer selected alone: its line follows it at every change (D303); a group's activities four times a second; a
	// source's amount at every change (J5b); a group of buildings (an F key) four times a second, its sites and queues
	// followed without their delegates. The other blocks follow their delegates.
	using namespace UE::HeroesOfLiberty::UI::GameScreen;
	const int32 NumSelectedUnits = Controller.IsValid() ? Controller->GetSelectedUnits().Num() : 0;
	SinceGroupRefresh += InDeltaTime;
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
	RefreshIdleWorkers();
	RefreshControlGroups(InDeltaTime);
	UpdateHead();
	CheckPopulationBlocked();
	UpdateMessage(InDeltaTime);
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
	ProductionHandle.Reset();
	ConstructionHandle.Reset();
	BoundBuilding = Building;
	if (Building)
	{
		ProductionHandle = Building->OnProductionChanged.AddUObject(this, &UHOLGameScreen::Refresh);
		// A foundation's state and work (J6a.3, D312): its name, its health.
		ConstructionHandle = Building->OnConstructionChanged.AddUObject(this, &UHOLGameScreen::Refresh);
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
	RefreshIdleWorkers();
	RefreshGrid(Own, PlayerController && PlayerController->HasBuildingGroup(), PlayerState);
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
	SetShown(SelectionHealthRow, true);
	if (!bSite || !bOwn)
	{
		RefreshUnitActivity(nullptr);
	}
	if (SelectionHealth)
	{
		// No damage before M5: a building has the health of its work (D312), whole once finished.
		const int32 MaxHealth = Type ? Type->MaxHealth : 0;
		const int64 TotalWork = Type && !Building->IsFinished()
			? HOLConstruction::GetTotalWork(HOLConstruction::GetBuildSteps(Type->BuildTime, GetDefault<UHOLSimulationSettings>()->GetStepSeconds())) : 0;
		SelectionHealth->SetText(FormatRatio(HOLConstruction::GetHealth(MaxHealth, Building->IsFinished() ? 1 : Building->GetConstructionWork(),
			Building->IsFinished() ? 1 : TotalWork), MaxHealth));
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
	TArray<FText> Lines;
	// Units of several types (the user's trial of J7b): first the subgroup Tab made active, whose cells the grid shows,
	// « Tous (8) » or « Paysan × 3 », as a group of buildings.
	FHOLSubgroup Subgroup;
	const AHOLPlayerController* PlayerController = Controller.Get();
	if (bOwn && PlayerController && PlayerController->GetActiveUnitSubgroup(Subgroup))
	{
		Lines.Add(Subgroup.bAll ? FText::Format(LOCTEXT("SubgroupAll", "Tous ({0})"), Subgroup.Members.Num())
			: FText::Format(LOCTEXT("UnitCount", "{0} × {1}"), HOLGameNames::GetUnitName(Subgroup.TypeName, FindUnitType(Subgroup.TypeName)), Subgroup.Members.Num()));
	}
	for (const TPair<FName, int32>& Type : PerType)
	{
		const FText Name = HOLGameNames::GetUnitName(Type.Key, FindUnitType(Type.Key));
		Lines.Add(Views.Num() == 1 ? Name : FText::Format(LOCTEXT("UnitCount", "{0} × {1}"), Name, Type.Value));
		ShownTypes.Add(Views.Num() == 1 ? Type.Key.ToString() : FString::Printf(TEXT("%s x%d"), *Type.Key.ToString(), Type.Value));
	}
	ShownSelection = FString::Printf(TEXT("%s (%s)"), *FString::Join(ShownTypes, TEXT(", ")), bOwn ? TEXT("own") : TEXT("other"));

	if (SelectionName)
	{
		SelectionName->SetText(FText::Join(FText::AsCultureInvariant(TEXT("\n")), Lines));
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
	// A group, its workers by activity (J5b, D298): the round trip counts as gathering (D307); building (J6a.4) on its own.
	int32 Gathering = 0, Moving = 0, Idle = 0, Building = 0;
	for (const FHOLUnitView& View : Views)
	{
		if (View.CarryCapacity > 0)
		{
			++(IsGatheringActivity(View.Activity) ? Gathering : View.Activity == EHOLUnitActivity::Building ? Building
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
	if (Moving > 0)
	{
		Parts.Add(FText::Format(LOCTEXT("GroupMoving", "{0} en marche"), Moving));
	}
	if (Idle > 0)
	{
		Parts.Add(FText::Format(LOCTEXT("GroupIdle", "{0} {0}|plural(one=inactif,other=inactifs)"), Idle));
	}
	SetDetailLine(Parts.IsEmpty() ? FString() : FString::Printf(TEXT(", gathering %d building %d moving %d idle %d"), Gathering, Building, Moving, Idle),
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
	TArray<FText> Lines;
	// First the subgroup Tab made active (the user's 2nd trial of J6a.5), whose cells the grid shows: « Tous (7) »,
	// « Maison × 3 », « Centre principal » (each main centre is one alone).
	FHOLSubgroup Subgroup;
	const AHOLPlayerController* PlayerController = Controller.Get();
	if (PlayerController && PlayerController->GetActiveSubgroup(Subgroup))
	{
		const FText TypeName = Subgroup.bAll ? FText::GetEmpty() : HOLGameNames::GetBuildingName(Subgroup.TypeName, UHOLBuildingSettings::FindType(Subgroup.TypeName));
		Lines.Add(Subgroup.bAll ? FText::Format(LOCTEXT("SubgroupAll", "Tous ({0})"), Subgroup.Members.Num())
			: HOLCommandGrid::IsAloneInGroup(Subgroup.TypeName) ? TypeName : FText::Format(LOCTEXT("UnitCount", "{0} × {1}"), TypeName, Subgroup.Members.Num()));
	}
	for (const TPair<FName, int32>& Type : PerType)
	{
		Lines.Add(FText::Format(LOCTEXT("UnitCount", "{0} × {1}"), HOLGameNames::GetBuildingName(Type.Key, UHOLBuildingSettings::FindType(Type.Key)), Type.Value));
		ShownTypes.Add(FString::Printf(TEXT("%s x%d"), *Type.Key.ToString(), Type.Value));
	}
	ShownSelection = FString::Printf(TEXT("%d buildings (own): %s, subgroup %s"), Buildings.Num(), *FString::Join(ShownTypes, TEXT(" ")),
		PlayerController ? *PlayerController->DescribeSubgroup() : TEXT(""));
	if (SelectionName)
	{
		SelectionName->SetText(FText::Join(FText::AsCultureInvariant(TEXT("\n")), Lines));
	}
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

	// Exact in solo and on the server; on a client, the top of the level's sixteenth (J7-Q5: the exact health stays on
	// the server), so an unhurt unit reads its maximum and a unit at level 8 of 50 reads 27/50.
	const bool bShown = Unit && Unit->MaxHealth > 0;
	const FString Shown = bShown ? FString::Printf(TEXT(", hp %d/%d%s"), Unit->Health, Unit->MaxHealth, Unit->bHealthExact ? TEXT("") : TEXT(" (level)"))
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
	SetShown(GridPanel, bGridShown, ESlateVisibility::Visible);

	// Only a site of the player's own, selected alone (or alone in a group's subgroup), has « Annuler » (HOLCommandGrid::GetCommand).
	TArray<AHOLBuilding*> CommandBuildings;
	if (PlayerController)
	{
		PlayerController->GetCommandBuildings(CommandBuildings);
	}
	const AHOLBuilding* Site = CommandBuildings.Num() == 1 ? CommandBuildings[0] : nullptr;
	const FHOLBuildingTypeRow* Type = Site ? Site->GetType() : nullptr;
	const FHOLResources Stock = PlayerState ? PlayerState->GetResources() : FHOLResources();
	for (int32 Cell = 0; Cell < GridCells.Num() && Cell < ShownGridTypes.Num(); ++Cell)
	{
		const FHOLGridCommand Command = bGridShown && Commands.IsValidIndex(Cell) ? Commands[Cell] : FHOLGridCommand();
		ShownGridTypes[Cell] = Command.Command == EHOLGridCommand::CancelConstruction ? CancelName : Command.TypeName;
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
			// What a cancel gives back (D311): all before the first work, half after.
			const FHOLResources Refund = Type ? HOLConstruction::GetCancelRefund(Type->Cost, Site->GetConstructionState(),
				Site->GetConstructionWork()) : FHOLResources();
			GridCell->SetCommand(LOCTEXT("Cancel", "Annuler"), EHOLGridCellState::Ready);
			GridCell->SetTooltip(LOCTEXT("CancelSite", "Annuler la construction"), Refund, FHOLResources(), Site && Site->HasWorkStarted()
				? LOCTEXT("RefundHalf", "Rend la moitié du coût") : LOCTEXT("RefundAll", "Rend tout le coût"));
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
			GridCell->ClearUnit();
			break;
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
	if (PlayerController && Cell >= 0 && Cell < AHOLPlayerController::NumGridCells)
	{
		UE_LOG(LogHOLUI, Display, TEXT("HUD: grid cell %s clicked"), GridKeys[Cell]);
		PlayerController->ActivateGridCell(Cell);
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

bool UHOLGameScreen::ClickWidget(const FName Name)
{
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
			if (!bGridShown || !GridCell->GetIsEnabled())
			{
				return false;
			}
			HandleGridCellClicked(Cell);
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
	// The gatherers and the idle workers last (J5b): the checks of J3 read the line from its start.
	const FString Workers = bEconomyShown
		? FString::Printf(TEXT(", gatherers %s, idle %d"), *ShownGatherers.ToString(), FMath::Max(ShownIdleWorkers, 0))
		: FString();
	// The short message shown (D313), after the queue: the checks of J3 read the line from its start.
	const FString Message = MessageKey.IsEmpty() ? FString() : FString::Printf(TEXT(", message %s"), *MessageKey);
	return FString::Printf(TEXT("resources %s, selection %s%s%s, grid %s, queue %s%s%s"), *Economy, *ShownSelection, *ShownActivity, *ShownHealth, *Grid, *Queue,
		*Message, *Workers);
}

FString UHOLGameScreen::DescribeCells() const
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	TArray<FString> Parts;
	for (int32 Cell = 0; Cell < GridCells.Num(); ++Cell)
	{
		const FString Content = bGridShown && GridCells[Cell] ? GridCells[Cell]->DescribeContent() : FString();
		if (!Content.IsEmpty())
		{
			Parts.Add(FString::Printf(TEXT("%s: %s"), GridKeys[Cell], *Content.Replace(TEXT("\n"), TEXT(" "))));
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
		const FString Tooltip = bGridShown && GridCells[Cell] ? GridCells[Cell]->DescribeTooltip() : FString();
		if (!Tooltip.IsEmpty())
		{
			Parts.Add(FString::Printf(TEXT("%s: %s"), GridKeys[Cell], *Tooltip));
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

	// The control groups' bar (J7a.5, D30), bottom left (the cheats' panel holds the top left): a slot per group not empty, « 1 · 9 », in the keys' order.
	GroupsPanel = MakePanel(TEXT("GroupsPanel"));
	GroupsPanel->SetPadding(FMargin(6.f, 4.f));
	GroupsPanel->SetVisibility(ESlateVisibility::Collapsed);
	UOverlaySlot* GroupsSlot = Frame->AddChildToOverlay(GroupsPanel);
	GroupsSlot->SetHorizontalAlignment(HAlign_Left);
	GroupsSlot->SetVerticalAlignment(VAlign_Bottom);
	GroupsSlot->SetPadding(FMargin(ScreenMargin));
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

	// Selection block, bottom centre: name, owner (with its colour), health.
	SelectionPanel = MakePanel(TEXT("SelectionPanel"));
	UOverlaySlot* SelectionSlot = Frame->AddChildToOverlay(SelectionPanel);
	SelectionSlot->SetHorizontalAlignment(HAlign_Center);
	SelectionSlot->SetVerticalAlignment(VAlign_Bottom);
	SelectionSlot->SetPadding(FMargin(0.f, 0.f, 0.f, ScreenMargin));
	USizeBox* SelectionBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SelectionBox"));
	SelectionBox->SetMinDesiredWidth(SelectionMinWidth);
	SelectionPanel->SetContent(SelectionBox);
	UVerticalBox* SelectionColumn = MakeVerticalBox(TEXT("SelectionColumn"));
	SelectionBox->SetContent(SelectionColumn);

	SelectionName = MakeText(TEXT("SelectionName"), UHOLTextStyleHUDHeading::StaticClass(), FText::GetEmpty());
	SelectionColumn->AddChildToVerticalBox(SelectionName)->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));
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
	SelectionColumn->AddChildToVerticalBox(SelectionActivity)->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));

	// Bottom right: the production queue, then the command grid under it.
	UVerticalBox* CommandColumn = MakeVerticalBox(TEXT("CommandColumn"));
	UOverlaySlot* CommandSlot = Frame->AddChildToOverlay(CommandColumn);
	CommandSlot->SetHorizontalAlignment(HAlign_Right);
	CommandSlot->SetVerticalAlignment(VAlign_Bottom);
	CommandSlot->SetPadding(FMargin(ScreenMargin));

	// The short message (D313), above the queue and the grid, wrapped within the grid's width; it lets the clicks through.
	MessagePanel = MakePanel(TEXT("MessagePanel"));
	UVerticalBoxSlot* MessageSlot = CommandColumn->AddChildToVerticalBox(MessagePanel);
	MessageSlot->SetHorizontalAlignment(HAlign_Right);
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

	// Command grid (D30): 3 rows of 4 square cells, A Z E R / Q S D F / W X C V.
	GridPanel = MakePanel(TEXT("GridPanel"));
	GridPanel->SetPadding(FMargin(6.f));
	CommandColumn->AddChildToVerticalBox(GridPanel)->SetHorizontalAlignment(HAlign_Fill);
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

	// Nothing selected yet.
	SelectionPanel->SetVisibility(ESlateVisibility::Collapsed);
	QueuePanel->SetVisibility(ESlateVisibility::Collapsed);
	GridPanel->SetVisibility(ESlateVisibility::Collapsed);
}

#undef LOCTEXT_NAMESPACE
