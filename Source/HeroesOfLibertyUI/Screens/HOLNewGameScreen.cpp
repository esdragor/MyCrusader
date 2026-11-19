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
#include "Match/HOLFactionChoice.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/World.h"
#include "HeroesOfLibertyUILog.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Map/HOLMapRules.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLMatchConfig.h"
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
	/** IA0.6 S2: the AI level, in the place the name column leaves in solo (the grid keeps its width). */
	constexpr float LevelWidth = NameWidth;
	constexpr float OptionWidth = 200.f;
	constexpr float ToggleWidth = 120.f;
	constexpr float OptionLabelWidth = 170.f;
	/** "Équipes ensemble" over the swatch and colour columns, its value over the team column. */
	constexpr float TeamsTogetherLabelWidth = SwatchSize + 8.f + ColorWidth;
	constexpr float ClosedSwatchOpacity = 0.2f;

	/** List values in the order of EHOLSlotState. */
	TArray<FText> StateNames()
	{
		return { LOCTEXT("Closed", "Fermé"), LOCTEXT("Human", "Humain"), LOCTEXT("AI", "IA") };
	}

	/** List values in the order of EHOLAIDifficulty, the names the game gives them (« IA Normal 2 »). */
	TArray<FText> LevelNames()
	{
		return { HOLForumMessages::GetDifficultyName(EHOLAIDifficulty::Easy), HOLForumMessages::GetDifficultyName(EHOLAIDifficulty::Normal),
			HOLForumMessages::GetDifficultyName(EHOLAIDifficulty::Hard), HOLForumMessages::GetDifficultyName(EHOLAIDifficulty::Evolving) };
	}

	/** IA0.6 S2: shows or takes out a widget and its cell, the cell keeping its width when bKeepPlace (Hidden) or not (Collapsed). */
	void ShowCell(UWidget* Widget, const bool bShown, const bool bKeepPlace)
	{
		if (!Widget)
		{
			return;
		}
		const ESlateVisibility Visibility = bShown ? ESlateVisibility::Visible : bKeepPlace ? ESlateVisibility::Hidden : ESlateVisibility::Collapsed;
		Widget->SetVisibility(Visibility);
		if (USizeBox* Cell = Cast<USizeBox>(Widget->GetParent()))
		{
			Cell->SetVisibility(bShown || bKeepPlace ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		}
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

	/** A lobby slot's state (network mode); IA0.6 L4: "IA" last (the leader's, LobbyAIState), shown with the AI's name. */
	TArray<FText> LobbyStateNames()
	{
		return { LOCTEXT("Open", "Ouvert"), LOCTEXT("Closed", "Fermé"), LOCTEXT("NotReady", "Pas prêt"), LOCTEXT("Ready", "Prêt"), LOCTEXT("AI", "IA") };
	}

	/** IA0.6 L4: the value of "IA" in LobbyStateNames. */
	constexpr int32 LobbyAIState = 4;

	/** The lobby's state read twice a second (Manager/PROTOCOL.md). */
	constexpr float ReadPeriod = 0.5f;

	/** F8 (D522): the fog's list, Normal first (index = 2 - EHOLFogMode). */
	TArray<FText> FogNames()
	{
		return { LOCTEXT("FogNormal", "Normal"), LOCTEXT("FogExplored", "Carte explorée"), LOCTEXT("FogDisabled", "Sans brouillard") };
	}

	TArray<FText> OffOn()
	{
		return { LOCTEXT("Off", "Non"), LOCTEXT("On", "Oui") };
	}

	FName SlotWidgetName(const TCHAR* Prefix, const int32 SlotIndex)
	{
		return FName(FString::Printf(TEXT("%s%d"), Prefix, SlotIndex));
	}

	/** TC3: the size list, index = EHOLMapSize (0, None, "—": a test map, never chosen). */
	TArray<FText> SizeNames()
	{
		TArray<FText> Names = { UHOLNewGameScreen::GetSizeName(EHOLMapSize::None) };
		for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
		{
			Names.Add(UHOLNewGameScreen::GetSizeName(Info.Size));
		}
		return Names;
	}
}

namespace UE::HeroesOfLiberty::UI::NewGame
{
	/** TC-Q8: the name of a size alone ("Petite"); empty for None. */
	FText GetShortSizeName(const EHOLMapSize Size)
	{
		switch (Size)
		{
		case EHOLMapSize::Small: return LOCTEXT("Size_Small", "Petite");
		case EHOLMapSize::Medium: return LOCTEXT("Size_Medium", "Moyenne");
		case EHOLMapSize::Normal: return LOCTEXT("Size_Normal", "Normale");
		case EHOLMapSize::Large: return LOCTEXT("Size_Large", "Grande");
		case EHOLMapSize::Huge: return LOCTEXT("Size_Huge", "Immense");
		default: return FText::GetEmpty();
		}
	}
}

FText UHOLNewGameScreen::GetSizeName(const EHOLMapSize Size)
{
	const FText Name = UE::HeroesOfLiberty::UI::NewGame::GetShortSizeName(Size);
	if (Name.IsEmpty())
	{
		return FText::AsCultureInvariant(TEXT("—"));
	}
	return FText::Format(LOCTEXT("SizeWithPlayers", "{0} ({1} joueurs)"), Name, FHOLMapSizeRules::GetNumPlaces(Size));
}

FText UHOLNewGameScreen::GetLobbyMapName(const FString& Map, const FString& Size)
{
	if (Map.IsEmpty())
	{
		return FText::GetEmpty();
	}
	const FText MapName = GetMapName(FHOLMapRules::GetAssetPath(*Map));
	const FText SizeName = UE::HeroesOfLiberty::UI::NewGame::GetShortSizeName(FHOLMapSizeRules::FindByKey(Size));
	return SizeName.IsEmpty() ? MapName : FText::Format(LOCTEXT("LobbyMapAndSize", "{0} — {1}"), MapName, SizeName);
}

FText UHOLNewGameScreen::GetMapName(const FSoftObjectPath& Map)
{
	// C0.1: the table's name (Tools/Data/Maps.csv), the asset name for a map it does not know (a test map).
	return FHOLMapRules::GetDisplayName(Map);
}

UHOLNewGameScreen* UHOLNewGameScreen::Open(APlayerController* PlayerController)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	if (!Layout)
	{
		return nullptr;
	}
	// NG-6b (D851): the script class of Script/UI/HOLNewGameScreen.as is the screen (the base class has no layout); without it (script not loaded) nothing opens.
	const TSubclassOf<UHOLNewGameScreen> ScriptClass = FindScriptClass();
	if (!ScriptClass)
	{
		UE_LOG(LogHOLUI, Error, TEXT("New game screen: the script class UHOLNewGameScreenScript is not loaded, the screen cannot open"));
		return nullptr;
	}
	return Cast<UHOLNewGameScreen>(Layout->PushMenu(ScriptClass.Get()));
}

