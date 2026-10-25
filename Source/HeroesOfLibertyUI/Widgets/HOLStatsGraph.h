#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Brushes/SlateColorBrush.h"
#include "HOLStatsGraph.generated.h"

/**
 * The curves of the statistics screen's « Chronologie » (C5), as Age of Empires II's timeline: one curve a player in its
 * colour over the match time, the time axis in minutes:seconds under it, the values' round graduations on its left, the
 * events (a defeat, a forum fallen) as dated vertical marks in the player's colour, the legend under the axis. Drawn in
 * code (NativePaint); its size is the SizeBox's around it. The axes' rules are HOLMatchStats's (GetNiceStep, ...).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLStatsGraph : public UUserWidget
{
	GENERATED_BODY()

public:
	struct FCurve
	{
		FText Name;
		FLinearColor Color = FLinearColor::White;
		/** X the match time in seconds, Y the value, in time order. */
		TArray<FVector2D> Points;
	};

	struct FMark
	{
		int32 TimeSeconds = 0;
		FLinearColor Color = FLinearColor::White;
		FText Label;
	};

	/** The curves and marks to draw; the axes follow them. */
	void SetData(TArray<FCurve> InCurves, TArray<FMark> InMarks);

	/** "time 0..60 step 10, values 0..8 step 2": the axes as drawn, for the logs. */
	FString DescribeAxes() const;

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TArray<FCurve> Curves;
	TArray<FMark> Marks;
	int32 TimeStep = 1;
	int32 TimeEnd = 1;
	int32 ValueStep = 1;
	int32 ValueEnd = 1;
	FSlateColorBrush WhiteBrush = FSlateColorBrush(FLinearColor::White);
};
