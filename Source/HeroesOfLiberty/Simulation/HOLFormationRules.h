// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Pure rules of the simple formation (J7a, J7-Q3), on the ground plane (cm). A group's move order makes it arrive in
 * rows across the march: how wide the rows are (one shape for Peasants, a square for one kind of soldier, odd rows for
 * several kinds, or the width the right-button drag gives), which kind of soldier stands where (the kind in front
 * first, a shared row with the new kind in its centre and the kind that ends there on its wings; the shooters always
 * behind the melee, the siege in rows of its own at the back), where the places are in the world, and which unit goes
 * to which place without two paths crossing. No engine state: tested on their own.
 */
namespace HOLFormation
{
	/**
	 * Rows of Peasants (the user's rule, an exception: soldiers never use it): up to 5, one row; beyond, rows of
	 * ceil(sqrt(2 N)), full from the front, the last one shorter (10 -> 5, 5; 15 -> 6, 6, 3; 20 -> 7, 7, 6).
	 * OutWidths holds the number of places of each row, front first.
	 */
	HEROESOFLIBERTY_API void MakePeasantRows(int32 Count, TArray<int32>& OutWidths);

	/**
	 * Rows of a single kind of soldier (the user's rule, 2026-10-20): W the smallest odd number at least sqrt(N), full rows
	 * of W from the front, the remainder in a shorter last row (12 -> 5, 5, 2; 20 -> 5 x 4; 50 -> 9 x 5, 5; 2 -> 2).
	 */
	HEROESOFLIBERTY_API void MakeSquareRows(int32 Count, TArray<int32>& OutWidths);

	/**
	 * Rows of several kinds of soldiers: W the smallest odd number at least sqrt(N), R = N / W rows of W, a remainder
	 * e = N - R W. When R >= 2 and e <= R, the e / 2 first rows take W + 2 and, e odd, the next one W + 1; otherwise one
	 * more row of e (24 -> 7, 7, 5, 5; 40 -> 9, 9, 8, 7, 7; 60 -> 11, 11, 11, 9, 9, 9; 4 -> 3, 1).
	 */
	HEROESOFLIBERTY_API void MakeMixedRows(int32 Count, TArray<int32>& OutWidths);

	/** Rows of a width the player gave (the right-button drag, in columns): full rows, the last one shorter. */
	HEROESOFLIBERTY_API void MakeRowsOfWidth(int32 Count, int32 Width, TArray<int32>& OutWidths);

	/**
	 * Class of a kind in a mixed formation (the user's rule, 2026-10-22): melee, shooters (HOLCombat::IsRangedAttack),
	 * siege (EHOLArmorCategory::Siege), then the Peasants (workers) with soldiers. Read from the kind's row priority
	 * (GetRowClass), so the kinds sorted by priority are sorted by class.
	 */
	enum class ERowClass : uint8
	{
		Melee,
		Ranged,
		Siege,
		Worker,
	};

	/**
	 * Which kind stands on each place. CountOfKind holds the number of units of each kind, indexed by row priority
	 * (GetRowPriority: Spearman, Man-at-arms, other melee, Archer, other shooters, siege, Peasants; zero counts allowed).
	 * InOutRowWidths holds the rows the group's shape gives (MakeGroupRows, or MakeRowsOfWidth for the drag); the kinds
	 * fill them in priority order, front to back, under the classes' rule (the user's rule, 2026-10-22):
	 * - the front row holds only melee when the group has some (cut to the melee's number when it is smaller), so a
	 *   shooter always stands behind the melee; from the second row on, a row may hold both;
	 * - the siege never shares a row: it starts a row of its own behind the others, and the Peasants after it too;
	 * - what does not fit because of that goes to more rows at the back, as wide as the next-to-last row given (the last
	 *   one shorter); without a cut (one class, or none needed), the rows stay as given.
	 * On return, InOutRowWidths holds the rows as filled. In a row, the places are taken from the centre outwards: the
	 * class further back first (shooters in the centre, melee on the wings), and within a class by the kinds that start
	 * in it (by priority), then by the kind that ends in it (started in a row in front), on the wings; an odd wing has its
	 * extra unit on the left. OutKindOfPlace is row by row, front first, each row from left to right; it holds an index
	 * into CountOfKind (INDEX_NONE on a place left over when the rows hold more places than units).
	 */
	HEROESOFLIBERTY_API void PlaceKinds(TArray<int32>& InOutRowWidths, TConstArrayView<int32> CountOfKind, TArray<int32>& OutKindOfPlace);

