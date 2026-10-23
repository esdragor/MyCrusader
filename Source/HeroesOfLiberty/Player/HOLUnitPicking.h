// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FHOLMatchConfig;

/** A unit as drawn on the screen (pixels), for the selection (J4, D288): light units have no collision to trace. */
struct FHOLPickCandidate
{
	/** Feet and top of the head, projected. */
	FVector2D Foot = FVector2D::ZeroVector;
	FVector2D Head = FVector2D::ZeroVector;
	/** Half the body's width on the screen. */
	double HalfWidth = 0.;
	/** Distance from the camera: the nearest one is in front. */
	double Depth = 0.;
};

/** A unit as drawn, with its type, its owner and whether it is the player's own: the double click (A.1) takes its type. */
struct FHOLTypedPickCandidate
{
	FHOLPickCandidate Pick;
	FName TypeName;
	bool bOwn = false;
	/** Its owner's slot (D.2: a double click on an enemy takes that enemy's units of its type alone). */
	int32 OwnerPlayer = INDEX_NONE;
};

/**
 * The side of a unit for the local player, in the order a selection keeps them (D.1, AoE4's way): a box that takes
 * several sides keeps only the first present, the player's own, then an ally's, then an enemy's, then a neutral one.
 * Allies are the slots of the player's team (D.3, GetSide); no neutral unit exists yet.
 */
enum class EHOLSelectionSide : uint8
{
	Own,
	Ally,
	Enemy,
	Neutral
};

/**
 * Pure rules of the selection of units on the screen (J4, D288), tested on their own: a click takes the unit whose
 * drawn body holds the cursor, the one in front when bodies overlap; a box takes the units whose body's middle is in it.
 */
namespace HOLUnitPicking
{
	/** A press that moves farther than this (pixels) before its release draws a box instead of clicking. */
	inline constexpr double BoxThreshold = 6.;

	/**
	 * Margin (pixels on a viewport 1080 pixels high, scaled with its height) around an enemy's drawn body within which a
	 * right click still takes it as the target (J7c, retour d'essai 8, loop 3: "very hard to target an enemy"; 25 px chosen
	 * at the user's trial); the console variable hol.Select.TargetMargin starts at it.
	 */
	inline constexpr double TargetMarginAt1080 = 25.;

	/** Index of the candidate under Cursor, or INDEX_NONE. */
	HEROESOFLIBERTY_API int32 PickAt(TConstArrayView<FHOLPickCandidate> Candidates, const FVector2D& Cursor);

	/** Distance (pixels) from Cursor to a candidate's drawn body (the one PickAt tests), 0 on it. */
	HEROESOFLIBERTY_API double GetBodyDistance(const FHOLPickCandidate& Candidate, const FVector2D& Cursor);

	/**
	 * The candidate under Cursor as PickAt (bOutDirect true): a direct hit always wins. Else the one whose body is the
	 * nearest to Cursor within Margin pixels (bOutDirect false; the one in front at the same distance); INDEX_NONE.
	 */
	HEROESOFLIBERTY_API int32 PickNear(TConstArrayView<FHOLPickCandidate> Candidates, const FVector2D& Cursor, double Margin, bool& bOutDirect);

	/** A margin given at 1080 pixels high, in pixels on a viewport ViewportHeight pixels high (as is when unknown). */
	HEROESOFLIBERTY_API double GetScaledMargin(double MarginAt1080, double ViewportHeight);

	/** Indices of the candidates whose body's middle is inside the box between two corners (any order). */
	HEROESOFLIBERTY_API void PickInBox(TConstArrayView<FHOLPickCandidate> Candidates, const FVector2D& CornerA, const FVector2D& CornerB, TArray<int32>& OutIndices);

	/** True when a press at Start released at End is a box, not a click. */
	HEROESOFLIBERTY_API bool IsBox(const FVector2D& Start, const FVector2D& End);

	/** Two clicks closer in time than this (seconds, Windows' default) make a double click (A.1). */
	inline constexpr double DoubleClickSeconds = 0.5;

