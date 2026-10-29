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
	const FText MapName = GetMapName(FSoftObjectPath(FString::Printf(TEXT("/Game/Maps/%s.%s"), *Map, *Map)));
	const FText SizeName = UE::HeroesOfLiberty::UI::NewGame::GetShortSizeName(FHOLMapSizeRules::FindByKey(Size));
	return SizeName.IsEmpty() ? MapName : FText::Format(LOCTEXT("LobbyMapAndSize", "{0} — {1}"), MapName, SizeName);
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

	// TC3: the size under the map, then "Équipes ensemble" (the places are drawn at the launch).
	UHorizontalBox* SizeLine = AddLine(0.f);
	AddCell(SizeLine, MakeText(LOCTEXT("Size", "Taille"), UHOLTextStyleBody::StaticClass()), NameWidth);
	SizeDropdown = AddDropdown(SizeLine, TEXT("SizeDropdown"), SizeNames(), 0, StateWidth + FactionWidth + 8.f);
	AddCell(SizeLine, MakeText(LOCTEXT("TeamsTogether", "Équipes ensemble"), UHOLTextStyleBody::StaticClass()), TeamsTogetherLabelWidth);
	TeamsTogetherDropdown = AddDropdown(SizeLine, TEXT("TeamsTogetherDropdown"), OffOn(), 1, TeamWidth);

	UHorizontalBox* Headings = AddLine(16.f);
	// IA0.6 S2 (Q1): in solo one "Joueur" column, its list showing the name the slot will have, then the AI's level; the
	// lobby keeps its name column and "Emplacement" (Refresh shows the one or the other).
	AddCell(Headings, MakeText(LOCTEXT("Player", "Joueur"), UHOLTextStyleNote::StaticClass(), TEXT("HeadName")), NameWidth);
	AddCell(Headings, MakeText(LOCTEXT("Player", "Joueur"), UHOLTextStyleNote::StaticClass(), TEXT("HeadSlot")), StateWidth);
	AddCell(Headings, MakeText(LOCTEXT("Level", "Niveau"), UHOLTextStyleNote::StaticClass(), TEXT("HeadLevel")), LevelWidth);
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
		AddDropdown(Row, SlotWidgetName(TEXT("SlotLevel"), SlotIndex), LevelNames(), static_cast<int32>(EHOLAIDifficulty::Normal), LevelWidth);
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

	NoteText = AddText(Column, TEXT("Note"), LOCTEXT("GreyedTeamsOpen", "Grisé : pas encore dans le jeu (factions, mode et options)."),
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

		if (SlotStates[SlotIndex])
		{
			// IA0.6: "IA" in solo, and in the lobby for its leader (its own list, LobbyStateNames, L4).
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
		// Greyed (D240): factions are not in the game yet. TC-Q12: the teams are (camps of the combat, FHOLCampTable;
		// victory by team), chosen in solo too.
		if (SlotFactions[SlotIndex])
		{
			SlotFactions[SlotIndex]->SetIsEnabled(false);
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
	if (SizeDropdown)
	{
		SizeDropdown->OnSelectionChanged.AddUObject(this, &UHOLNewGameScreen::HandleSizeChosen);
	}
	if (TeamsTogetherDropdown)
	{
		TeamsTogetherDropdown->OnSelectionChanged.AddUObject(this, &UHOLNewGameScreen::HandleTeamsTogetherChosen);
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
	// TC3 loop 2: the lobby's size and "Équipes ensemble", the places drawn by the server at the launch.
	Config.MapSize = Lobby.GetMapSize();
	Config.bTeamsTogether = Lobby.bTeamsTogether;
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
			SlotConfig.Faction = Row->Faction == TEXT("BlackLegions") ? EHOLFaction::BlackLegions : EHOLFaction::DawnOrder;
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

	if (HeadingText)
	{
		HeadingText->SetText(FText::FromString(Lobby.Name));
	}
	const int32 Yours = Lobby.FindYourSlot();
	for (int32 SlotIndex = 0; SlotIndex < FHOLMatchConfig::MaxSlots; ++SlotIndex)
	{
		const FHOLLobbySlot* Row = Lobby.Slots.IsValidIndex(SlotIndex) ? &Lobby.Slots[SlotIndex] : nullptr;
		const bool bPlayer = Row && Row->State == EHOLLobbySlotState::Player;
		const bool bAI = Row && Row->State == EHOLLobbySlotState::AI;
		const bool bMine = SlotIndex == Yours;
		if (SlotNames[SlotIndex])
		{
			// IA0.6 L4 (Q5): an AI's name is in its state list ("IA Normal 3"), as in solo.
			SlotNames[SlotIndex]->SetText(bAI ? FText::GetEmpty() : !bPlayer ? FText::Format(LOCTEXT("PlayerN", "Joueur {0}"), SlotIndex + 1)
				: Row->PlayerName == Lobby.Leader ? FText::Format(LOCTEXT("Leader", "♛ {0}"), FText::FromString(Row->PlayerName))
				: FText::FromString(Row->PlayerName));
		}
		// Open, closed, not ready, ready, AI (LobbyStateNames).
		const int32 State = !Row || Row->State == EHOLLobbySlotState::Open ? 0 : bAI ? LobbyAIState : !bPlayer ? 1 : Row->bReady ? 3 : 2;
		const FText AIName = bAI ? HOLForumMessages::GetAIName(SlotIndex, Row->Level) : FText::GetEmpty();
		// The leader opens and closes the slots no player holds, or puts an AI there (IA0.6 L4, Q4): Open, Closed and AI.
		if (SlotStates[SlotIndex])
		{
			SlotStates[SlotIndex]->SetSelectedIndex(State);
			SlotStates[SlotIndex]->SetIsEnabled(Lobby.CanToggleSlot(SlotIndex));
			SlotStates[SlotIndex]->SetOptionEnabled(2, false);
			SlotStates[SlotIndex]->SetOptionEnabled(3, false);
			if (bAI)
			{
				SlotStates[SlotIndex]->SetShownText(AIName);
			}
		}
		else if (SlotIndex == 0 && SlotYou)
		{
			SlotYou->SetText(bAI ? AIName : LobbyStateNames()[State]);
		}
		// Only my line is mine (M6-Q4), and an AI's is the leader's (IA0.6 L4); the faction is not changeable yet.
		const bool bEditable = bMine || Lobby.CanEditAISlot(SlotIndex);
		if (SlotColors[SlotIndex])
		{
			SlotColors[SlotIndex]->SetIsEnabled(bEditable);
		}
		if (SlotTeams[SlotIndex])
		{
			SlotTeams[SlotIndex]->SetIsEnabled(bEditable);
		}
	}
	// The map, its size, "Équipes ensemble" and the options are the leader's; the mode has one value yet.
	const bool bLeader = Lobby.IsYouLeader();
	for (UHOLDropdown* Setting : { MapDropdown.Get(), SizeDropdown.Get(), TeamsTogetherDropdown.Get(), NomadDropdown.Get(), RegicideDropdown.Get(),
		CatastropheDropdown.Get() })
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
		// IA0.6 L4: an AI "a<colour>" (no Ready).
		SlotsShown.Add(Row.State == EHOLLobbySlotState::Open ? FString(TEXT("o")) : Row.State == EHOLLobbySlotState::Closed ? FString(TEXT("x"))
			: Row.State == EHOLLobbySlotState::AI ? FString::Printf(TEXT("a%d"), Row.ColorIndex)
			: FString::Printf(TEXT("c%d%s"), Row.ColorIndex, Row.bReady ? TEXT("r") : TEXT("")));
	}
	const FString Logged = FString::Printf(TEXT("%d players (%s), you %s, leader %s, %d/%d ready, launch %s, slots %s"), Players.Num(),
		*FString::Join(Players, TEXT(", ")), *Lobby.You, *Lobby.Leader, NumReady, Players.Num(), bLeader ? TEXT("shown") : TEXT("hidden"),
		*FString::Join(SlotsShown, TEXT(" ")));
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
	// "Équipes ensemble" stands over the team column.
	TArray<UWidget*>& Teams = OutColumns.AddDefaulted_GetRef();
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
	// TC3 (TC-Q4): the slots past the places of the size are closed, not changeable (the lobby sets its own, loop 2).
	const int32 NumPlaces = FHOLMapSizeRules::GetNumPlaces(Config);
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
	// TC3: a map chosen in the list is played at a size (a test configuration had none).
	if (!bLobby && Config.MapSize == EHOLMapSize::None)
	{
		ChooseSize(FHOLMapSizeRules::GetDefaultSize(Config.GetNumOccupiedSlots()));
	}
	Refresh();
	if (bLobby)
	{
		SendSettings();
	}
}

void UHOLNewGameScreen::HandleSizeChosen(const int32 Value)
{
	// A size too small is greyed in the list: this one holds the players.
	if (!bLobby)
	{
		ChooseSize(static_cast<EHOLMapSize>(Value));
	}
	else if (FHOLMapSizeRules::Find(static_cast<EHOLMapSize>(Value)))
	{
		// TC3 loop 2: the leader's size, sent; the manager closes the slots past it (and moves the players below).
		Config.MapSize = static_cast<EHOLMapSize>(Value);
		SendSettings();
	}
	Refresh();
}

void UHOLNewGameScreen::HandleTeamsTogetherChosen(const int32 Value)
{
	Config.bTeamsTogether = Value == 1;
	if (bLobby)
	{
		SendSettings();
	}
	Refresh();
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
		// TC3: the size by its key (none for a map without sizes), "Équipes ensemble".
		const FHOLMapSizeInfo* Size = FHOLMapSizeRules::Find(Config.MapSize);
		const FString SizeKey = Size ? FString(Size->Key) : FString();
		const FString What = FString::Printf(TEXT("settings %s size %s teams together %d nomad %d regicide %d catastrophe %d"), *Config.Map.GetAssetName(),
			*SizeKey, Config.bTeamsTogether, Config.bNomad, Config.bRegicide, Config.bCatastrophe);
		UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
		Client->UpdateSettings(Config.Map.GetAssetName(), Config.bNomad, Config.bRegicide, Config.bCatastrophe,
			FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error) { HandleWritten(Written, Error, What); }),
			SizeKey, Config.bTeamsTogether);
	}
}

