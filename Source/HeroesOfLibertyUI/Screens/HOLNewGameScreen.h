#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMatchConfig.h"
#include "Screens/HOLScreen.h"
#include "HOLNewGameScreen.generated.h"

class UImage;

/**
 * New game screen (D240, D244): the match configuration of a single-player game, read by the game mode when the
 * map opens. Acting now: the map, the occupied slots and their colours (no two occupied slots share one); greyed
 * (D240): AI, factions, teams, mode and options, shown with their values. Launch fills UHOLMatchSubsystem and
 * opens the map (D243).
 */
UCLASS(Config = Game)
class HEROESOFLIBERTYUI_API UHOLNewGameScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
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

	/** Maps offered by the screen ([/Script/HeroesOfLibertyUI.HOLNewGameScreen] in DefaultGame.ini). */
	UPROPERTY(Config)
	TArray<FSoftObjectPath> Maps;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLRotator> MapRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> ModeRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> NomadRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> RegicideRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> CatastropheRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> LaunchButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> BackButton;

	/** Per slot, found by name (SlotState1..7, SlotFaction0..7, SlotColor0..7, SlotSwatch0..7, SlotTeam0..7) so that a
	 *  Blueprint layout can give them. Slot 0 is the player: no state widget. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLRotator>> SlotStates;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLRotator>> SlotFactions;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLRotator>> SlotColors;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> SlotSwatches;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLRotator>> SlotTeams;

private:
	void Refresh();
	void HandleMapRotated(int32 Value);
	void HandleStateRotated(int32 SlotIndex, int32 Value);
	void HandleColorRotated(int32 SlotIndex, int32 Value);

	/** A colour held by another occupied slot. */
	bool IsColorTaken(int32 SlotIndex, int32 ColorIndex) const;
	/** First colour from From, going by Step, that no other occupied slot holds (From itself if all are). */
	int32 FindFreeColor(int32 SlotIndex, int32 From, int32 Step) const;

	FHOLMatchConfig Config;
};