	/**
	 * The places in the world, in the order of PlaceKinds (row by row, front first, each from left to right). Target is
	 * the middle of the front row (the point clicked, or the middle of the drag: the front stands there); Forward is
	 * the march's direction (normalised here). Each row is centred on the march's axis, ColumnSpacing between two
	 * places of a row (an even row has its places at +-0.5 spacing), RowSpacing between two rows.
	 */
	HEROESOFLIBERTY_API void MakePlaces(TConstArrayView<int32> RowWidths, const FVector2D& Target, const FVector2D& Forward,
		double ColumnSpacing, double RowSpacing, TArray<FVector2D>& OutPlaces);

	/**
	 * Gives the units of one kind the places of that kind by depth (the user's rule, J7c.2: on a half turn the last row
	 * turns on the spot, the one before it fills the new front row, and so on). The places are grouped in rows by their
	 * depth along Forward (10 cm apart at most in a row; rows cut short or with holes are fine); the rows are filled front
	 * first by the units furthest forward along Forward. Units within 60 cm of the depth of the last one a row takes are
	 * a tie: the row takes those nearest across to its places the surer units leave free (each surer unit, from left to
	 * right, takes the free place nearest across), then the one further forward. In a row, units and places are paired
	 * by their order from left to right (a row with more places than units leaves free those making the paths shortest),
	 * then two units of the row swap while that makes their paths shorter in total, so no two paths of a row cross
	 * (D230: two units whose paths cross go through each other); paths of different rows may cross on a sidestep.
	 * Positions and Places are world points (as many places as units, or more: the extra ones stay free, at the back).
	 * OutPlaceOfUnit[i] is the index in Places given to unit i.
	 */
	HEROESOFLIBERTY_API void AssignPlaces(TConstArrayView<FVector2D> Positions, TConstArrayView<FVector2D> Places, const FVector2D& Forward,
		TArray<int32>& OutPlaceOfUnit);

	/**
	 * Gaps of a group's formation (J7a.4), in body diameters of its largest unit: across the march, a diameter and a
	 * quarter (150 cm for the table's 60 cm radius, the honeycomb of D287 was 1.2); between two rows, a diameter and a
	 * half (180 cm), so the rows read as rows.
	 */
	inline constexpr double ColumnSpacingInDiameters = 1.25;
	inline constexpr double RowSpacingInDiameters = 1.5;

	/**
	 * The shape of a group's rows (J7-Q3), from the number of units of each kind, sorted by row priority (zero counts
	 * allowed): Peasants alone (bPeasantsOnly), their rule; a single kind of soldier, a square; several kinds, the
	 * rows of several kinds (Peasants with soldiers are one more kind, the last one: behind the soldiers).
	 */
	HEROESOFLIBERTY_API void MakeGroupRows(TConstArrayView<int32> CountOfKind, bool bPeasantsOnly, TArray<int32>& OutWidths);

	/**
	 * A group's places (J7a.4): its rows (MakeGroupRows, as PlaceKinds fills them), the kinds on them (PlaceKinds), the places in the world
	 * (MakePlaces, Target in the middle of the front row), each brought where a unit can stand by ProjectPlace (the
	 * navmesh, called once per place in the order of OutPlaces), then each kind's units given its places
	 * (AssignPlaces, from the projected places). KindOfUnit[i] is the kind of unit i, an index into the kinds sorted by
	 * row priority. OutPlaces: the projected places; OutPlaceOfUnit[i]: the index in OutPlaces given to unit i.
	 */
	HEROESOFLIBERTY_API void PlaceGroup(TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit, bool bPeasantsOnly,
		const FVector2D& Target, const FVector2D& Forward, double ColumnSpacing, double RowSpacing,
		TFunctionRef<FVector2D(const FVector2D&)> ProjectPlace, TArray<FVector2D>& OutPlaces, TArray<int32>& OutPlaceOfUnit,
		int32 FrontWidth = 0);

