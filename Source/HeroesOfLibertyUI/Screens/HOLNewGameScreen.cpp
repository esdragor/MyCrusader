#include "Screens/HOLNewGameScreen.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "HeroesOfLibertyUILog.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Match/HOLMatchSubsystem.h"
#include "Match/HOLPlayerColors.h"
#include "Net/HOLManagerClient.h"
#include "Net/HOLManagerErrorText.h"
#include "TimerManager.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLDropdown.h"
#include "Widgets/HOLRootLayout.h"

#define LOCTEXT_NAMESPACE "HOLNewGameScreen"

namespace UE::HeroesOfLiberty::UI::NewGame
{
	constexpr float NameWidth = 140.f;
	constexpr float StateWidth = 220.f;
	constexpr float FactionWidth = 280.f;
	constexpr float SwatchSize = 28.f;
	constexpr float ColorWidth = 200.f;
	constexpr float TeamWidth = 160.f;
	constexpr float OptionWidth = 200.f;
	constexpr float ToggleWidth = 120.f;
	constexpr float OptionLabelWidth = 170.f;
	constexpr float ClosedSwatchOpacity = 0.2f;

	/** List values in the order of EHOLSlotState. */
	TArray<FText> StateNames()
	{
		return { LOCTEXT("Closed", "Fermé"), LOCTEXT("Human", "Humain"), LOCTEXT("AI", "IA") };
	}

	/** List values in the order of EHOLFaction (§ 13). */
	TArray<FText> FactionNames()
	{
		return {
			LOCTEXT("DawnOrder", "Ordre de l'Aube"),
			LOCTEXT("BlackLegions", "Légions Noires"),
			LOCTEXT("DragonChildren", "Enfants du Dragon"),
			LOCTEXT("ShadowCircle", "Cercle de l'Ombre"),
			LOCTEXT("FireHeirs", "Héritiers du Feu"),
			LOCTEXT("Templars", "Templiers"),
		};
	}

	TArray<FText> TeamNames()
	{
		TArray<FText> Names = { LOCTEXT("NoTeam", "Aucune") };
		for (int32 Team = 1; Team <= FHOLMatchConfig::MaxTeams; ++Team)
		{
			Names.Add(FText::AsNumber(Team));
		}
		return Names;
	}

	TArray<FText> ColorNames()
	{
		TArray<FText> Names;
		for (int32 Color = 0; Color < HOLPlayerColors::NumColors; ++Color)
		{
			Names.Add(HOLPlayerColors::GetColorName(Color));
		}
		// E2-2: last, at HOLPlayerColors::RandomColor; drawn at launch.
		Names.Add(LOCTEXT("RandomColour", "Aléatoire"));
		return Names;
	}

	/** The swatch of "Aléatoire" (E2-2): no player colour yet. */
	const FLinearColor RandomSwatch = HOLUIStyle::TextMuted;

	TArray<FLinearColor> ColorSwatches()
	{
		TArray<FLinearColor> Swatches;
		for (int32 Color = 0; Color < HOLPlayerColors::NumColors; ++Color)
		{
			Swatches.Add(FLinearColor(HOLPlayerColors::GetColor(Color)));
		}
		Swatches.Add(RandomSwatch);
		return Swatches;
	}

	/** A lobby slot's state (network mode). */
	TArray<FText> LobbyStateNames()
	{
		return { LOCTEXT("Open", "Ouvert"), LOCTEXT("Closed", "Fermé"), LOCTEXT("NotReady", "Pas prêt"), LOCTEXT("Ready", "Prêt") };
	}

	/** The lobby's state read twice a second (Manager/PROTOCOL.md). */
	constexpr float ReadPeriod = 0.5f;

	TArray<FText> OffOn()
	{
		return { LOCTEXT("Off", "Non"), LOCTEXT("On", "Oui") };
	}

	FName SlotWidgetName(const TCHAR* Prefix, const int32 SlotIndex)
	{
		return FName(FString::Printf(TEXT("%s%d"), Prefix, SlotIndex));
	}
}

FText UHOLNewGameScreen::GetMapName(const FSoftObjectPath& Map)
{
	// One line per map of the list; the names are provisional, the GDD names no map yet.
	const FString Asset = Map.GetAssetName();
	if (Asset == TEXT("Plain"))
	{
		return LOCTEXT("Map_Plain", "Plaine");
	}
	return FText::AsCultureInvariant(Asset);
}

UHOLNewGameScreen* UHOLNewGameScreen::Open(APlayerController* PlayerController)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	return Layout ? Cast<UHOLNewGameScreen>(Layout->PushMenu(StaticClass())) : nullptr;
}

