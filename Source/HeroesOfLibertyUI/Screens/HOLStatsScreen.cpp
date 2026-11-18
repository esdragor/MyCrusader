#include "Screens/HOLStatsScreen.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "HeroesOfLibertyUILog.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchStatsSubsystem.h"
#include "Match/HOLPlayerColors.h"
#include "TimerManager.h"
#include "UObject/UObjectIterator.h"
#include "Screens/HOLGameMenuScreen.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRootLayout.h"
#include "Widgets/HOLStatsGraph.h"

#define LOCTEXT_NAMESPACE "HOLStatsScreen"

namespace UE::HeroesOfLiberty::UI::StatsScreen
{
	/** The tabs, in their order on the screen and in TabPages. */
	enum ETab : int32 { Summary, Military, Economy, Timeline, NumTabs };

	/** Space between two columns, and under a row. */
	constexpr float ColumnGap = 28.f;
	constexpr float RowGap = 6.f;
	/** The name column, wide enough for the lobby's names. */
	constexpr float NameWidth = 220.f;
	// The player's colour swatch before a name.
	constexpr float SwatchSize = 14.f;
	constexpr float SwatchGap = 8.f;
	/** The timeline's graph. */
	constexpr float GraphWidth = 1100.f;
	constexpr float GraphHeight = 440.f;
	/** While the screen is open, the statistics are asked again this often (s, C6): a spectator's curves grow. */
	constexpr float RequestInterval = 10.f;

	/** The series' button names, in EHOLMatchStatsSeries's order. */
	const TCHAR* const SeriesNames[] = { TEXT("Workers"), TEXT("Military"), TEXT("Population"), TEXT("Buildings"), TEXT("Gathered"), TEXT("Stock") };
	static_assert(UE_ARRAY_COUNT(SeriesNames) == static_cast<int32>(EHOLMatchStatsSeries::Num));
}

UHOLStatsScreen* UHOLStatsScreen::Open(APlayerController* PlayerController)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	if (!Layout || !Layout->IsOverMatch())
	{
		return nullptr;
	}
	// ST-3: the screen is the script's (Script/UI/HOLStatsScreen.as); the C++ class builds nothing by itself.
	const TSubclassOf<UHOLStatsScreen> ScriptClass = FindScriptClass();
	if (!ScriptClass)
	{
		UE_LOG(LogHOLUI, Warning, TEXT("Stats: no script class (Script/UI/HOLStatsScreen.as)"));
		return nullptr;
	}
	return Cast<UHOLStatsScreen>(Layout->PushMenu(ScriptClass));
}

void UHOLStatsScreen::BuildLayout()
{
	// The screen is built by the script (Script/UI/HOLStatsScreen.as, BuildStats) with the AddStats* helpers above.
	BP_BuildStats();
}

void UHOLStatsScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UHOLButton* const Tabs[] = { SummaryTab, MilitaryTab, EconomyTab, TimelineTab };
	for (int32 Tab = 0; Tab < UE_ARRAY_COUNT(Tabs); ++Tab)
	{
		if (UHOLButton* Button = Tabs[Tab])
		{
			// Toggleable, or a selected button could never be deselected (as the Settings screen's tabs).
			Button->SetIsSelectable(true);
			Button->SetIsToggleable(true);
			Button->OnClicked().AddWeakLambda(this, [this, Tab]() { SelectTab(Tab); });
		}
	}
	for (int32 Series = 0; Series < SeriesButtons.Num(); ++Series)
	{
		if (UHOLButton* Button = SeriesButtons[Series])
		{
			Button->SetIsSelectable(true);
			Button->SetIsToggleable(true);
			Button->OnClicked().AddWeakLambda(this, [this, Series]() { SelectSeries(Series); });
		}
	}
	if (BackButton)
	{
		BackButton->OnClicked().AddUObject(this, &UHOLStatsScreen::Close);
	}
	SelectTab(0);
	SelectSeries(SeriesIndex);
}

void UHOLStatsScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	UHOLGameMenuScreen::SetMatchMenuInput(GetOwningPlayer(), true, this);
	// The screen comes back from the layout's pool with its buttons deselected: its tab and its series again as last shown.
	SelectTab(TabIndex);
	SelectSeries(SeriesIndex);
	bRefused = false;
	if (UHOLMatchStatsSubsystem* Subsystem = GetWorld() ? GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr)
	{
		if (!UpdatedHandle.IsValid())
		{
			UpdatedHandle = Subsystem->GetOnStatsUpdated().AddUObject(this, &UHOLStatsScreen::Refresh);
			RefusedHandle = Subsystem->GetOnStatsRefused().AddUObject(this, &UHOLStatsScreen::OnRefused);
		}
		Refresh();
		Subsystem->RequestStats();
		const TWeakObjectPtr<UHOLMatchStatsSubsystem> WeakSubsystem(Subsystem);
		GetWorld()->GetTimerManager().SetTimer(RequestTimer, FTimerDelegate::CreateWeakLambda(this, [WeakSubsystem]()
		{
			if (UHOLMatchStatsSubsystem* Asked = WeakSubsystem.Get())
			{
				Asked->RequestStats();
			}
		}), UE::HeroesOfLiberty::UI::StatsScreen::RequestInterval, /*bLoop*/ true);
	}
	UE_LOG(LogHOLUI, Display, TEXT("Stats screen: opened"));
}

void UHOLStatsScreen::NativeOnDeactivated()
{
	if (UHOLMatchStatsSubsystem* Subsystem = GetWorld() ? GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>() : nullptr)
	{
		Subsystem->GetOnStatsUpdated().Remove(UpdatedHandle);
		Subsystem->GetOnStatsRefused().Remove(RefusedHandle);
	}
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RequestTimer);
	}
	UpdatedHandle.Reset();
	RefusedHandle.Reset();
	UHOLGameMenuScreen::SetMatchMenuInput(GetOwningPlayer(), false, this);
	Super::NativeOnDeactivated();
}

void UHOLStatsScreen::Close()
{
	if (IsActivated())
	{
		DeactivateWidget();
		UE_LOG(LogHOLUI, Display, TEXT("Stats screen: closed"));
	}
}

bool UHOLStatsScreen::NativeOnHandleBackAction()
{
	Close();
	return true;
}

UWidget* UHOLStatsScreen::NativeGetDesiredFocusTarget() const
{
	// The tab shown (the screen reopens on the tab shown last), its frame the one lit.
	UHOLButton* const Tabs[] = { SummaryTab, MilitaryTab, EconomyTab, TimelineTab };
	return Tabs[FMath::Clamp(TabIndex, 0, static_cast<int32>(UE_ARRAY_COUNT(Tabs)) - 1)];
}

void UHOLStatsScreen::SelectTab(const int32 Index)
{
	TabIndex = Index;
	if (TabPages)
	{
		TabPages->SetActiveWidgetIndex(Index);
	}
	UHOLButton* const Tabs[] = { SummaryTab, MilitaryTab, EconomyTab, TimelineTab };
	for (int32 Tab = 0; Tab < UE_ARRAY_COUNT(Tabs); ++Tab)
	{
		if (Tabs[Tab])
		{
			Tabs[Tab]->SetIsSelected(Tab == Index, /*bGiveClickFeedback*/ false);
		}
	}
	UE_LOG(LogHOLUI, Display, TEXT("Stats screen: tab %d"), Index);
}

void UHOLStatsScreen::SelectSeries(const int32 Index)
{
	SeriesIndex = FMath::Clamp(Index, 0, static_cast<int32>(EHOLMatchStatsSeries::Num) - 1);
	for (int32 Series = 0; Series < SeriesButtons.Num(); ++Series)
	{
		if (SeriesButtons[Series])
		{
			SeriesButtons[Series]->SetIsSelected(Series == SeriesIndex, /*bGiveClickFeedback*/ false);
		}
	}
	RefreshTimeline();
}

void UHOLStatsScreen::RefreshTimeline()
{
	// The graph is drawn by the script (RefreshTimeline of the script, BP_RefreshTimeline here) from the statistics now held.
	BP_RefreshTimeline(SeriesIndex);
}

void UHOLStatsScreen::OnRefused()
{
	bRefused = true;
	Refresh();
}

void UHOLStatsScreen::Refresh()
{
	// The tables are shown by the script (Script/UI/HOLStatsScreen.as, RefreshStats): at each OnStatsUpdated and refusal.
	BP_RefreshStats(bRefused);
}

TSubclassOf<UHOLStatsScreen> UHOLStatsScreen::FindScriptClass()
{
	// Angelscript's hot reload keeps the older classes, flagged CLASS_NewerVersionExists; the C++ class is concrete (until ST-3), so it is skipped.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (*It != StaticClass() && It->IsChildOf(StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
		{
			return *It;
		}
	}
	return nullptr;
}

UVerticalBox* UHOLStatsScreen::MakeStatsColumn()
{
	return MakeDialogColumn();
}