	/**
	 * The same, the places an obstacle takes (a resource, a building: IsFree false) first settled around it
	 * (SettleBlockedPlaces), so the formation stays on Target and wraps around the obstacle; ProjectPlace then brings
	 * every place where a unit can stand (a settled one is free, so stays there; one left blocked falls back on
	 * ProjectPlace's own rule).
	 */
	HEROESOFLIBERTY_API void PlaceGroup(TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit, bool bPeasantsOnly,
		const FVector2D& Target, const FVector2D& Forward, double ColumnSpacing, double RowSpacing,
		TFunctionRef<bool(const FVector2D&)> IsFree, TFunctionRef<FVector2D(const FVector2D&)> ProjectPlace,
		TArray<FVector2D>& OutPlaces, TArray<int32>& OutPlaceOfUnit, int32 FrontWidth = 0);

	/**
	 * Settling of the formation places an obstacle takes (the user's option A, MV.5): each blocked place goes to the
	 * nearest free point, at least SettleMinGapInSpacings spacings from every other place, so no two units pile up and
	 * the formation wraps around the obstacle. Blocked places are settled in the order of Places (front rows first, as
	 * MakePlaces gives them), each looking on rings around its own place (half a spacing apart, a point every half
	 * spacing along each ring, up to SettleMaxRadiusInSpacings spacings away); on the nearest ring with a point that
	 * keeps the gap from the unblocked places and the ones already settled and that IsFree, the point nearest Anchor
	 * (the formation's target: it stays tight). Unblocked places do not move. OutSettled[i] is true for a blocked place
	 * moved; a blocked place with no such point is left where it was, for the caller's fallback. Returns the number of
	 * places settled.
	 */
	inline constexpr double SettleMinGapInSpacings = 0.9;
	inline constexpr double SettleMaxRadiusInSpacings = 8.;
	HEROESOFLIBERTY_API int32 SettleBlockedPlaces(TArray<FVector2D>& Places, TConstArrayView<bool> IsBlocked, const FVector2D& Anchor,
		double Spacing, TFunctionRef<bool(const FVector2D&)> IsFree, TArray<bool>& OutSettled);

	/**
	 * Row priority of a unit's kind in a group's formation (J7-Q3, the classes of 2026-10-22): the melee first (Spearman
	 * 0, Man-at-arms 1, other melee 2), then the shooters (bRanged: Archer 3, others 4), the siege (bSiege) 5, then the
	 * Peasants (workers: CarryCapacity > 0) PeasantRowPriority, behind them. Shared by the server's formation and the
	 * drag's preview. GetRowClass: the class of a priority.
	 */
	inline constexpr int32 OtherMeleeRowPriority = 2;
	inline constexpr int32 ArcherRowPriority = 3;
	inline constexpr int32 OtherRangedRowPriority = 4;
	inline constexpr int32 SiegeRowPriority = 5;
	inline constexpr int32 PeasantRowPriority = 6;
	HEROESOFLIBERTY_API int32 GetRowPriority(FName TypeName, int32 CarryCapacity, bool bRanged, bool bSiege);
	inline ERowClass GetRowClass(const int32 RowPriority)
	{
		return RowPriority >= PeasantRowPriority ? ERowClass::Worker : RowPriority >= SiegeRowPriority ? ERowClass::Siege
			: RowPriority >= ArcherRowPriority ? ERowClass::Ranged : ERowClass::Melee;
	}

	/**
	 * The front's width (columns) a right-button drag gives (J7a.4, J7-Q3): one column at each end of the drag and one
	 * every ColumnSpacing between them, so the front row spans the drag at most (floor(Length / ColumnSpacing) + 1),
	 * at least 1, at most Count (the group's units). It replaces the default widths of MakeGroupRows (PlaceKinds then
	 * fills them under the classes' rule, as it does the default ones).
	 */
	HEROESOFLIBERTY_API int32 GetFrontWidth(double Length, double ColumnSpacing, int32 Count);

	/**
	 * Where a group faces after a right-button drag (the user's 2nd trial of J7a.4): the drag is an arrow, from where the
	 * button was pressed (the arrival point, the middle of the front row) to the mouse; the group faces along it (mouse
	 * dragged to the right: facing right). Normalised; +X for a null arrow.
	 */
	HEROESOFLIBERTY_API FVector2D GetFrontFacing(const FVector2D& Arrow);