UHOLNewGameScreen* UHOLNewGameScreen::Find(APlayerController* PlayerController)
{
	const UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	return Layout ? Cast<UHOLNewGameScreen>(Layout->GetActiveMenu()) : nullptr;
}

void UHOLNewGameScreen::BuildLayout()
{
	using namespace UE::HeroesOfLiberty::UI::NewGame;

	UVerticalBox* Column = MakeCenteredColumn();
	HeadingText = AddText(Column, TEXT("Heading"), LOCTEXT("Heading", "Nouvelle partie"), UHOLTextStyleHeading::StaticClass(), 24.f);

	TArray<FText> MapNames;
	for (const FSoftObjectPath& Map : Maps)
	{
		MapNames.Add(GetMapName(Map));
	}
	// The grid: the map, one line per slot under a line of column headings, then the mode and its options.
	const auto AddCell = [this](UHorizontalBox* Row, UWidget* Content, const float Width)
	{
		USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Box->SetWidthOverride(Width);
		Box->SetContent(Content);
		UHorizontalBoxSlot* CellSlot = Row->AddChildToHorizontalBox(Box);
		CellSlot->SetVerticalAlignment(VAlign_Center);
		CellSlot->SetPadding(FMargin(4.f, 0.f));
	};
	const auto MakeText = [this](const FText& Text, const TSubclassOf<UCommonTextStyle> TextStyle, const FName Name = NAME_None)
	{
		UCommonTextBlock* Block = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), Name);
		Block->SetStyle(TextStyle);
		Block->SetText(Text);
		return Block;
	};
	// Lines start on the same left edge, the one of the slot lines (the widest).
	const auto AddLine = [this, Column](const float TopPadding)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UVerticalBoxSlot* RowSlot = Column->AddChildToVerticalBox(Row);
		RowSlot->SetHorizontalAlignment(HAlign_Left);
		RowSlot->SetPadding(FMargin(0.f, TopPadding, 0.f, 4.f));
		return Row;
	};

	UHorizontalBox* MapLine = AddLine(0.f);
	AddCell(MapLine, MakeText(LOCTEXT("Map", "Carte"), UHOLTextStyleBody::StaticClass()), NameWidth);
	MapDropdown = AddDropdown(MapLine, TEXT("MapDropdown"), MapNames, 0, StateWidth + FactionWidth + 8.f);

	UHorizontalBox* Headings = AddLine(16.f);
	AddCell(Headings, MakeText(LOCTEXT("Player", "Joueur"), UHOLTextStyleNote::StaticClass()), NameWidth);
	AddCell(Headings, MakeText(LOCTEXT("Slot", "Emplacement"), UHOLTextStyleNote::StaticClass()), StateWidth);
	AddCell(Headings, MakeText(LOCTEXT("Faction", "Faction"), UHOLTextStyleNote::StaticClass()), FactionWidth);
	AddCell(Headings, MakeText(LOCTEXT("Colour", "Couleur"), UHOLTextStyleNote::StaticClass()), SwatchSize + 8.f + ColorWidth);
	AddCell(Headings, MakeText(LOCTEXT("Team", "Équipe"), UHOLTextStyleNote::StaticClass()), TeamWidth);

	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		UHorizontalBox* Row = AddLine(0.f);
		AddCell(Row, MakeText(FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), SlotIndex + 1), UHOLTextStyleBody::StaticClass(),
			SlotWidgetName(TEXT("SlotName"), SlotIndex)), NameWidth);
		if (SlotIndex == 0)
		{
			AddCell(Row, MakeText(LOCTEXT("You", "Vous"), UHOLTextStyleBody::StaticClass(), TEXT("SlotYou0")), StateWidth);
		}
		else
		{
			AddDropdown(Row, SlotWidgetName(TEXT("SlotState"), SlotIndex), StateNames(), 0, StateWidth);
		}
		AddDropdown(Row, SlotWidgetName(TEXT("SlotFaction"), SlotIndex), FactionNames(), 0, FactionWidth);
		UImage* Swatch = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), SlotWidgetName(TEXT("SlotSwatch"), SlotIndex));
		Swatch->SetDesiredSizeOverride(FVector2D(SwatchSize, SwatchSize));
		AddCell(Row, Swatch, SwatchSize);
		AddDropdown(Row, SlotWidgetName(TEXT("SlotColor"), SlotIndex), ColorNames(), 0, ColorWidth)->SetOptionSwatches(ColorSwatches());
		AddDropdown(Row, SlotWidgetName(TEXT("SlotTeam"), SlotIndex), TeamNames(), 0, TeamWidth);
	}

	UHorizontalBox* Options = AddLine(16.f);
	AddCell(Options, MakeText(LOCTEXT("Mode", "Mode"), UHOLTextStyleBody::StaticClass()), NameWidth);
	ModeDropdown = AddDropdown(Options, TEXT("ModeDropdown"), { LOCTEXT("Standard", "Standard") }, 0, OptionWidth);
	AddCell(Options, MakeText(LOCTEXT("Nomad", "Nomade"), UHOLTextStyleBody::StaticClass()), OptionLabelWidth);
	NomadDropdown = AddDropdown(Options, TEXT("NomadDropdown"), OffOn(), 0, ToggleWidth);
	AddCell(Options, MakeText(LOCTEXT("Regicide", "Régicide"), UHOLTextStyleBody::StaticClass()), OptionLabelWidth);
	RegicideDropdown = AddDropdown(Options, TEXT("RegicideDropdown"), OffOn(), 0, ToggleWidth);
	AddCell(Options, MakeText(LOCTEXT("Catastrophe", "Catastrophe"), UHOLTextStyleBody::StaticClass()), OptionLabelWidth);
	CatastropheDropdown = AddDropdown(Options, TEXT("CatastropheDropdown"), OffOn(), 0, ToggleWidth);

	NoteText = AddText(Column, TEXT("Note"), LOCTEXT("Greyed", "Grisé : pas encore dans le jeu (IA, factions, équipes, mode et options)."),
		UHOLTextStyleNote::StaticClass(), 4.f);
	ErrorText = AddText(Column, TEXT("ErrorText"), FText::GetEmpty(), UHOLTextStyleBody::StaticClass(), 4.f);
	ErrorText->SetColorAndOpacity(HOLUIStyle::Missing);
	ErrorText->SetVisibility(ESlateVisibility::Collapsed);
	AddColumnHint(Column);
	UHorizontalBox* Buttons = AddButtonRow(Column);
	BackButton = AddButton(Buttons, TEXT("BackButton"), LOCTEXT("Back", "Retour"));
	ReadyButton = AddButton(Buttons, TEXT("ReadyButton"), LOCTEXT("ReadyAction", "Prêt"));
	ReadyButton->SetVisibility(ESlateVisibility::Collapsed);
	LaunchButton = AddButton(Buttons, TEXT("LaunchButton"), LOCTEXT("Launch", "Lancer"));
}

