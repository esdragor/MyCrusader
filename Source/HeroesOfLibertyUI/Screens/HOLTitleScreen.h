#pragma once

#include "CoreMinimal.h"
#include "Match/HOLFactionChoice.h"
#include "Screens/HOLScreen.h"
#include "HOLTitleScreen.generated.h"

class UCommonTextBlock;
class UHOLButton;
class UVerticalBox;

/**
 * Title screen (D240): the game's name and the way into the rest of the menu (Play, Multiplayer, Settings, Quit). Root of the
 * menu layer: Back does nothing here, Quit asks for a confirmation in the modal layer. Its column, texts, buttons and their
 * clicks are written in script (a class deriving from this one, FindScriptClass: Script/UI/HOLTitleScreen.as; ECR-1, D840/D839);
 * this base keeps what the script cannot reach: the triple click of the faction cheat (FReply overrides), the focus, the
 * localised labels (LOCTEXT) and the widget helpers.
 */
UCLASS(Abstract, Blueprintable)
class HEROESOFLIBERTYUI_API UHOLTitleScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	UHOLTitleScreen();

	/** Opens the quit confirmation over the menu of the given player (Quit button, hol.UI.Open Quit). */
	static void AskToQuit(APlayerController* PlayerController);

	/** The script's title screen (the newest class deriving from this one, as UHOLConfirmDialogScreen::FindScriptClass); null: none. */
	UFUNCTION(BlueprintPure, Category = "HOL|TitleScreen")
	static TSubclassOf<UHOLTitleScreen> FindScriptClass();

	/** The label of a button by its widget name (PlayButton, MultiplayerButton, SettingsButton, QuitButton), localised in this C++ namespace HOLTitleScreen; empty for another name. */
	UFUNCTION(BlueprintPure, Category = "HOL|TitleScreen")
	static FText ButtonLabel(FName ButtonName);

	/** UHOLScreen::MakeCenteredColumn for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|TitleScreen")
	UVerticalBox* MakeTitleColumn();

	/** UHOLScreen::AddText for the script: bTitle = the game's name (title style, 48 below), else the heading style (8 below). */
	UFUNCTION(BlueprintCallable, Category = "HOL|TitleScreen")
	UCommonTextBlock* AddTitleText(UVerticalBox* Column, FName Name, const FText& Text, bool bTitle);

	/** UHOLScreen::AddButton for the script; its clicks: OnButtonBaseClicked. */
	UFUNCTION(BlueprintCallable, Category = "HOL|TitleScreen")
	UHOLButton* AddTitleButton(UVerticalBox* Column, FName Name, const FText& Label);

	/** The version (R1.7), small in the bottom right corner of the root overlay, not translated; sets and returns Version (null without the overlay). */
	UFUNCTION(BlueprintCallable, Category = "HOL|TitleScreen")
	UCommonTextBlock* AddVersionText();

	/** The clicks' actions: Play (the New game screen), Multiplayer, Settings, Quit (the confirmation). */
	UFUNCTION(BlueprintCallable, Category = "HOL|TitleScreen")
	void OpenNewGame();
	UFUNCTION(BlueprintCallable, Category = "HOL|TitleScreen")
	void OpenMultiplayer();
	UFUNCTION(BlueprintCallable, Category = "HOL|TitleScreen")
	void OpenSettings();
	UFUNCTION(BlueprintCallable, Category = "HOL|TitleScreen")
	void AskQuit();

	/** Set by the script (BuildTitle): the name, the version and the four buttons. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|TitleScreen")
	TObjectPtr<class UCommonTextBlock> Title;

	/** The game's version (R1.7), bottom right. */
	UPROPERTY(BlueprintReadWrite, Category = "HOL|TitleScreen")
	TObjectPtr<class UCommonTextBlock> Version;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|TitleScreen")
	TObjectPtr<class UHOLButton> PlayButton;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|TitleScreen")
	TObjectPtr<class UHOLButton> MultiplayerButton;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|TitleScreen")
	TObjectPtr<class UHOLButton> SettingsButton;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|TitleScreen")
	TObjectPtr<class UHOLButton> QuitButton;

protected:
	virtual void BuildLayout() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	/** The title's triple click: the faction cheat (HOLFactionChoice), session only, no sign of it on screen. */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	/** Slate sends the second click of a quick pair as a double click, not as a press: it counts too. */
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	FReply CountTitleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, bool& bHandled);

	/** The script builds the title's column, buttons and version (in place of BuildLayout). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|TitleScreen")
	void BP_BuildTitle();

private:
	HOLFactionChoice::FClickCounter TitleClicks;
};
