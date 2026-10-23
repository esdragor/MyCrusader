#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLGameMenu.generated.h"

/**
 * The in-game menu (D356, V1-Q1), over the match in the menu layer of AHOLGameHUD: Escape with nothing to undo or the
 * top bar's « Menu » opens it, Back (Escape) and « Reprendre » close it. The match never pauses (a networked RTS): only
 * this player's input goes to the menu. « Abandonner » asks for a confirmation in the modal layer, then surrenders
 * (AHOLPlayerController::RequestSurrender); absent for a defeated player or a match over (D357). Settings come later.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLGameMenu : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** Opens the menu over the match of the given player, or gives the one already open (Escape, « Menu », hol.UI.Open GameMenu). */
	static UHOLGameMenu* Open(APlayerController* PlayerController);
	static UHOLGameMenu* Find(const APlayerController* PlayerController);
	/** Opens the surrender confirmation over the menu (« Abandonner », hol.UI.Open Surrender); false when not offered. */
	static bool AskToSurrender(APlayerController* PlayerController);

	/** « Abandonner » is offered: the player still standing, the match not over. */
	bool CanSurrender() const;

	/**
	 * A screen over the match opens or closes (this menu, the end banner): tells the controller (Escape then goes to
	 * Back), gives Back its key while open (IMC_UI). Closed, both stay while another match screen is still open in the
	 * menu layer (V1.4a: the menu closed over the end banner, Escape still closes the banner).
	 */
	static void SetMatchMenuInput(APlayerController* PlayerController, bool bOpen, const UCommonActivatableWidget* Screen);

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> Title;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> ResumeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> SurrenderButton;

	/** « Quitter vers le menu », in the place of « Abandonner » for a spectator or a match over (V1 trial). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> QuitButton;

private:
	void HandleSurrender();
	void HandleQuitToMenu();
	/** Whether the player is defeated and the match over; false (nothing offered) without a game or player state. */
	bool GetMatchState(bool& bDefeated, bool& bMatchOver) const;
	/** « Abandonner » or « Quitter vers le menu », whichever is offered. */
	void RefreshSurrender();
	/** Tells the controller the menu is open or closed, gives Back its key while open (IMC_UI). */
	void SetOpen(bool bOpen);
};
