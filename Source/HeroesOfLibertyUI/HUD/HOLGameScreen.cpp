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
	/** Seconds between two readings of a selected group's activities. */
	constexpr float GroupRefreshInterval = 0.25f;
	const FName IdleWorkersName(TEXT("IdleWorkers"));

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
	}
	SelectionHandle.Reset();
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
	// source's amount at every change (J5b). The other blocks follow their delegates.
	using namespace UE::HeroesOfLiberty::UI::GameScreen;
	const int32 NumSelectedUnits = Controller.IsValid() ? Controller->GetSelectedUnits().Num() : 0;
	SinceGroupRefresh += InDeltaTime;
	if (NumSelectedUnits == 1)
	{
		FHOLUnitView Unit;
		if (GetWorld() && HOLUnitView::Find(*GetWorld(), Controller->GetSelectedUnits()[0], Unit) && Unit.CarryCapacity > 0)
		{
			RefreshUnitActivity(&Unit);
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
	UpdateHead();
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
	else
	{
		RefreshSelection(Selected, Selected && Selected == Own);
	}
	RefreshIdleWorkers();
	RefreshGrid(Own, PlayerState);
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
	if (!Building)
	{
		ShownSelection = TEXT("none");
		RefreshUnitActivity(nullptr);
		return;
	}
	const FHOLBuildingTypeRow* Type = Building->GetType();
	ShownSelection = FString::Printf(TEXT("%s (%s)%s"), *Building->GetTypeName().ToString(), bOwn ? TEXT("own") : TEXT("other"),
		Building->IsFinished() ? TEXT("") : TEXT(", foundation"));

	if (SelectionName)
	{
		// A building not finished shows as its foundation (J6a.3; J6a.5 adds its progress and its cancel cell).
		const FText Name = HOLGameNames::GetBuildingName(Building->GetTypeName(), Type);
		SelectionName->SetText(Building->IsFinished() ? Name : FText::Format(LOCTEXT("FoundationName", "Fondation : {0}"), Name));
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
	SetShown(SelectionHealthRow, true);
	RefreshUnitActivity(nullptr);
	if (SelectionHealth)
	{
		// No damage before M5: a building has the health of its work (D312), whole once finished.
		const int32 MaxHealth = Type ? Type->MaxHealth : 0;
		const int64 TotalWork = Type && !Building->IsFinished()
			? HOLConstruction::GetTotalWork(HOLConstruction::GetBuildSteps(Type->BuildTime, GetDefault<UHOLSimulationSettings>()->GetStepSeconds())) : 0;
		SelectionHealth->SetText(FormatRatio(HOLConstruction::GetHealth(MaxHealth, Building->IsFinished() ? 1 : Building->GetConstructionWork(),
			Building->IsFinished() ? 1 : TotalWork), MaxHealth));
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
	if (Views.IsEmpty())
	{
		ShownSelection = TEXT("none");
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
	PerType.KeySort(FNameLexicalLess());
	TArray<FString> ShownTypes;
	TArray<FText> Lines;
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
	const FHOLUnitTypeRow* Type = Views.Num() == 1 ? FindUnitType(Views[0].TypeName) : nullptr;
	SetShown(SelectionHealthRow, Type != nullptr);
	if (SelectionHealth && Type)
	{
		// No damage before M5: a unit is whole.
		SelectionHealth->SetText(FormatRatio(Type->MaxHealth, Type->MaxHealth));
	}
	if (Views.Num() == 1)
	{
		// One gatherer shows what it does and what it carries (J5, D298).
		RefreshUnitActivity(Views[0].CarryCapacity > 0 ? &Views[0] : nullptr);
		return;
	}
	// A group, its workers by activity (J5b, D298): the round trip counts as gathering (D307).
	int32 Gathering = 0, Moving = 0, Idle = 0;
	for (const FHOLUnitView& View : Views)
	{
		if (View.CarryCapacity > 0)
		{
			++(IsGatheringActivity(View.Activity) ? Gathering : View.Activity == EHOLUnitActivity::Moving ? Moving : Idle);
		}
	}
	TArray<FText> Parts;
	if (Gathering > 0)
	{
		Parts.Add(FText::Format(LOCTEXT("GroupGathering", "{0} en récolte"), Gathering));
	}
	if (Moving > 0)
	{
		Parts.Add(FText::Format(LOCTEXT("GroupMoving", "{0} en marche"), Moving));
	}
	if (Idle > 0)
	{
		Parts.Add(FText::Format(LOCTEXT("GroupIdle", "{0} {0}|plural(one=inactif,other=inactifs)"), Idle));
	}
	SetDetailLine(Parts.IsEmpty() ? FString() : FString::Printf(TEXT(", gathering %d moving %d idle %d"), Gathering, Moving, Idle),
		FText::Join(FText::AsCultureInvariant(TEXT(" · ")), Parts));
}

void UHOLGameScreen::RefreshSourceSelection(const int32 SourceIndex)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	const UHOLSourceSubsystem* Sources = GetWorld() ? GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	const FHOLSource* Source = Sources && Sources->GetSources().IsValidIndex(SourceIndex) ? &Sources->GetSources()[SourceIndex] : nullptr;
	const FName TypeName = Source && Sources->GetTypeNames().IsValidIndex(Source->TypeIndex) ? Sources->GetTypeNames()[Source->TypeIndex] : NAME_None;
	SetShown(SelectionPanel, !TypeName.IsNone(), ESlateVisibility::Visible);
	if (TypeName.IsNone())
	{
		ShownSelection = TEXT("none");
		SetDetailLine(FString(), FText::GetEmpty());
		return;
	}
	// Nobody's: no owner, no health; its name, and what it still holds (« Filon d'or — 1840 », D298).
	SetShown(SelectionOwnerRow, false);
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
		: Unit->Activity == EHOLUnitActivity::Moving ? LOCTEXT("ActivityMoving", "En marche") : LOCTEXT("ActivityIdle", "Inactif");
	SetDetailLine(Shown, bLoad
		? FText::Format(LOCTEXT("ActivityWithLoad", "{0} — {1} {2}/{3}"), Activity, GetResourceName(Unit->LoadResource),
			Unit->LoadAmount, Unit->CarryCapacity)
		: Activity);
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

void UHOLGameScreen::RefreshGrid(const AHOLBuilding* OwnBuilding, const AHOLPlayerState* PlayerState)
{
	using namespace UE::HeroesOfLiberty::UI::GameScreen;

	bGridShown = OwnBuilding != nullptr;
	SetShown(GridPanel, bGridShown, ESlateVisibility::Visible);

	const AHOLPlayerController* PlayerController = Controller.Get();
	const FHOLBuildingTypeRow* Type = OwnBuilding ? OwnBuilding->GetType() : nullptr;
	const bool bQueueFull = !Type || OwnBuilding->GetProductionQueue().Num() >= Type->MaxQueue;
	for (int32 Cell = 0; Cell < GridCells.Num() && Cell < ShownGridTypes.Num(); ++Cell)
	{
		const FName TypeName = bGridShown && PlayerController ? PlayerController->GetGridCellUnitType(Cell) : NAME_None;
		ShownGridTypes[Cell] = TypeName;
		UHOLGridCell* GridCell = GridCells[Cell];
		if (!GridCell)
		{
			continue;
		}
		if (TypeName.IsNone())
		{
			GridCell->ClearUnit();
			continue;
		}
		// Greyed when the server would refuse it (D282): queue full or not enough resources. The population is only
		// checked when the unit starts: a full population makes it wait in the queue.
		const FHOLUnitTypeRow* UnitType = FindUnitType(TypeName);
		const FHOLResources Cost = UnitType ? UnitType->Cost : FHOLResources();
		const bool bCanProduce = !bQueueFull && PlayerState && PlayerState->GetResources().Covers(Cost);
		GridCell->SetUnit(HOLGameNames::GetUnitName(TypeName, UnitType), Cost, bCanProduce);
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
				const bool bEnabled = GridCells.IsValidIndex(Cell) && GridCells[Cell] && GridCells[Cell]->GetIsEnabled();
				Cells.Add(FString::Printf(TEXT("%s=%s %s"), GridKeys[Cell], *ShownGridTypes[Cell].ToString(), bEnabled ? TEXT("enabled") : TEXT("disabled")));
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
	return FString::Printf(TEXT("resources %s, selection %s%s, grid %s, queue %s%s"), *Economy, *ShownSelection, *ShownActivity, *Grid, *Queue,
		*Workers);
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
	SelectionActivity = MakeText(TEXT("SelectionActivity"), UHOLTextStyleHUD::StaticClass(), FText::GetEmpty());
	SelectionColumn->AddChildToVerticalBox(SelectionActivity)->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));

	// Bottom right: the production queue, then the command grid under it.
	UVerticalBox* CommandColumn = MakeVerticalBox(TEXT("CommandColumn"));
	UOverlaySlot* CommandSlot = Frame->AddChildToOverlay(CommandColumn);
	CommandSlot->SetHorizontalAlignment(HAlign_Right);
	CommandSlot->SetVerticalAlignment(VAlign_Bottom);
	CommandSlot->SetPadding(FMargin(ScreenMargin));

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
