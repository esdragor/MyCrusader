#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMatchConfig.h"
#include "Net/HOLManagerClientRules.h"
#include "Screens/HOLScreen.h"
#include "HOLNewGameScreen.generated.h"

class UCommonTextBlock;
class UImage;

/**
 * New game screen (D240, D244): the match configuration of a single-player game, read by the game mode when the
 * map opens, every value chosen in a drop-down list. Acting now: the map, the occupied slots and their colours (no two
 * occupied slots share one: a colour held by another slot is greyed in the list), the teams (TC-Q12); greyed (D240): AI,
 * factions, mode and options, shown with their values. Launch fills UHOLMatchSubsystem and opens the map (D243).
 */
UCLASS(Config = Game)
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
	void Launch();

	/**
	 * The network mode (M6-Q3, R4.4): this screen is the lobby the manager client has just entered. Its state is read
	 * twice a second (the presence, Manager/PROTOCOL.md) and shown: the map, the options, each slot's player, colour,
	 * team and state (open, closed, ready or not); the other players' lines are greyed. My colour, team and Ready are
	 * sent (R4.5); the leader also sends the map, its size and "Équipes ensemble" (TC3), the options, open / closed slots
	 * and Launch. Back leaves the lobby; a lobby lost (manager gone, removed) goes back to the list, which says why.
	 */
	void EnterLobby(const FHOLLobby& InLobby);
	bool IsLobby() const { return bLobby; }

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	/** Slot state (with the map and the mode above and below), faction, colour, team (D250). */
	virtual void GetColumns(TArray<TArray<UWidget*>>& OutColumns) const override;

	/** Maps offered by the screen ([/Script/HeroesOfLibertyUI.HOLNewGameScreen] in DefaultGame.ini), shown by their
	 *  name (GetMapName). */
	UPROPERTY(Config)
	TArray<FSoftObjectPath> Maps;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLDropdown> MapDropdown;

	/** TC3 (TC-Q3, TC-Q4): the size of the map, under it; the sizes too small for the players greyed. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> SizeDropdown;

	/** TC3 (TC-Q6): "Équipes ensemble", the allies on neighbouring places when the places are drawn. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> TeamsTogetherDropdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> ModeDropdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> NomadDropdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> RegicideDropdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLDropdown> CatastropheDropdown;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> LaunchButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> BackButton;

	/** My Ready (network mode only, a toggle). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> ReadyButton;

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
	void Refresh();
	/** Asks the manager for the lobby's state (one call at a time). */
	void ReadLobby();
	void ShowLobby(const FHOLLobby& InLobby);
	void HandleMapChosen(int32 Value);
	void HandleSizeChosen(int32 Value);
	void HandleTeamsTogetherChosen(int32 Value);
	/**
	 * TC3: the menu's size (FHOLMapSizeRules::FitSlotsToSize: the slots past its places closed, a player past them moved
	 * below), places drawn at the launch; false (nothing changed) when the players do not fit it.
	 */
	bool ChooseSize(EHOLMapSize Size);
	void HandleStateChosen(int32 SlotIndex, int32 Value);
	void HandleColorChosen(int32 SlotIndex, int32 Value);
	void HandleTeamChosen(int32 SlotIndex, int32 Value);
	void HandleOptionChosen(int32 Value);
	void ToggleReady();
	/** Network mode: the map and options sent (leader). */
	void SendSettings();
	/**
	 * IA0.6 L4 (Q4): network mode, the leader puts an AI on a slot (nothing set), or changes an AI's colour or team
	 * (UHOLManagerClient::SetSlotAI).
	 */
	void SendSlotAI(int32 SlotIndex, TOptional<int32> ColorIndex, TOptional<int32> Team);
	/** IA0.6 L4: network mode, the slot holds an AI in the lobby read last. */
	bool IsLobbyAI(int32 SlotIndex) const;
	/** Network mode: "Lancer" sent (leader); what follows is B6. */
	void StartMatch();
	/** The answer of a change: the state shown, or the refusal in red (the next read shows the truth again). */
	void HandleWritten(const FHOLLobby& Written, const struct FHOLManagerError& Error, const FString& What);
	void ShowError(const FText& Text);
	class UHOLManagerClient* GetClient() const;

	/** A colour held by another occupied slot. */
	bool IsColorTaken(int32 SlotIndex, int32 ColorIndex) const;
	/** First colour from From onwards that no other occupied slot holds (From itself if all are). */
	int32 FindFreeColor(int32 SlotIndex, int32 From) const;

	FHOLMatchConfig Config;

	bool bLobby = false;
	FHOLLobby Lobby;
	FTimerHandle ReadTimer;
	bool bReading = false;
	/** The lobby in_game: opens its server, once (R4.7). */
	void JoinMatch();
	/** The players last logged ("Lobby: 2 players ..."), logged again when it changes. */
	FString LoggedPlayers;
};
