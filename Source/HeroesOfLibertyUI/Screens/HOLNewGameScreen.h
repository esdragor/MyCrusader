#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLNewGameReadLibrary.h"
#include "Net/HOLManagerClientRules.h"
#include "Screens/HOLScreen.h"
#include "HOLNewGameScreen.generated.h"

class UCommonTextBlock;
class UHorizontalBox;
class UHOLButton;
class UHOLDropdown;
class UImage;
class UVerticalBox;

/**
 * New game screen (D240, D244): the match configuration of a single-player game, read by the game mode when the
 * map opens, every value chosen in a drop-down list. Acting now: the map, the occupied slots and their colours (no two
 * occupied slots share one: a colour held by another slot is greyed in the list), the teams (TC-Q12); greyed (D240): AI,
 * factions, mode and options, shown with their values. Launch fills UHOLMatchSubsystem and opens the map (D243).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLNewGameScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** The translated name of a map, or its asset name for a map that has none yet. */
	static FText GetMapName(const FSoftObjectPath& Map);

	/** TC3 (TC-Q8): the translated name of a size with its players, "Petite (2 joueurs)"; "—" for None (a test map). */
	static FText GetSizeName(EHOLMapSize Size);

	/**
	 * TC3: a lobby's map with its size, as the list of lobbies shows it: "Plaine — Grande" (the size's name without its
	 * players); the map alone without a size; empty without a map (an older manager).
	 */
	static FText GetLobbyMapName(const FString& Map, const FString& Size);

	/** Opens the screen over the menu of the given player (Play button, hol.UI.Open NewGame). */
	static UHOLNewGameScreen* Open(APlayerController* PlayerController);

	/** The screen if it is the open menu screen of the given player. */
	static UHOLNewGameScreen* Find(APlayerController* PlayerController);

	/**
	 * Shows a configuration (normalised). A map missing from the list becomes the first one, at the smallest size that
	 * holds its players, places drawn (TC3: the menu's own configuration). A configuration of the list's map without a
	 * size (the test scenarios, quick_check, hol.UI.NewGame.Launch with a file) is launched as it is: that map's own file
	 * (Plain, the test map, TC-Q11), fixed places; its size shows "—" until one is chosen.
	 */
	void SetConfig(const FHOLMatchConfig& InConfig);
	const FHOLMatchConfig& GetConfig() const { return Config; }

	/** Gives the configuration to the match subsystem and opens its map (Launch button, hol.UI.NewGame.Launch). */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void Launch();

	/**
	 * The network mode (M6-Q3, R4.4): this screen is the lobby the manager client has just entered. Its state is read
	 * twice a second (the presence, Manager/PROTOCOL.md) and shown: the map, the options, each slot's player, colour,
	 * team and state (open, closed, ready or not); the other players' lines are greyed. My colour, team and Ready are
	 * sent (R4.5); the leader also sends the map, its size and "Équipes ensemble" (TC3), the options, open / closed slots
	 * and Launch. Back leaves the lobby; a lobby lost (manager gone, removed) goes back to the list, which says why.
	 */
	void EnterLobby(const FHOLLobby& InLobby);
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	bool IsLobby() const { return bLobby; }

	// ---- NG-1a (D851, step 4): what the script's screen (a class deriving from this one) calls; the C++ screen still plays. ----

	/** The script's screen (the newest class deriving from this one, as UHOLSettingsScreen::FindScriptClass); null: none. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static TSubclassOf<UHOLNewGameScreen> FindScriptClass();

	/** The script's layout: called by BuildLayout instead of the C++ one when the object is of a class other than UHOLNewGameScreen itself. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|NewGame")
	void BP_BuildNewGame();

	/** Shows the configuration and the lobby again in the widgets (the lists, the greyed values, the lobby's lines). */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void Refresh();
	/** Network mode: my Ready, a toggle. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void ToggleReady();
	/** SP1: network mode, Spectateur / Jouer. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void ToggleSpectator();
	/** Network mode: the map and options sent (leader). */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void SendSettings();
	/** SendSlotAI for the script: -1 leaves the colour or the team unset, an empty Faction leaves the faction unset. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void SendSlotAIChoice(int32 SlotIndex, int32 ColorIndex, int32 Team, const FString& Faction);
	/** The manager's refusal (or any message) shown in red under the grid; empty text hides it. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void ShowError(const FText& Text);
	/** IA0.6 L4: network mode, the slot holds an AI in the lobby read last. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	bool IsLobbyAI(int32 SlotIndex) const;

	/** The maps offered, as names (GetMapName), in the order of the table. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	TArray<FText> GetMapNames() const;
	/** The values of a list by kind: SlotState (Closed, Human, AI), LobbyState (Open, Closed, NotReady, Ready, AI), Level (the AI's), Faction (6, the Templars last), Team (None, 1..), Color (the colours and "Aléatoire" last), Fog, OffOn, Size (index = EHOLMapSize, "—" first), Mode. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static TArray<FText> GetNewGameOptions(FName Kind);
	/** The size alone ("Petite"; empty for 0 / None), Size = EHOLMapSize as an int. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static FText GetShortSizeNameOf(int32 Size);
	/** The size with its players ("Petite (2 joueurs)"; "—" for 0 / None), Size = EHOLMapSize as an int. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static FText GetSizeNameOf(int32 Size);
	/** The swatches of the colour list ("Aléatoire" last), for UHOLDropdown::SetOptionSwatches. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static TArray<FLinearColor> GetNewGameColorSwatches();
	/** A width of the grid by key: Name, State, Faction, Swatch, Color, Team, Level, Option, Toggle, OptionLabel, TeamsTogetherLabel, MapList (State + Faction + 8), FogList (Team + 60); 0 for another key. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static float GetNewGameWidth(FName Key);

	/** UHOLScreen::MakeCenteredColumn for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	UVerticalBox* MakeNewGameColumn();
	/** A line of the column: style 0 the heading (24 under it), 1 a note (4 under it), 2 the body text (4 under it). */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	UCommonTextBlock* AddNewGameText(UVerticalBox* Column, FName Name, const FText& Text, int32 Style);
	/** A line of the grid, left-aligned, TopPadding above and 4 under it (a line of cells: AddNewGameCellText, AddNewGameDropdown, AddNewGameSwatch). */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	UHorizontalBox* AddNewGameRow(UVerticalBox* Column, float TopPadding);
	/** A text in a cell of the given width (style 1 the note, 2 the body text). */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	UCommonTextBlock* AddNewGameCellText(UHorizontalBox* Row, FName Name, const FText& Text, int32 Style, float Width);
	/** A drop-down list in the row (UHOLScreen::AddDropdown); its choice: UHOLDropdown::OnChosen. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	UHOLDropdown* AddNewGameDropdown(UHorizontalBox* Row, FName Name, const TArray<FText>& Options, int32 SelectedIndex, float Width);
	/** A colour square in a cell of its own width (SlotSwatch<N>). */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	UImage* AddNewGameSwatch(UHorizontalBox* Row, FName Name);
	/** The gamepad hint under the grid (UHOLScreen::AddColumnHint). */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	UCommonTextBlock* AddNewGameHint(UVerticalBox* Column);
	/** UHOLScreen::AddButtonRow for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	UHorizontalBox* AddNewGameButtonRow(UVerticalBox* Column);
	/** A button of the button row (UHOLScreen::AddButton); its clicks: OnButtonBaseClicked / OnClicked(). */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	UHOLButton* AddNewGameButton(UHorizontalBox* Row, FName Name, const FText& Label);

	// ---- NG-1c (D851, step 4): the raccords the script's gestures need (state in, state out, texts); none changes the C++ screen. ----

	/** True in a script class that handles the drop-downs' choices itself (UHOLDropdown::OnChosen): the base then does not subscribe its Handle*Chosen. Set with `default bScriptOwnsGestures = true;`. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HOL|NewGame")
	bool bScriptOwnsGestures = false;

	/** One label of the screen by its LOCTEXT key (Heading, Map, Fog, Size, TeamsTogether, Player, Slot, Level, Faction, Colour, Team, You, Mode, Nomad, Regicide, Catastrophe, SacredSites, GreyedTeamsOpen, LobbyGreyed, Back, LeaveLobby, ReadyAction, NotReadyAction, BecomeSpectator, BecomePlayer, Launch, Starting, NoMatchAddress, TooManyPlayers); empty for another key. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static FText GetNewGameText(FName Key);
	/** "Joueur 1".."Joueur 8" (the PlayerN text of slot SlotIndex, 0-based). */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static FText GetNewGamePlayerName(int32 SlotIndex);

	// ---- NG-1d (D851, step 4): what the script needs to own the lobby display (BP_ShowLobby); the C++ screen is unchanged unless the flag is set. ----

	/** True in a script class that shows the lobby itself: ShowLobby then calls BP_ShowLobby() instead of ShowLobbyWidgets() (the heading, the 8 lines, Ready, Spectate, spectators, Launch). Set with `default bScriptOwnsLobbyDisplay = true;`. The reading, the sends, the Starting state and JoinMatch stay in the base. Ready and Spectate stay wired to ToggleReady / ToggleSpectator by the base: the script must NOT subscribe them. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HOL|NewGame")
	bool bScriptOwnsLobbyDisplay = false;

	/** Called by ShowLobby after Lobby, Config and Refresh() are up to date, when bScriptOwnsLobbyDisplay: GetLobbyView() reads the lobby. In script: `UFUNCTION(BlueprintOverride) void ShowLobby()`. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|NewGame")
	void BP_ShowLobby();

	/** HOLForumMessages::GetAIName: "IA Difficile 3" (Slot 0-based, Level = EHOLAIDifficulty 0 Easy, 1 Normal, 2 Hard, 3 Evolving; clamped). */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static FText GetNewGameAIName(int32 Slot, int32 Level);
	/** "Crown Name" (the leader's crown). */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static FText GetNewGameLeaderName(const FString& Name);
	/** "Spectateurs : A, Crown B" (the leader's name crowned); empty without spectators. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static FText GetNewGameSpectatorsText(const TArray<FString>& Spectators, const FString& Leader);
	/** The name column of the current lobby's line: empty for an AI, "Joueur n" for a slot no player holds, crowned name for the leader, else the name. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	FText GetNewGameLobbyLineName(int32 SlotIndex) const;
	/** The AI's name of the current lobby's line ("IA Difficile 3"); empty when the line is no AI. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	FText GetNewGameLobbyAIName(int32 SlotIndex) const;
	/** The manager's name of a faction by its EHOLFaction value ("DawnOrder"...); empty out of range. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	static FString GetNewGameFactionName(int32 Faction);
	/** The label of a button of the screen by its widget name (LaunchButton, BackButton, ReadyButton, SpectateButton, or a script's own); nothing for an unknown name. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void SetNewGameButtonLabel(FName ButtonName, const FText& Label);

	/** The manager's client exists (the sends do nothing without it). */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	bool HasClient() const;
	/** FHOLLobby predicates on the current lobby; all false in solo. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	bool CanToggleLobbySlot(int32 SlotIndex) const;
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	bool CanEditLobbySlot(int32 SlotIndex) const;
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	bool CanEditLobbyAISlot(int32 SlotIndex) const;
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	bool IsLobbyYouSpectator() const;
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	bool IsLobbyYouLeader() const;
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	bool LobbyHasOpenSlot() const;
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	bool CanStartLobby() const;

	/** The lobby read last, flattened for the script (UHOLNewGameReadLibrary::MakeLobbyView); the default view (bHasMatch false) in solo. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	FHOLNewGameLobbyView GetLobbyView() const;

	/** The configuration shown, as its JSON text (FHOLMatchConfig::ToJson): read it with HOLMatchConfigRead::Describe / HOLNewGameRead::DescribeGlobals. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	FString GetConfigJson() const;
	/** SetConfig(FromJson(Json)): normalised (a colour "Aleatoire" becomes a real one), a map missing from the list replaced; Refresh included. False (nothing changed) when the JSON is unreadable. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	bool SetConfigJson(const FString& Json);

	/** The index in GetMapNames() of the configuration's map; -1 when it is not in the list. */
	UFUNCTION(BlueprintPure, Category = "HOL|NewGame")
	int32 GetConfigMapIndex() const;
	/** Field writes of the configuration, NOT normalised, no Refresh (call Refresh() once the gesture is done). The map by its index in GetMapNames(); out of range = unchanged. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void SetConfigMapIndex(int32 Index);
	/** Lobby: the size as it is (EHOLMapSize as int), unchanged when it is no size of the table. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void SetConfigMapSize(int32 Size);
	/** Solo: ChooseSize (FitSlotsToSize, places drawn); false (nothing changed) when the players do not fit the size. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	bool ChooseNewGameSize(int32 Size);
	/** TeamsTogether, FogMode (EHOLFogMode: 0 Disabled, 1 Explored, 2 Normal = 2 - the list's index), Mode (0 Standard, 1 Diplomacy); both clamped. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void SetConfigGlobals(bool bTeamsTogether, int32 FogMode, int32 Mode);
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void SetConfigOptions(bool bNomad, bool bRegicide, bool bCatastrophe, bool bSacredSites);
	/** One slot: State (EHOLSlotState: 0 Closed, 1 Human, 2 AI), Faction (EHOLFaction), ColorIndex (RandomColor allowed), Team; -1 leaves a field as it is; an index outside 0..7 does nothing. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void SetConfigSlot(int32 SlotIndex, int32 State, int32 Faction, int32 ColorIndex, int32 Team);

	/** Lobby, my line: -1 leaves a field unset (UHOLManagerClient::UpdateMe); Ready and Spectator are 0 / 1; Faction empty = unset. The answer is shown by the base (HandleWritten). Nothing without a lobby. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void SendMyChoice(int32 ColorIndex, int32 Team, int32 Ready, int32 Spectator, const FString& Faction);
	/** Lobby, leader: the slot opened or closed (UHOLManagerClient::SetSlot). Nothing without a lobby. */
	UFUNCTION(BlueprintCallable, Category = "HOL|NewGame")
	void SendSlotOpen(int32 SlotIndex, bool bOpen);

	/** The faction cheat (HOLFactionChoice) switched while the screen is open (hol.Faction.Cheat): the lists show the Templars or not at once. */
	void OnFactionCheatChanged() { Refresh(); }

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	/** Slot state (with the map and the mode above and below), faction, colour, team (D250). */
	virtual void GetColumns(TArray<TArray<UWidget*>>& OutColumns) const override;

	/** Maps offered by the screen: the files of the table (C0.1, FHOLMapRules, Tools/Data/Maps.csv), in its order, shown
	 *  by their name (GetMapName). */
	UPROPERTY(Transient)
	TArray<FSoftObjectPath> Maps;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLDropdown> MapDropdown;

	/** TC3 (TC-Q3, TC-Q4): the size of the map, under it; the sizes too small for the players greyed. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> SizeDropdown;

	/** TC3 (TC-Q6): "Équipes ensemble", the allies on neighbouring places when the places are drawn. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> TeamsTogetherDropdown;

	/** F8 (D522): the fog of the match; the list's index is 2 - EHOLFogMode (Normal first, then "Carte explorée", "Sans brouillard"). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> FogDropdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> ModeDropdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> NomadDropdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> RegicideDropdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> CatastropheDropdown;

	/** S7 (SacredSitesInProgress.md, D699): "Sites sacrés", off by default; in solo too (the sites are in the game). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> SacredSitesDropdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> LaunchButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> BackButton;

	/** My Ready (network mode only, a toggle). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> ReadyButton;

	/** SP1 (D642): network mode only, « Spectateur » leaves my slot, « Jouer » takes the first open one (a toggle). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> SpectateButton;

	/** SP1: the lobby's spectators ("Spectateurs : ♛ Carol, Dave"), empty and hidden without any. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SpectatorsText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> NoteText;

	/** The manager's refusal of my last change, in red (network mode). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> ErrorText;

	/** Per slot, found by name (SlotState1..7, SlotFaction0..7, SlotColor0..7, SlotSwatch0..7, SlotTeam0..7) so that a
	 *  Blueprint layout can give them. Slot 0 is the player: no state widget. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLDropdown>> SlotStates;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLDropdown>> SlotFactions;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLDropdown>> SlotColors;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> SlotSwatches;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLDropdown>> SlotTeams;

	/** "Joueur 1".."Joueur 8" (SlotName0..7); the players' names in the network mode (IA0.6 S2: shown in a lobby only). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonTextBlock>> SlotNames;

	/** IA0.6 S2: an AI slot's level (SlotLevel0..7), Normal and greyed, shown on an AI line in solo only. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLDropdown>> SlotLevels;

	/** IA0.6 S2: the headings the solo and the lobby change: the name column (lobby), the slot column's, the level's (solo). */
	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> HeadName;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> HeadSlot;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> HeadLevel;

	/** Slot 0's state ("Vous", SlotYou0), slot 0 having no state list. */
	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> SlotYou;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> HeadingText;