UHOLNewGameScreen* UHOLNewGameScreen::Find(APlayerController* PlayerController)
{
	const UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	return Layout ? Cast<UHOLNewGameScreen>(Layout->GetActiveMenu()) : nullptr;
}

void UHOLNewGameScreen::BuildLayout()
{
	// The screen is laid out by its script class (Script/UI/HOLNewGameScreen.as, BP_BuildNewGame); the base class itself has no layout (NG-6b).
	BP_BuildNewGame();
}

void UHOLNewGameScreen::NativeOnInitialized()
{
	using namespace UE::HeroesOfLiberty::UI::NewGame;

	// C0.1: the maps of the table, in its order (the first one is the default), before the layout lists them.
	Maps.Reset();
	for (const FHOLMapInfo& Map : FHOLMapRules::GetMaps())
	{
		Maps.Add(FHOLMapRules::GetAssetPath(Map.Key));
	}
	Super::NativeOnInitialized();

	// The script class laid itself out in BP_BuildNewGame (Super above): the members are found by their widget's name, as the Slot* below, so that
	// the base's behaviour (Launch, Refresh, ReadLobby, ShowError, focus, the columns) works on that layout. Heading is HeadingText's name, Note NoteText's.
	MapDropdown = Cast<UHOLDropdown>(WidgetTree->FindWidget(TEXT("MapDropdown")));
	SizeDropdown = Cast<UHOLDropdown>(WidgetTree->FindWidget(TEXT("SizeDropdown")));
	TeamsTogetherDropdown = Cast<UHOLDropdown>(WidgetTree->FindWidget(TEXT("TeamsTogetherDropdown")));
	FogDropdown = Cast<UHOLDropdown>(WidgetTree->FindWidget(TEXT("FogDropdown")));
	ModeDropdown = Cast<UHOLDropdown>(WidgetTree->FindWidget(TEXT("ModeDropdown")));
	NomadDropdown = Cast<UHOLDropdown>(WidgetTree->FindWidget(TEXT("NomadDropdown")));
	RegicideDropdown = Cast<UHOLDropdown>(WidgetTree->FindWidget(TEXT("RegicideDropdown")));
	CatastropheDropdown = Cast<UHOLDropdown>(WidgetTree->FindWidget(TEXT("CatastropheDropdown")));
	SacredSitesDropdown = Cast<UHOLDropdown>(WidgetTree->FindWidget(TEXT("SacredSitesDropdown")));
	LaunchButton = Cast<UHOLButton>(WidgetTree->FindWidget(TEXT("LaunchButton")));
	BackButton = Cast<UHOLButton>(WidgetTree->FindWidget(TEXT("BackButton")));
	ReadyButton = Cast<UHOLButton>(WidgetTree->FindWidget(TEXT("ReadyButton")));
	SpectateButton = Cast<UHOLButton>(WidgetTree->FindWidget(TEXT("SpectateButton")));
	SpectatorsText = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("SpectatorsText")));
	NoteText = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("Note")));
	ErrorText = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("ErrorText")));
	HeadingText = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("Heading")));

	SlotYou = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("SlotYou0")));
	HeadName = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("HeadName")));
	HeadSlot = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("HeadSlot")));
	HeadLevel = Cast<UCommonTextBlock>(WidgetTree->FindWidget(TEXT("HeadLevel")));
	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		SlotStates.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotState"), SlotIndex))));
		SlotFactions.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotFaction"), SlotIndex))));
		SlotColors.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotColor"), SlotIndex))));
		SlotSwatches.Add(Cast<UImage>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotSwatch"), SlotIndex))));
		SlotTeams.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotTeam"), SlotIndex))));
		SlotNames.Add(Cast<UCommonTextBlock>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotName"), SlotIndex))));
		SlotLevels.Add(Cast<UHOLDropdown>(WidgetTree->FindWidget(SlotWidgetName(TEXT("SlotLevel"), SlotIndex))));
		// IA0.6 S2 (Q1): the level is shown, not chosen yet (Normal).
		if (SlotLevels[SlotIndex])
		{
			SlotLevels[SlotIndex]->SetIsEnabled(false);
		}
	}
	for (UHOLDropdown* Greyed : { NomadDropdown.Get(), RegicideDropdown.Get(), CatastropheDropdown.Get() })
	{
		if (Greyed)
		{
			Greyed->SetIsEnabled(false);
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
	if (SpectateButton)
	{
		SpectateButton->OnClicked().AddUObject(this, &UHOLNewGameScreen::ToggleSpectator);
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

void UHOLNewGameScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	// The stack reuses a closed screen: the cheat may have changed on the title screen since the lists were last filled.
	UpdateFactionLists();
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
	if (SpectateButton)
	{
		SpectateButton->SetVisibility(ESlateVisibility::Visible);
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
	Config.bSacredSites = Lobby.bSacredSites;
	// TC3 loop 2: the lobby's size and "Équipes ensemble", the places drawn by the server at the launch.
	Config.MapSize = Lobby.GetMapSize();
	Config.bTeamsTogether = Lobby.bTeamsTogether;
	Config.FogMode = Lobby.GetFogMode();
	Config.Mode = Lobby.GetMatchMode();
	Config.bDrawPlaces = true;
	TArray<FString> Players;
	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		FHOLMatchSlot& SlotConfig = Config.Slots[SlotIndex];
		const FHOLLobbySlot* Row = Lobby.Slots.IsValidIndex(SlotIndex) ? &Lobby.Slots[SlotIndex] : nullptr;
		const bool bPlayer = Row && Row->State == EHOLLobbySlotState::Player;
		// IA0.6 L4: an AI holds its slot as in solo (colour, team, level), without a name.
		const bool bAI = Row && Row->State == EHOLLobbySlotState::AI;
		SlotConfig.State = bPlayer ? EHOLSlotState::Human : bAI ? EHOLSlotState::AI : EHOLSlotState::Closed;
		SlotConfig.PlayerName = bPlayer ? Row->PlayerName : FString();
		if (bPlayer || bAI)
		{
			SlotConfig.ColorIndex = FMath::Clamp(Row->ColorIndex, 0, HOLPlayerColors::RandomColor);
			SlotConfig.Team = FMath::Clamp(Row->Team, 0, FHOLMatchConfig::MaxTeams);
			// The manager's name of the faction (the enum's); an unknown one reads as the Dawn.
			if (!HOLFactionChoice::TryFindByName(Row->Faction, SlotConfig.Faction))
			{
				SlotConfig.Faction = EHOLFaction::DawnOrder;
			}
		}
		if (bAI)
		{
			SlotConfig.Difficulty = Row->Level;
		}
		if (bPlayer)
		{
			Players.Add(Row->PlayerName);
		}
	}
	Refresh();

	const bool bLeader = Lobby.IsYouLeader();
	const bool bSpectator = Lobby.IsYouSpectator();
	// The script class shows the lobby (BP_ShowLobby: GetLobbyView and the GetNewGame* texts); the reading, the sends and the errors stay here.
	BP_ShowLobby();
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
		// IA0.6 L4: an AI "a<colour>" (no Ready).
		SlotsShown.Add(Row.State == EHOLLobbySlotState::Open ? FString(TEXT("o")) : Row.State == EHOLLobbySlotState::Closed ? FString(TEXT("x"))
			: Row.State == EHOLLobbySlotState::AI ? FString::Printf(TEXT("a%d"), Row.ColorIndex)
			: FString::Printf(TEXT("c%d%s"), Row.ColorIndex, Row.bReady ? TEXT("r") : TEXT("")));
	}
	const FString Logged = FString::Printf(TEXT("%d players (%s), you %s%s, leader %s, %d/%d ready, launch %s, slots %s, spectators (%s)"), Players.Num(),
		*FString::Join(Players, TEXT(", ")), *Lobby.You, bSpectator ? TEXT(" (spectator)") : TEXT(""), *Lobby.Leader, NumReady, Players.Num(),
		bLeader ? TEXT("shown") : TEXT("hidden"), *FString::Join(SlotsShown, TEXT(" ")), *FString::Join(Lobby.Spectators, TEXT(", ")));
	// FactionChoiceInProgress.md: the factions of the occupied slots as the manager gave them, logged when they change (the lobby test reads them).
	TArray<FString> FactionNamesRead;
	for (const FHOLLobbySlot& Row : Lobby.Slots)
	{
		if (Row.State == EHOLLobbySlotState::Player || Row.State == EHOLLobbySlotState::AI)
		{
			FactionNamesRead.Add(Row.Faction);
		}
	}
	const FString FactionsRead = FString::Join(FactionNamesRead, TEXT(","));
	if (FactionsRead != LoggedFactions)
	{
		LoggedFactions = FactionsRead;
		UE_LOG(LogHOLUI, Display, TEXT("Lobby: factions %s"), *FactionsRead);
	}
	if (Logged != LoggedPlayers)
	{
		LoggedPlayers = Logged;
		UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s \"%s\" map %s %s teams together %d: %s"), *Lobby.Id, *Lobby.Name, *Lobby.Map, *Lobby.Size,
			Lobby.bTeamsTogether, *Logged);
	}
}

