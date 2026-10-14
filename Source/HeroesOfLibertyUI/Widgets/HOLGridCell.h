#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "HOLGridCell.generated.h"

class UCommonTextBlock;
class UHorizontalBox;
class UImage;
class UVerticalBox;
struct FHOLResources;

/**
 * A cell of the command grid (D30, J3): a square with its key in the top left corner and, when it produces a unit,
 * the unit's name and its cost (only the resources it takes, each after the resource's marker). An empty cell is a
 * plain dark square, greyed. Never takes the keyboard focus: the grid's keys belong to the game (IMC_Game). Built in
 * code when no Blueprint gives it a layout (D242).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLGridCell : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UHOLGridCell();

	void SetKey(const FText& InKey);
	/** Shows a unit, clickable or greyed (not enough resources, queue full). */
	void SetUnit(const FText& InName, const FHOLResources& Cost, bool bInEnabled);
	/** Shows the key only, greyed. */
	void ClearUnit();

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> Key;

	/** Name and cost, dimmed together when the cell is greyed. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> UnitBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> UnitName;

private:
	/** Marker and amount of each resource on the cost line, in the order Food, Wood, Stone, Gold. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UImage>> CostMarkers;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonTextBlock>> CostAmounts;
};
