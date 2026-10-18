// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * One move order (D226, D286): "go to this location", for a group of units. Orders always pass through a queue,
 * even in solo, applied at the next fixed step. The group is kept whole: its units share their paths by clusters
 * and are given their slots around the target together (D287).
 */
struct FHOLMoveOrder
{
	/** Stable identifiers of the units (D223). */
	TArray<uint32> UnitIds;
	/** On the navmesh (brought back to it when the order is issued, D228). */
	FVector Target = FVector::ZeroVector;
	/** Slot of the player who gave it (D288): only its own units obey. INDEX_NONE: no check (the server's own orders). */
	int32 PlayerIndex = INDEX_NONE;
	/**
	 * A walk a unit's task gives it (J5: to its source, to the deposit): the task goes on. Any other move order ends the
	 * unit's gathering, its load kept (D296).
	 */
	bool bFromTask = false;
	/**
	 * The arrow of a right-button drag (J7a.4, the user's 2nd trial), from its base to the mouse (cm, on the ground; Target
	 * is its base, the middle of the front row): a group in formation faces along it. Null: a plain click, the default
	 * rows facing the march.
	 */
	FVector2D FrontLine = FVector2D::ZeroVector;
	/** Ctrl + right-button drag (the user's 1st trial of J7a.4): the front as wide as FrontLine; otherwise its default width. */
	bool bFrontWidth = false;
	/**
	 * Shift (the user's 1st trial of J7a.4): the move waits until the units' current move is over (their queue of moves,
	 * UHOLUnitOrderProcessor), its places made then from the units still in it; units with no move go at once.
	 */
	bool bQueue = false;
};

/** One gather order (J5, D296): "gather this source", for a group of units. Applied by the gathering at the next step. */
struct FHOLGatherOrder
{
	/** Stable identifiers of the units (D223). */
	TArray<uint32> UnitIds;
	/** The source (an index of UHOLSourceSubsystem). */
	int32 SourceIndex = INDEX_NONE;
	/** Slot of the player who gave it (D288): only its own units obey. INDEX_NONE: no check (the server's own orders). */
	int32 PlayerIndex = INDEX_NONE;
};
