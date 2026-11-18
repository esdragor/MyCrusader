#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLSettingsScreen.generated.h"

class UCommonTextBlock;
class UHorizontalBox;
class UHOLButton;
class UHOLRotator;
class UVerticalBox;
class UWidgetSwitcher;

/**
 * Settings screen (D245): tabs Video, Sound, Controls (camera speed and edge scrolling since J1, D253; remapping greyed
 * until the game has enough actions) and Language (French, English, Japanese). Each change is applied and saved at once (UHOLGameUserSettings,
 * GameUserSettings.ini); the language is switched live and saved by the engine.
 * Its layout, rotators and the way a change is applied are written in script (a class deriving from this one,
 * FindScriptClass: Script/UI/HOLSettingsScreen.as; D840, as UHOLEndBannerScreen); this base keeps what the script cannot reach:
 * the opening (Open, Find: Settings button, hol.UI.Open Settings), the columns of the keyboard and gamepad navigation, the
 * localized texts (LOCTEXT, so that HeroesOfLiberty.Localization.Translations stays green), the display of the window
 * (r.SetRes) and the engine's calls that need a native type.
 */
UCLASS(Abstract, Blueprintable)
class HEROESOFLIBERTYUI_API UHOLSettingsScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** Opens the screen over the menu of the given player (Settings button, hol.UI.Open Settings); the script's class, null (and a warning) without one. */
	static UHOLSettingsScreen* Open(APlayerController* PlayerController);

	/** The screen if it is the open menu screen of the given player. */
	static UHOLSettingsScreen* Find(APlayerController* PlayerController);

	/** The script's screen (the newest class deriving from this one, as UHOLEndBannerScreen::FindScriptClass); null: none. */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static TSubclassOf<UHOLSettingsScreen> FindScriptClass();

	/** 0 Video, 1 Sound, 2 Controls, 3 Language. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	void SelectTab(int32 Index);

	/** The index of the page shown (0 Video ... 3 Language), -1 without pages. */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	int32 GetActiveTab() const;

	/** UHOLScreen::MakeCenteredColumn for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UVerticalBox* MakeSettingsColumn();
	/** A line of the column: style 0 the heading (16 under it), 1 a note (0 under it), UHOLScreen::AddText for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UCommonTextBlock* AddSettingsText(UVerticalBox* Column, FName Name, const FText& Text, int32 Style);
	/** UHOLScreen::AddButtonRow for the script. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UHorizontalBox* AddSettingsRow(UVerticalBox* Column);
	/** A button of the row (UHOLScreen::AddButton); its clicks: OnButtonBaseClicked. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UHOLButton* AddSettingsButton(UHorizontalBox* Row, FName Name, const FText& Label);
	/** A button straight in a page's column (the Remap button of Controls: UHOLScreen::AddButton over a vertical box). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UHOLButton* AddSettingsPageButton(UVerticalBox* Page, FName Name, const FText& Label);
	/** A tab button of the row: selectable and toggleable (or a selected one could never be deselected; a click selects it again by SelectTab). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UHOLButton* AddSettingsTab(UHorizontalBox* Row, FName Name, const FText& Label);
	/** The switcher of the pages (TabPages), added under the tabs; a page: AddSettingsPage. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UWidgetSwitcher* AddSettingsPages(UVerticalBox* Column);
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UVerticalBox* AddSettingsPage();
	/** The two side-by-side columns of the Video page: the pair, then each column (bLast: the right one). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UHorizontalBox* AddSettingsPair(UVerticalBox* Page);
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UVerticalBox* AddSettingsPairColumn(UHorizontalBox* Pair, bool bLast);
	/** The gamepad hint under the Video page (UHOLScreen::AddColumnHint). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UCommonTextBlock* AddSettingsHint(UVerticalBox* Page);
	/**
	 * A label and its rotator (UHOLScreen::AddRotatorRow); layout 0 the page's default widths, 1 the display column of Video,
	 * 2 its quality column. A change calls BP_SettingChanged(Name, Value).
	 */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	UHOLRotator* AddSettingsRotator(UVerticalBox* Column, FName Name, const FText& Label, const TArray<FText>& Options, int32 Index, int32 Layout);

	/** A text of the screen by its key (Heading, Video, Sound, Controls, Language, LanguageLabel, Back, Resolution, WindowMode, VSync, FrameRate, RenderScale, Quality, the 8 groups by name, Volume_<Master|Music|Effects|Voice>, SoundNote, Remap, RemapNote, CameraSpeed, EdgeScroll). */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static FText TextOf(FName Key);
	/** The values of a rotator by kind: WindowMode, OffOn, FrameRate, RenderScale, Level (5 levels), Quality (levels and Custom), Volume (steps), CameraSpeed, Language. */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static TArray<FText> OptionsOf(FName Kind);

	/** The resolutions offered (the supported ones from 1280x720 and the one shown), as names; fills the list ChooseResolution reads. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	TArray<FText> GetResolutionNames();
	/** The resolution of the window as it is (r.SetRes, which the command line can impose over the saved settings), as an index of GetResolutionNames. */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	int32 GetShownResolutionIndex() const;
	/** The window mode as it is: 0 Fullscreen, 1 WindowedFullscreen, 2 Windowed. */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static int32 GetShownWindowMode();
	/** Sets the resolution of that index with the window mode shown (not applied nor saved: the script does). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	void ChooseResolution(int32 Index);
	/** Sets the window mode with the resolution shown (not applied nor saved). */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	static void ChooseWindowMode(int32 Mode);

	/** Index of the frame rate limit among the rotator's values (the nearest; 5 = unlimited), and its setting. */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static int32 GetFrameRateIndex();
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	static void SetFrameRateIndex(int32 Index);
	/** Same for the render scale (50 to 100 %). */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static int32 GetRenderScaleIndex();
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	static void SetRenderScaleIndex(int32 Index);
	/** The overall quality as a rotator index: 0 to 4, or 5 (Custom) when the groups differ. */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static int32 GetQualityIndex();
	/** The level to set for the overall quality rotator at Value: Custom (5) cannot be chosen, the rotator passes over it. */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static int32 ResolveQualityChoice(int32 Value);
	/** The level (0 to 4) of a scalability group (0 ViewDistance, 1 AntiAliasing, 2 GlobalIllumination, 3 Texture, 4 VisualEffect, 5 Foliage, 6 Shading, 7 Landscape), and its setting. */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static int32 GetGroupLevel(int32 Group);
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	static void SetGroupLevel(int32 Group, int32 Level);
	/** The language as an index (0 fr, 1 en, 2 ja), and the switch (live, saved by the engine: [Internationalization] of GameUserSettings.ini). */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static int32 GetLanguageIndex();
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	static void SetLanguageIndex(int32 Index);

	/** Closes the screen, as Back does. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	void CloseSettings();

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> TabPages;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> VideoTab;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> SoundTab;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> ControlsTab;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> LanguageTab;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> BackButton;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> ResolutionRotator;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> WindowModeRotator;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> VSyncRotator;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> FrameRateRotator;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> RenderScaleRotator;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> QualityRotator;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> LanguageRotator;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> RemapButton;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> CameraSpeedRotator;

	UPROPERTY(BlueprintReadWrite, Category = "HOL|Settings", meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> EdgeScrollRotator;

	/** Per scalability group (Quality_ViewDistance...) and per volume (Volume_Master...), in order; the script fills them (the navigation columns read them). */
	UPROPERTY(BlueprintReadWrite, Transient, Category = "HOL|Settings")
	TArray<TObjectPtr<UHOLRotator>> GroupRotators;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "HOL|Settings")
	TArray<TObjectPtr<UHOLRotator>> VolumeRotators;

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	/** On the Video tab: display, quality (D250). */
	virtual void GetColumns(TArray<TArray<UWidget*>>& OutColumns) const override;

	/** The script builds the screen (in place of BuildLayout). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Settings")
	void BP_BuildSettings();

	/** The script shows the values of the settings (once the screen is built). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Settings")
	void BP_RefreshSettings();

	/** A rotator made by AddSettingsRotator changed to Value (its name as given). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Settings")
	void BP_SettingChanged(FName Name, int32 Value);

private:
	TArray<FIntPoint> Resolutions;
};
