#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Input/UIActionBindingHandle.h"
#include "Match/HOLResources.h"
#include "Player/HOLCommandGrid.h"
#include "HOLGameScreen.generated.h"

class AHOLBuilding;
class AHOLPlayerController;
class AHOLPlayerState;
class UBorder;
class USizeBox;
class UCommonTextBlock;
class UHOLGridCell;
class UHOLQueueItem;
class UHorizontalBox;
class UImage;
class UProgressBar;
class UWrapBox;
struct FHOLUnitTypeRow;

/**
 * Game screen of a match (J3, D281): the HUD of D201 with only the blocks the game has a use for so far, at their
 * final places. Top left, the four resources with how many of the player's units gather each (J5b, D298), the
 * population and the idle workers ("." selects the next one, so does a click); along the bottom, from the left (the user's
 * 2nd trial of G.1, AoE4's way, fixed places), the selection's block: the selected building (its
 * own or another player's, D203: name, owner, health), units (J4, D288: one unit as a building, several by type and
 * number, the workers among them by activity) or resource source (J5b: its name and what it still holds);
 * then the command panel (D30, 3 x 4, the orders of the player's units or the grid of its own selected building) and,
 * above it, that building's production queue, whose head shows its progress, computed on this machine
 * every frame from the server's clock (D282). With the player's Peasants selected, the grid shows the buildings they
 * build in four age panels right of the command panel (G.1: I to IV, a medallion each, Q S D F / W X C V); bottom right,
 * the minimap's place (none yet). A cell shows its name only (no icon yet, no cost: the
 * user's trial of J6a.5); its tooltip gives name, cost (the missing amounts in red) and effect; greyed when the stock
 * does not cover it. A site selected shows its progress, its builders and an « Annuler » cell (V). Several of the
 * player's buildings selected together (F1, F2, F3, D30) show their active subgroup (Tab: « Tous (7) », « Maison × 6 »,
 * « Centre principal »), then by type and number, the grid that subgroup's cells (the user's 2nd trial of J6a.5). A
 * short message above the grid says why a command did not go through (« Pas assez de bois », « Aucun bâtiment
 * militaire ») or that the production waits for room (« Population maximale — construisez des maisons »).
 * The HUD reads the match and never changes it: a click on a cell or on
 * an item of the queue goes to the controller, then the server (D252); the grid's keys are the controller's own
 * (IMC_Game). Built in code when no Blueprint gives it a layout (D242).
 * Top right, « Menu » opens the in-game menu (D356), as Escape with nothing selected.
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
	 * workers' counter (IdleWorkers) or « Menu » (MenuButton), found by its name, by the same path as the mouse (hol.HUD.Click). False when it is
	 * hidden or greyed.
	 */
	bool ClickWidget(FName Name);

	/** The tooltips of the grid's cells: "A: Maison | W 50 | +10 population; Z: ..." (hol.HUD.Report). */
	FString DescribeTooltips() const;
	/** What the grid's cells write: "A: Maison; Z: Camp de bûcherons; ..." (hol.HUD.Report). */
	FString DescribeCells() const;

	/** Seconds a short message stays above the grid (D313), the last of them fading out. */
	static constexpr float MessageSeconds = 3.f;
	static constexpr float MessageFadeSeconds = 0.5f;

	/** The top bar (resources, population, idle workers): the cheats' panel stays under it (UHOLCheatPanel). */
	UBorder* GetTopBar() const { return TopBar; }

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

	/** Top right: opens the in-game menu (D356). */
	UPROPERTY(Transient)
	TObjectPtr<UHOLQueueItem> MenuButton;

	/** The control groups' bar (J7a.5, D30), bottom left: one slot per group not empty, 1 to 9 then 0. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> GroupsPanel;

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

	/** A building not finished (J6a.5, D313): its progress, a bar and « 45 % ». */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> SelectionProgressRow;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> SelectionProgress;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> SelectionProgressText;

	/** The short message above the grid (D313). */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> MessagePanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> MessageText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> QueuePanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> QueueProgress;

	/** "Population full", in place of the progress bar while the head waits for room. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> QueueNote;

	/**
	 * The one command panel (the user's 2nd trial of G.1, AoE4's way): 3 x 4 cells at fixed places, the empty ones dark,
	 * right of the selection's block: the orders of the player's units (Q attack-move, S stop, V hold) or the grid of its
	 * building or group (production, « Annuler »). Hidden (its place kept) without anything of the player's selected.
	 */
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
	/** Several of the player's buildings (an F key, D30): the active subgroup, then by type and number, the sites among them. */
	void RefreshBuildingGroup(TConstArrayView<AHOLBuilding*> Buildings);
	/**
	 * The selection's chips (G.1e, AoE4's way), from ChipLabels and ActiveChip that the selection's refresh filled: shown
	 * in place of the name, wrapping in the block's width, the active one's edge lit; none: the name alone. Written when changed.
	 */
	void RefreshChips();
	/** A chip clicked: its subgroup made active, as Tab (chip N is the subgroup N, « Tous » the first). */
	void HandleChipClicked(int32 Chip);
	/**
	 * The line of a gatherer selected alone (J5, D298), or hidden (null): what it does and carries, read again every
	 * frame and written when it changed (D303).
	 */
	void RefreshUnitActivity(const struct FHOLUnitView* Unit);
	/** One unit's health in the selection block (J7b.5), or none (nullptr: a group, a building, a source). */
	void RefreshUnitHealth(const struct FHOLUnitView* Unit);
	/** The line under the name: Shown is what Describe writes of it (empty: hidden); written only when it changed. */
	void SetDetailLine(const FString& Shown, const FText& Text);
	/** The idle workers' counter, written when their number changed. */
	void RefreshIdleWorkers();
	void HandleIdleWorkersClicked();
	void HandleMenuClicked();
	/**
	 * The control groups' bar (J7a.5), read again four times a second (units die, groups change): a slot per group not
	 * empty, its digit and number, its types in the tooltip; the group just selected lit briefly. Logs « HUD groups: ... »
	 * when what it shows changed.
	 */
	void RefreshControlGroups(float DeltaSeconds);
	/** A slot clicked: the group selected, as its key (twice quickly: the camera goes to it). */
	void HandleGroupSlotClicked(int32 Group);
	/** A slot's widget name, ControlGroup0 to ControlGroup9 (hol.HUD.Click). */
	static FName GroupSlotName(int32 Group);
	/** OwnBuilding: the player's own building selected alone; bOwnGroup: several of its buildings (an F key). */
	void RefreshGrid(const AHOLBuilding* OwnBuilding, bool bOwnGroup, const AHOLPlayerState* PlayerState);
	void RefreshQueue(const AHOLBuilding* OwnBuilding);
	/** The head of the queue: its progress (every frame) or the population note. */
	void UpdateHead();

	/** A cell of the command panel: an order with the player's units, else the grid's command. */
	void HandleGridCellClicked(int32 Cell);
	/** A building of the builders' age panels clicked (AHOLPlayerController::ClickAge); the medallions take no click. */
	void HandleAgeClicked(int32 Age, int32 Cell);
	/**
	 * The builders' age panels (the user's trials of G.1, AoE4's way), right of the command panel: each the width of its
	 * buildings' columns, only its rows used, a medallion (numeral, key) on its top edge; an age not reached faded, the
	 * chosen one's medallion and edge lit.
	 */
	void RefreshAges(const AHOLPlayerState* PlayerState);
	void HandleQueueItemClicked(int32 Index);

	/** A command that did not go through (D313), from the controller: its short message. */
	void HandleCommandFeedback(const FHOLCommandFeedback& Feedback);
	/** Shows a short message above the grid for MessageSeconds; Key names it in the logs and in Describe. */
	void ShowMessage(const FString& Key, const FText& Text);
	/** The message's age: it fades, then hides. */
	void UpdateMessage(float DeltaSeconds);
	/**
	 * One of the player's buildings has a unit waiting for room in the population (D313): when that starts, the message
	 * « Population maximale — construisez des maisons ».
	 */
	void CheckPopulationBlocked();

	const FHOLUnitTypeRow* FindUnitType(FName TypeName) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLGridCell>> GridCells;

	/** The builders' age panels, I to IV, and their medallions (numeral, key A Z E R; no click). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> AgePanels;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> AgeMedallions;
	/** Each medallion's tooltip as Report writes it (« Palier II — Essor | Palier non atteint »), and the look shown (-1: none yet). */
	TArray<FString> ShownAgeTooltips;
	TArray<int32> ShownAgeLooks;

	/** Each age's building cells, Q S D F / W X C V (NumAges x NumAgeCells), and their boxes in the panel's grid. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLGridCell>> AgeCells;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USizeBox>> AgeCellBoxes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLQueueItem>> QueueItems;

	/** The selection's chips (G.1e): a pool of SelectionChipN in the wrap box SelectionChips, each in its frame (lit: active). */
	UPROPERTY(Transient)
	TObjectPtr<UWrapBox> SelectionChips;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLQueueItem>> ChipItems;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> ChipFrames;
	/** What the selection's refresh wants shown (empty: the name alone), and what the chips show (-2: nothing yet). */
	TArray<FText> ChipLabels;
	int32 ActiveChip = INDEX_NONE;
	TArray<FString> ShownChips;
	int32 ShownActiveChip = -2;

	/** The control groups' slots, by group (0 to 9). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UHOLQueueItem>> GroupSlots;
	/** What each slot shows (its number of units, « b » and its buildings; empty: hidden) and the line logged of it. */
	TArray<FString> ShownGroups;
	FString ShownGroupsLine;
	float SinceGroupsRefresh = 1.f;
	/** The group lit (just selected) and since when (real time). */
	int32 LitGroup = INDEX_NONE;
	double LitGroupTime = -1.0;

	TWeakObjectPtr<AHOLPlayerController> Controller;
	TWeakObjectPtr<AHOLPlayerState> BoundPlayerState;
	TWeakObjectPtr<AHOLBuilding> BoundBuilding;
	FDelegateHandle SelectionHandle;
	FDelegateHandle EconomyHandle;
	FDelegateHandle ProductionHandle;
	FDelegateHandle ConstructionHandle;
	FDelegateHandle FeedbackHandle;

	/** What the blocks show, for Describe. */
	bool bEconomyShown = false;
	FHOLResources ShownResources;
	int32 ShownPopulationUsed = 0;
	int32 ShownPopulationCap = 0;
	FHOLResources ShownGatherers;
	int32 ShownIdleWorkers = INDEX_NONE;
	/** Seconds since a group's activities (or a group of buildings) were read again. */
	float SinceGroupRefresh = 0.f;
	FString ShownSelection = TEXT("none");
	/**
	 * After ShownSelection: ", Gathering Wood 4/10" for a gatherer selected alone, ", gathering 8 moving 1 idle 2" for a
	 * group, ", 1840 left" for a source; else empty.
	 */
	FString ShownActivity;
	/** ", hp 37/50" for one unit selected (J7b.5), for Report; empty otherwise. */
	FString ShownHealth;
	bool bGridShown = false;
	TArray<FName> ShownGridTypes;
	bool bOrdersShown = false;
	TArray<FName> ShownOrderTypes;
	/** The builders' age panels, for Report: their buildings (NumAges x NumAgeCells), the ages reached, the chosen one. */
	bool bAgesShown = false;
	TArray<FName> ShownAgeTypes;
	TArray<bool> ShownAgeReached;
	int32 ShownChosenAge = INDEX_NONE;
	bool bQueueShown = false;
	TArray<FName> ShownQueueTypes;
	EHeadState HeadState = EHeadState::None;
	float HeadProgress = 0.f;
	/** The short message shown (its key, empty: none) and its age (s). */
	FString MessageKey;
	float MessageAge = 0.f;
	/** A unit of the player waits for room in the population (the last check). */
	bool bPopulationBlocked = false;
};
