#include "Widgets/HOLStatsGraph.h"

#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Match/HOLMatchStats.h"
#include "Rendering/DrawElements.h"
#include "Style/HOLUIStyles.h"

namespace UE::HeroesOfLiberty::UI::StatsGraph
{
	/** Room around the plot: the values' labels on the left, the marks' labels on top, the times and the legend under. */
	constexpr float Left = 64.f;
	constexpr float Right = 20.f;
	constexpr float Top = 44.f;
	constexpr float Bottom = 70.f;
	constexpr float LabelGap = 8.f;
	constexpr float FontSize = 14.f;
	constexpr float CurveThickness = 3.f;
	/**
	 * Each curve drawn this many pixels apart vertically, centred on the true value (C6): two players with the same
	 * values read as two touching stripes, the first one never hidden under the second.
	 */
	constexpr float CurveSpread = CurveThickness;
	constexpr float SwatchSize = 14.f;
	constexpr float LegendGap = 28.f;
	/** Graduations at most on each axis. */
	constexpr int32 MaxTimeTicks = 6;
	constexpr int32 MaxValueTicks = 5;
}

void UHOLStatsGraph::SetData(TArray<FCurve> InCurves, TArray<FMark> InMarks)
{
	Curves = MoveTemp(InCurves);
	Marks = MoveTemp(InMarks);
	double MaxTime = 0.0;
	double MaxValue = 0.0;
	for (const FCurve& Curve : Curves)
	{
		for (const FVector2D& Point : Curve.Points)
		{
			MaxTime = FMath::Max(MaxTime, Point.X);
			MaxValue = FMath::Max(MaxValue, Point.Y);
		}
	}
	for (const FMark& Mark : Marks)
	{
		MaxTime = FMath::Max(MaxTime, static_cast<double>(Mark.TimeSeconds));
	}
	TimeStep = HOLMatchStats::GetNiceStep(FMath::CeilToInt32(MaxTime), UE::HeroesOfLiberty::UI::StatsGraph::MaxTimeTicks, /*bTimeAxis*/ true);
	TimeEnd = HOLMatchStats::GetAxisEnd(FMath::CeilToInt32(MaxTime), TimeStep);
	ValueStep = HOLMatchStats::GetNiceStep(FMath::CeilToInt32(MaxValue), UE::HeroesOfLiberty::UI::StatsGraph::MaxValueTicks);
	ValueEnd = HOLMatchStats::GetAxisEnd(FMath::CeilToInt32(MaxValue), ValueStep, /*bHeadroom*/ true);
	Invalidate(EInvalidateWidgetReason::Paint);
}

FString UHOLStatsGraph::DescribeAxes() const
{
	return FString::Printf(TEXT("time 0..%d step %d, values 0..%d step %d"), TimeEnd, TimeStep, ValueEnd, ValueStep);
}