	/**
	 * The speed a unit of a group in formation walks at (J7a.4, J7-Q3): the slowest speed of the units of its order
	 * still on their way (Slowest, cm/s), when it is slower than its own; 0 or less: no group, its own speed.
	 */
	inline float GetMarchSpeed(const float OwnSpeed, const float Slowest)
	{
		return Slowest > 0.f ? FMath::Min(OwnSpeed, Slowest) : OwnSpeed;
	}

	/**
	 * Back to its place after a push (J7a.4, reported from D293; faster after the user's 1st trial): a unit standing on
	 * its formation place, pushed farther than ReturnDistanceInRadii of its radius from it (60 cm for the table's radius:
	 * half a body), walks back once the push is over: ReturnRestSteps steps in a row (0.25 s at 20 steps a second)
	 * without being pushed, i.e. neither driven farther from its place by more than RestStepDistance in a step nor
	 * shoved more than PushStepDistance (the slow drift of two bodies easing apart, or a slide along its place, is not a
	 * push: it kept "still" false for a second or more). A unit that stopped farther than that from its place (taken,
	 * out of reach) does not hold it; a walk back that ends farther tries again, MaxReturnTries walks in a row at most,
	 * then lets it go: no endless back and forth against another unit.
	 */
	inline constexpr double ReturnDistanceInRadii = 1.;
	inline constexpr double RestStepDistance = 2.;
	inline constexpr double PushStepDistance = 8.;
	inline constexpr int32 ReturnRestSteps = 5;
	inline constexpr int32 MaxReturnTries = 3;

	/** A unit's hold on its formation place, on the server (FHOLMoveOrderFragment). */
	struct FPlaceHold
	{
		/** Where it stood at the last step it was watched. */
		FVector2D LastLocation = FVector2D::ZeroVector;
		/** Steps in a row away from its place, not pushed. */
		int32 RestSteps = 0;
		/** Walks back in a row that ended short of it. */
		int32 FailedReturns = 0;
		/** It holds its place (Slot of its order): its last order was a formation move, and it reached the place. */
		bool bHeld = false;
		/** It walks back to its place. */
		bool bReturning = false;

		void Clear() { *this = FPlaceHold(); }
	};

	/**
	 * Its walk to the place ended (the order's arrival, or the walk back) at Location: it holds the place if it stands
	 * within the return distance of it; else an arrival lets it go, a walk back keeps it for another try (MaxReturnTries
	 * walks back in a row at most).
	 */
	HEROESOFLIBERTY_API void EndWalkToPlace(FPlaceHold& Hold, const FVector2D& Location, const FVector2D& Place, double Radius);

	/**
	 * One step of a standing unit holding its place: true when it should walk back now (away from it, and not pushed for
	 * ReturnRestSteps steps; bReturning is set). A unit on its place, or one still pushed, stays.
	 */
	HEROESOFLIBERTY_API bool WatchPlace(FPlaceHold& Hold, const FVector2D& Location, const FVector2D& Place, double Radius);