void UHOLNewGameScreen::GetColumns(TArray<TArray<UWidget*>>& OutColumns) const
{
	TArray<UWidget*>& States = OutColumns.AddDefaulted_GetRef();
	States.Add(MapDropdown);
	States.Add(SizeDropdown);
	States.Append(SlotStates);
	States.Add(ModeDropdown);
	OutColumns.Emplace(SlotFactions);
	OutColumns.Emplace(SlotColors);
	// F8: "Brouillard", then "Équipes ensemble", stand over the team column.
	TArray<UWidget*>& Teams = OutColumns.AddDefaulted_GetRef();
	Teams.Add(FogDropdown);
	Teams.Add(TeamsTogetherDropdown);
	Teams.Append(SlotTeams);
}

void UHOLNewGameScreen::SetConfig(const FHOLMatchConfig& InConfig)
{
	Config = InConfig;
	Config.Normalize();
	if (!Maps.IsEmpty() && !Maps.Contains(Config.Map))
	{
		Config.Map = Maps[0];
		// F8 (D522): the game's fog, Normal, offered by default (a test configuration has none).
		Config.FogMode = EHOLFogMode::Normal;
		// TC3: the menu's own configuration: the smallest size that holds the players, places drawn.
		if (Config.MapSize == EHOLMapSize::None)
		{
			ChooseSize(FHOLMapSizeRules::GetDefaultSize(Config.GetNumOccupiedSlots()));
		}
	}
	Refresh();
}

bool UHOLNewGameScreen::ChooseSize(const EHOLMapSize Size)
{
	if (Size == EHOLMapSize::None || !FHOLMapSizeRules::FitSlotsToSize(Config.Slots, Size))
	{
		return false;
	}
	Config.MapSize = Size;
	Config.bDrawPlaces = true;
	UE_LOG(LogHOLUI, Display, TEXT("New game: size %s, %d places"), FHOLMapSizeRules::Find(Size)->Key, FHOLMapSizeRules::GetNumPlaces(Size));
	return true;
}