void UHOLNewGameScreen::NativeOnInitialized()
{
	using namespace UE::HeroesOfLiberty::UI::NewGame;

	Super::NativeOnInitialized();

	SlotYou = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("SlotYou0")));
	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		SlotStates.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotState"), SlotIndex))));
		SlotFactions.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotFaction"), SlotIndex))));
		SlotColors.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotColor"), SlotIndex))));
		SlotSwatches.Add(Cast<UImage>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotSwatch"), SlotIndex))));
		SlotTeams.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotTeam"), SlotIndex))));
		SlotNames.Add(Cast<UCommonTextBlock>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotName"), SlotIndex))));

		if (SlotStates[SlotIndex])
		{
			// Greyed in the list (D240): AI players are not in the game yet.
			SlotStates[SlotIndex]->SetOptionEnabled(static_cast<int32>(EHOLSlotState::AI), false);
			SlotStates[SlotIndex]->OnSelectionChanged.AddWeakLambda(this, [this, SlotIndex](const int32 Value)
			{
				HandleStateChosen(SlotIndex, Value);
			});
		}
		if (SlotColors[SlotIndex])
		{
			SlotColors[SlotIndex]->OnSelectionChanged.AddWeakLambda(this, [this, SlotIndex](const int32 Value)
			{
				HandleColorChosen(SlotIndex, Value);
			});
		}
		if (SlotTeams[SlotIndex])
		{
			SlotTeams[SlotIndex]->OnSelectionChanged.AddWeakLambda(this, [this, SlotIndex](const int32 Value)
			{
				HandleTeamChosen(SlotIndex, Value);
			});
		}
		// Greyed (D240): factions and teams are not in the game yet.
		for (UHOLDropdown* Greyed : { SlotFactions[SlotIndex].Get(), SlotTeams[SlotIndex].Get() })
		{
			if (Greyed)
			{
				Greyed->SetIsEnabled(false);
			}
		}
	}
	for (UHOLDropdown* Greyed : { ModeDropdown.Get(), NomadDropdown.Get(), RegicideDropdown.Get(), CatastropheDropdown.Get() })
	{
		if (Greyed)
		{
			Greyed->SetIsEnabled(false);
		}
	}
	if (MapDropdown)
	{
		MapDropdown->OnSelectionChanged.AddUObject(this, &UHOLNewGameScreen::HandleMapChosen);
	}
	for (UHOLDropdown* Option : { NomadDropdown.Get(), RegicideDropdown.Get(), CatastropheDropdown.Get() })
	{
		if (Option)
		{
			Option->OnSelectionChanged.AddUObject(this, &UHOLNewGameScreen::HandleOptionChosen);
		}
	}
	if (LaunchButton)
	{
		LaunchButton->OnClicked().AddUObject(this, &UHOLNewGameScreen::Launch);
	}
	if (ReadyButton)
	{
		ReadyButton->OnClicked().AddUObject(this, &UHOLNewGameScreen::ToggleReady);
	}
	if (BackButton)
	{
		BackButton->OnClicked().AddUObject(this, &UHOLNewGameScreen::Close);
	}

	// The configuration of the last launch, or the default one.
	const UGameInstance* GameInstance = GetGameInstance();
	const UHOLMatchSubsystem* Match = GameInstance ? GameInstance->GetSubsystem<UHOLMatchSubsystem>() : nullptr;
	SetConfig(Match ? Match->GetConfigOrDefault() : FHOLMatchConfig::MakeDefault());
}