void UHOLNewGameScreen::SendSlotAI(const int32 SlotIndex, const TOptional<int32> ColorIndex, const TOptional<int32> Team)
{
	UHOLManagerClient* Client = GetClient();
	if (!Client)
	{
		return;
	}
	// IA0.6 L4 (Q4): "slot 2 ai" puts the AI, "ai 2 colour 5" and "ai 2 team 2" change it.
	const FString What = ColorIndex.IsSet() ? FString::Printf(TEXT("ai %d colour %d"), SlotIndex, ColorIndex.GetValue())
		: Team.IsSet() ? FString::Printf(TEXT("ai %d team %d"), SlotIndex, Team.GetValue()) : FString::Printf(TEXT("slot %d ai"), SlotIndex);
	UE_LOG(LogHOLUI, Display, TEXT("Lobby: %s sent"), *What);
	Client->SetSlotAI(SlotIndex, ColorIndex, Team, FHOLManagerLobbyDone::CreateWeakLambda(this, [this, What](const FHOLLobby& Written, const FHOLManagerError& Error)
	{
		HandleWritten(Written, Error, What);
	}));
}

bool UHOLNewGameScreen::IsLobbyAI(const int32 SlotIndex) const
{
	return bLobby && Lobby.Slots.IsValidIndex(SlotIndex) && Lobby.Slots[SlotIndex].State == EHOLLobbySlotState::AI;
}

