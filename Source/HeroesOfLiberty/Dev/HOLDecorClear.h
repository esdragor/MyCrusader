// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;

/**
 * Test tool (hol.Decor.ClearNear X Y R): the scenery of a map that is NOT a resource source, taken away around a ground point so
 * that a scenario can show what stands there (the neutral camp of `neutrals`, in a clearing). Two kinds of scenery:
 *  - the trunks and crowns of Plain's grove: static mesh actors whose first material is MI_Trunk or MI_Crown (Tools/Editor/maps/plain.py);
 *  - the instances of the instanced meshes of the map's scenery (maps/common.py spawn_decor), but never those of the source field
 *    (AHOLSourceField draws the registry's sources: the game's trees, which this tool leaves alone).
 * Nothing of the rules changes: sources, ground, ridges and buildings stay.
 */
namespace UE::HeroesOfLiberty::Decor
{
	struct FDecorCount
	{
		/** Grove actors (a trunk or a crown each). */
		int32 Actors = 0;
		/** Scenery instances. */
		int32 Instances = 0;
		/** Standing wood sources (the game's trees), counted or taken only when bSources is asked (hol.Decor.ClearNear ... Trees). */
		int32 Sources = 0;

		int32 Total() const { return Actors + Instances + Sources; }
	};

	/**
	 * What stands within Radius (cm) of the ground point At: Plain's grove actors and the scenery instances; with bSources, the
	 * standing WOOD sources too (UHOLSourceSubsystem, the game's trees), which are left alone otherwise.
	 */
	HEROESOFLIBERTY_API FDecorCount CountNear(const UWorld& World, const FVector2D& At, double Radius, bool bSources = false);

	/**
	 * Takes all of it away (actors destroyed, instances removed); returns what was taken. With bSources the standing wood sources of the
	 * radius leave the map as well (UHOLSourceSubsystem::RemoveSource: display, navmesh, sight, every client; server or solo only).
	 */
	HEROESOFLIBERTY_API FDecorCount ClearNear(UWorld& World, const FVector2D& At, double Radius, bool bSources = false);
}