UWidget* UHOLNewGameScreen::NativeGetDesiredFocusTarget() const
{
	// In the lobby, Launch is the leader's only.
	return bLobby ? (Lobby.IsYouLeader() ? LaunchButton : ReadyButton) : LaunchButton;
}

void UHOLNewGameScreen::NativeOnDeactivated()
{
	if (bLobby)
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ReadTimer);
		}
		// Back (or a lobby lost: then the token is gone and nothing is sent).
		const UGameInstance* GameInstance = GetGameInstance();
		if (UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr; Client && Client->IsInLobby())
		{
			Client->LeaveLobby();
			UE_LOG(LogHOLUI, Display, TEXT("Lobby: left %s"), *Lobby.Id);
		}
		bLobby = false;
	}
	Super::NativeOnDeactivated();
}

void UHOLNewGameScreen::EnterLobby(const FHOLLobby& InLobby)
{
	using namespace UE::HeroesOfLiberty::UI::NewGame;

	bLobby = true;
	LoggedPlayers.Reset();
	for (UHOLDropdown* State : SlotStates)
	{
		if (State)
		{
			State->SetOptions(LobbyStateNames(), 0);
		}
	}
	if (BackButton)
	{
		BackButton->SetLabel(LOCTEXT("LeaveLobby", "Quitter le salon"));
	}
	if (ReadyButton)
	{
		ReadyButton->SetVisibility(ESlateVisibility::Visible);
	}
	if (NoteText)
	{
		NoteText->SetText(LOCTEXT("LobbyGreyed", "Grisé : pas encore dans le jeu (factions, mode) ; seul le chef règle la carte, les options et les emplacements."));
	}
	ShowLobby(InLobby);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ReadTimer, FTimerDelegate::CreateUObject(this, &UHOLNewGameScreen::ReadLobby), ReadPeriod, true);
	}
	if (BackButton)
	{
		BackButton->SetFocus();
	}
}

void UHOLNewGameScreen::ReadLobby()
{
	const UGameInstance* GameInstance = GetGameInstance();
	UHOLManagerClient* Client = GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (!bLobby || bReading || !Client)
	{
		return;
	}
	bReading = true;
	Client->ReadLobby(FHOLManagerLobbyDone::CreateWeakLambda(this, [this](const FHOLLobby& Read, const FHOLManagerError& Error)
	{
		bReading = false;
		if (!bLobby)
		{
			return;
		}
		if (Error.IsError())
		{
			// Manager gone, removed, lobby gone: back to the list, which says why.
			UE_LOG(LogHOLUI, Display, TEXT("Lobby: lost %s (%s, HTTP %d %s: %s)"), *Lobby.Id, *GetManagerErrorText(Error).ToString(), Error.HttpCode,
				*Error.Code, *Error.Message);
			const UGameInstance* Owner = GetGameInstance();
			if (UHOLManagerClient* Manager = Owner ? Owner->GetSubsystem<UHOLManagerClient>() : nullptr)
			{
				Manager->SetLobbyLoss(Error);
			}
			Close();
			return;
		}
		ShowLobby(Read);
	}));
}

