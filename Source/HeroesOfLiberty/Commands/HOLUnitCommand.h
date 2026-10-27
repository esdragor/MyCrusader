// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AHOLBuilding;

/** What a move order asks besides the walk (J7b.3, J7-Q4): the combat orders go through the same queue (Shift included). */
enum class EHOLOrderKind : uint8
{
	/** Go there (the units take on nobody on the way). */
	Move,
	/** Take on the unit TargetUnitId (an enemy, alive: checked by the server), whatever the aggression radius. */
	Attack,
	/** Go there, taking on the enemies met on the way (HOLCombat::AttackMoveRadius), then on again. */
	AttackMove,
	/** Stand: walks, queue and fight forgotten. */
	Stop,
	/** Stand and hold the position: only enemies in reach, never a chase (until a move order). */
	Hold,
	/** Strike the enemy building TargetBuilding (D345.2b: finished or a site seen; checked by the server) until it falls. */
	AttackBuilding,
	/**
	 * Unload at TargetBuilding, one of the player's deposits (D485, EtapeDefini L.5b: a right click on it, with Shift or
	 * not): through the queue of moves, so that Shift waits for the walks before it. When it starts, the units carrying a
	 * resource it takes get a drop-off (UHOLCommandSubsystem::IssueDropOffOrder, the gathering's), the others walk to
	 * Target as a Move.
	 */
	DropOff,
	/**
	 * Gather the source SourceIndex (EtapeDefini L.6: Shift + right click on a resource, for the workers without a queue of
	 * work): through the queue of moves, so that it waits for the walks before it. When it starts, the units get the
	 * gathering's Shift order (UHOLCommandSubsystem::IssueGatherOrder, bQueue): with no queue of work, at once.
	 */
	Gather,
	/**
	 * Build (or help to build) the site TargetBuilding (L.6: Shift + right click on a site, for the workers without a queue
	 * of work): as Gather, the builders' Shift order when it starts (UHOLCommandSubsystem::IssueBuildOrder, bQueue).
	 */
	Build,
};

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
	/** What it asks besides the walk (J7b.3). */
	EHOLOrderKind Kind = EHOLOrderKind::Move;
	/** Attack: the unit to take on. */
	uint32 TargetUnitId = 0;
	/** AttackBuilding: the building to strike (D345.2b). DropOff: the deposit to unload at (D485). Build: the site (L.6). */
	TWeakObjectPtr<AHOLBuilding> TargetBuilding;
	/** Gather: the source (an index of UHOLSourceSubsystem, L.6). */
	int32 SourceIndex = INDEX_NONE;
	/**
	 * A walk the combat gives a unit (J7b.3: its approach to its target, its attack-move resumed): the unit's
	 * FHOLCombatFragment::EngageSerial when it was given; a newer player order changed it, the walk is dropped. 0: none.
	 */
	uint32 CombatStamp = 0;
	/** A player's order (D328, given through its controller): its units' chain is drawn, with Shift or not. */
	bool bChain = false;
	/** With bChain: units just born sent to their building's rally point (D328): their step is a Rally (drawn white). */
	bool bRally = false;
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
	/** A player's order (D328, given through its controller): its units' chain is drawn until they reach the source. */
	bool bChain = false;
	/** With bChain: units just born sent to their building's rally point (D328): their step is a Rally (drawn white). */
	bool bRally = false;
	/**
	 * A drop-off order (D485, EtapeDefini L), SourceIndex INDEX_NONE: the units carrying a resource this building takes
	 * unload there, then stand idle (lost on the way: the nearest deposit taking their load).
	 */
	TWeakObjectPtr<AHOLBuilding> Deposit;
};