void UHOLNewGameScreen::HandleTeamChosen(const int32 SlotIndex, const int32 Value)
{
	Config.Slots[SlotIndex].Team = Value;
	if (IsLobbyAI(SlotIndex))
	{
		SendSlotAI(SlotIndex, {}, Value);
		return;
	}
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
		// Open (0) or closed (1), or an AI (IA0.6 L4), the leader's (LobbyStateNames); the next read puts back the manager's value.
		if (Value == UE::HeroesOfLiberty::UI::NewGame::LobbyAIState)
		{
			SendSlotAI(SlotIndex, {}, {});
			return;
		}
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
	// TC3 (TC-Q4): a slot past the places of the size stays closed (its list is greyed).
	const int32 NumPlaces = FHOLMapSizeRules::GetNumPlaces(Config);
	if (SlotIndex >= NumPlaces)
	{
		UE_LOG(LogHOLUI, Display, TEXT("New game: slot %d stays closed, past the %d places of the size"), SlotIndex, NumPlaces);
		Refresh();
		return;
	}
	// Closed, Human or AI (IA0.6): an AI holds its slot like a human (colour, team).
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
	if (IsLobbyAI(SlotIndex))
	{
		SendSlotAI(SlotIndex, Value, {});
		return;
	}
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

#undef LOCTEXT_NAMESPACE
