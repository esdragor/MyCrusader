#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "HOLCheatPanel.generated.h"

class AHOLPlayerController;
class IConsoleVariable;
class UBorder;
class UCommonTextBlock;
class UHOLQueueItem;
class USlider;
class UVerticalBox;
class UWidget;

/**
 * The cheats' panel of a match (D291): a button per development cheat (hol.Cheat.*), under the top bar on the left;
 * its first button folds and unfolds it, and the choice is kept from one match to the next (GameUserSettings.ini). Under
 * the buttons, sliders for the units' walking speed and their gathering speed (x0.5 to x5, hol.Cheat.UnitSpeed and
 * hol.Cheat.GatherSpeed).
 * A click goes to the controller, then the server, as the console command does. Only a development build makes one
 * (AHOLGameHUD, !UE_BUILD_SHIPPING); a Shipping server refuses the cheats anyway. Its labels are not translated: a
 * development tool, in the language of the trials. Built in code (D242).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLCheatPanel : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UHOLCheatPanel();

	/** The widget it stays under (the game screen's top bar), at the screen's margin and a gap below it. */
	void SetAnchorWidget(UWidget* InAnchor) { Anchor = InAnchor; }

	bool IsOpen() const { return bOpen; }
	void SetOpen(bool bInOpen);

	/**
	 * Clicks a button found by its name (CheatToggle, CheatFood, CheatWood, CheatStone, CheatGold, CheatAll,
	 * CheatPopulation, CheatFastBuild), by the same path as the mouse (hol.HUD.Click). False when it is folded away.
	 */
	bool ClickWidget(FName Name);

	/** "cheats open, fast build off, unit speed x1.0, gather speed x1.0" or "cheats folded" (hol.HUD.Report). */
	FString Describe() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	enum class ECheat : uint8
	{
		Food,
		Wood,
		Stone,
		Gold,
		All,
		Population,
		FastBuild,
	};

	/** The sliders, in the order of their specs (HOLCheatPanel.cpp). */
	enum class ESpeed : uint8
	{
		Units,
		Gather,
		Num,
	};
	static constexpr int32 NumSpeeds = static_cast<int32>(ESpeed::Num);

	void BuildLayout();
	void HandleToggleClicked();
	void HandleCheatClicked(ECheat Cheat);
	/** The toggle's, the fast build's and the sliders' labels, from bOpen, bFastBuild and Speeds. */
	void RefreshLabels();
	/** Solo or listen server: what the panel shows follows the server's settings (also changed by the console). */
	void FollowServerSettings();

	/** A slider moved: the new scale, by steps, sent to the server when it changed. */
	void HandleSpeedChanged(ESpeed Speed, float Value);

	UFUNCTION()
	void HandleUnitSpeedChanged(float Value);

	UFUNCTION()
	void HandleGatherSpeedChanged(float Value);

	UPROPERTY(Transient)
	TObjectPtr<UBorder> Panel;

	UPROPERTY(Transient)
	TObjectPtr<UHOLQueueItem> Toggle;

	/** The cheats' buttons, under the toggle: collapsed while the panel is folded. */
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> CheatColumn;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLQueueItem>> CheatButtons;

	/** Each slider's label and the slider, in the order of ESpeed. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonTextBlock>> SpeedLabels;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USlider>> SpeedSliders;

	TWeakObjectPtr<UWidget> Anchor;
	TWeakObjectPtr<AHOLPlayerController> Controller;
	bool bOpen = false;
	/**
	 * What the last click asked for, or at first the server's setting when it is this machine (solo): a client never
	 * learns that another player changed it.
	 */
	bool bFastBuild = false;
	/** The sliders' scales, the same way (hol.Units.SpeedScale, hol.Gather.SpeedScale). */
	float Speeds[NumSpeeds] = { 1.f, 1.f };
	/** The server's settings, when it is this machine (solo, listen server); else null. */
	IConsoleVariable* ServerBuildTime = nullptr;
	IConsoleVariable* ServerSpeeds[NumSpeeds] = {};
	/** The anchor's height last placed under, so that the slot is touched only when it changes. */
	float PlacedAnchorHeight = -1.f;
};