private:
	/** Asks the manager for the lobby's state (one call at a time). */
	void ReadLobby();
	void ShowLobby(const FHOLLobby& InLobby);
	/** NG-1d: the C++ display of the lobby (heading, lines, Ready, Spectate, spectators, Launch): the part of ShowLobby a script class replaces (bScriptOwnsLobbyDisplay). */
	void ShowLobbyWidgets();
	void HandleMapChosen(int32 Value);
	void HandleSizeChosen(int32 Value);
	void HandleTeamsTogetherChosen(int32 Value);
	/** F8: the fog's list. */
	void HandleFogChosen(int32 Value);
	/** D5: the mode's list (0 Standard, 1 Diplomatie = EHOLMatchMode). */
	void HandleModeChosen(int32 Value);
	/**
	 * TC3: the menu's size (FHOLMapSizeRules::FitSlotsToSize: the slots past its places closed, a player past them moved
	 * below), places drawn at the launch; false (nothing changed) when the players do not fit it.
	 */
	bool ChooseSize(EHOLMapSize Size);
	void HandleStateChosen(int32 SlotIndex, int32 Value);
	void HandleColorChosen(int32 SlotIndex, int32 Value);
	void HandleTeamChosen(int32 SlotIndex, int32 Value);
	/** FactionChoiceInProgress.md: a faction of the list (a greyed one cannot be taken); in a lobby, sent for my line or the AI's. */
	void HandleFactionChosen(int32 SlotIndex, int32 Value);
	/** The faction lists hold five entries, six with the cheat (the Templars last, so the index is the enum's value); the greyed ones greyed. */
	void UpdateFactionLists();
	void HandleOptionChosen(int32 Value);
	/**
	 * IA0.6 L4 (Q4): network mode, the leader puts an AI on a slot (nothing set), or changes an AI's colour or team
	 * (UHOLManagerClient::SetSlotAI).
	 */
	void SendSlotAI(int32 SlotIndex, TOptional<int32> ColorIndex, TOptional<int32> Team, const FString& Faction = FString());
	/** Network mode: "Lancer" sent (leader); what follows is B6. */
	void StartMatch();
	/** The answer of a change: the state shown, or the refusal in red (the next read shows the truth again). */
	void HandleWritten(const FHOLLobby& Written, const struct FHOLManagerError& Error, const FString& What);
	class UHOLManagerClient* GetClient() const;

	/** A colour held by another occupied slot. */
	bool IsColorTaken(int32 SlotIndex, int32 ColorIndex) const;
	/** First colour from From onwards that no other occupied slot holds (From itself if all are). */
	int32 FindFreeColor(int32 SlotIndex, int32 From) const;

	FHOLMatchConfig Config;

	/** The cheat the faction lists were last built for (unset: not built yet). */
	TOptional<bool> FactionListCheat;

	bool bLobby = false;
	FHOLLobby Lobby;
	FTimerHandle ReadTimer;
	bool bReading = false;
	/** The lobby in_game: opens its server, once (R4.7). */
	void JoinMatch();
	/** The players last logged ("Lobby: 2 players ..."), logged again when it changes. */
	FString LoggedPlayers;
	/** FactionChoiceInProgress.md: the factions last logged ("Lobby: factions Templars,DawnOrder"), logged again when they change. */
	FString LoggedFactions;
};