void UHOLNewGameScreen::Refresh()
{
	using namespace UE::HeroesOfLiberty::UI::NewGame;

	if (MapDropdown)
	{
		MapDropdown->SetSelectedIndex(FMath::Max(0, Maps.IndexOfByKey(Config.Map)));
	}
	// TC3 (TC-Q4): the sizes too small for the players greyed, "—" never chosen.
	const int32 NumPlayers = Config.GetNumOccupiedSlots();
	if (SizeDropdown)
	{
		SizeDropdown->SetSelectedIndex(static_cast<int32>(Config.MapSize));
		SizeDropdown->SetOptionEnabled(static_cast<int32>(EHOLMapSize::None), false);
		for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
		{
			SizeDropdown->SetOptionEnabled(static_cast<int32>(Info.Size), FHOLMapSizeRules::IsAllowed(Info.Size, NumPlayers));
		}
	}
	if (TeamsTogetherDropdown)
	{
		TeamsTogetherDropdown->SetSelectedIndex(Config.bTeamsTogether ? 1 : 0);
	}
	if (FogDropdown)
	{
		FogDropdown->SetSelectedIndex(2 - static_cast<int32>(Config.FogMode));
	}
	if (ModeDropdown)
	{
		ModeDropdown->SetSelectedIndex(static_cast<int32>(Config.Mode));
	}
	// TC3 (TC-Q4): the slots past the places of the size are closed, not changeable (the lobby sets its own, loop 2).
	const int32 NumPlaces = FHOLMapSizeRules::GetNumPlaces(Config);
	UpdateFactionLists();
	const bool bFactionCheat = HOLFactionChoice::IsCheatOn();
	// IA0.6 S2: the name column and "Emplacement" in a lobby only; the level column in solo, and in a lobby with an AI
	// (L4: a lobby without one keeps its width).
	const bool bLevelColumn = !bLobby || Config.Slots.ContainsByPredicate([](const FHOLMatchSlot& Each) { return Each.State == EHOLSlotState::AI; });
	ShowCell(HeadName, bLobby, false);
	ShowCell(HeadLevel, bLevelColumn, false);
	if (HeadSlot)
	{
		HeadSlot->SetText(bLobby ? LOCTEXT("Slot", "Emplacement") : LOCTEXT("Player", "Joueur"));
	}
	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		const FHOLMatchSlot& SlotConfig = Config.Slots[SlotIndex];
		const bool bOccupied = SlotConfig.State != EHOLSlotState::Closed;
		const bool bAI = SlotConfig.State == EHOLSlotState::AI;
		if (SlotStates[SlotIndex])
		{
			// The lobby's list has its own values (LobbyStateNames), set by ShowLobby.
			if (!bLobby)
			{
				SlotStates[SlotIndex]->SetSelectedIndex(static_cast<int32>(SlotConfig.State));
				SlotStates[SlotIndex]->SetIsEnabled(SlotIndex < NumPlaces);
				// IA0.6 S2 (Q1): the name the slot will have, as the game names it (GetAIName, GetSlotName); "Fermé" stays.
				if (bAI)
				{
					SlotStates[SlotIndex]->SetShownText(HOLForumMessages::GetAIName(SlotIndex, SlotConfig.Difficulty));
				}
				else if (bOccupied)
				{
					SlotStates[SlotIndex]->SetShownText(FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), SlotIndex + 1));
				}
			}
		}
		ShowCell(SlotNames[SlotIndex], bLobby, false);
		if (SlotLevels[SlotIndex])
		{
			SlotLevels[SlotIndex]->SetSelectedIndex(static_cast<int32>(SlotConfig.Difficulty));
			ShowCell(SlotLevels[SlotIndex], bLevelColumn && bAI, bLevelColumn);
		}
		if (SlotFactions[SlotIndex])
		{
			SlotFactions[SlotIndex]->SetSelectedIndex(static_cast<int32>(SlotConfig.Faction));
			// A line on a faction the list does not hold (the Templars of another player or of the leader's AI, without the cheat here) says its name all the same.
			if (!HOLFactionChoice::IsListed(SlotConfig.Faction, bFactionCheat))
			{
				SlotFactions[SlotIndex]->SetShownText(UE::HeroesOfLiberty::UI::NewGame::FactionNames()[static_cast<int32>(SlotConfig.Faction)]);
			}
			// Solo: a line somebody holds. The lobby's (mine, the leader's AIs) is set by ShowLobby.
			if (!bLobby)
			{
				SlotFactions[SlotIndex]->SetIsEnabled(bOccupied);
			}
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
	if (SacredSitesDropdown)
	{
		SacredSitesDropdown->SetSelectedIndex(Config.bSacredSites ? 1 : 0);
		// S7 (D699, HOLSiteOption.as BoxEnabled): greyed on a map without a site layer (its count known before loading, FHOLMapRules);
		// in a lobby, the leader's alone like the other options.
		SacredSitesDropdown->SetIsEnabled((!bLobby || Lobby.IsYouLeader()) && FHOLMapRules::GetSiteCount(Config.Map) > 0);
	}
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
		// TC3: the size by its key (none for a map without sizes), "Équipes ensemble".
		const FHOLMapSizeInfo* Size = FHOLMapSizeRules::Find(Config.MapSize);
		const FString SizeKey = Size ? FString(Size->Key) : FString();
		const FString What = FString::Printf(TEXT("settings %s size %s teams together %d nomad %d regicide %d catastrophe %d sacred sites %d"),
			*Config.Map.GetAssetName(), *SizeKey, Config.bTeamsTogether, Config.bNomad, Config.bRegicide, Config.bCatastrophe, Config.bSacredSites);
		UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
		Client->UpdateSettings(Config.Map.GetAssetName(), Config.bNomad, Config.bRegicide, Config.bCatastrophe,
			FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error) { HandleWritten(Written, Error, What); }),
			SizeKey, Config.bTeamsTogether, StaticEnum<EHOLFogMode>()->GetNameStringByValue(static_cast<int64>(Config.FogMode)),
			StaticEnum<EHOLMatchMode>()->GetNameStringByValue(static_cast<int64>(Config.Mode)), FString(), Config.bSacredSites);
	}
}

void UHOLNewGameScreen::SendSlotAI(const int32 SlotIndex, const TOptional<int32> ColorIndex, const TOptional<int32> Team, const FString& Faction)
{
	UHOLManagerClient* Client = GetClient();
	if (!Client)
	{
		return;
	}
	// IA0.6 L4 (Q4): "slot 2 ai" puts the AI, "ai 2 colour 5" and "ai 2 team 2" change it.
	const FString What = ColorIndex.IsSet() ? FString::Printf(TEXT("ai %d colour %d"), SlotIndex, ColorIndex.GetValue())
		: Team.IsSet() ? FString::Printf(TEXT("ai %d team %d"), SlotIndex, Team.GetValue())
			: !Faction.IsEmpty() ? FString::Printf(TEXT("ai %d faction %s"), SlotIndex, *Faction) : FString::Printf(TEXT("slot %d ai"), SlotIndex);
	UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
	Client->SetSlotAI(SlotIndex, ColorIndex, Team, FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error)
	{
		HandleWritten(Written, Error, What);
	}), Faction);
}

