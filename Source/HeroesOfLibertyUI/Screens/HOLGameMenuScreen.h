#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLGameMenuScreen.generated.h"

class UCommonTextBlock;
class UHOLButton;
class UVerticalBox;

/**
 * What the in-game menu reads of the match (MIG-CPP bascule GameMenu, D830/D835): the script makes its view from it
 * (HOLGameMenuScript::Make, Script/UI/HOLGameMenuScript.as), as UHOLGameMenu's RefreshSurrender and RefreshPause did.
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTYUI_API FHOLGameMenuFacts
{
	GENERATED_BODY()

	/** False without a game or player state: « Reprendre » alone. */
	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	bool bKnown = false;

	/** The player's slot; -1: a spectator. */
	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	int32 MySlot = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	bool bDefeated = false;

	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	bool bMatchOver = false;

	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	bool bPaused = false;

	/** Solo: no count of pauses (AHOLGameState::IsPauseFree). */
	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	bool bPauseFree = false;

	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	int32 PausesLeft = 0;

	/** UHOLPauseLibrary::CanPauseNow for this slot. */
	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	bool bCanPauseNow = false;

	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	int32 DefeatedMask = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	int32 WinnerMask = 0;

	/** EHOLMatchOrigin's value (0 Local, 1 Manager, 2 Lobby: the script's EHOLGameMenuOrigin). */
	UPROPERTY(BlueprintReadOnly, Category = "HOL|GameMenu")
	int32 Origin = 0;
};

/**
 * The in-game menu (D356, V1-Q1), over the match in the menu layer of AHOLGameHUD; its layout, its buttons and their
 * refresh are written in script (a class deriving from this one, FindScriptClass: Script/UI/HOLGameMenuScreen.as), this
 * base keeps what the script cannot reach: the opening (the root layout), the surrender confirmation (a C++ callback),
 * the controller's calls and the input of the screens over the match (IMC_UI). Escape with nothing to undo or the top
 * bar's « Menu » opens it, Back (Escape) and « Reprendre » close it; the match never pauses (a networked RTS).
 */
UCLASS(Abstract, Blueprintable)
class HEROESOFLIBERTYUI_API UHOLGameMenuScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** Opens the menu over the match of the given player, or gives the one already open (Escape, « Menu », hol.UI.Open GameMenu). */
	static UHOLGameMenuScreen* Open(APlayerController* PlayerController);
	static UHOLGameMenuScreen* Find(const APlayerController* PlayerController);
	/** Opens the surrender confirmation over the menu (« Abandonner », hol.UI.Open Surrender); false when not offered. */
	static bool AskToSurrender(APlayerController* PlayerController);

	/**
	 * A screen over the match opens or closes (this menu, the end banner, the statistics): tells the controller (Escape
	 * then goes to Back), gives Back its key while open (IMC_UI). Closed, both stay while another match screen is still
	 * open in the menu layer (V1.4a: the menu closed over the end banner, Escape still closes the banner).
	 */
	static void SetMatchMenuInput(APlayerController* PlayerController, bool bOpening, const UCommonActivatableWidget* Screen);

	/** The script's menu (the newest class deriving from this one, as UHOLScriptScreen::FindScriptClass); null: none. */
	UFUNCTION(BlueprintPure, Category = "HOL|GameMenu")
	static TSubclassOf<UHOLGameMenuScreen> FindScriptClass();

	/** What the menu of this player reads of the match (bKnown false without a game or player state). */
	UFUNCTION(BlueprintPure, Category = "HOL|GameMenu")
	static FHOLGameMenuFacts ReadFacts(APlayerController* PlayerController);

	/** UHOLButton::SetLabel for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	static void SetButtonLabel(UHOLButton* Button, const FText& Label);

	/** A line of LogHOLUI (Display), the menu's lines read by the tests and scenarios (« Game menu: ... »). */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	static void MenuLog(const FString& Line);

	/** UHOLScreen::MakeDialogColumn for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	UVerticalBox* MakeMenuColumn();

	/** The title (heading style, 16 below), UHOLScreen::AddText for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	UCommonTextBlock* AddMenuTitle(UVerticalBox* Column, FName Name, const FText& Text);

	/** UHOLScreen::AddButton for the script; its clicks: OnButtonBaseClicked. */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	UHOLButton* AddMenuButton(UVerticalBox* Column, FName Name, const FText& Label);

	/** « Abandonner »: the confirmation over the menu (AskToSurrender). */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	void AskSurrender();

	/** « Pause »: the menu closes, the server pauses (AHOLPlayerController::RequestPause) if this slot may. */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	void PauseMatch();

	/** « Quitter vers le menu / la liste »: from the manager, the list (the lobby forgotten), else the main menu. */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	void QuitMatch();

	/** « Retour au salon »: the lobby after the match. */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	void ReturnToLobby();

	/** « Résultat »: the end banner again (it closes this menu as it opens). */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	void ShowResult();

	/** The widget focused on opening (« Reprendre »), set by the script. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|GameMenu")
	TObjectPtr<UWidget> FocusTarget;

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	/** The script builds the menu's column (in place of BuildLayout). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	void BP_BuildMenu();

	/** The script shows the buttons offered (each frame: a defeat or the match's end may come while open); bOpening: just opened. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	void BP_RefreshMenu(bool bOpening);

private:
	/** « Abandonner » offered to this player (UHOLGameMenuRulesBridge::NativeCanSurrender on the facts). */
	static bool CanSurrender(APlayerController* PlayerController);
};
