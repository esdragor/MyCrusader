#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLEndBanner.generated.h"

enum class EHOLEndBanner : uint8;
enum class EHOLMatchOrigin : uint8;

/**
 * The end banner (D357, V1-Q2), over the match in the menu layer of AHOLGameHUD, the match going on behind: a large
 * « Victoire » or « Défaite » and two buttons. « Continuer à regarder » (« Continuer » for a winner) or Back close it,
 * the player then watching as a spectator; « Quitter vers le menu » leaves the match for MainMenu. Which banner and
 * when: HOLGameMenuRules::GetEndBanner and ShouldOpenEndBanner, asked by the HUD at each change of AHOLGameState.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLEndBanner : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** Opens the banner of Result over the match, the in-game menu closed first; null when no match layout. */
	static UHOLEndBanner* Open(APlayerController* PlayerController, EHOLEndBanner Result);

	EHOLEndBanner GetResult() const { return Result; }
	/** The result changed while the banner shows (e.g. Defeat, then the team's Victory): its title and labels follow. */
	void SetResult(EHOLEndBanner InResult);
	/** The line under the title (the spectator's winner, M6-Q10); empty hides it. */
	void SetDetail(const FText& InDetail);

	/** « Continuer à regarder » / « Continuer »: closes the banner, the match watched (D357). */
	void Continue();
	/** « Quitter vers le menu »: leaves the match (and its network game) for the MainMenu map. */
	void QuitToMenu();
	/** « Retour au salon » (match from a lobby, R4.8): leaves the match, the menu then joins the lobby again. */
	void ReturnToLobby();
	/** « Quitter vers la liste » (match from the manager, a lobby's or watched): leaves it for the Multiplayer screen. */
	void QuitToList();
	/** The way out of the match for the MainMenu map, shared with the in-game menu's « Quitter vers le menu ». */
	static void LeaveMatch(APlayerController* PlayerController);

	/** The player closed the banner of this result (« Continuer », Back): the HUD does not show it again. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnClosedByPlayer, EHOLEndBanner);
	FOnClosedByPlayer OnClosedByPlayer;

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual bool NativeOnHandleBackAction() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> Title;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UCommonTextBlock> Detail;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> ContinueButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> QuitButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> ReturnButton;

	/** « Statistiques » (C4): the statistics screen over the banner, for everyone. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> StatsButton;

private:
	/** « Retour au salon », « Quitter vers la liste » / « vers le menu »: from the result and the match's origin. */
	void RefreshWayBack();

	EHOLEndBanner Result;
	/** Where the match was opened from (UHOLManagerClient::GetMatchOrigin) when the banner opened. */
	EHOLMatchOrigin Origin {};
};
