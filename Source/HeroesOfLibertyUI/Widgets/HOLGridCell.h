#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Match/HOLResources.h"
#include "HOLGridCell.generated.h"

class UBorder;
class UCommonTextBlock;
class UHorizontalBox;
class UImage;
class UOverlaySlot;
class USizeBox;
class UVerticalBox;
struct FHOLTradeGain;
class UHOLGridCell;

/**
 * The zone of a cell's face (GALADRIEL-1122g): a status line of its own at the bottom of the cell (Bientot, always visible) and,
 * above it, the zone of the name bounded to whole lines, so that a name of 3 to 4 lines no longer pushes the status out of the cell.
 */
struct FHOLGridCellNameZone
{
	/** Height of the name's zone, a whole number of lines; 0: not bounded (a cell without a status). */
	float NameHeight = 0.f;
	/** The lines of the name that fit in that zone (at least 1); 0 without a status. */
	int32 NameLines = 0;
	/** Height kept at the bottom of the cell for the status line; 0 without a status. */
	float StatusHeight = 0.f;
};

/** The status line's text once reduced to fit a width (GALADRIEL-1122h): Scale in [MinStatusScale, 1], Width the text's width after it. */
struct FHOLGridCellStatusFit
{
	float Scale = 1.f;
	float Width = 0.f;
};

/** How a cell of the grid takes a click. */
UENUM(BlueprintType)
enum class EHOLGridCellState : uint8
{
	/** Clickable, its command goes through. */
	Ready,
	/**
	 * Clickable but dimmed (D313), the resources missing in red in its tooltip: the click does nothing but say why (« Pas
	 * assez de bois »).
	 */
	Greyed,
	/** Not clickable. */
	Disabled,
};

/** A cell's click, for the script (HUD H-8: the age panels' cells are built by the script). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHOLGridCellClickedEvent, UHOLGridCell*, Cell);

/**
 * A cell of the command grid (D30, J3): a square with its key in the top left corner and, when it has a command (a unit
 * produced, a building placed, J6a.5, or a site cancelled), its icon, or its name while there is none (the user's trial
 * of J6a.5: no cost on the cell, HOLCommandGrid::GetCellFace). Hovered, a tooltip: name, cost (only the resources it
 * takes, each after the resource's marker; what the stock lacks in red), effect (D313). An empty cell is a plain dark
 * square, greyed. Never takes the keyboard focus: the grid's keys belong to the game (IMC_Game).
 * Built in code when no Blueprint gives it a layout (D242).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLGridCell : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UHOLGridCell();

	/** Broadcast by the button's click (HUD H-8): the script's age panels listen to it. */
	UPROPERTY(BlueprintAssignable, Category = "HeroesOfLiberty|UI")
	FHOLGridCellClickedEvent OnCellClicked;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SetKey(const FText& InKey);
	/** Shows a command: its name (no icon yet), dimmed when greyed; its cost is the tooltip's. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void SetCommand(const FText& InName, EHOLGridCellState InState);
	/**
	 * D781 (a Market's trade cell): under the name set by SetCommand (which hides it), « +X » then the logo of what the trade
	 * gives (the resource's marker until the resource icons exist), dimmed as the name.
	 */
	void SetGain(const FHOLTradeGain& Gain);
	/**
	 * Its tooltip (D313): the name, the cost line (or what a cancel gives back; Missing holds, for each resource, more
	 * than 0 where the stock does not cover the cost, shown in red) and what it gives; an empty name: none.
	 */
	void SetTooltip(const FText& InName, const FHOLResources& Cost, const FHOLResources& Missing, const FText& InEffect);
	/** Shows the key only, greyed. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UI")
	void ClearUnit();

	/**
	 * A status under the name (Bientot, the technologies without an effect, D601): a line of its own at the bottom of the cell,
	 * the name's zone bounded above it. Empty: none, the name is not bounded. SetCommand clears it: set it after SetCommand.
	 */
	void SetStatus(const FText& InStatus);
	/** The height the name's zone is bounded to while a status is shown (0: not bounded), for the checks. */
	float GetNameZoneHeight() const;
	/**
	 * The zone of the name and of the status in a square cell of side CellSize: TopBand is the band under the key's line, LineHeight the
	 * height of a text line. With a status: the status line takes one line at the bottom, the name keeps the whole lines left (at least 1).
	 * Without: nothing is bounded, the cell is as before.
	 */
	static FHOLGridCellNameZone ComputeNameZone(float CellSize, float TopBand, float LineHeight, bool bHasStatus);

	/** The side of a cell of the grid (the script's size box, HOLCommandGridPanel.as CellSize, and HOLGameScreen.cpp CellSize). */
	static constexpr float DefaultCellSize = 76.f;
	/** The band under the key's line, above the name. */
	static constexpr float NameTopBand = 12.f;
	/** The room left on each side of the status line's text in the cell. */
	static constexpr float StatusSidePad = 2.f;
	/** The status line's smallest scale (the text stays readable); a narrower cell cuts it (the scale box clips). */
	static constexpr float MinStatusScale = 0.6f;
	/** The width of the status line's text at its natural size, in Slate units (0: no status); estimated when there is no renderer. */
	float MeasureStatusWidth() const;
	/**
	 * The reduction of the status line so that its text fits in AvailableWidth (a cell at 1280x720 is about 51 px wide, "Coming soon" is wider):
	 * Scale is 1 when it fits, else AvailableWidth / TextWidth bounded below by MinStatusScale; Width is the text's width once reduced.
	 */
	static FHOLGridCellStatusFit ComputeStatusFit(float TextWidth, float AvailableWidth);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|UI")
	EHOLGridCellState GetState() const { return State; }
	/** "Maison | W 50 | +10 population": what the tooltip says, for the scripted checks (hol.HUD.Report). */
	FString DescribeTooltip() const;
	/**
	 * "Maison": what the cell itself writes (empty: nothing), for the scripted checks (hol.HUD.Report); a gain shown (D781)
	 * follows it with its logo's resource: "Bois +96 [Wood]".
	 */
	FString DescribeContent() const;

