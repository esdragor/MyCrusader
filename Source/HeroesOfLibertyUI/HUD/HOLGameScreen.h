#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Input/UIActionBindingHandle.h"
#include "Match/HOLResources.h"
#include "HOLGameScreen.generated.h"

class AHOLBuilding;
class AHOLPlayerController;
class AHOLPlayerState;
class UBorder;
class UCommonTextBlock;
class UHOLGridCell;
class UHOLQueueItem;
class UHorizontalBox;
class UImage;
class UProgressBar;
struct FHOLUnitTypeRow;

/**
 * Game screen of a match (J3, D281): the HUD of D201 with only the blocks the game has a use for so far, at their
 * final places. Top left, the four resources with how many of the player's units gather each (J5b, D298), the
 * population and the idle workers ("." selects the next one, so does a click); bottom centre, the selected building (its
 * own or another player's, D203: name, owner, health), units (J4, D288: one unit as a building, several by type and
 * number, the workers among them by activity) or resource source (J5b: its name and what it still holds);
 * bottom right, the command grid (D30) of the player's own selected
 * building and, above it, that building's production queue, whose head shows its progress, computed on this machine
 * every frame from the server's clock (D282). The HUD reads the match and never changes it: a click on a cell or on
 * an item of the queue goes to the controller, then the server (D252); the grid's keys are the controller's own
 * (IMC_Game). Built in code when no Blueprint gives it a layout (D242).
 * Input stays the game's: input mode All, no Back handler (Escape deselects through IA_Deselect), never the keyboard
 * focus. A click on a panel stops at the HUD; a click anywhere else reaches the game (only the panels are hit-test
 * visible).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLGameScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UHOLGameScreen();

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	/**
	 * What the HUD shows, on one line: "resources F 150 W 200 S 200 G 200, population 1/10, gatherers F 0 W 3 S 0 G 0,
	 * idle 2, selection MainCentre (own), grid A=Peasant enabled, queue 2 [Peasant 0.45, Peasant waiting]" (hol.HUD.Report).
	 */
	FString Describe() const;

	/**
	 * Clicks a cell of the grid (GridCell00 to GridCell11), an item of the queue (QueueItem0 to QueueItem4) or the idle
	 * workers' counter (IdleWorkers), found by its name, by the same path as the mouse (hol.HUD.Click). False when it is
	 * hidden or greyed.
	 */
	bool ClickWidget(FName Name);

	/** Items of the queue built by BuildLayout, the longest queue shown (D283: 5); one line, under the grid's width. */
	static constexpr int32 NumQueueItems = 5;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Builds the layout in code; only called when no Blueprint gave one. */
	void BuildLayout();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> TopBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> FoodAmount;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> WoodAmount;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> StoneAmount;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> GoldAmount;

	/** How many of the player's units gather each resource (J5b, D298): « (3) » after its amount. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> FoodGatherers;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> WoodGatherers;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> StoneGatherers;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> GoldGatherers;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> PopulationAmount;

	/** The idle workers' counter (J5b, D298, D30): a click selects the next one, as ".". Greyed when there is none. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHOLQueueItem> IdleWorkers;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> SelectionPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SelectionName;

	/** The owner's colour and name: hidden for a resource source, which is nobody's. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> SelectionOwnerRow;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> SelectionSwatch;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SelectionOwner;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> SelectionHealthRow;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SelectionHealth;

	/**
	 * The line under the name (J5, D298): what a gatherer does and carries (« Récolte — Bois 7/10 »), a group's workers by
	 * activity (« 8 en récolte · 2 inactifs »), what a source still holds (« Reste : 1840 »).
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SelectionActivity;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> QueuePanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> QueueProgress;

	/** "Population full", in place of the progress bar while the head waits for room. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> QueueNote;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> GridPanel;

private:
	/** What the head of the queue shows. */
	enum class EHeadState : uint8
	{
		None,
		/** Started: the progress bar. */
		InProgress,
		/** Not started, room in the population: it starts at the server's next step. */
		Starting,
		/** Not started, no room in the population: the note. */
		PopulationFull,
	};

	/** Follows the owning player's state (it reaches a client after the HUD is up): binds its economy; null unbinds. */
	void BindPlayerState(AHOLPlayerState* PlayerState);
	/** Follows the selected building: binds its production queue; null unbinds. */
	void BindBuilding(AHOLBuilding* Building);
	void HandleSelectionChanged();

	/** Updates every block from the match state (on the economy, selection and production delegates). */
	void Refresh();
	void RefreshTopBar(const AHOLPlayerState* PlayerState);
	void RefreshSelection(const AHOLBuilding* Building, bool bOwn);
	/**
	 * Selected units (never with a building): one is shown as a building is, several by type and number, with their
	 * workers by activity (read again four times a second).
	 */
	void RefreshUnitSelection(TConstArrayView<uint32> UnitIds);
	/** The selected resource source (J5b, D298): its name and what it still holds, read again every frame. */
	void RefreshSourceSelection(int32 SourceIndex);
	/**
	 * The line of a gatherer selected alone (J5, D298), or hidden (null): what it does and carries, read again every
	 * frame and written when it changed (D303).
	 */
	void RefreshUnitActivity(const struct FHOLUnitView* Unit);
	/** The line under the name: Shown is what Describe writes of it (empty: hidden); written only when it changed. */
	void SetDetailLine(const FString& Shown, const FText& Text);
	/** The idle workers' counter, written when their number changed. */
	void RefreshIdleWorkers();
	void HandleIdleWorkersClicked();
	void RefreshGrid(const AHOLBuilding* OwnBuilding, const AHOLPlayerState* PlayerState);
	void RefreshQueue(const AHOLBuilding* OwnBuilding);
	/** The head of the queue: its progress (every frame) or the population note. */
	void UpdateHead();

	void HandleGridCellClicked(int32 Cell);
	void HandleQueueItemClicked(int32 Index);

	const FHOLUnitTypeRow* FindUnitType(FName TypeName) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLGridCell>> GridCells;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLQueueItem>> QueueItems;

	TWeakObjectPtr<AHOLPlayerController> Controller;
	TWeakObjectPtr<AHOLPlayerState> BoundPlayerState;
	TWeakObjectPtr<AHOLBuilding> BoundBuilding;
	FDelegateHandle SelectionHandle;
	FDelegateHandle EconomyHandle;
	FDelegateHandle ProductionHandle;

	/** What the blocks show, for Describe. */
	bool bEconomyShown = false;
	FHOLResources ShownResources;
	int32 ShownPopulationUsed = 0;
	int32 ShownPopulationCap = 0;
	FHOLResources ShownGatherers;
	int32 ShownIdleWorkers = INDEX_NONE;
	/** Seconds since a group's activities were read again. */
	float SinceGroupRefresh = 0.f;
	FString ShownSelection = TEXT("none");
	/**
	 * After ShownSelection: ", Gathering Wood 4/10" for a gatherer selected alone, ", gathering 8 moving 1 idle 2" for a
	 * group, ", 1840 left" for a source; else empty.
	 */
	FString ShownActivity;
	bool bGridShown = false;
	TArray<FName> ShownGridTypes;
	bool bQueueShown = false;
	TArray<FName> ShownQueueTypes;
	EHeadState HeadState = EHeadState::None;
	float HeadProgress = 0.f;
};