void UHOLNewGameScreen::ShowLobby(const FHOLLobby& InLobby)
{
	using namespace UE::HeroesOfLiberty::UI::NewGame;

	Lobby = InLobby;
	const int32 MapIndex = Maps.IndexOfByPredicate([this](const FSoftObjectPath& Map) { return Map.GetAssetName() == Lobby.Map; });
	if (MapIndex != INDEX_NONE)
	{
		Config.Map = Maps[MapIndex];
	}
	Config.bNomad = Lobby.bNomad;
	Config.bRegicide = Lobby.bRegicide;
	Config.bCatastrophe = Lobby.bCatastrophe;
	TArray<FString> Players;
	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		FHOLMatchSlot& SlotConfig = Config.Slots[SlotIndex];
		const FHOLLobbySlot* Row = Lobby.Slots.IsValidIndex(SlotIndex) ? &Lobby.Slots[SlotIndex] : nullptr;
		const bool bPlayer = Row && Row->State == EHOLLobbySlotState::Player;
		SlotConfig.State = bPlayer ? EHOLSlotState::Human : EHOLSlotState::Closed;
		SlotConfig.PlayerName = bPlayer ? Row->PlayerName : FString();
		if (bPlayer)
		{
			SlotConfig.ColorIndex = FMath::Clamp(Row->ColorIndex, 0, HOLPlayerColors::RandomColor);
			SlotConfig.Team = FMath::Clamp(Row->Team, 0, FHOLMatchConfig::MaxTeams);
			SlotConfig.Faction = Row->Faction == TEXT("BlackLegions") ? EHOLFaction::BlackLegions : EHOLFaction::DawnOrder;
			Players.Add(Row->PlayerName);
		}
	}
	Refresh();

	if (HeadingText)
	{
		HeadingText->SetText(FText::FromString(Lobby.Name));
	}
	const int32 Yours = Lobby.FindYourSlot();
	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		const FHOLLobbySlot* Row = Lobby.Slots.IsValidIndex(SlotIndex) ? &Lobby.Slots[SlotIndex] : nullptr;
		const bool bPlayer = Row && Row->State == EHOLLobbySlotState::Player;
		const bool bMine = SlotIndex == Yours;
		if (SlotNames[SlotIndex])
		{
			SlotNames[SlotIndex]->SetText(!bPlayer ? FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), SlotIndex + 1)
				: Row->PlayerName == Lobby.Leader ? FText::Format(LOCTEXT("Leader", "♛ {0}"), FText::FromString(Row->PlayerName))
				: FText::FromString(Row->PlayerName));
		}
		// Open, closed, not ready, ready (LobbyStateNames).
		const int32 State = !Row || Row->State == EHOLLobbySlotState::Open ? 0 : !bPlayer ? 1 : Row->bReady ? 3 : 2;
		// The leader opens and closes the slots nobody holds: Open and Closed only.
		if (SlotStates[SlotIndex])
		{
			SlotStates[SlotIndex]->SetSelectedIndex(State);
			SlotStates[SlotIndex]->SetIsEnabled(Lobby.CanToggleSlot(SlotIndex));
			SlotStates[SlotIndex]->SetOptionEnabled(2, false);
			SlotStates[SlotIndex]->SetOptionEnabled(3, false);
		}
		else if (SlotIndex == 0 && SlotYou)
		{
			SlotYou->SetText(LobbyStateNames()[State]);
		}
		// Only my line is mine (M6-Q4); the faction is not changeable yet.
		if (SlotColors[SlotIndex])
		{
			SlotColors[SlotIndex]->SetIsEnabled(bMine);
		}
		if (SlotTeams[SlotIndex])
		{
			SlotTeams[SlotIndex]->SetIsEnabled(bMine);
		}
	}
	// The map and the options are the leader's; the mode has one value yet.
	const bool bLeader = Lobby.IsYouLeader();
	for (UHOLDropdown* Setting : { MapDropdown.Get(), NomadDropdown.Get(), RegicideDropdown.Get(), CatastropheDropdown.Get() })
	{
		if (Setting)
		{
			Setting->SetIsEnabled(bLeader);
		}
	}
	const bool bReady = Lobby.Slots.IsValidIndex(Yours) && Lobby.Slots[Yours].bReady;
	if (ReadyButton)
	{
		ReadyButton->SetLabel(bReady ? LOCTEXT("NotReadyAction", "Plus prêt") : LOCTEXT("ReadyAction", "Prêt"));
		ReadyButton->SetIsEnabled(Yours != INDEX_NONE && Lobby.State == EHOLLobbyState::Lobby);
	}
	// Launch: the leader's only, active when all are ready (the manager still decides).
	if (LaunchButton)
	{
		LaunchButton->SetVisibility(bLeader ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		LaunchButton->SetIsEnabled(Lobby.CanStart() && Lobby.State == EHOLLobbyState::Lobby);
	}
	if (Lobby.State == EHOLLobbyState::Starting)
	{
		ShowError(LOCTEXT("Starting", "Lancement…"));
	}
	else if (Lobby.State == EHOLLobbyState::InGame && Lobby.bHasMatch)
	{
		JoinMatch();
		return;
	}

	const int32 NumReady = Lobby.Slots.FilterByPredicate([](const FHOLLobbySlot& Row) { return Row.State == EHOLLobbySlotState::Player && Row.bReady; }).Num();
	TArray<FString> SlotsShown;
	for (const FHOLLobbySlot& Row : Lobby.Slots)
	{
		SlotsShown.Add(Row.State == EHOLLobbySlotState::Open ? FString(TEXT("o")) : Row.State == EHOLLobbySlotState::Closed ? FString(TEXT("x"))
			: FString::Printf(TEXT("c%d%s"), Row.ColorIndex, Row.bReady ? TEXT("r") : TEXT("")));
	}
	const FString Logged = FString::Printf(TEXT("%d players (%s), you %s, leader %s, %d/%d ready, launch %s, slots %s"), Players.Num(),
		*FString::Join(Players, TEXT(", ")), *Lobby.You, *Lobby.Leader, NumReady, Players.Num(), bLeader ? TEXT("shown") : TEXT("hidden"),
		*FString::Join(SlotsShown, TEXT(" ")));
	if (Logged != LoggedPlayers)
	{
		LoggedPlayers = Logged;
		UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s \"%s\" map %s: %s"), *Lobby.Id, *Lobby.Name, *Lobby.Map, *Logged);
	}
}

