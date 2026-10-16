#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HOLGameHUD.generated.h"

class AHOLPlayerController;
class UFont;
class UHOLGameScreen;

/**
 * HUD of a match (J3, D281): puts the game screen on the viewport of its local player; draws itself only the box the
 * player drags to select units (J4, D288) and the « +10 » rising above the player's Peasants as they unload at a
 * deposit (J5c.3, D298: its own unloads only, the server sends no other).
 * It lives in the interface module, which a dedicated server neither builds nor loads (D247): the match names it by
 * its class path, /Script/HeroesOfLibertyUI.HOLGameHUD, and only a player's machine ever makes one.
 */
UCLASS()
class HEROESOFLIBERTYUI_API AHOLGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	UHOLGameScreen* GetScreen() const { return Screen; }

	/** The game screen of a local player, if its HUD is this one. */
	static UHOLGameScreen* FindScreen(const APlayerController* PlayerController);

	virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** The player's « +10 » still showing: the resource's marker and the amount, rising and fading (J5c.3). */
	void DrawDepositPopups(const AHOLPlayerController& PlayerController);
	void DrawSelectionBox(const AHOLPlayerController& PlayerController);

	UPROPERTY(Transient)
	TObjectPtr<UHOLGameScreen> Screen;

	/** The interface font (F_HOL), for the « +10 ». */
	UPROPERTY(Transient)
	TObjectPtr<UFont> PopupFont;
};
