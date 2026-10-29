#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLGameMenu.generated.h"

/**
 * The in-game menu (D356, V1-Q1), over the match in the menu layer of AHOLGameHUD: Escape with nothing to undo or the
 * top bar's « Menu » opens it, Back (Escape) and « Reprendre » close it. The match never pauses (a networked RTS): only
 * this player's input goes to the menu. « Abandonner » asks for a confirmation in the modal layer, then surrenders
 * (AHOLPlayerController::RequestSurrender); absent for a defeated player or a match over (D357). Once the match is over,
 * the end banner's ways out (M6-Q7 revised: nobody is sent back, everyone leaves when he chooses): « Retour au salon »
 * for a lobby's player, « Quitter vers la liste » for a match from the manager, else « Quitter vers le menu », and
 * « Résultat » to open the closed banner again. Settings come later.
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

	/**
	 * P5 (D513): « Pause » (« Pause (2) » in a networked match: the pauses left, shared by all), for a player still in
	 * the match while it runs; greyed with none left. The server decides (AHOLPlayerController::RequestPause).
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> PauseButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> SurrenderButton;

	/** « Quitter vers le menu » (« vers la liste » from the manager), in the place of « Abandonner » for a spectator or a match over (V1 trial). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> QuitButton;

	/** « Retour au salon », a lobby's player once the match is over (HOLGameMenuRules::OffersReturnToLobby). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> ReturnButton;

	/** « Résultat »: the end banner again, once the match is over (closed by « Continuer à regarder »). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> ResultButton;

private:
	void HandleSurrender();
	void HandlePause();
	/** « Pause » shown, its count and greyed state (RefreshSurrender's tick). */
	void RefreshPause();
	void HandleQuitToMenu();
	void HandleReturnToLobby();
	void HandleShowResult();
	/** Whether the player is defeated and the match over; false (nothing offered) without a game or player state. */
	bool GetMatchState(bool& bDefeated, bool& bMatchOver) const;
	/** « Abandonner » or the ways out, whichever are offered. */
	void RefreshSurrender();
	/** Tells the controller the menu is open or closed, gives Back its key while open (IMC_UI). */
	void SetOpen(bool bOpen);
};