void UHOLNewGameScreen::GetColumns(TArray<TArray<UWidget*>>& OutColumns) const
{
	TArray<UWidget*>& States = OutColumns.AddDefaulted_GetRef();
	States.Add(MapDropdown);
	States.Append(SlotStates);
	States.Add(ModeDropdown);
	OutColumns.Emplace(SlotFactions);
	OutColumns.Emplace(SlotColors);
	OutColumns.Emplace(SlotTeams);
}

void UHOLNewGameScreen::SetConfig(const FHOLMatchConfig& InConfig)
{
	Config = InConfig;
	Config.Normalize();
	if (!Maps.IsEmpty() && !Maps.Contains(Config.Map))
	{
		Config.Map = Maps[0];
	}
	Refresh();
}

void UHOLNewGameScreen::Refresh()
{
	using namespace UE::HeroesOfLiberty::UI::NewGame;

	if (MapDropdown)
	{
		MapDropdown->SetSelectedIndex(FMath::Max(0, Maps.IndexOfByKey(Config.Map)));
	}
	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		const FHOLMatchSlot& SlotConfig = Config.Slots[SlotIndex];
		const bool bOccupied = SlotConfig.State != EHOLSlotState::Closed;
		if (SlotStates[SlotIndex])
		{
			SlotStates[SlotIndex]->SetSelectedIndex(static_cast<int32>(SlotConfig.State));
		}
		if (SlotFactions[SlotIndex])
		{
			SlotFactions[SlotIndex]->SetSelectedIndex(static_cast<int32>(SlotConfig.Faction));
		}
		if (SlotColors[SlotIndex])
		{
			SlotColors[SlotIndex]->SetSelectedIndex(SlotConfig.ColorIndex);
			SlotColors[SlotIndex]->SetIsEnabled(bOccupied);
			for (int32 Color = 0; Color < HOLPlayerColors::NumColors; ++Color)
			{
				SlotColors[SlotIndex]->SetOptionEnabled(Color, !IsColorTaken(SlotIndex, Color));
			}
		}
		if (SlotSwatches[SlotIndex])
		{
			FLinearColor Swatch = SlotConfig.ColorIndex == HOLPlayerColors::RandomColor ? RandomSwatch
				: FLinearColor(HOLPlayerColors::GetColor(SlotConfig.ColorIndex));
			Swatch.A = bOccupied ? 1.f : ClosedSwatchOpacity;
			SlotSwatches[SlotIndex]->SetColorAndOpacity(Swatch);
		}
		if (SlotTeams[SlotIndex])
		{
			SlotTeams[SlotIndex]->SetSelectedIndex(SlotConfig.Team);
		}
		// E2-4: a line nobody holds (closed, or open in a lobby) shows no faction, colour nor team; Hidden keeps the
		// columns in place and takes the fields out of the keyboard's way. Their values stay, shown again once taken.
		const ESlateVisibility Fields = bOccupied ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
		for (UWidget* Field : { static_cast<UWidget*>(SlotFactions[SlotIndex].Get()), static_cast<UWidget*>(SlotSwatches[SlotIndex].Get()),
			static_cast<UWidget*>(SlotColors[SlotIndex].Get()), static_cast<UWidget*>(SlotTeams[SlotIndex].Get()) })
		{
			if (Field)
			{
				Field->SetVisibility(Fields);
			}
		}
	}
	if (NomadDropdown)
	{
		NomadDropdown->SetSelectedIndex(Config.bNomad ? 1 : 0);
	}
	if (RegicideDropdown)
	{
		RegicideDropdown->SetSelectedIndex(Config.bRegicide ? 1 : 0);
	}
	if (CatastropheDropdown)
	{
		CatastropheDropdown->SetSelectedIndex(Config.bCatastrophe ? 1 : 0);
	}
}