	/**
	 * The block march (J7a.4, the user's 1st trial: as in AoE4). A group's move in formation is one block (its rows around
	 * its centre, facing the start of its path) that follows one path at its slowest unit's pace, each unit at its offset
	 * in the block (ahead, to the right) turned with the path, then re-forms on its arrival places. Shorter than
	 * BlockMarchMinDistanceInDepths of the block's size (its larger side, plus a row): straight to the places. Files on
	 * the move, rows at the arrival (playtest note 1 of J7b, the user's 5th trial: nobody slows down on the march, not
	 * even to take its slot): every member walks along the march at the block's pace all the way, its depth its own,
	 * sliding across to its file (its slot's offset across) at FileSlideMaxAngle at most; within BlockLagDistance of
	 * its file it is in it (a member). The block's point never waits for its members; its rows close up at the arrival's
	 * re-forming. A straggler or a re-forming unit walks to its slot at most BlockCatchUpShare of the block's pace (its own
	 * speed at most), to catch up. The path's direction is read over BlockTurnWindow before and after the block's point,
	 * so the block turns smoothly at a corner. FormUpTolerance: a unit stands on its arrival place.
	 */
	inline constexpr double BlockMarchMinDistanceInDepths = 1.;
	inline constexpr double FormUpTolerance = 30.;
	inline constexpr double BlockLagDistance = 40.;
	/**
	 * A member sliding to its file (the user's 5th trial of playtest note 1): across at most this angle (degrees) off the
	 * march, so the eye reads a slide, not a hook. One faster than the pace slides at the pace along (it loses no depth);
	 * the slowest kind (its speed the pace) loses 1 - cos of it along while it slides (6 % at 20 degrees), its depth
	 * following: nothing waits for it.
	 */
	inline constexpr double FileSlideMaxAngle = 20.;
	/** A member walking along the march below this share of the pace counts a slowed step (FBlockReport::SlowedSteps). */
	inline constexpr double BlockSlowedShare = 0.9;
	inline constexpr double BlockCatchUpShare = 1.08;
	inline constexpr double BlockTurnWindow = 200.;
	/**
	 * The block's arrival (the user's 3rd trial of J7a.4, as in AoE4): no rigid turn of the block. Over its last
	 * GetBlockBlendDistance cm each unit slides from its slot to a final place of its own kind (its row group), so the
	 * front stays the front of the new facing: a half turn mirrors the rows in place, a quarter turn is a short re-shuffle;
	 * the units of the order go through each other meanwhile (taking their slots, only units of two kinds do).
	 */
	/**
	 * While the block re-forms at its end (the user's 4th trial of J7a.4), its point slows down with the worst gap instead
	 * of stopping (GetBlockSoftPace): full pace up to BlockLagDistance, nothing from BlockSoftHoldDistance.
	 * BlockJoinDistance: the link of the group's body (FindStragglers).
	 */
	inline constexpr double BlockJoinDistance = 450.;
	/**
	 * Small blocks meeting (playtest note 1 of J7b, the user's rule: each separate packet marches in its own formation, the
	 * blocks merge when they meet, else at the arrival; never waiting nor slowing down, never a half turn on the march):
	 * all the blocks of an order share one formation, so when a smaller block's point (its point on its path) comes within
	 * BlockMergeDistance (cm) of a larger one's, on the march, its slots are about where the larger block's would put them
	 * and it merges into it: its units slide to their files at the larger block's point on the move (FormUp), keeping
	 * their depths (a little less than a row: 150 cm for Men-at-arms).
	 */
	inline constexpr double BlockMergeDistance = 130.;
	inline constexpr double BlockSoftHoldDistance = 160.;
	HEROESOFLIBERTY_API double GetBlockSoftPace(double MaxGap);
	/**
	 * The group's packets (playtest note 1 of J7b, the user's 2nd trial, small blocks: each separate packet marches in its
	 * own formation to the destination). A packet is a cluster of units linked by steps of at most LinkDistance (each unit
	 * of it within that distance of another one of it: a block of any size is one packet, however wide). OutPacketOfUnit[i]
	 * is unit i's packet, OutPacketSizes its units; packets numbered largest first (the body, packet 0), ties by their first
	 * unit. Returns the number of packets.
	 */
	HEROESOFLIBERTY_API int32 FindPackets(TConstArrayView<FVector2D> Positions, double LinkDistance, TArray<int32>& OutPacketOfUnit, TArray<int32>& OutPacketSizes);
	/**
	 * Stragglers (playtest note 1 of J7b: five soldiers waited for a sixth 10 m away before they set off): the units alone
	 * in their packet (FindPackets) outside the body (packet 0), when the body holds two units or more (OutIsStraggler[i]);
	 * each walks its own path to its slot kept in the block's last row. A packet of two or more is a small block of its own
	 * (two clusters of three: two blocks, no straggler); everyone apart: no stragglers, the whole group one block. Returns
	 * the number of stragglers.
	 */
	HEROESOFLIBERTY_API int32 FindStragglers(TConstArrayView<FVector2D> Positions, double LinkDistance, TArray<bool>& OutIsStraggler);
	/**
	 * A slot kept for each straggler (the user's trial of playtest note 1 of J7b: a straggler that came in by the middle
	 * or the side of the block shoved the others, who held it in turn and left it). Slots[i] is the slot first given to
	 * unit i (PlaceGroup's offset in the block: X ahead, Y to the right), so a slot of unit i's kind; Positions the units
	 * in the same frame. Each straggler (IsStraggler) keeps a free slot of its kind in the rearmost row its kind holds,
	 * the one nearest to its side (across the march, from where it comes); in the block's last row (within
	 * RearRowTolerance of its rearmost slot) the block marches with that gap and the straggler walks into it from
	 * behind; in a row ahead (a mixed group, its kind's rows in front), it is queued (OutQueued): it follows behind the
	 * block and takes its place at the arrival only. The units of the body then get the other slots of their kinds
	 * (AssignPlaces from their positions, no two ways of one kind crossing). OutSlotOfUnit[i] is the index in Slots
	 * given to unit i. Returns the number of queued stragglers.
	 */
	inline constexpr double RearRowTolerance = 10.;
	HEROESOFLIBERTY_API int32 ReserveRearSlots(TConstArrayView<FVector2D> Slots, TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit,
		TConstArrayView<bool> IsStraggler, TArray<int32>& OutSlotOfUnit, TArray<bool>& OutQueued);
	/**
	 * A straggler on its way (the user's trial of playtest note 1 of J7b): on its own path at its own speed while far;
	 * within BlockStragglerSteerDistance of its slot (or of its queue's point, a row behind the block), the line there
	 * clear on the navmesh, straight to it at its own speed, through the units of its order; back to a path beyond
	 * BlockStragglerPathDistance, or when the line is cut. Within BlockLagDistance of its slot, it is one of the block.
	 */
	inline constexpr double BlockStragglerSteerDistance = 600.;
	/** The block's report measures the closest two of its members from this step of its life on (0.5 s at 20 steps a second). */
	inline constexpr int32 BlockSpacingFromStep = 10;
	inline constexpr double BlockStragglerPathDistance = 900.;
	/**
	 * Reinforcements at the melee (MV.4.4, the user's rule A): the block marches formed; once one of its members comes
	 * within BlockBreakDistance (centre to centre, ~2 rows' depth) of an enemy alive or of a friend of its side fighting,
	 * the whole block breaks up and each member ends alone on its final leg to its place (no attack: a plain move).
	 */
	inline constexpr double BlockBreakDistance = 400.;
	/**
	 * The safety net (MV.4.4): a member holding the re-forming block this long (2 s at 20 steps a second: its HeldSteps,
	 * farther than BlockSoftHoldDistance from its slot) no longer holds the others; it leaves the block and ends alone.
	 */
	inline constexpr int32 BlockMemberStuckSteps = 40;
	/**
	 * The block breaks up (MV.4.4): on a plain move only, one of its
	 * members' closest enemy or striking friend (cm; negative: none) within BlockBreakDistance.
	 */
	inline bool ShouldBreakBlock(const bool bPlainMove, const double ClosestEnemy, const double ClosestFightingFriend)
	{
		return bPlainMove && ((ClosestEnemy >= 0. && ClosestEnemy <= BlockBreakDistance) || (ClosestFightingFriend >= 0. && ClosestFightingFriend <= BlockBreakDistance));
	}

