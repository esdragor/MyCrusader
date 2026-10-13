#pragma once

#include "CoreMinimal.h"
#include "Screens/HOLScreen.h"
#include "HOLSettingsScreen.generated.h"

class UWidgetSwitcher;

/**
 * Settings screen (D245): tabs Video, Sound, Controls (greyed until M4: remapping of D30, camera speed, edge scrolling)
 * and Language (French, English, Japanese). Each change is applied and saved at once (UHOLGameUserSettings,
 * GameUserSettings.ini); the language is switched live and saved by the engine.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLSettingsScreen : public UHOLScreen
{
	GENERATED_BODY()

public:
	/** Opens the screen over the menu of the given player (Settings button, hol.UI.Open Settings). */
	static UHOLSettingsScreen* Open(APlayerController* PlayerController);

	/** The screen if it is the open menu screen of the given player. */
	static UHOLSettingsScreen* Find(APlayerController* PlayerController);

	/** 0 Video, 1 Sound, 2 Controls, 3 Language. */
	void SelectTab(int32 Index);

protected:
	virtual void BuildLayout() override;
	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> TabPages;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLButton> VideoTab;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> SoundTab;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> ControlsTab;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> LanguageTab;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> BackButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UHOLRotator> ResolutionRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> WindowModeRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> VSyncRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> FrameRateRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> RenderScaleRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> QualityRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> LanguageRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLButton> RemapButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> CameraSpeedRotator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLRotator> EdgeScrollRotator;

	/** Per scalability group (Quality_ViewDistance...) and per volume (Volume_Master...), found by name so that a
	 *  Blueprint layout can give them. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLRotator>> GroupRotators;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLRotator>> VolumeRotators;

private:
	/** Shows the values of the settings. */
	void Refresh();
	/**
	 * Applies and saves the settings, then shows them (a change can move others: overall quality and groups). The
	 * resolution and the window mode are applied only when they are the ones changed, so that no other change resizes
	 * the window to saved values.
	 */
	void ApplyAndRefresh(bool bResolutionChanged = false);

	void HandleResolution(int32 Value);
	void HandleWindowMode(int32 Value);
	void HandleVSync(int32 Value);
	void HandleFrameRate(int32 Value);
	void HandleRenderScale(int32 Value);
	void HandleQuality(int32 Value);
	void HandleGroup(int32 Group, int32 Value);
	void HandleVolume(int32 Volume, int32 Value);
	void HandleLanguage(int32 Value);

	TArray<FIntPoint> Resolutions;
	TArray<float> FrameRates;
	TArray<float> RenderScales;
};