void UHOLNewGameScreen::HandleMapChosen(const int32 Value)
{
	if (Maps.IsValidIndex(Value))
	{
		Config.Map = Maps[Value];
	}
	Refresh();
	if (bLobby)
	{
		SendSettings();
	}
}

void UHOLNewGameScreen::HandleOptionChosen(int32)
{
	if (!bLobby)
	{
		return;
	}
	Config.bNomad = NomadDropdown && NomadDropdown->GetSelectedIndex() == 1;
	Config.bRegicide = RegicideDropdown && RegicideDropdown->GetSelectedIndex() == 1;
	Config.bCatastrophe = CatastropheDropdown && CatastropheDropdown->GetSelectedIndex() == 1;
	SendSettings();
}

UHOLManagerClient* UHOLNewGameScreen::GetClient() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UHOLManagerClient>() : nullptr;
}

void UHOLNewGameScreen::SendSettings()
{
	if (UHOLManagerClient* Client = GetClient())
	{
		const FString What = FString::Printf(TEXT("settings %s nomad %d regicide %d catastrophe %d"), *Config.Map.GetAssetName(), Config.bNomad,
			Config.bRegicide, Config.bCatastrophe);
		UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
		Client->UpdateSettings(Config.Map.GetAssetName(), Config.bNomad, Config.bRegicide, Config.bCatastrophe,
			FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error) { HandleWritten(Written, Error, What); }));
	}
}

void UHOLNewGameScreen::HandleTeamChosen(const int32 SlotIndex, const int32 Value)
{
	Config.Slots[SlotIndex].Team = Value;
	if (UHOLManagerClient* Client = bLobby ? GetClient() : nullptr)
	{
		const FString What = FString::Printf(TEXT("team %d"), Value);
		UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
		Client->UpdateMe({}, Value, {}, FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error)
		{
			HandleWritten(Written, Error, What);
		}));
	}
}

void UHOLNewGameScreen::ToggleReady()
{
	const int32 Mine = Lobby.FindYourSlot();
	UHOLManagerClient* Client = bLobby ? GetClient() : nullptr;
	if (!Client || !Lobby.Slots.IsValidIndex(Mine))
	{
		return;
	}
	const bool bReady = !Lobby.Slots[Mine].bReady;
	const FString What = FString::Printf(TEXT("ready %s"), bReady ? TEXT("on") : TEXT("off"));
	UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
	Client->UpdateMe({}, {}, bReady, FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error)
	{
		HandleWritten(Written, Error, What);
	}));
}

void UHOLNewGameScreen::StartMatch()
{
	UHOLManagerClient* Client = GetClient();
	if (!Client)
	{
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("Lobby: start sent"));
	Client->StartMatch(FHOLManagerStartDone::CreateWeakLambda(this, [this](const FString& MatchId, const FHOLManagerError& Error)
	{
		if (Error.IsError())
		{
			UE_LOG(LogHOLUI, Display, TEXT("Lobby: start refused (HTTP %d %s: %s)"), Error.HttpCode, *Error.Code, *Error.Message);
			ShowError(GetManagerErrorText(Error));
			return;
		}
		// The server is reached once the lobby's state says in_game (ShowLobby).
		UE_LOG(LogHOLUI, Display, TEXT("Lobby: start accepted (HTTP %d), match %s"), Error.HttpCode, *MatchId);
		ShowError(FText::GetEmpty());
	}));
}

void UHOLNewGameScreen::JoinMatch()
{
	UHOLManagerClient* Client = GetClient();
	if (!bLobby || !Client)
	{
		return;
	}
	// Once: no more reading, and travelling away does not leave the lobby (its token serves after the match, R4.8).
	bLobby = false;
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReadTimer);
	}
	const FString Host = FHOLManagerClientRules::MakeMatchAddress(Client->GetUrl(), Lobby.MatchPort, FString(), FString());
	UE_LOG(LogHOLUI, Display, TEXT("Lobby: joining match %s at %s as %s"), *Lobby.MatchId, *Host.Left(Host.Find(TEXT("?"))),
		*Lobby.MatchPlayerName);
	if (!Client->OpenMatch(Lobby.MatchPort, Lobby.MatchPlayerName, Lobby.MatchGameVersion))
	{
		ShowError(LOCTEXT("NoMatchAddress", "Adresse du serveur de partie introuvable."));
	}
}