	/** A friend fighting for ShouldBreakBlock (MV.4.4): one striking (in reach of its target), not one only seeking or nearing it. */
	inline bool IsFightingFriend(const bool bAttacking, const bool bApproaching, const bool bHasTarget)
	{
		return bAttacking;
	}

	/** It no longer holds the others (MV.4.4): held BlockMemberStuckSteps steps, it leaves the block and ends alone. */
	inline bool IsMemberStuck(const int32 Steps)
	{
		return Steps >= BlockMemberStuckSteps;
	}
	/**
	 * The arrival blended into the march (the user's 4th trial): over the last GetBlockBlendDistance of the path (the
	 * block's size + BlockBlendMargin, half the path at most), each unit's target slides from its slot in the block to its
	 * final place (given once by kind when the blend starts), GetBlockBlend of the way at Remaining cm from the end.
	 */
	inline constexpr double BlockBlendMargin = 250.;
	HEROESOFLIBERTY_API double GetBlockBlendDistance(double BlockSize, double PathLength);
	HEROESOFLIBERTY_API double GetBlockBlend(double Remaining, double BlendDistance);
	/**
	 * The units' final places at a block's arrival: PlaceOfUnit[i] is the place unit i holds (one each), KindOfUnit[i] its
	 * kind; the places of each kind are given again to its units by AssignPlaces from where they stand (Positions,
	 * Forward the final facing): by depth, so no two straight ways of one kind and one row cross. OutPlaceOfUnit[i] is an index into Places.
	 */
	HEROESOFLIBERTY_API void AssignPlacesByKind(TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit,
		TConstArrayView<FVector2D> Places, const FVector2D& Forward, TArray<int32>& OutPlaceOfUnit);

