#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "HOLCheatPanel.generated.h"

class AHOLPlayerController;
class UBorder;
class UHOLQueueItem;
class UVerticalBox;
class UWidget;

/**
 * The cheats' panel of a match (D291): a button per development cheat (hol.Cheat.*), under the top bar on the left;
 * its first button folds and unfolds it, and the choice is kept from one match to the next (GameUserSettings.ini).
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

	/** "cheats open, fast build off" or "cheats folded" (hol.HUD.Report). */
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

	void BuildLayout();
	void HandleToggleClicked();
	void HandleCheatClicked(ECheat Cheat);
	/** The toggle's and the fast build's labels, from bOpen and bFastBuild. */
	void RefreshLabels();

	UPROPERTY(Transient)
	TObjectPtr<UBorder> Panel;

	UPROPERTY(Transient)
	TObjectPtr<UHOLQueueItem> Toggle;

	/** The cheats' buttons, under the toggle: collapsed while the panel is folded. */
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> CheatColumn;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLQueueItem>> CheatButtons;

	TWeakObjectPtr<UWidget> Anchor;
	TWeakObjectPtr<AHOLPlayerController> Controller;
	bool bOpen = false;
	/**
	 * What the last click asked for, or at first the server's setting when it is this machine (solo): a client never
	 * learns that another player changed it.
	 */
	bool bFastBuild = false;
	/** The anchor's height last placed under, so that the slot is touched only when it changes. */
	float PlacedAnchorHeight = -1.f;
};
