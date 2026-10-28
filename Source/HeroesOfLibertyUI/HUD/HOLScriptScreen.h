#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HOLScriptScreen.generated.h"

class UBorder;
class UButton;
class UProgressBar;
class UCommonTextBlock;
class UCommonTextStyle;

/**
 * The game screen's part written in Angelscript (J8a.5 loop 2): the HUD's blocks move to script one by one, the hero's
 * block first (Script/UI/HOLGameScreenScript.as). UHOLGameScreen makes the one script class derived from this one
 * (FindScriptClass) and lays it over its whole frame; like the frame, it lets the clicks through to the game (only its
 * panels, MakePanel, take them). The script builds its tree in OnInitialized (ConstructWidget, SetRootWidget: the names
 * are found by hol.HUD.Click), refreshes it in Refresh (RefreshSeconds) and says what it shows in Describe.
 */
UCLASS(Abstract)
class HEROESOFLIBERTYUI_API UHOLScriptScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * What the script's blocks show, one line per block "<block>: <what>" ("hero: alive level 1 hp 450/450"), lines
	 * separated by "\n": hol.HUD.Report writes each one as "HUD <line>". Empty: none.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|UI")
	FString Describe() const;

	/** A click on the widget named Name by the mouse's path (hol.HUD.Click); false: not the script's, hidden or greyed. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|UI")
	bool ClickWidget(FName Name);

	/** Called every RefreshSeconds while shown, and once at its construction: the blocks read the game and redraw. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HeroesOfLiberty|UI")
	void Refresh();

	/**
	 * Refresh at once, the timer started again: hol.HUD.Report calls it before Describe, the screen then showing what the
	 * game holds now (the tests' waits draw no frame, so tick no widget).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void RefreshNow();

	/** The local player's slot, INDEX_NONE (-1) for a spectator or without a player state yet. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	int32 GetLocalSlot() const;

	/** A HUD panel, the C++ screen's look (rounded, HOLUIStyle::HUDPanel), the only kind of widget taking clicks. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UBorder* MakePanel(FName Name);

	/** A text of the HUD in the style TextStyle (UHOLTextStyleHUD, UHOLTextStyleHUDLabel...), empty. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UCommonTextBlock* MakeText(FName Name, TSubclassOf<UCommonTextStyle> TextStyle);

	/**
	 * A HUD button, the look of the C++ screen's (UHOLHUDButtonStyle: dark, a gold edge hovered and pressed, plain dark
	 * greyed), never focusable; the script sets its content (SetContent) and binds OnClicked. Its clicks stop there.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UButton* MakeButton(FName Name);

	/**
	 * Button greyed or not, by its look only (UHOLHUDButtonStyleGreyed's: plain dark, a dim edge hovered): it stays enabled
	 * so that its tooltip shows and its content keeps its colours (a red price); the script ignores its clicks.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	static void SetButtonGreyed(UButton* Button, bool bGreyed);

	/** A HUD bar as the site's progress (dark background, rounded), filled in FillColor, at 0. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	UProgressBar* MakeBar(FName Name, FLinearColor FillColor);

	/** The local player's colour (HOLPlayerColors, as the menu's swatches), grey for a spectator. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	FLinearColor GetLocalColor() const;

	/** The HUD's colours (HOLUIStyle): its text, its labels', a missing amount (red). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	static FLinearColor GetTextColor();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	static FLinearColor GetMutedColor();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	static FLinearColor GetMissingColor();

	/**
	 * The room of a block bottom left, above the selection's block and the control groups' bar (UHOLGameScreen's
	 * places): the padding of a block aligned bottom left in the script's frame.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	FMargin GetBottomLeftPlace() const;

	/** The class made by the game screen: the newest script class derived from this one, not abstract; null: none. */
	static UClass* FindScriptClass();

	/** Seconds between two Refresh (4 per second). */
	static constexpr float RefreshSeconds = 0.25f;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	float RefreshLeft = 0.f;
};