bool UHOLNewGameScreen::IsLobbyAI(const int32 SlotIndex) const
{
	return bLobby && Lobby.Slots.IsValidIndex(SlotIndex) && Lobby.Slots[SlotIndex].State == EHOLLobbySlotState::AI;
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

void UHOLNewGameScreen::ToggleSpectator()
{
	UHOLManagerClient* Client = bLobby ? GetClient() : nullptr;
	if (!Client)
	{
		return;
	}
	const bool bToSpectator = !Lobby.IsYouSpectator();
	const FString What = FString::Printf(TEXT("spectator %s"), bToSpectator ? TEXT("on") : TEXT("off"));
	UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
	Client->UpdateMe({}, {}, {}, FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error)
	{
		HandleWritten(Written, Error, What);
	}), bToSpectator);
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

void UHOLNewGameScreen::UpdateFactionLists()
{
	const bool bCheat = HOLFactionChoice::IsCheatOn();
	if (FactionListCheat.IsSet() && FactionListCheat.GetValue() == bCheat)
	{
		return;
	}
	FactionListCheat = bCheat;
	const TArray<FText> AllNames = UE::HeroesOfLiberty::UI::NewGame::FactionNames();
	const TArray<EHOLFaction> Listed = HOLFactionChoice::GetListed(bCheat);
	TArray<FText> Names;
	for (const EHOLFaction Faction : Listed)
	{
		Names.Add(AllNames[static_cast<int32>(Faction)]);
	}
	for (UHOLDropdown* List : SlotFactions)
	{
		if (!List)
		{
			continue;
		}
		List->SetOptions(Names, List->GetSelectedIndex());
		for (int32 Index = 0; Index < Listed.Num(); ++Index)
		{
			List->SetOptionEnabled(Index, HOLFactionChoice::IsChoosable(Listed[Index], bCheat));
		}
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

void UHOLNewGameScreen::Launch()
{
	if (bLobby)
	{
		StartMatch();
		return;
	}
	FHOLMatchConfig Launched = Config;
	// A Templar line left from the cheat (switched off since) plays the Dawn: the Templars are the cheat's only.
	for (FHOLMatchSlot& LaunchedSlot : Launched.Slots)
	{
		LaunchedSlot.Faction = HOLFactionChoice::Sanitize(LaunchedSlot.Faction, HOLFactionChoice::IsCheatOn());
	}
	Launched.Seed = static_cast<int32>(FDateTime::UtcNow().GetTicks() & MAX_int32);
	// A scenario whose .json imposes a "seed" (V4-Q2 = C, D608) plays with it, even from the menu's launch.
	if (TOptional<int32>& Imposed = HOLTestHooks::PendingImposedSeed(); Imposed.IsSet())
	{
		Launched.Seed = Imposed.GetValue();
		Imposed.Reset();
	}
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
	// TC3: refused here rather than on the map (the list greys what does not fit; a configuration given by a file may not).
	FString PlacesError;
	if (!FHOLMapSizeRules::CheckPlaces(Launched, &PlacesError))
	{
		UE_LOG(LogHOLUI, Error, TEXT("New game: refused, %s"), *PlacesError);
		ShowError(LOCTEXT("TooManyPlayers", "Trop de joueurs pour cette taille de carte."));
		return;
	}
	// TC3 (TC-Q3): the file of the map at its size (Plain + Small: Plain_Small); without a size, the map as it is.
	const FSoftObjectPath MapFile = FHOLMapSizeRules::GetMapFile(Launched.Map, Launched.MapSize);
	Match->SetConfig(Launched);
	UE_LOG(LogHOLUI, Display, TEXT("New game: launching %s, %d occupied slots, seed %d"), *MapFile.ToString(),
		Launched.GetNumOccupiedSlots(), Launched.Seed);

	// The menu's input config (menu mode) would linger on the viewport once its screens are gone: the match starts in
	// game mode, the mouse free and its cursor shown, as the RTS camera and its edge scrolling want (D254, D255).
	if (UCommonUIActionRouterBase* Router = ULocalPlayer::GetSubsystem<UCommonUIActionRouterBase>(GetOwningLocalPlayer()))
	{
		Router->SetActiveUIInputConfig(FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::NoCapture), this);
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, TSoftObjectPtr<UWorld>(MapFile));
}

// ---- NG-1a (D851, step 4): the base the script's screen stands on (the C++ screen above still plays). ----

namespace UE::HeroesOfLiberty::UI::NewGame
{
	/** NG-1a: a widget in a fixed-width cell of a row (the grid's cells, as BuildLayout's AddCell). */
	USizeBox* AddNewGameCell(UWidgetTree& Tree, UHorizontalBox& Row, UWidget* Content, const float Width)
	{
		USizeBox* Box = Tree.ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Box->SetWidthOverride(Width);
		Box->SetContent(Content);
		UHorizontalBoxSlot* CellSlot = Row.AddChildToHorizontalBox(Box);
		CellSlot->SetVerticalAlignment(VAlign_Center);
		CellSlot->SetPadding(FMargin(4.f, 0.f));
		return Box;
	}

	TSubclassOf<UCommonTextStyle> NewGameTextStyle(const int32 Style)
	{
		return Style == 0 ? TSubclassOf<UCommonTextStyle>(UHOLTextStyleHeading::StaticClass())
			: Style == 1 ? TSubclassOf<UCommonTextStyle>(UHOLTextStyleNote::StaticClass())
			: TSubclassOf<UCommonTextStyle>(UHOLTextStyleBody::StaticClass());
	}
}

TSubclassOf<UHOLNewGameScreen> UHOLNewGameScreen::FindScriptClass()
{
	// Angelscript's hot reload keeps the older classes, flagged CLASS_NewerVersionExists.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
		{
			// The C++ class itself is concrete until the switch (NG-6): only a class deriving from it is the script's.
			if (*It != StaticClass())
			{
				return *It;
			}
		}
	}
	return nullptr;
}

void UHOLNewGameScreen::SendSlotAIChoice(const int32 SlotIndex, const int32 ColorIndex, const int32 Team, const FString& Faction)
{
	SendSlotAI(SlotIndex, ColorIndex >= 0 ? TOptional<int32>(ColorIndex) : TOptional<int32>(),
		Team >= 0 ? TOptional<int32>(Team) : TOptional<int32>(), Faction);
}

TArray<FText> UHOLNewGameScreen::GetMapNames() const
{
	TArray<FText> Names;
	for (const FSoftObjectPath& Map : Maps)
	{
		Names.Add(GetMapName(Map));
	}
	return Names;
}