UCommonTextBlock* UHOLStatsScreen::AddStatsText(UVerticalBox* Column, const FName Name, const FText& Text, const int32 Style)
{
	if (Style == 0)
	{
		return AddText(Column, Name, Text, UHOLTextStyleHeading::StaticClass(), 16.f);
	}
	return AddText(Column, Name, Text, UHOLTextStyleBody::StaticClass(), 0.f);
}

UHorizontalBox* UHOLStatsScreen::AddStatsRow(UVerticalBox* Column)
{
	return AddButtonRow(Column);
}

UHOLButton* UHOLStatsScreen::AddStatsButton(UHorizontalBox* Row, const FName Name, const FText& Label)
{
	return AddButton(Row, Name, Label);
}

UWidgetSwitcher* UHOLStatsScreen::AddStatsPages(UVerticalBox* Column)
{
	TabPages = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("TabPages"));
	UVerticalBoxSlot* PagesSlot = Column->AddChildToVerticalBox(TabPages);
	PagesSlot->SetHorizontalAlignment(HAlign_Center);
	PagesSlot->SetPadding(FMargin(0.f, 24.f, 0.f, 24.f));
	Grids.Reset();
	return TabPages;
}

UGridPanel* UHOLStatsScreen::AddStatsGridPage()
{
	if (!TabPages)
	{
		return nullptr;
	}
	UGridPanel* Grid = WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass());
	TabPages->AddChild(Grid);
	Grids.Add(Grid);
	return Grid;
}

UVerticalBox* UHOLStatsScreen::AddStatsTimelinePage()
{
	if (!TabPages)
	{
		return nullptr;
	}
	UVerticalBox* TimelinePage = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	TabPages->AddChild(TimelinePage);
	SeriesButtons.Reset();
	return TimelinePage;
}

UHorizontalBox* UHOLStatsScreen::AddStatsSeriesRow(UVerticalBox* TimelinePage)
{
	UHorizontalBox* SeriesRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	TimelinePage->AddChildToVerticalBox(SeriesRow)->SetHorizontalAlignment(HAlign_Center);
	return SeriesRow;
}

UHOLButton* UHOLStatsScreen::AddStatsSeriesButton(UHorizontalBox* Row, const FName Name, const FText& Label)
{
	UHOLButton* Button = AddButton(Row, Name, Label);
	SeriesButtons.Add(Button);
	return Button;
}

UHOLStatsGraph* UHOLStatsScreen::AddStatsGraph(UVerticalBox* TimelinePage)
{
	using namespace UE::HeroesOfLiberty::UI::StatsScreen;
	USizeBox* GraphBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	GraphBox->SetWidthOverride(GraphWidth);
	GraphBox->SetHeightOverride(GraphHeight);
	Graph = WidgetTree->ConstructWidget<UHOLStatsGraph>(UHOLStatsGraph::StaticClass(), TEXT("StatsGraph"));
	GraphBox->AddChild(Graph);
	UVerticalBoxSlot* GraphSlot = TimelinePage->AddChildToVerticalBox(GraphBox);
	GraphSlot->SetHorizontalAlignment(HAlign_Center);
	GraphSlot->SetPadding(FMargin(0.f, 16.f, 0.f, 0.f));
	return Graph;
}

void UHOLStatsScreen::AddStatsCell(UGridPanel* Grid, const int32 Row, const int32 Column, const FText& Text, const bool bHeader, const bool bColored,
	const FLinearColor& Color)
{
	using namespace UE::HeroesOfLiberty::UI::StatsScreen;
	if (!Grid)
	{
		return;
	}
	UCommonTextBlock* Cell = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
	Cell->SetStyle(bHeader ? UHOLTextStyleNote::StaticClass() : UHOLTextStyleBody::StaticClass());
	Cell->SetText(Text);
	// The player's colour as a swatch before the name, as the lobby's slots: the text style would override the
	// text's own colour once the cell is built.
	UWidget* CellContent = Cell;
	if (bColored)
	{
		UHorizontalBox* NameBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UImage* Swatch = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Swatch->SetColorAndOpacity(Color);
		Swatch->SetDesiredSizeOverride(FVector2D(SwatchSize, SwatchSize));
		UHorizontalBoxSlot* SwatchSlot = NameBox->AddChildToHorizontalBox(Swatch);
		SwatchSlot->SetVerticalAlignment(VAlign_Center);
		SwatchSlot->SetPadding(FMargin(0.f, 0.f, SwatchGap, 0.f));
		NameBox->AddChildToHorizontalBox(Cell)->SetVerticalAlignment(VAlign_Center);
		CellContent = NameBox;
	}
	UGridSlot* CellSlot = Grid->AddChildToGrid(CellContent, Row, Column);
	// The name on the left, the figures on the right of their column, as a sheet of numbers.
	CellSlot->SetHorizontalAlignment(Column == 0 ? HAlign_Left : HAlign_Right);
	CellSlot->SetVerticalAlignment(VAlign_Center);
	CellSlot->SetPadding(FMargin(Column == 0 ? 0.f : ColumnGap, 0.f, 0.f, Row == 0 ? RowGap * 2.f : RowGap));
	if (Column == 0)
	{
		Cell->SetMinDesiredWidth(bColored ? NameWidth - SwatchSize - SwatchGap : NameWidth);
	}
}

