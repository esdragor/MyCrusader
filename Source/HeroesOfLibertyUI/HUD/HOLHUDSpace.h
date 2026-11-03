// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/Box2D.h"

class UObject;
struct FGeometry;

/**
 * Where a drawn widget is, in the HUD's own space: Slate units from the top left of the game's viewport (the window's client area), the
 * space the panels are laid out in. FGeometry::GetAbsolutePosition is in DESKTOP space: it adds the window's place on the screen and its
 * title bar, which a full-screen window (at 0,0, no bar) hides; a windowed game read them as if they were the HUD's own, and the Diplomacy
 * button landed over the players' block (2026-11-03, E-diplomacy-compare). Every place read from a geometry goes through here.
 */
namespace HOLHUDSpace
{
	/** A desktop (absolute) point in the viewport's Slate units; without a game viewport (no world): the point over the DPI scale. */
	HEROESOFLIBERTYUI_API FVector2D AbsoluteToViewport(const UObject* WorldContext, const FVector2D& Absolute);

	/** The box of a geometry in the viewport's Slate units. */
	HEROESOFLIBERTYUI_API FBox2D BoxOf(const UObject* WorldContext, const FGeometry& Geometry);
}
