#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMatchConfig.h"
#include "Screens/HOLScreen.h"
#include "HOLNewGameScreen.generated.h"

class UImage;

/**
 * New game screen (D240, D244): the match configuration of a single-player game, read by the game mode when the
 * map opens, every value chosen in a drop-down list. Acting now: the map, the occupied slots and their colours (no two
 * occupied slots share one: a colour held by another slot is greyed in the list); greyed (D240): AI, factions, teams,
 * mode and options, shown with their values. Launch fills UHOLMatchSubsystem and opens the map (D243).
 */
UCLASS(Config = Game)
class HEROESOFLIBERTYUI_API UHOLNewGameScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** The translated name of a map, or its asset name for a map that has none yet. */
	static FText GetMapName(const FSoftObjectPath& Map);

	/** Opens the screen over the menu of the given player (Play button, hol.UI.Open NewGame). */
	static UHOLNewGameScreen* Open(APlayerController* PlayerController);

	/** The screen if it is the open menu screen of the given player. */
	static UHOLNewGameScreen* Find(APlayerController* PlayerController);

	/** Shows a configuration (normalised; a map missing from the list becomes the first one). */
	void SetConfig(const FHOLMatchConfig& InConfig);
	const FHOLMatchConfig& GetConfig() const { return Config; }

	/** Gives the configuration to the match subsystem and opens its map (Launch button, hol.UI.NewGame.Launch). */
	void Launch();

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	/** Slot state (with the map and the mode above and below), faction, colour, team (D250). */
	virtual void GetColumns(TArray<TArray<UWidget*>>& OutColumns) const override;

	/** Maps offered by the screen ([/Script/HeroesOfLibertyUI.HOLNewGameScreen] in DefaultGame.ini), shown by their
	 *  name (GetMapName). */
	UPROPERTY(Config)
	TArray<FSoftObjectPath> Maps;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLDropdown> MapDropdown;

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

private:
	void Refresh();
	void HandleMapChosen(int32 Value);
	void HandleStateChosen(int32 SlotIndex, int32 Value);
	void HandleColorChosen(int32 SlotIndex, int32 Value);

	/** A colour held by another occupied slot. */
	bool IsColorTaken(int32 SlotIndex, int32 ColorIndex) const;
	/** First colour from From onwards that no other occupied slot holds (From itself if all are). */
	int32 FindFreeColor(int32 SlotIndex, int32 From) const;

	FHOLMatchConfig Config;
};
