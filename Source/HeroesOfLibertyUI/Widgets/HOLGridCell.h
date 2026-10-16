#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Match/HOLResources.h"
#include "HOLGridCell.generated.h"

class UBorder;
class UCommonTextBlock;
class UHorizontalBox;
class UImage;
class UVerticalBox;

/** How a cell of the grid takes a click. */
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

	void SetKey(const FText& InKey);
	/** Shows a command: its name (no icon yet), dimmed when greyed; its cost is the tooltip's. */
	void SetCommand(const FText& InName, EHOLGridCellState InState);
	/**
	 * Its tooltip (D313): the name, the cost line (or what a cancel gives back; Missing holds, for each resource, more
	 * than 0 where the stock does not cover the cost, shown in red) and what it gives; an empty name: none.
	 */
	void SetTooltip(const FText& InName, const FHOLResources& Cost, const FHOLResources& Missing, const FText& InEffect);
	/** Shows the key only, greyed. */
	void ClearUnit();

	EHOLGridCellState GetState() const { return State; }
	/** "Maison | W 50 | +10 population": what the tooltip says, for the scripted checks (hol.HUD.Report). */
	FString DescribeTooltip() const;
	/** "Maison": what the cell itself writes (empty: nothing), for the scripted checks (hol.HUD.Report). */
	FString DescribeContent() const;

protected:
	virtual void NativeOnInitialized() override;

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

	EHOLGridCellState State = EHOLGridCellState::Disabled;
	FString ShownTooltip;
};
