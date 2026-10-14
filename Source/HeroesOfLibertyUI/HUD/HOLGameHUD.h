#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HOLGameHUD.generated.h"

class UHOLGameScreen;

/**
 * HUD of a match (J3, D281): puts the game screen on the viewport of its local player; draws itself only the box the
 * player drags to select units (J4, D288).
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
	UPROPERTY(Transient)
	TObjectPtr<UHOLGameScreen> Screen;
};
