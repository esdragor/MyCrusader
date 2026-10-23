#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HOLGameHUD.generated.h"

class AHOLGameState;
class AHOLPlayerController;
class UHOLEndBanner;
enum class EHOLEndBanner : uint8;
class UFont;
class UHOLCheatPanel;
class UHOLGameScreen;
class UHOLRootLayout;
class UHOLScenarioPanel;

/**
 * HUD of a match (J3, D281): puts the game screen on the viewport of its local player; draws itself only the box the
 * player drags to select units (J4, D288) and the « +10 » rising above the player's Peasants as they unload at a
 * deposit (J5c.3, D298: its own unloads only, the server sends no other). In a development build, also the cheats'
 * panel under the top bar (D291) and the test scenario's panel at the top centre (hol.Scenario). Over them all, the
 * layers of the in-game menu (D356), which Escape with nothing selected opens (AHOLPlayerController::OnGameMenuRequested).
 * It lives in the interface module, which a dedicated server neither builds nor loads (D247): the match names it by
 * its class path, /Script/HeroesOfLibertyUI.HOLGameHUD, and only a player's machine ever makes one.
 */
UCLASS()
class HEROESOFLIBERTYUI_API AHOLGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	UHOLGameScreen* GetScreen() const { return Screen; }
	/** The layers over the game screen (D356): the in-game menu, its confirmations. */
	UHOLRootLayout* GetMenuLayout() const { return MenuLayout; }
	/** The cheats' panel (D291), in a development build only; null in Shipping. */
	UHOLCheatPanel* GetCheatPanel() const { return CheatPanel; }
	/** The test scenario's panel, in a development build only; null in Shipping. */
	UHOLScenarioPanel* GetScenarioPanel() const { return ScenarioPanel; }

	/** The game screen of a local player, if its HUD is this one. */
	static UHOLGameScreen* FindScreen(const APlayerController* PlayerController);
	static UHOLCheatPanel* FindCheatPanel(const APlayerController* PlayerController);
	static UHOLScenarioPanel* FindScenarioPanel(const APlayerController* PlayerController);

	virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** The player's « +10 » still showing: the resource's marker and the amount, rising and fading (J5c.3). */
	void DrawDepositPopups(const AHOLPlayerController& PlayerController);
	void DrawSelectionBox(const AHOLPlayerController& PlayerController);
	/**
	 * Listens to the match's end (AHOLGameState::OnVictoryChanged, D357); retried until the game state and the player's
	 * slot are known on this machine (a client), then the end banner is checked at once (a state received all at once).
	 */
	void WatchVictory();
	/** Opens, updates or leaves the end banner (HOLGameMenuRules::GetEndBanner, ShouldOpenEndBanner; V1-Q2). */
	void RefreshEndBanner();

	UPROPERTY(Transient)
	TObjectPtr<UHOLGameScreen> Screen;

	UPROPERTY(Transient)
	TObjectPtr<UHOLCheatPanel> CheatPanel;

	UPROPERTY(Transient)
	TObjectPtr<UHOLRootLayout> MenuLayout;

	/** The test scenario's panel, in a development build only. */
	UPROPERTY(Transient)
	TObjectPtr<UHOLScenarioPanel> ScenarioPanel;

	UPROPERTY(Transient)
	TObjectPtr<UHOLEndBanner> EndBanner;

	TWeakObjectPtr<AHOLGameState> WatchedGameState;
	FTimerHandle WatchVictoryTimer;
	/** The last banner the player closed by « Continuer à regarder » (EHOLEndBanner::None: none). */
	EHOLEndBanner ClosedEndBanner;

	/** The interface font (F_HOL), for the « +10 ». */
	UPROPERTY(Transient)
	TObjectPtr<UFont> PopupFont;
};