TArray<FText> UHOLNewGameScreen::GetNewGameOptions(const FName Kind)
{
	namespace NG = UE::HeroesOfLiberty::UI::NewGame;
	if (Kind == TEXT("SlotState")) { return NG::StateNames(); }
	if (Kind == TEXT("LobbyState")) { return NG::LobbyStateNames(); }
	if (Kind == TEXT("Level")) { return NG::LevelNames(); }
	if (Kind == TEXT("Faction")) { return NG::FactionNames(); }
	if (Kind == TEXT("Team")) { return NG::TeamNames(); }
	if (Kind == TEXT("Color")) { return NG::ColorNames(); }
	if (Kind == TEXT("Fog")) { return NG::FogNames(); }
	if (Kind == TEXT("OffOn")) { return NG::OffOn(); }
	if (Kind == TEXT("Size")) { return NG::SizeNames(); }
	if (Kind == TEXT("Mode")) { return { LOCTEXT("Standard", "Standard"), LOCTEXT("ModeDiplomacy", "Diplomatie") }; }
	return {};
}

FText UHOLNewGameScreen::GetShortSizeNameOf(const int32 Size)
{
	return UE::HeroesOfLiberty::UI::NewGame::GetShortSizeName(static_cast<EHOLMapSize>(Size));
}

FText UHOLNewGameScreen::GetSizeNameOf(const int32 Size)
{
	return GetSizeName(static_cast<EHOLMapSize>(Size));
}

TArray<FLinearColor> UHOLNewGameScreen::GetNewGameColorSwatches()
{
	return UE::HeroesOfLiberty::UI::NewGame::ColorSwatches();
}

float UHOLNewGameScreen::GetNewGameWidth(const FName Key)
{
	namespace NG = UE::HeroesOfLiberty::UI::NewGame;
	if (Key == TEXT("Name")) { return NG::NameWidth; }
	if (Key == TEXT("State")) { return NG::StateWidth; }
	if (Key == TEXT("Faction")) { return NG::FactionWidth; }
	if (Key == TEXT("Swatch")) { return NG::SwatchSize; }
	if (Key == TEXT("Color")) { return NG::ColorWidth; }
	if (Key == TEXT("Team")) { return NG::TeamWidth; }
	if (Key == TEXT("Level")) { return NG::LevelWidth; }
	if (Key == TEXT("Option")) { return NG::OptionWidth; }
	if (Key == TEXT("Toggle")) { return NG::ToggleWidth; }
	if (Key == TEXT("OptionLabel")) { return NG::OptionLabelWidth; }
	if (Key == TEXT("TeamsTogetherLabel")) { return NG::TeamsTogetherLabelWidth; }
	if (Key == TEXT("MapList")) { return NG::StateWidth + NG::FactionWidth + 8.f; }
	if (Key == TEXT("FogList")) { return NG::TeamWidth + 60.f; }
	return 0.f;
}

UVerticalBox* UHOLNewGameScreen::MakeNewGameColumn()
{
	return MakeCenteredColumn();
}

UCommonTextBlock* UHOLNewGameScreen::AddNewGameText(UVerticalBox* Column, const FName Name, const FText& Text, const int32 Style)
{
	if (!Column)
	{
		return nullptr;
	}
	UCommonTextBlock* Block = AddText(Column, Name, Text, UE::HeroesOfLiberty::UI::NewGame::NewGameTextStyle(Style), Style == 0 ? 24.f : 4.f);
	// NG-1c: the manager's refusal is red, as in the C++ layout (the script does not know the colour).
	if (Block && Name == TEXT("ErrorText"))
	{
		Block->SetColorAndOpacity(HOLUIStyle::Missing);
	}
	return Block;
}

UHorizontalBox* UHOLNewGameScreen::AddNewGameRow(UVerticalBox* Column, const float TopPadding)
{
	if (!Column)
	{
		return nullptr;
	}
	// Lines start on the same left edge, the one of the slot lines (the widest).
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UVerticalBoxSlot* RowSlot = Column->AddChildToVerticalBox(Row);
	RowSlot->SetHorizontalAlignment(HAlign_Left);
	RowSlot->SetPadding(FMargin(0.f, TopPadding, 0.f, 4.f));
	return Row;
}

UCommonTextBlock* UHOLNewGameScreen::AddNewGameCellText(UHorizontalBox* Row, const FName Name, const FText& Text, const int32 Style, const float Width)
{
	if (!Row)
	{
		return nullptr;
	}
	UCommonTextBlock* Block = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), Name);
	Block->SetStyle(UE::HeroesOfLiberty::UI::NewGame::NewGameTextStyle(Style));
	Block->SetText(Text);
	UE::HeroesOfLiberty::UI::NewGame::AddNewGameCell(*WidgetTree, *Row, Block, Width);
	return Block;
}

UHOLDropdown* UHOLNewGameScreen::AddNewGameDropdown(UHorizontalBox* Row, const FName Name, const TArray<FText>& Options, const int32 SelectedIndex, const float Width)
{
	return Row ? AddDropdown(Row, Name, Options, SelectedIndex, Width) : nullptr;
}

UImage* UHOLNewGameScreen::AddNewGameSwatch(UHorizontalBox* Row, const FName Name)
{
	if (!Row)
	{
		return nullptr;
	}
	UImage* Swatch = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
	Swatch->SetDesiredSizeOverride(FVector2D(UE::HeroesOfLiberty::UI::NewGame::SwatchSize, UE::HeroesOfLiberty::UI::NewGame::SwatchSize));
	UE::HeroesOfLiberty::UI::NewGame::AddNewGameCell(*WidgetTree, *Row, Swatch, UE::HeroesOfLiberty::UI::NewGame::SwatchSize);
	return Swatch;
}

UCommonTextBlock* UHOLNewGameScreen::AddNewGameHint(UVerticalBox* Column)
{
	return Column ? AddColumnHint(Column) : nullptr;
}

UHorizontalBox* UHOLNewGameScreen::AddNewGameButtonRow(UVerticalBox* Column)
{
	return Column ? AddButtonRow(Column) : nullptr;
}

UHOLButton* UHOLNewGameScreen::AddNewGameButton(UHorizontalBox* Row, const FName Name, const FText& Label)
{
	return Row ? AddButton(Row, Name, Label) : nullptr;
}

// ---- NG-1c (D851, step 4): the raccords the script's gestures need; the C++ screen above is not changed by any of them. ----