TArray<FText> UHOLStatsScreen::HeadersOf(const int32 Tab)
{
	switch (Tab)
	{
	case 0:
		return { LOCTEXT("Player", "Joueur"), LOCTEXT("Team", "Équipe"), LOCTEXT("Result", "Résultat"),
			LOCTEXT("MilitaryScore", "Score militaire"), LOCTEXT("EconomyScore", "Score économique"), LOCTEXT("Total", "Total") };
	case 1:
		return { LOCTEXT("Player", "Joueur"), LOCTEXT("Killed", "Tués"), LOCTEXT("Lost", "Perdus"), LOCTEXT("BuildingsLost", "Bâtiments perdus"),
			LOCTEXT("MaxMilitary", "Armée max") };
	case 2:
		return { LOCTEXT("Player", "Joueur"), LOCTEXT("Food", "Nourriture"), LOCTEXT("Wood", "Bois"), LOCTEXT("Stone", "Pierre"), LOCTEXT("Gold", "Or"),
			LOCTEXT("Trained", "Produits"), LOCTEXT("MaxWorkers", "Péons max"), LOCTEXT("MaxPopulation", "Population max") };
	default:
		return {};
	}
}

FText UHOLStatsScreen::TextOf(const FName Key)
{
	const auto Is = [&Key](const TCHAR* Name) { return Key == FName(Name); };
	if (Is(TEXT("Heading"))) { return LOCTEXT("Heading", "Statistiques"); }
	if (Is(TEXT("Summary"))) { return LOCTEXT("Summary", "Résumé"); }
	if (Is(TEXT("Military"))) { return LOCTEXT("Military", "Militaire"); }
	if (Is(TEXT("Economy"))) { return LOCTEXT("Economy", "Économie"); }
	if (Is(TEXT("Timeline"))) { return LOCTEXT("Timeline", "Chronologie"); }
	if (Is(TEXT("SeriesWorkers"))) { return LOCTEXT("SeriesWorkers", "Péons"); }
	if (Is(TEXT("SeriesMilitary"))) { return LOCTEXT("SeriesMilitary", "Armée"); }
	if (Is(TEXT("SeriesPopulation"))) { return LOCTEXT("SeriesPopulation", "Population"); }
	if (Is(TEXT("SeriesBuildings"))) { return LOCTEXT("SeriesBuildings", "Bâtiments"); }
	if (Is(TEXT("SeriesGathered"))) { return LOCTEXT("SeriesGathered", "Ressources récoltées"); }
	if (Is(TEXT("SeriesStock"))) { return LOCTEXT("SeriesStock", "Stock"); }
	if (Is(TEXT("Back"))) { return LOCTEXT("Back", "Retour"); }
	if (Is(TEXT("Refused"))) { return LOCTEXT("Refused", "Les statistiques seront disponibles à la fin de la partie."); }
	if (Is(TEXT("Waiting"))) { return LOCTEXT("Waiting", "Statistiques en attente du serveur…"); }
	if (Is(TEXT("Winner"))) { return LOCTEXT("Winner", "Vainqueur"); }
	if (Is(TEXT("Defeated"))) { return LOCTEXT("Defeated", "Vaincu"); }
	if (Is(TEXT("Playing"))) { return LOCTEXT("Playing", "En jeu"); }
	if (Is(TEXT("Dash"))) { return FText::AsCultureInvariant(TEXT("—")); }
	return FText::GetEmpty();
}

FText UHOLStatsScreen::NumberText(const int32 Value)
{
	return FText::AsNumber(Value);
}

FText UHOLStatsScreen::MakeMarkLabel(const int32 Kind, const FText& PlayerName)
{
	return FText::Format(Kind == static_cast<int32>(EHOLMatchStatsEvent::Defeated) ? LOCTEXT("MarkDefeated", "{0} vaincu")
		: LOCTEXT("MarkForumFell", "Forum de {0} tombé"), PlayerName);
}

#undef LOCTEXT_NAMESPACE