	/** Length (cm) of a polyline on the ground. */
	HEROESOFLIBERTY_API double GetLineLength(TConstArrayView<FVector2D> Points);

	/** The point Distance along a polyline (clamped to its ends); its first point for a single one, zero for none. */
	HEROESOFLIBERTY_API FVector2D GetPointOnLine(TConstArrayView<FVector2D> Points, double Distance);

	/**
	 * The polyline's direction at Distance: from the point Window before it to the point Window after it (clamped to the
	 * ends), normalised; Fallback when they meet (a single point).
	 */
	HEROESOFLIBERTY_API FVector2D GetDirectionOnLine(TConstArrayView<FVector2D> Points, double Distance, double Window, const FVector2D& Fallback);

	/** An offset in the block (X ahead, Y to the right: right = (-Forward.Y, Forward.X)) turned into the world, and back. */
	HEROESOFLIBERTY_API FVector2D BlockOffsetToWorld(const FVector2D& Offset, const FVector2D& Forward);
	HEROESOFLIBERTY_API FVector2D WorldToBlockOffset(const FVector2D& Delta, const FVector2D& Forward);

	/**
	 * A unit's slot in the marching block: the block's point Distance along its path plus its offset (X ahead, Y to the
	 * right) turned with the path's direction there (GetDirectionOnLine over BlockTurnWindow).
	 */
	HEROESOFLIBERTY_API FVector2D GetSlotOnPath(TConstArrayView<FVector2D> Points, double Distance, const FVector2D& Offset, const FVector2D& Fallback);

	/**
	 * The block's corridor along its path (MV.5, the user's rule: "B if the detour is not too long, otherwise A"), so its
	 * files clear an obstacle ahead of time instead of finding it on contact. Samples every SampleStep cm along the path;
	 * FreeLeft[i] / FreeRight[i] the free distances (cm, >= 0) from the path's centre line across, to the left and to the
	 * right (a unit's centre can stand there: the navmesh is eroded by its radius). The block's files span
	 * [OffsetMinY, OffsetMaxY] across (Y to the right, as BlockOffsetToWorld). A file at offset Y walks at
	 * OutShift[i] + OutSqueeze[i] * Y across: B, the whole block shifts sideways (by as little as fits, MaxShift at
	 * most); A, where that is not enough, the files squeeze (the largest squeeze in [0, 1] that fits with such a shift).
	 * Both then ramp along the path, never faster than MaxSlope across per cm along for the outermost file (anticipation
	 * before the obstacle, recovery after), the shift kept inside each sample's free interval when the slope allows it.
	 */
	inline constexpr double CorridorSampleStep = 100.;
	inline constexpr double CorridorMinShift = 300.;
	inline constexpr double CorridorMaxReach = 1000.;
	inline constexpr double CorridorMargin = 20.;
	HEROESOFLIBERTY_API void MakeCorridor(TConstArrayView<double> FreeLeft, TConstArrayView<double> FreeRight, double SampleStep,
		double OffsetMinY, double OffsetMaxY, double MaxShift, double MaxSlope, TArray<double>& OutShift, TArray<double>& OutSqueeze);

	/** The corridor's shift and squeeze at Distance along the path (linear between samples, clamped; none: 0 and 1). */
	HEROESOFLIBERTY_API void GetCorridorAt(TConstArrayView<double> Shift, TConstArrayView<double> Squeeze, double SampleStep, double Distance,
		double& OutShift, double& OutSqueeze);
}