	/**
	 * True when a click at Second (pixels, time SecondTime) ends a double click begun at First: soon enough and no farther
	 * than a box's threshold. FirstTime negative: no click before.
	 */
	HEROESOFLIBERTY_API bool IsDoubleClick(double FirstTime, const FVector2D& First, double SecondTime, const FVector2D& Second);

	/**
	 * A double click (or Ctrl + click, as AoE4) on the candidate Clicked (A.1): every one of the player's own candidates of
	 * its type whose body's middle is on the screen (0 to ViewportSize), Clicked first. Clicked another player's (D.2): the
	 * candidates of its type and of its owner (OwnerPlayer) on the screen, never the player's own nor another enemy's.
	 * Clicked not a candidate: none.
	 */
	HEROESOFLIBERTY_API void PickSameTypeOnScreen(TConstArrayView<FHOLTypedPickCandidate> Candidates, int32 Clicked, const FVector2D& ViewportSize,
		TArray<int32>& OutIndices);

	/**
	 * The side of a unit of the slot OwnerSlot for the local player of LocalSlot (D.1, D.3): its own; an ally when the two
	 * slots are of one camp of the match's configuration (HOLVictoryRules::GetCamp: the same team 1..4); else an enemy.
	 * No neutral unit exists yet.
	 */
	HEROESOFLIBERTY_API EHOLSelectionSide GetSide(const FHOLMatchConfig& Config, int32 OwnerSlot, int32 LocalSlot);

	/**
	 * The first side present among Sides (D.1): Own before Ally before Enemy before Neutral; false (OutSide untouched) when
	 * Sides is empty.
	 */
	HEROESOFLIBERTY_API bool GetTopSide(TConstArrayView<EHOLSelectionSide> Sides, EHOLSelectionSide& OutSide);

	/** Indices (in their order) of the units of the first side present among Sides (D.1, GetTopSide); none if empty. */
	HEROESOFLIBERTY_API void KeepTopSide(TConstArrayView<EHOLSelectionSide> Sides, TArray<int32>& OutIndices);

	/**
	 * What a box selects (D.1) from the boxed units (and their sides) and, with Shift (bAdd), the selection so far
	 * (Selected, all of the side SelectedSide). The boxed units keep their first side present only. Without Shift, or
	 * nothing selected: those. With Shift, the first side of the whole wins: the boxed ones of a better side than the
	 * selection replace it, of a worse side (or none boxed) leave it as is; of its side, they are added to it, or removed
	 * from it when all of them already are (Shift + box on own units, as before D.1); without bRemoveWhenAll, only added
	 * (Shift + double click, A.1 and D.2).
	 */
	HEROESOFLIBERTY_API void CombineBox(TConstArrayView<uint32> Selected, EHOLSelectionSide SelectedSide, TConstArrayView<uint32> Boxed,
		TConstArrayView<EHOLSelectionSide> BoxedSides, bool bAdd, TArray<uint32>& OutUnits, bool bRemoveWhenAll = true);

	/**
	 * What a click on the unit Clicked (of the side ClickedSide) selects (D.2): itself alone without Shift (bAdd) or with
	 * nothing selected; with Shift, as a box on it alone (CombineBox): of a better side than the selection, itself alone;
	 * of a worse side, the selection as is; of its side, added, or removed when it already is selected.
	 */
	HEROESOFLIBERTY_API void CombineClick(TConstArrayView<uint32> Selected, EHOLSelectionSide SelectedSide, uint32 Clicked,
		EHOLSelectionSide ClickedSide, bool bAdd, TArray<uint32>& OutUnits);

	/**
	 * The "." key (D30, J5b): of the idle workers (identifiers in increasing order), the first one after Last, or the
	 * first of all after the last one or when Last is not one of them; 0 when there is none. Each press goes to the next.
	 */
	HEROESOFLIBERTY_API uint32 NextInCycle(TConstArrayView<uint32> SortedIds, uint32 Last);
}