int32 UHOLStatsGraph::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, const bool bParentEnabled) const
{
	using namespace UE::HeroesOfLiberty::UI::StatsGraph;
	LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const FVector2f Size = FVector2f(AllottedGeometry.GetLocalSize());
	const FVector2f PlotMin(Left, Top);
	const FVector2f PlotMax(Size.X - Right, Size.Y - Bottom);
	if (PlotMax.X <= PlotMin.X || PlotMax.Y <= PlotMin.Y)
	{
		return LayerId;
	}
	const auto ToPlot = [&](const double Time, const double Value)
	{
		return FVector2f(FMath::Lerp(PlotMin.X, PlotMax.X, static_cast<float>(Time / TimeEnd)),
			FMath::Lerp(PlotMax.Y, PlotMin.Y, static_cast<float>(Value / ValueEnd)));
	};

	FSlateFontInfo Font;
	GetDefault<UHOLTextStyleNote>()->GetFont(Font);
	Font.Size = FontSize;
	const TSharedRef<FSlateFontMeasure> Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FPaintGeometry Whole = AllottedGeometry.ToPaintGeometry();
	const auto DrawText = [&](const FString& Text, const FVector2f& Position, const FVector2f& Pivot, const FLinearColor& Color, const int32 Layer)
	{
		const FVector2f TextSize = FVector2f(Measure->Measure(Text, Font));
		FSlateDrawElement::MakeText(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(Position - TextSize * Pivot)),
			Text, Font, ESlateDrawEffect::None, Color);
		return TextSize;
	};
	const auto DrawLine = [&](const FVector2f& From, const FVector2f& To, const FLinearColor& Color, const float Thickness, const int32 Layer)
	{
		FSlateDrawElement::MakeLines(OutDrawElements, Layer, Whole, TArray<FVector2f>{ From, To }, ESlateDrawEffect::None, Color, true, Thickness);
	};

	// The plot's panel, its horizontal graduations and the axes.
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(PlotMax - PlotMin, FSlateLayoutTransform(PlotMin)), &WhiteBrush,
		ESlateDrawEffect::None, FLinearColor(0.f, 0.f, 0.f, 0.35f));
	const int32 GridLayer = LayerId + 1;
	const FLinearColor GridColor(1.f, 1.f, 1.f, 0.12f);
	for (int32 Value = 0; Value <= ValueEnd; Value += ValueStep)
	{
		const FVector2f At = ToPlot(0.0, Value);
		DrawLine(At, FVector2f(PlotMax.X, At.Y), GridColor, 1.f, GridLayer);
		DrawText(FString::FromInt(Value), At - FVector2f(LabelGap, 0.f), FVector2f(1.f, 0.5f), HOLUIStyle::TextMuted, GridLayer);
	}
	for (int32 Time = 0; Time <= TimeEnd; Time += TimeStep)
	{
		const FVector2f At = ToPlot(Time, 0.0);
		DrawLine(At, At + FVector2f(0.f, 6.f), HOLUIStyle::TextMuted, 1.f, GridLayer);
		DrawText(HOLMatchStats::FormatMatchTime(Time), At + FVector2f(0.f, LabelGap), FVector2f(0.5f, 0.f), HOLUIStyle::TextMuted, GridLayer);
	}
	DrawLine(FVector2f(PlotMin.X, PlotMax.Y), PlotMax, HOLUIStyle::TextMuted, 1.5f, GridLayer);
	DrawLine(PlotMin, FVector2f(PlotMin.X, PlotMax.Y), HOLUIStyle::TextMuted, 1.5f, GridLayer);

	// The events' marks: a vertical line at their time, their label above the plot (staggered when several are near).
	const int32 MarkLayer = GridLayer + 1;
	for (int32 Index = 0; Index < Marks.Num(); ++Index)
	{
		const FMark& Mark = Marks[Index];
		const FVector2f Foot = ToPlot(Mark.TimeSeconds, 0.0);
		const FLinearColor LineColor(Mark.Color.R, Mark.Color.G, Mark.Color.B, 0.8f);
		DrawLine(FVector2f(Foot.X, PlotMin.Y - 4.f), Foot, LineColor, 2.f, MarkLayer);
		const float Row = (Index % 2) * (FontSize + 4.f);
		const float Pivot = Foot.X > (PlotMin.X + PlotMax.X) * 0.5f ? 1.f : 0.f;
		DrawText(Mark.Label.ToString(), FVector2f(Foot.X, PlotMin.Y - 6.f - Row), FVector2f(Pivot, 1.f), Mark.Color, MarkLayer);
	}

	// The curves: a black under-stroke so that the darker colours read on the panel, the colour over it; each shifted by
	// CurveSpread from the next, the band centred on the true value, so that equal curves lie side by side.
	const int32 CurveLayer = MarkLayer + 1;
	for (int32 CurveIndex = 0; CurveIndex < Curves.Num(); ++CurveIndex)
	{
		const FCurve& Curve = Curves[CurveIndex];
		const FVector2f Shift(0.f, -CurveSpread * (CurveIndex - (Curves.Num() - 1) * 0.5f));
		TArray<FVector2f> Points;
		Points.Reserve(Curve.Points.Num());
		for (const FVector2D& Point : Curve.Points)
		{
			Points.Add(ToPlot(Point.X, Point.Y) + Shift);
		}
		if (Points.Num() == 1)
		{
			Points.Add(Points[0] + FVector2f(1.f, 0.f));
		}
		if (Points.Num() > 1)
		{
			FSlateDrawElement::MakeLines(OutDrawElements, CurveLayer, Whole, Points, ESlateDrawEffect::None, FLinearColor(0.f, 0.f, 0.f, 0.7f), true,
				CurveThickness + 2.f);
			FSlateDrawElement::MakeLines(OutDrawElements, CurveLayer + 1, Whole, Points, ESlateDrawEffect::None, Curve.Color, true, CurveThickness);
		}
	}

	// The legend under the time axis: a swatch and the name a player.
	const int32 LegendLayer = CurveLayer + 2;
	FVector2f At(PlotMin.X, Size.Y - SwatchSize - 4.f);
	for (const FCurve& Curve : Curves)
	{
		FSlateDrawElement::MakeBox(OutDrawElements, LegendLayer, AllottedGeometry.ToPaintGeometry(FVector2f(SwatchSize, SwatchSize), FSlateLayoutTransform(At)),
			&WhiteBrush, ESlateDrawEffect::None, Curve.Color);
		const FVector2f NameSize = DrawText(Curve.Name.ToString(), At + FVector2f(SwatchSize + LabelGap, SwatchSize * 0.5f), FVector2f(0.f, 0.5f),
			HOLUIStyle::Text, LegendLayer);
		At.X += SwatchSize + LabelGap + NameSize.X + LegendGap;
	}
	return LegendLayer + 1;
}
