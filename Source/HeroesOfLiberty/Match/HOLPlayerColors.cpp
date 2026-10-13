// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLPlayerColors.h"

#define LOCTEXT_NAMESPACE "HOLPlayerColors"

namespace HOLPlayerColors
{
	namespace
	{
		const FColor Colors[NumColors] = {
			FColor(40, 90, 220),	// blue
			FColor(210, 40, 40),	// red
			FColor(40, 170, 60),	// green
			FColor(230, 200, 40),	// yellow
			FColor(40, 190, 200),	// teal
			FColor(150, 60, 190),	// purple
			FColor(230, 120, 30),	// orange
			FColor(230, 120, 170),	// pink
		};

		int32 Wrap(const int32 ColorIndex)
		{
			return ((ColorIndex % NumColors) + NumColors) % NumColors;
		}
	}

	FColor GetColor(const int32 ColorIndex)
	{
		return Colors[Wrap(ColorIndex)];
	}

	const FVector3f& GetLinearColor(const int32 ColorIndex)
	{
		static const TArray<FVector3f> Linear = []
		{
			TArray<FVector3f> Result;
			for (const FColor& Color : Colors)
			{
				const FLinearColor Converted(Color);
				Result.Add(FVector3f(Converted.R, Converted.G, Converted.B));
			}
			return Result;
		}();
		return Linear[Wrap(ColorIndex)];
	}

	FText GetColorName(const int32 ColorIndex)
	{
		// Source texts in French, the source language (D245).
		static const FText Names[NumColors] = {
			LOCTEXT("Blue", "Bleu"),
			LOCTEXT("Red", "Rouge"),
			LOCTEXT("Green", "Vert"),
			LOCTEXT("Yellow", "Jaune"),
			LOCTEXT("Teal", "Turquoise"),
			LOCTEXT("Purple", "Violet"),
			LOCTEXT("Orange", "Orange"),
			LOCTEXT("Pink", "Rose"),
		};
		return Names[Wrap(ColorIndex)];
	}
}

#undef LOCTEXT_NAMESPACE
