// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLAttackCursor.h"

#include "Rendering/DrawElements.h"
#include "Widgets/SLeafWidget.h"

namespace
{
	/** A ring, four ticks across it and a dot: dark under red, readable on grass as on snow. */
	class SHOLAttackCursor final : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(SHOLAttackCursor) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs) {}

		static constexpr float Size = 36.f;

		virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(Size, Size); }

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2f Centre(Size * 0.5f, Size * 0.5f);
			constexpr float Ring = 11.f;
			constexpr float TickIn = 5.f;
			constexpr float TickOut = 17.f;
			constexpr int32 Segments = 32;

			TArray<FVector2f> Circle;
			for (int32 Index = 0; Index <= Segments; ++Index)
			{
				const float Angle = 2.f * UE_PI * Index / Segments;
				Circle.Add(Centre + Ring * FVector2f(FMath::Cos(Angle), FMath::Sin(Angle)));
			}
			const TArray<TArray<FVector2f>> Ticks = {
				{Centre + FVector2f(-TickOut, 0.f), Centre + FVector2f(-TickIn, 0.f)},
				{Centre + FVector2f(TickIn, 0.f), Centre + FVector2f(TickOut, 0.f)},
				{Centre + FVector2f(0.f, -TickOut), Centre + FVector2f(0.f, -TickIn)},
				{Centre + FVector2f(0.f, TickIn), Centre + FVector2f(0.f, TickOut)},
				{Centre + FVector2f(-1.f, 0.f), Centre + FVector2f(1.f, 0.f)}};

			const FPaintGeometry Geometry = AllottedGeometry.ToPaintGeometry();
			const auto DrawAll = [&](const FLinearColor& Color, const float Thickness)
			{
				FSlateDrawElement::MakeLines(OutDrawElements, LayerId, Geometry, Circle, ESlateDrawEffect::None, Color, true, Thickness);
				for (const TArray<FVector2f>& Tick : Ticks)
				{
					FSlateDrawElement::MakeLines(OutDrawElements, LayerId, Geometry, Tick, ESlateDrawEffect::None, Color, true, Thickness);
				}
			};
			DrawAll(FLinearColor(0.f, 0.f, 0.f, 0.85f), 5.f);
			++LayerId;
			DrawAll(FLinearColor(1.f, 0.06f, 0.04f, 1.f), 2.5f);
			return LayerId;
		}
	};
}

namespace HOLAttackCursor
{
	TSharedRef<SWidget> Make()
	{
		return SNew(SHOLAttackCursor);
	}
}