protected:
	virtual void NativeOnInitialized() override;
	/** Fits the status line's text to the cell's width (ComputeStatusFit): the cell's width is known only once laid out. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Key;

	/** The name (the icon to come), dimmed when the cell is greyed. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> UnitBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> UnitName;

private:
	/** Fills a cost line: a marker and an amount for each resource Cost takes, the missing ones in red. */
	static void ShowCost(TConstArrayView<TObjectPtr<UImage>> Markers, TConstArrayView<TObjectPtr<UCommonTextBlock>> Amounts, const FHOLResources& Cost,
		const FHOLResources& Missing, float Opacity);
	/** Builds a cost line in Row (Food, Wood, Stone, Gold) into Markers and Amounts. */
	void BuildCostLine(UHorizontalBox* Row, const FString& Prefix, TArray<TObjectPtr<UImage>>& Markers, TArray<TObjectPtr<UCommonTextBlock>>& Amounts);

	/** The tooltip (D313): a HUD panel with the name, the cost line and the effect. */
	UPROPERTY(Transient)
	TObjectPtr<UBorder> TooltipPanel;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> TooltipName;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> TooltipCostLine;

	/** Marker and amount of each resource on the tooltip's cost line, in the order Food, Wood, Stone, Gold. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> TooltipMarkers;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonTextBlock>> TooltipAmounts;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> TooltipEffect;

	/** D781: the gain line under the name of a trade cell, « +X » then the marker of the resource received. */
	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> GainLine;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> GainAmount;

	UPROPERTY(Transient)
	TObjectPtr<UImage> GainMarker;

	/** The name's box (bounded to whole lines while a status is shown), the name's slot of the cell and the status line. */
	UPROPERTY(Transient)
	TObjectPtr<USizeBox> NameBox;

	UPROPERTY(Transient)
	TObjectPtr<UOverlaySlot> UnitSlot;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> StatusText;

	/** The status line's clipping box (the text, reduced to the cell's width, never spills onto the next cell) and the scale applied to the text. */
	UPROPERTY(Transient)
	TObjectPtr<USizeBox> StatusClip;

	float AppliedStatusScale = 1.f;

	/** The height the name's zone is bounded to (GetNameZoneHeight); 0: not bounded. */
	float NameZoneHeight = 0.f;

	/** The button's native click, broadcast as OnCellClicked. */
	void BroadcastCellClicked();

	EHOLGridCellState State = EHOLGridCellState::Disabled;
	FString ShownTooltip;
	/** The key last given by SetKey: kept so that one given before NativeOnInitialized built the text block is shown. */
	FText KeyText;
	/** The resource of the gain's logo, while the gain line is shown (DescribeContent). */
	EHOLResource ShownGainLogo = EHOLResource::Gold;
};
