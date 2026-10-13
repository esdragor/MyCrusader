// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Player colours (D233), chosen by the client: a unit only carries its owner's index, and the menu shows the
 * same palette when a slot picks its colour (D240, D244). One place for both, so a swatch in the menu is the
 * colour the units are drawn with. An index of NumColors and above wraps.
 */
namespace HOLPlayerColors
{
	constexpr int32 NumColors = 8;

	/** sRGB colour, for the interface. */
	HEROESOFLIBERTY_API FColor GetColor(int32 ColorIndex);

	/** Linear base colour, written into an instance's custom data 0-2 (M_UnitPlayerColor). */
	HEROESOFLIBERTY_API const FVector3f& GetLinearColor(int32 ColorIndex);

	/** Localised name shown in the menu (D245). */
	HEROESOFLIBERTY_API FText GetColorName(int32 ColorIndex);
}
