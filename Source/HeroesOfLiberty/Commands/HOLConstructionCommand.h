// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AHOLBuilding;

/**
 * One construction order of a player (J6a.3, D311): place a foundation of a type at a ground point, or cancel one of its
 * buildings not yet finished. Like the production orders, it waits in the order queue and the server applies it at its
 * next step, checking everything again (D288): the client's ghost is never trusted.
 */
struct FHOLConstructionCommand
{
	enum class EKind : uint8
	{
		Place,
		Cancel,
	};

	EKind Kind = EKind::Place;
	/** Slot of the player who gave the order (D244): it pays, it owns the foundation, only it cancels its own. */
	int32 PlayerIndex = INDEX_NONE;
	/** Place: the building type (a row of the building type table that builders build). */
	FName TypeName;
	/** Place: the ground point the footprint is snapped around (HOLConstruction::SnapToGrid). */
	FVector Location = FVector::ZeroVector;
	/** Cancel: the building. */
	TWeakObjectPtr<AHOLBuilding> Building;
	/**
	 * Place (J6a.4, D312): the units the player had selected (D223 identifiers), sent to build the foundation once placed;
	 * with bQueue (Shift), it goes at the end of their queue of sites.
	 */
	TArray<uint32> UnitIds;
	bool bQueue = false;
};

/**
 * One build order (J6a.4, D312): units sent to build one of their player's sites not yet finished, a foundation just
 * placed (UHOLConstructionSubsystem) or a site helped (a right click on it). Applied by UHOLBuildProcessor at the next
 * step, which checks it again (D288): the site its owner's, unfinished; only the owner's units that build.
 */
struct FHOLBuildOrder
{
	/** Stable identifiers of the units (D223). */
	TArray<uint32> UnitIds;
	TWeakObjectPtr<AHOLBuilding> Building;
	/** Slot of the player who gave it (D288): only its own units obey, on its own site. INDEX_NONE: no check (the server's own orders). */
	int32 PlayerIndex = INDEX_NONE;
	/** Shift (D311): the site goes at the end of each unit's queue; otherwise it replaces it. */
	bool bQueue = false;
	/**
	 * Or, with bQueue, a source to gather in its turn (Shift + right click on a resource, the user's trial of J6a.4; an index
	 * of UHOLSourceSubsystem), Building left empty: it goes in the same queue as the sites, in the order given.
	 */
	int32 SourceIndex = INDEX_NONE;
	/** A player's order (D328, given through its controller): its builders' chain is drawn, with Shift or not. */
	bool bChain = false;
	/** With bChain: units just born sent to their building's rally point, a site (D328): drawn as a Rally until they start building. */
	bool bRally = false;
};