FText UHOLNewGameScreen::GetNewGameText(const FName Key)
{
	// One source for the labels: the script asks here instead of re-declaring them (same keys as the LOCTEXT of the layout and of Refresh).
	if (Key == TEXT("Heading")) { return LOCTEXT("Heading", "Nouvelle partie"); }
	if (Key == TEXT("Map")) { return LOCTEXT("Map", "Carte"); }
	if (Key == TEXT("Fog")) { return LOCTEXT("Fog", "Brouillard"); }
	if (Key == TEXT("Size")) { return LOCTEXT("Size", "Taille"); }
	if (Key == TEXT("TeamsTogether")) { return LOCTEXT("TeamsTogether", "Équipes ensemble"); }
	if (Key == TEXT("Player")) { return LOCTEXT("Player", "Joueur"); }
	if (Key == TEXT("Slot")) { return LOCTEXT("Slot", "Emplacement"); }
	if (Key == TEXT("Level")) { return LOCTEXT("Level", "Niveau"); }
	if (Key == TEXT("Faction")) { return LOCTEXT("Faction", "Faction"); }
	if (Key == TEXT("Colour")) { return LOCTEXT("Colour", "Couleur"); }
	if (Key == TEXT("Team")) { return LOCTEXT("Team", "Équipe"); }
	if (Key == TEXT("You")) { return LOCTEXT("You", "Vous"); }
	if (Key == TEXT("Mode")) { return LOCTEXT("Mode", "Mode"); }
	if (Key == TEXT("Nomad")) { return LOCTEXT("Nomad", "Nomade"); }
	if (Key == TEXT("Regicide")) { return LOCTEXT("Regicide", "Régicide"); }
	if (Key == TEXT("Catastrophe")) { return LOCTEXT("Catastrophe", "Catastrophe"); }
	if (Key == TEXT("SacredSites")) { return LOCTEXT("SacredSites", "Sites sacrés"); }
	if (Key == TEXT("GreyedTeamsOpen")) { return LOCTEXT("GreyedTeamsOpen", "Grisé : pas encore dans le jeu (factions, mode et options)."); }
	if (Key == TEXT("LobbyGreyed")) { return LOCTEXT("LobbyGreyed", "Grisé : pas encore dans le jeu (factions, mode) ; seul le chef règle la carte, les options et les emplacements."); }
	if (Key == TEXT("Back")) { return LOCTEXT("Back", "Retour"); }
	if (Key == TEXT("LeaveLobby")) { return LOCTEXT("LeaveLobby", "Quitter le salon"); }
	if (Key == TEXT("ReadyAction")) { return LOCTEXT("ReadyAction", "Prêt"); }
	if (Key == TEXT("NotReadyAction")) { return LOCTEXT("NotReadyAction", "Plus prêt"); }
	if (Key == TEXT("BecomeSpectator")) { return LOCTEXT("BecomeSpectator", "Spectateur"); }
	if (Key == TEXT("BecomePlayer")) { return LOCTEXT("BecomePlayer", "Jouer"); }
	if (Key == TEXT("Launch")) { return LOCTEXT("Launch", "Lancer"); }
	if (Key == TEXT("Starting")) { return LOCTEXT("Starting", "Lancement…"); }
	if (Key == TEXT("NoMatchAddress")) { return LOCTEXT("NoMatchAddress", "Adresse du serveur de partie introuvable."); }
	if (Key == TEXT("TooManyPlayers")) { return LOCTEXT("TooManyPlayers", "Trop de joueurs pour cette taille de carte."); }
	return FText::GetEmpty();
}

FText UHOLNewGameScreen::GetNewGamePlayerName(const int32 SlotIndex)
{
	return FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), SlotIndex + 1);
}

FText UHOLNewGameScreen::GetNewGameAIName(const int32 SlotIndex, const int32 Level)
{
	return HOLForumMessages::GetAIName(SlotIndex, static_cast<EHOLAIDifficulty>(FMath::Clamp(Level, 0, 3)));
}

FText UHOLNewGameScreen::GetNewGameLeaderName(const FString& Name)
{
	return FText::Format(LOCTEXT("Leader", "♛ {0}"), FText::FromString(Name));
}

FText UHOLNewGameScreen::GetNewGameSpectatorsText(const TArray<FString>& Spectators, const FString& Leader)
{
	TArray<FString> Names;
	for (const FString& Name : Spectators)
	{
		Names.Add(Name == Leader ? GetNewGameLeaderName(Name).ToString() : Name);
	}
	return Names.IsEmpty() ? FText::GetEmpty() : FText::Format(LOCTEXT("Spectators", "Spectateurs : {0}"), FText::FromString(FString::Join(Names, TEXT(", "))));
}

FText UHOLNewGameScreen::GetNewGameLobbyLineName(const int32 SlotIndex) const
{
	const FHOLLobbySlot* Row = Lobby.Slots.IsValidIndex(SlotIndex) ? &Lobby.Slots[SlotIndex] : nullptr;
	if (Row && Row->State == EHOLLobbySlotState::AI)
	{
		return FText::GetEmpty();
	}
	if (!Row || Row->State != EHOLLobbySlotState::Player)
	{
		return GetNewGamePlayerName(SlotIndex);
	}
	return Row->PlayerName == Lobby.Leader ? GetNewGameLeaderName(Row->PlayerName) : FText::FromString(Row->PlayerName);
}

FText UHOLNewGameScreen::GetNewGameLobbyAIName(const int32 SlotIndex) const
{
	const bool bAI = Lobby.Slots.IsValidIndex(SlotIndex) && Lobby.Slots[SlotIndex].State == EHOLLobbySlotState::AI;
	return bAI ? HOLForumMessages::GetAIName(SlotIndex, Lobby.Slots[SlotIndex].Level) : FText::GetEmpty();
}

FString UHOLNewGameScreen::GetNewGameFactionName(const int32 Faction)
{
	return Faction >= 0 && Faction <= static_cast<int32>(EHOLFaction::Templars) ? HOLFactionChoice::GetName(static_cast<EHOLFaction>(Faction)) : FString();
}

void UHOLNewGameScreen::SetNewGameButtonLabel(const FName ButtonName, const FText& Label)
{
	if (UHOLButton* Button = WidgetTree ? Cast<UHOLButton>(WidgetTree->FindWidget(ButtonName)) : nullptr)
	{
		Button->SetLabel(Label);
	}
}

bool UHOLNewGameScreen::HasClient() const
{
	return GetClient() != nullptr;
}

bool UHOLNewGameScreen::CanToggleLobbySlot(const int32 SlotIndex) const
{
	return bLobby && Lobby.CanToggleSlot(SlotIndex);
}

