#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLEndBannerScreen.generated.h"

class UCommonTextBlock;
class UHorizontalBox;
class UHOLButton;
class UVerticalBox;
enum class EHOLEndBanner : uint8;
enum class EHOLMatchOrigin : uint8;

/**
 * The end banner (D357, V1-Q2), over the match in the menu layer of AHOLGameHUD, the match going on behind: a large
 * « Victoire » or « Défaite » and its buttons. Its layout, buttons and refresh are written in script (a class deriving
 * from this one, FindScriptClass: Script/UI/HOLEndBannerScreen.as, over HOLEndBannerScript::Make; MIG-CPP bascule
 * EndBanner, D830/D835, as UHOLGameMenuScreen); this base keeps what the script cannot reach: the opening (the root
 * layout, the in-game menu giving way), the controller's and manager's calls, the way out of the match, the delegate the
 * HUD listens to and the input of the screens over the match. « Continuer (à regarder) » or Back close it, the player
 * then watching as a spectator. Which banner and when: UHOLGameMenuRulesBridge::GetEndBanner and ShouldOpenEndBanner, asked by
 * the HUD at each change of AHOLGameState.
 */
UCLASS(Abstract, Blueprintable)
class HEROESOFLIBERTYUI_API UHOLEndBannerScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** Opens the banner of Result over the match, the in-game menu closed first; null when no match layout or no script class. */
	static UHOLEndBannerScreen* Open(APlayerController* PlayerController, EHOLEndBanner Result);

	EHOLEndBanner GetResult() const { return Result; }
	/** The result changed while the banner shows (e.g. Defeat, then the team's Victory): the script's view follows. */
	void SetResult(EHOLEndBanner InResult);
	/** The line under the title (the spectator's winner, M6-Q10); empty hides it. */
	void SetDetail(const FText& InDetail);

	/** The way out of the match for the MainMenu map, shared with the in-game menu's « Quitter vers le menu ». */
	static void LeaveMatch(APlayerController* PlayerController);

	/** The script's banner (the newest class deriving from this one, as UHOLGameMenuScreen::FindScriptClass); null: none. */
	UFUNCTION(BlueprintPure, Category = "HOL|EndBanner")
	static TSubclassOf<UHOLEndBannerScreen> FindScriptClass();

	/** The result shown, EHOLEndBanner's value (0 None, 1 Defeat, 2 Victory, 3 MatchOver: the script's EHOLGameMenuBanner). */
	UFUNCTION(BlueprintPure, Category = "HOL|EndBanner")
	int32 GetResultValue() const;

	/** Where the match was opened from, EHOLMatchOrigin's value (0 Local, 1 Manager, 2 Lobby), read when the banner opened. */
	UFUNCTION(BlueprintPure, Category = "HOL|EndBanner")
	int32 GetOriginValue() const;

	/** UHOLScreen::MakeDialogColumn for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|EndBanner")
	UVerticalBox* MakeBannerColumn();

	/** A line of the column: the title (title style, 24; bTitle) or the detail (body, 16), UHOLScreen::AddText for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|EndBanner")
	UCommonTextBlock* AddBannerText(UVerticalBox* Column, FName Name, const FText& Text, bool bTitle);

	/** UHOLScreen::AddButtonRow for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|EndBanner")
	UHorizontalBox* AddBannerRow(UVerticalBox* Column);

	/** UHOLScreen::AddButton for the script; its clicks: OnButtonBaseClicked. */
	UFUNCTION(BlueprintCallable, Category = "HOL|EndBanner")
	UHOLButton* AddBannerButton(UHorizontalBox* Row, FName Name, const FText& Label);

	/** « Continuer à regarder » / « Continuer »: closes the banner, the match watched (D357). */
	UFUNCTION(BlueprintCallable, Category = "HOL|EndBanner")
	void Continue();
	/** « Quitter vers le menu »: leaves the match (and its network game) for the MainMenu map. */
	UFUNCTION(BlueprintCallable, Category = "HOL|EndBanner")
	void QuitToMenu();
	/** « Retour au salon » (match from a lobby, R4.8): leaves the match, the menu then joins the lobby again. */
	UFUNCTION(BlueprintCallable, Category = "HOL|EndBanner")
	void ReturnToLobby();
	/** « Quitter vers la liste » (match from the manager, a lobby's or watched): leaves it for the Multiplayer screen. */
	UFUNCTION(BlueprintCallable, Category = "HOL|EndBanner")
	void QuitToList();
	/** « Statistiques » (C4): the statistics screen over the banner. */
	UFUNCTION(BlueprintCallable, Category = "HOL|EndBanner")
	void ShowStats();

	/** The title, set by the script (BuildBanner). */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|EndBanner")
	TObjectPtr<UCommonTextBlock> Title;

	/** The line under the title, set by the script; SetDetail shows it. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|EndBanner")
	TObjectPtr<UCommonTextBlock> Detail;

	/** The widget focused on opening (« Continuer »), set by the script. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|EndBanner")
	TObjectPtr<UWidget> FocusTarget;

	/** The player closed the banner of this result (« Continuer », Back): the HUD does not show it again. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnClosedByPlayer, EHOLEndBanner);
	FOnClosedByPlayer OnClosedByPlayer;

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual bool NativeOnHandleBackAction() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	/** The script builds the banner's column (in place of BuildLayout). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|EndBanner")
	void BP_BuildBanner();

	/** The script shows the view of the result and the origin (on opening and at each SetResult). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|EndBanner")
	void BP_RefreshBanner();

private:
	EHOLEndBanner Result {};
	/** Where the match was opened from (UHOLManagerClient::GetMatchOrigin) when the banner opened. */
	EHOLMatchOrigin Origin {};
};