void UHOLNewGameScreen::HandleWritten(const FHOLLobby& Written, const FHOLManagerError& Error, const FString& What)
{
	if (!bLobby)
	{
		return;
	}
	if (Error.IsError())
	{
		const FText Text = GetManagerErrorText(Error);
		UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s refused: %s (HTTP %d %s)"), *What, *Text.ToString(), Error.HttpCode, *Error.Code);
		ShowError(Text);
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s accepted"), *What);
	ShowError(FText::GetEmpty());
	ShowLobby(Written);
}

void UHOLNewGameScreen::ShowError(const FText& Text)
{
	if (ErrorText)
	{
		ErrorText->SetText(Text);
		ErrorText->SetVisibility(Text.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UHOLNewGameScreen::HandleStateChosen(const int32 SlotIndex, const int32 Value)
{
	if (bLobby)
	{
		// Open (0) or closed (1), the leader's (LobbyStateNames); the next read puts back the manager's value.
		UHOLManagerClient* Client = GetClient();
		if (Client && Value <= 1)
		{
			const FString What = FString::Printf(TEXT("slot %d %s"), SlotIndex, Value == 0 ? TEXT("open") : TEXT("closed"));
			UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
			Client->SetSlot(SlotIndex, Value == 0, FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error)
			{
				HandleWritten(Written, Error, What);
			}));
		}
		return;
	}
	// AI is greyed in the list (D240): only Closed and Human come here.
	FHOLMatchSlot& SlotConfig = Config.Slots[SlotIndex];
	SlotConfig.State = static_cast<EHOLSlotState>(Value);
	if (SlotConfig.State != EHOLSlotState::Closed && IsColorTaken(SlotIndex, SlotConfig.ColorIndex))
	{
		SlotConfig.ColorIndex = FindFreeColor(SlotIndex, SlotConfig.ColorIndex);
	}
	Refresh();
}

void UHOLNewGameScreen::HandleColorChosen(const int32 SlotIndex, const int32 Value)
{
	// A colour held by another occupied slot is greyed in the list: this one is free.
	Config.Slots[SlotIndex].ColorIndex = Value;
	Refresh();
	if (UHOLManagerClient* Client = bLobby ? GetClient() : nullptr)
	{
		const FString What = FString::Printf(TEXT("colour %d"), Value);
		UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
		Client->UpdateMe(Value, {}, {}, FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error)
		{
			HandleWritten(Written, Error, What);
		}));
	}
}

bool UHOLNewGameScreen::IsColorTaken(const int32 SlotIndex, const int32 ColorIndex) const
{
	// E2-2: several slots can be on "Aléatoire".
	if (ColorIndex == HOLPlayerColors::RandomColor)
	{
		return false;
	}
	for (int32 Other = 0; Other < Config.Slots.Num(); ++Other)
	{
		if (Other != SlotIndex && Config.Slots[Other].State != EHOLSlotState::Closed && Config.Slots[Other].ColorIndex == ColorIndex)
		{
			return true;
		}
	}
	return false;
}

int32 UHOLNewGameScreen::FindFreeColor(const int32 SlotIndex, const int32 From) const
{
	constexpr int32 NumColors = HOLPlayerColors::NumColors;
	for (int32 Tried = 0; Tried < NumColors; ++Tried)
	{
		const int32 Color = (From + Tried) % NumColors;
		if (!IsColorTaken(SlotIndex, Color))
		{
			return Color;
		}
	}
	return From;
}

void UHOLNewGameScreen::Launch()
{
	if (bLobby)
	{
		StartMatch();
		return;
	}
	FHOLMatchConfig Launched = Config;
	Launched.Seed = static_cast<int32>(FDateTime::UtcNow().GetTicks() & MAX_int32);
	// E2-2: "Aléatoire" becomes a colour nobody else has, before Normalize clamps it.
	FRandomStream ColorStream(Launched.Seed);
	Launched.DrawRandomColors(ColorStream);
	Launched.Normalize();

	UGameInstance* GameInstance = GetGameInstance();
	UHOLMatchSubsystem* Match = GameInstance ? GameInstance->GetSubsystem<UHOLMatchSubsystem>() : nullptr;
	if (!Match || Launched.Map.IsNull())
	{
		UE_LOG(LogHOLUI, Error, TEXT("New game: no match subsystem or no map, nothing launched"));
		return;
	}
	Match->SetConfig(Launched);
	UE_LOG(LogHOLUI, Display, TEXT("New game: launching %s, %d occupied slots, seed %d"), *Launched.Map.ToString(),
		Launched.GetNumOccupiedSlots(), Launched.Seed);

	// The menu's input config (menu mode) would linger on the viewport once its screens are gone: the match starts in
	// game mode, the mouse free and its cursor shown, as the RTS camera and its edge scrolling want (D254, D255).
	if (UCommonUIActionRouterBase* Router = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetOwningLocalPlayer()))
	{
		Router->SetActiveUIInputConfig(FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::NoCapture), this);
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, TSoftObjectPtr<UWorld>(Launched.Map));
}

#undef LOCTEXT_NAMESPACE