bool UHOLNewGameScreen::CanEditLobbySlot(const int32 SlotIndex) const
{
	return bLobby && Lobby.CanEditSlot(SlotIndex);
}

bool UHOLNewGameScreen::CanEditLobbyAISlot(const int32 SlotIndex) const
{
	return bLobby && Lobby.CanEditAISlot(SlotIndex);
}

bool UHOLNewGameScreen::IsLobbyYouSpectator() const
{
	return bLobby && Lobby.IsYouSpectator();
}

bool UHOLNewGameScreen::IsLobbyYouLeader() const
{
	return bLobby && Lobby.IsYouLeader();
}

bool UHOLNewGameScreen::LobbyHasOpenSlot() const
{
	return bLobby && Lobby.HasOpenSlot();
}

bool UHOLNewGameScreen::CanStartLobby() const
{
	return bLobby && Lobby.CanStart();
}

FHOLNewGameLobbyView UHOLNewGameScreen::GetLobbyView() const
{
	return UHOLNewGameReadLibrary::MakeLobbyView(Lobby);
}

FString UHOLNewGameScreen::GetConfigJson() const
{
	return Config.ToJson();
}

bool UHOLNewGameScreen::SetConfigJson(const FString& Json)
{
	FHOLMatchConfig Parsed;
	if (!FHOLMatchConfig::FromJson(Json, Parsed))
	{
		return false;
	}
	SetConfig(Parsed);
	return true;
}

int32 UHOLNewGameScreen::GetConfigMapIndex() const
{
	return Maps.IndexOfByKey(Config.Map);
}

void UHOLNewGameScreen::SetConfigMapIndex(const int32 Index)
{
	if (Maps.IsValidIndex(Index))
	{
		Config.Map = Maps[Index];
	}
}

void UHOLNewGameScreen::SetConfigMapSize(const int32 Size)
{
	if (FHOLMapSizeRules::Find(static_cast<EHOLMapSize>(Size)))
	{
		Config.MapSize = static_cast<EHOLMapSize>(Size);
	}
}

bool UHOLNewGameScreen::ChooseNewGameSize(const int32 Size)
{
	return ChooseSize(static_cast<EHOLMapSize>(Size));
}

void UHOLNewGameScreen::SetConfigGlobals(const bool bTeamsTogether, const int32 FogMode, const int32 Mode)
{
	Config.bTeamsTogether = bTeamsTogether;
	Config.FogMode = static_cast<EHOLFogMode>(FMath::Clamp(FogMode, 0, 2));
	Config.Mode = Mode == 1 ? EHOLMatchMode::Diplomacy : EHOLMatchMode::Standard;
}

void UHOLNewGameScreen::SetConfigOptions(const bool bNomad, const bool bRegicide, const bool bCatastrophe, const bool bSacredSites)
{
	Config.bNomad = bNomad;
	Config.bRegicide = bRegicide;
	Config.bCatastrophe = bCatastrophe;
	Config.bSacredSites = bSacredSites;
}

void UHOLNewGameScreen::SetConfigSlot(const int32 SlotIndex, const int32 State, const int32 Faction, const int32 ColorIndex, const int32 Team)
{
	if (!Config.Slots.IsValidIndex(SlotIndex))
	{
		return;
	}
	FHOLMatchSlot& SlotConfig = Config.Slots[SlotIndex];
	if (State >= 0)
	{
		SlotConfig.State = static_cast<EHOLSlotState>(State);
	}
	if (Faction >= 0)
	{
		SlotConfig.Faction = static_cast<EHOLFaction>(Faction);
	}
	if (ColorIndex >= 0)
	{
		SlotConfig.ColorIndex = ColorIndex;
	}
	if (Team >= 0)
	{
		SlotConfig.Team = Team;
	}
}

void UHOLNewGameScreen::SendMyChoice(const int32 ColorIndex, const int32 Team, const int32 Ready, const int32 Spectator, const FString& Faction)
{
	UHOLManagerClient* Client = bLobby ? GetClient() : nullptr;
	if (!Client)
	{
		return;
	}
	// ELROND-1118x: ONE field changed = the text of the cpp gesture it replaces ("colour 3", "team 2", "ready on", "spectator off", "faction Templars"), the
	// journal line the lobby test (and the cpp screen) write: "Lobby: colour 3 accepted". Several fields: the whole line.
	const int32 NumFields = (ColorIndex >= 0 ? 1 : 0) + (Team >= 0 ? 1 : 0) + (Ready >= 0 ? 1 : 0) + (Spectator >= 0 ? 1 : 0) + (Faction.IsEmpty() ? 0 : 1);
	const FString What = NumFields != 1 ? FString::Printf(TEXT("me colour %d team %d ready %d spectator %d faction %s"), ColorIndex, Team, Ready, Spectator, *Faction)
		: ColorIndex >= 0 ? FString::Printf(TEXT("colour %d"), ColorIndex)
		: Team >= 0 ? FString::Printf(TEXT("team %d"), Team)
		: Ready >= 0 ? FString::Printf(TEXT("ready %s"), Ready == 1 ? TEXT("on") : TEXT("off"))
		: Spectator >= 0 ? FString::Printf(TEXT("spectator %s"), Spectator == 1 ? TEXT("on") : TEXT("off"))
		: FString::Printf(TEXT("faction %s"), *Faction);
	UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
	Client->UpdateMe(ColorIndex >= 0 ? TOptional<int32>(ColorIndex) : TOptional<int32>(), Team >= 0 ? TOptional<int32>(Team) : TOptional<int32>(),
		Ready >= 0 ? TOptional<bool>(Ready == 1) : TOptional<bool>(),
		FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error)
		{
			HandleWritten(Written, Error, What);
		}), Spectator >= 0 ? TOptional<bool>(Spectator == 1) : TOptional<bool>(), Faction);
}

void UHOLNewGameScreen::SendSlotOpen(const int32 SlotIndex, const bool bOpen)
{
	UHOLManagerClient* Client = bLobby ? GetClient() : nullptr;
	if (!Client)
	{
		return;
	}
	const FString What = FString::Printf(TEXT("slot %d %s"), SlotIndex, bOpen ? TEXT("open") : TEXT("closed"));
	UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
	Client->SetSlot(SlotIndex, bOpen, FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error)
	{
		HandleWritten(Written, Error, What);
	}));
}

#undef LOCTEXT_NAMESPACE
