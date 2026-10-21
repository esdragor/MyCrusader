#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "HOLScenarioPanel.generated.h"

class UBorder;
class UCommonTextBlock;
class UHOLQueueItem;
class UOverlaySlot;
class UVerticalBox;
class UWidget;

/**
 * The test scenario's panel (hol.Scenario, Tools/Scenarios/README.md): at the top centre of the screen, its name and
 * its state (en préparation…, prêt, or the step that never came, in red), then its notes, one per line, what the trial
 * asks to try. Its « Relancer » starts the same scenario again (hol.Scenario <Name>: the map is reloaded); its « × »
 * closes it until the next scenario; a scenario started again replaces it.
 * What it shows is kept by the game module (HOLTestHooks::GetScenarioDisplay), so that the panel of a match opened by
 * the scenario shows it too. Only a development build makes one (AHOLGameHUD, !UE_BUILD_SHIPPING); its texts are not
 * translated, as the cheats' panel's. Built in code (D242).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLScenarioPanel : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UHOLScenarioPanel();

	/** The widget it stays under (the game screen's top bar, which reaches past the middle), a gap below it. */
	void SetAnchorWidget(UWidget* InAnchor) { Anchor = InAnchor; }

	/** Shows the scenario's display as it is now: hidden before any scenario or once closed. */
	void Refresh();

	/** "scenario order_lines ready, 3 notes", "scenario closed" or "no scenario" (hol.HUD.Report). */
	FString Describe() const;

	/** Clicks ScenarioRestart or ScenarioClose as the mouse would (hol.HUD.Click); false for another name or when hidden. */
	bool ClickWidget(FName Name);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BuildLayout();
	void HandleCloseClicked();
	void HandleRestartClicked();

	UPROPERTY(Transient)
	TObjectPtr<UBorder> Panel;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> Title;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> State;

	UPROPERTY(Transient)
	TObjectPtr<UHOLQueueItem> RestartButton;

	UPROPERTY(Transient)
	TObjectPtr<UHOLQueueItem> CloseButton;

	/** One text a note, made again when they change. */
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> NoteColumn;

	/** The notes shown, so that a refresh with the same ones keeps their texts. */
	TArray<FString> ShownNotes;

	TWeakObjectPtr<UWidget> Anchor;
	/** The anchor's height last placed under, so that the slot is touched only when it changes. */
	float PlacedAnchorHeight = -1.f;

	UPROPERTY(Transient)
	TObjectPtr<UOverlaySlot> WidthSlot;
};
