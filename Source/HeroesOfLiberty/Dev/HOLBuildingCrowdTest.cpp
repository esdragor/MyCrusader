// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Containers/Ticker.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLStartSlot.h"
#include "Match/HOLPlayerState.h"
#include "MassEntitySubsystem.h"
#include "Math/RandomStream.h"
#include "Player/HOLPlayerController.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLUnitView.h"

/**
 * A group on an enemy House (the user's trial of 2026-10-25: Men-at-arms sent on a House melt into one mass at one point
 * of its wall instead of spreading along it). Each repetition: an enemy House (slot 1, hol.Cheat.SpawnBuilding's path)
 * beyond the player's ridge, a group of the player's (6 to 12, random) born at a random angle and distance from it, then
 * ordered on it as the player's right click does (RequestAttackBuilding: command queue, order processor, combat and
 * movement as in a match). The House keeps its health (RestoreHealth each frame) so it stands until the measure.
 * EtapeDefini O.7 (the user's rule of 2026-10-28, from an AoE4 clip, replacing the exact places of J and O.5): a crowd, no
 * place. Each melee walks to the building and strikes as soon as it is in reach (D497, 400 cm), those behind push it
 * towards the wall (never outwards), one blocked out of reach behind the crowd slides sideways to a spot in reach, so the
 * crowd packs against the wall and spreads round the sides as it grows; no pile, no back-and-forth.
 * From its birth on (Shift: the fall), each unit is followed every frame (its path, when it first comes in reach, when it first strikes, how
 * far it moves once striking). Once every unit of the group strikes, plus SettleSeconds (or at TimeoutSeconds), the measure:
 *   (1) every unit strikes (Attacking) and stands within its type's building reach of the wall (+ ReachSlack);
 *   (2) no two units closer than MinPairFactor x radius (no pile: the original defect of O, bodies at 0-23 cm);
 *   (3) no body in the visible wall (EtapeDefini J, trial of 2026-10-26): the signed distance from each body's edge to the
 *       visible shape (the body widened by BodyInset - VisualInset, D324) at least -VisibleWallSlack;
 *   (4) no detour (the user's 2nd trial: "il va faire des allers vers un autre point alors que son point de départ était
 *       valable"): (4a) the path walked from the order to its first strike at most DetourFactor x the straight line from its
 *       start to where it first struck, plus DetourSlackRadii radii, for a unit with free room (see (5b); one that met the
 *       crowd first goes round it by the rule, its path logged only); (4b) once striking, no walk away: its move from where it
 *       first struck during its own walk in (FHOLCombatFragment::bApproaching), less its part towards the wall, at most
 *       WanderRadii radii; its move while standing (shoved along the wall by the crowd packing, the rule) is logged only;
 *   (5) quick: (5a) every unit strikes within StrikeDelaySeconds of first coming in reach (rule (1): no walk on to a place);
 *       (5b) a unit with free room (no striking friend within CrowdRadii radii of it before it came in reach) strikes
 *       within QuickFactor x its straight walk's time (straight line / the type's Speed) plus QuickSlackSeconds, from
 *       the start of its walk (by itself at rest, or at the order);
 *   (6) packed, "pushed towards the building": a unit leaned on from behind (a friend farther from the visible wall by half a
 *       radius or more, within 2 radii + PackSlack) stands at the wall (its edge within PackedGap of the visible wall) or
 *       leans itself on a friend nearer the wall; a unit alone, or the crowd's last row, may stand where it came in reach;
 *   (7) overflow, logged only (O.7b loop 9): the rows deep off the visible wall (a row's edges 2 radii apart, row 1's within
 *       a radius of the wall; D497: three rows strike): a staggered crowd counts a 4th at ~310 cm, still in reach (315);
 *       (1) fails a unit out of reach. A House's three rings hold ~80 Men-at-arms, beyond any group here (see -Group).
 *   Before O.7 (O.5's blocks): three rows exactly, a front of a third of the group centred on the arrival, rows 2-3 behind
 *   its columns, one front arc at contact without hole: all dropped with the places (a crowd has no columns).
 * EtapeDefini O: the same measure when the group takes the House on WITHOUT a direct order (Mode, groups of 12 to 20):
 *   Order      the right click above (RequestAttackBuilding), groups of 6 to 12 (EtapeDefini J's test, unchanged);
 *   AttackMove an attack-move onto the House's middle (RequestAttackMove, as the player's A + click): no enemy unit about,
 *              each unit takes the House on by itself on its walk (UHOLCombatProcessor's building search, AttackMove mode);
 *   Idle       no order at all: the group born at rest near the House, within its aggression radius (the search at rest);
 *   Shift      a right click on a first House, then Shift + right click on a second one (RequestAttackBuilding with
 *              bQueue); once all strike the first one it falls (hol.Cheat.DamageBuilding's path) and the second is measured
 *              (each unit followed from the fall on).
 *   hol.Test.BuildingCrowd <Count> [Seed=1] [Type=ManAtArms] [Mode=Order|AttackMove|Idle|Shift] [group=N] [quit]
 * group=N: every group of N units (O.7: 30 to 40 for the overflow (7)); else the sizes above.
 * One line per repetition ("BuildingCrowd N/M: ... OK" or "... FAIL: why"), then "BuildingCrowd: done, M runs, F failed".
 * Positions in the frame of the player's centre: X towards the map's middle, Y across (as hol.Test.Crossings).
 */
namespace UE::HeroesOfLiberty::Dev::BuildingCrowd
{
	/** The Houses' places (frame, cm), one per repetition in turn: 30 m apart, beyond the ridge (X 3000). */
	const FVector2D Places[] = { FVector2D(6500., -2500.), FVector2D(9500., 2500.), FVector2D(6500., 2500.), FVector2D(9500., -2500.) };
	constexpr int32 MinGroup = 6;
	constexpr int32 MaxGroup = 12;
	/** EtapeDefini O: the modes without a direct order, bigger groups (the user's ~20 in attack-move) so a pile shows. */
	constexpr int32 MinBigGroup = 12;
	constexpr int32 MaxBigGroup = 20;
	/** Shift: the House ordered first, this far (cm) across from the measured (queued) one, towards the frame's middle line. */
	constexpr double SecondHouseOffset = 1600.;
	/** Idle: the group born this far (cm) from the House's middle at least (its body clear), within its aggression radius. */
	constexpr double MinIdleStart = 700.;
	/** Shift: after the fall, the group counted as striking the queued House only from then on (s). */
	constexpr double ShiftSettleDelay = 1.;
	/** The group's birth: from the House's middle, at this distance (cm), and its spread (radius). */
	constexpr double MinStart = 1200.;
	constexpr double MaxStart = 2200.;
	constexpr double BirthSpread = 250.;
	constexpr double OrderDelay = 1.;
	constexpr double SettleSeconds = 2.;
	constexpr double TimeoutSeconds = 25.;
	constexpr double ReachSlack = 10.;
	constexpr double MinPairFactor = 1.5;
	/**
	 * (3): a body may stand this far (cm) in the visible wall: the guard's 1 cm and a crowd's few cm of push, a sixth of a
	 * radius, unseen from the camera; the trial's bodies stood 40 to 85 cm in it.
	 */
	constexpr double VisibleWallSlack = 10.;
	/**
	 * (4a) the path to the first strike at most DetourFactor x the straight line, plus DetourSlackRadii radii. The slack: a
	 * group born in a disc of BirthSpread walks through itself at the start, and a friend's body is passed by a side step of
	 * about a radius each way (2 radii). The factor: a unit that slides sideways at the crowd's back (rule (3)) walks an L
	 * (in, then along), the straight line its hypotenuse: a slide of 3/4 of its walk in is ~1.25 (a slide of a column, 132 cm,
	 * after 1000 cm in is 1.02); a walk to another place and back, the trial's defect, is 1.5 or more on short walks.
	 */
	constexpr double DetourFactor = 1.25;
	constexpr double DetourSlackRadii = 2.;
	/**
	 * (4b) once striking, its moves sideways (each frame's, its part towards the wall left out) while its own walk in is still
	 * on, at most this many radii (90 cm for a Man-at-arms): summed frame by frame, the separation's shoves as it presses in
	 * between two friends (O.7b loop 6: 64-77 cm with the walk itself straight in); a walk along the crowd to another spot
	 * (a column away = 2.2 radii, loop 4: 94-206 cm) fails. Shoved while standing (the crowd packing along the wall, the
	 * user's rule from the AoE4 clip) is logged, not judged.
	 */
	constexpr double WanderRadii = 1.5;
	/**
	 * (5a) in reach (its edge within the type's building reach of the wall, as the combat measures it), it strikes within
	 * this (s): the combat looks for a target every SearchSteps steps (0.2 s at 20 Hz) and starts striking the step it is in
	 * reach; plus a frame or two of this test's sampling. A walk on from 400 cm to a place at 95 cm (O.5's front row, 0.6 to
	 * 0.7 s at a Man-at-arms' speed, measured: Order seed 1) fails, as a wait for a place. Shift: ShiftStrikeDelaySeconds
	 * more (O.7b loop 9: 0.52-0.59 s measured): the queue hands the next House to the units at its own step after the fall,
	 * the combat finds it at its next search step, then strikes: two or three steps, not one.
	 */
	constexpr double StrikeDelaySeconds = 0.5;
	constexpr double ShiftStrikeDelaySeconds = 0.3;
	/**
	 * (5b) a unit with free room strikes within QuickFactor x (straight line to where it first struck / Speed) +
	 * QuickSlackSeconds from the start of its walk (MoveAt): the order's step, the walk's start (speed reached in a few frames) and a search step (1 s in all),
	 * the factor (4a)'s swerves. "Free room": no friend already striking within CrowdRadii radii of it before it came in
	 * reach (half a body's gap between them: one that walks into the crowd's back may be slowed by a push).
	 */
	constexpr double QuickFactor = 1.25;
	constexpr double QuickSlackSeconds = 1.;
	constexpr double CrowdRadii = 2.5;
	/** (5b) its walk started once this far (cm) from its start (a frame or two after it set off). */
	constexpr double MoveGap = 10.;
	/**
	 * (6) packed: "at the wall" = its edge within PackedGap (cm, two thirds of a Man-at-arms' radius) of the visible wall:
	 * the keep-out lets bodies stand from 1 cm off it and eases them out of it, a crowd's press a few tens of cm. Leaning =
	 * within 2 radii + PackSlack (half a radius: bodies pushed together touch within the separation's few cm; 2.2 radii
	 * apart sideways still lean), and nearer the wall (or farther, for one behind) by LeanRadii radii or more (a neighbour
	 * at the same depth does not push it in).
	 */
	constexpr double PackedGap = 40.;
	constexpr double PackSlackRadii = 0.5;
	constexpr double LeanRadii = 0.5;
	/** (7) the rows deep expected (D497: three rings strike, reach 400 cm = 315 cm off the visible wall); logged only. */
	constexpr int32 BlockRows = 3;

	enum class EPhase : uint8 { Spawn, WaitBirth, Walk, Pause };
	enum class EMode : uint8 { Order, AttackMove, Idle, Shift };

	const TCHAR* ModeName(const EMode Mode)
	{
		return Mode == EMode::AttackMove ? TEXT("AttackMove") : Mode == EMode::Idle ? TEXT("Idle") : Mode == EMode::Shift ? TEXT("Shift") : TEXT("Order");
	}

	/**
	 * (4) and (5): one unit followed from its birth on (Shift: from the first House's fall), times in s from then, world
	 * positions. From the birth, not the order: a group born within its aggression radius of the House (1600 cm for a
	 * Man-at-arms, births at 1200-2200 cm from its middle) takes it on by itself during OrderDelay (the search at rest).
	 */
	struct FTrack
	{
		FVector2D Start = FVector2D::ZeroVector;
		FVector2D Last = FVector2D::ZeroVector;
		/** When it was first seen. */
		double FirstAt = 0.;
		/** When it first stood MoveGap away from its start (-1: never): its walk's start, by itself or at the order. */
		double MoveAt = -1.;
		/** The path walked until its first strike. */
		double Path = 0.;
		/** When its edge first came within the building reach of the wall (-1: never). */
		double InReachAt = -1.;
		/** When it first struck (-1: not yet), where, its edge's distance to the wall there. */
		double StrikeAt = -1.;
		FVector2D StrikePos = FVector2D::ZeroVector;
		double StrikeEdge = 0.;
		/** Once striking: its farthest move from StrikePos, its part towards the wall left out (log only). */
		double Wander = 0.;
		/** Once striking: its moves sideways (each frame's, its part towards the wall left out) during its own walk in (4b). */
		double Walked = 0.;
		/** Once striking: its moves sideways while standing: shoved by the crowd (log only). */
		double Shoved = 0.;
		/** Frames not striking once struck (log only). */
		int32 Lapses = 0;
		/** A friend already striking within CrowdRadii of it before it came in reach: no free room, (5b) not asked. */
		bool bCrowded = false;
	};

	struct FRun
	{
		int32 Count = 0;
		FString Type = TEXT("ManAtArms");
		EMode Mode = EMode::Order;
		/** group=N: every group of N units (0: the mode's random sizes). */
		int32 FixedGroup = 0;
		/** The type's walk speed (cm/s), (5b). */
		double Speed = 0.;
		TMap<uint32, FTrack> Tracks;
		/** When the units' follow started (the group's birth; Shift: the fall), -1 before. */
		double FollowStart = -1.;
		double Aggression = 0.;
		bool bQuit = false;
		FRandomStream Random;
		int32 Slot = 0;
		int32 EnemySlot = 1;
		double Reach = 0.;
		FVector2D Centre = FVector2D::ZeroVector;
		double CentreZ = 0.;
		FVector2D Radial = FVector2D(1., 0.);
		FVector2D Across = FVector2D(0., 1.);
		EPhase Phase = EPhase::Spawn;
		int32 Trip = 0;
		int32 GroupSize = 0;
		double Angle = 0.;
		double StartDistance = 0.;
		FVector2D HousePoint = FVector2D::ZeroVector;
		TWeakObjectPtr<AHOLBuilding> House;
		/** Shift: the House ordered first (felled once all strike it); House is the queued one, measured. */
		TWeakObjectPtr<AHOLBuilding> FirstHouse;
		FVector2D FirstHousePoint = FVector2D::ZeroVector;
		bool bFirstFelled = false;
		/** The group's middle (world) when it took the House on: at the order, at rest at the start, after the fall (Shift); log only since O.7. */
		FVector2D ArrivalFrom = FVector2D::ZeroVector;
		TSet<uint32> Before;
		TArray<uint32> Group;
		double PhaseStart = 0.;
		double AllStrikingSince = -1.;
		int32 Failed = 0;
		TArray<FString> Failures;
		double SumMinPair = 0.;
		FTSTicker::FDelegateHandle Ticker;

		FVector2D ToWorld(const FVector2D& Frame) const { return Centre + Radial * Frame.X + Across * Frame.Y; }
	};
	TSharedPtr<FRun> Current;

	UWorld* GetGameWorld()
	{
		UWorld* GameWorld = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->IsGameWorld())
			{
				GameWorld = Context.World();
			}
		}
		return GameWorld;
	}

	void Finish(FRun& Run, UWorld* World)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("BuildingCrowd: done, %d runs, %d failed, mean closest pair %.0f cm%s%s"), Run.Trip, Run.Failed,
			Run.Trip > 0 ? Run.SumMinPair / Run.Trip : 0., Run.Failures.IsEmpty() ? TEXT("") : TEXT(": "), *FString::Join(Run.Failures, TEXT(", ")));
		FTSTicker::GetCoreTicker().RemoveTicker(Run.Ticker);
		if (Run.bQuit && World)
		{
			GEngine->Exec(World, TEXT("quit"));
		}
	}

	/** Distance from P to the box (0 inside). */
	double DistanceToBox(const FVector2D& P, const FBox2D& Box)
	{
		const FVector2D Nearest(FMath::Clamp(P.X, Box.Min.X, Box.Max.X), FMath::Clamp(P.Y, Box.Min.Y, Box.Max.Y));
		return FVector2D::Distance(P, Nearest);
	}

	/** Signed distance from P to the box: positive outside, negative inside (to its nearest side). */
	double SignedDistanceToBox(const FVector2D& P, const FBox2D& Box)
	{
		if (P.X > Box.Min.X && P.X < Box.Max.X && P.Y > Box.Min.Y && P.Y < Box.Max.Y)
		{
			return -FMath::Min(FMath::Min(P.X - Box.Min.X, Box.Max.X - P.X), FMath::Min(P.Y - Box.Min.Y, Box.Max.Y - P.Y));
		}
		return DistanceToBox(P, Box);
	}

	/** The visible shape of the House (D324): its body widened by BodyInset - VisualInset on every side. */
	FBox2D GetVisibleBox(const AHOLBuilding& House)
	{
		const double VisualGap = FMath::Max(0., static_cast<double>(UHOLBuildingSettings::GetBodyInset() - UHOLBuildingSettings::GetVisualInset()));
		return House.GetBodyBounds().ExpandBy(VisualGap);
	}

	/** Length of the round: the box widened by Offset, quarter circles at its corners. */
	double RoundLength(const FBox2D& Box, const double Offset)
	{
		return 2. * ((Box.Max.X - Box.Min.X) + (Box.Max.Y - Box.Min.Y)) + 2. * UE_DOUBLE_PI * Offset;
	}

	/**
	 * Where P stands along the round (cm from the box's Min corner, counter-clockwise): along a side, or by its angle round a
	 * corner (Offset x angle, Offset the round's radius there). A P inside goes onto its nearest side.
	 */
	double RoundCoordinate(FVector2D P, const FBox2D& Box, const double Offset)
	{
		const double W = Box.Max.X - Box.Min.X, H = Box.Max.Y - Box.Min.Y, A = 0.5 * UE_DOUBLE_PI * Offset;
		if (P.X >= Box.Min.X && P.X <= Box.Max.X && P.Y >= Box.Min.Y && P.Y <= Box.Max.Y)
		{
			const double L = P.X - Box.Min.X, R = Box.Max.X - P.X, D = P.Y - Box.Min.Y, U = Box.Max.Y - P.Y;
			const double Least = FMath::Min(FMath::Min(L, R), FMath::Min(D, U));
			P = Least == D ? FVector2D(P.X, Box.Min.Y - 1.) : Least == R ? FVector2D(Box.Max.X + 1., P.Y) : Least == U ? FVector2D(P.X, Box.Max.Y + 1.) : FVector2D(Box.Min.X - 1., P.Y);
		}
		const bool bLeft = P.X < Box.Min.X, bRight = P.X > Box.Max.X, bDown = P.Y < Box.Min.Y, bUp = P.Y > Box.Max.Y;
		if (bDown && !bLeft && !bRight) return P.X - Box.Min.X;
		if (bDown && bRight) { const FVector2D D = P - FVector2D(Box.Max.X, Box.Min.Y); return W + Offset * FMath::Atan2(D.X, -D.Y); }
		if (bRight && !bUp) return W + A + (P.Y - Box.Min.Y);
		if (bRight && bUp) { const FVector2D D = P - Box.Max; return W + A + H + Offset * FMath::Atan2(D.Y, D.X); }
		if (bUp && !bLeft) return W + H + 2. * A + (Box.Max.X - P.X);
		if (bUp && bLeft) { const FVector2D D = P - FVector2D(Box.Min.X, Box.Max.Y); return 2. * W + H + 2. * A + Offset * FMath::Atan2(-D.X, D.Y); }
		if (bLeft && !bDown) return 2. * W + H + 3. * A + (Box.Max.Y - P.Y);
		const FVector2D D = P - Box.Min;
		return 2. * W + 2. * H + 3. * A + Offset * FMath::Atan2(-D.Y, -D.X);
	}

	void LogUnits(const FRun& Run, const TArray<const FHOLUnitView*>& Alive, const AHOLBuilding& House, const FBox2D& Box);

	/** The middle of the group's living units (world). */
	FVector2D GroupMiddle(const FRun& Run, const TArray<FHOLUnitView>& Units)
	{
		FVector2D Sum = FVector2D::ZeroVector;
		int32 Count = 0;
		for (const FHOLUnitView& Unit : Units)
		{
			if (Run.Group.Contains(Unit.UnitId))
			{
				Sum += FVector2D(Unit.Location);
				++Count;
			}
		}
		return Count > 0 ? Sum / Count : Sum;
	}

	/** Follows each unit of the group from the order on, every frame (Tick): (4) and (5)'s data. */
	void FollowUnits(FRun& Run, const TArray<FHOLUnitView>& Units, const AHOLBuilding& House, const double Elapsed)
	{
		const FBox2D Box = House.GetBodyBounds();
		UWorld* World = House.GetWorld();
		const UHOLUnitSubsystem* UnitSubsystem = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
		UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
		TArray<const FHOLUnitView*> Alive;
		for (const FHOLUnitView& Unit : Units)
		{
			if (Run.Group.Contains(Unit.UnitId))
			{
				Alive.Add(&Unit);
			}
		}
		// Shift: the strikes on the felled House still show for a few frames (ShiftSettleDelay).
		const bool bStrikesCount = Run.Mode != EMode::Shift || Elapsed >= ShiftSettleDelay;
		for (const FHOLUnitView* Unit : Alive)
		{
			const FVector2D P(Unit->Location);
			FTrack* Track = Run.Tracks.Find(Unit->UnitId);
			if (!Track)
			{
				Track = &Run.Tracks.Add(Unit->UnitId);
				Track->Start = P;
				Track->Last = P;
				Track->FirstAt = Elapsed;
			}
			const double Edge = DistanceToBox(P, Box) - Unit->Radius;
			const bool bStrikes = bStrikesCount && Unit->Activity == EHOLUnitActivity::Attacking;
			if (Track->MoveAt < 0. && FVector2D::Distance(P, Track->Start) > MoveGap)
			{
				Track->MoveAt = Elapsed;
			}
			if (Track->InReachAt < 0. && Edge <= Run.Reach)
			{
				Track->InReachAt = Elapsed;
			}
			if (Track->StrikeAt < 0.)
			{
				Track->Path += FVector2D::Distance(P, Track->Last);
				if (bStrikes)
				{
					Track->StrikeAt = Elapsed;
					Track->StrikePos = P;
					Track->StrikeEdge = Edge;
				}
				else if (Track->InReachAt < 0. && !Track->bCrowded)
				{
					for (const FHOLUnitView* Other : Alive)
					{
						const FTrack* OtherTrack = Other != Unit ? Run.Tracks.Find(Other->UnitId) : nullptr;
						if (OtherTrack && OtherTrack->StrikeAt >= 0. && FVector2D::Distance(P, FVector2D(Other->Location)) <= CrowdRadii * Unit->Radius)
						{
							Track->bCrowded = true;
							break;
						}
					}
				}
			}
			else
			{
				// (4b) its move since it first struck, less its part towards the wall (a push in is the rule).
				const FVector2D Moved = P - Track->StrikePos;
				const double Inwards = FMath::Clamp(Track->StrikeEdge - Edge, 0., Moved.Size());
				Track->Wander = FMath::Max(Track->Wander, FMath::Sqrt(FMath::Max(0., Moved.SizeSquared() - Inwards * Inwards)));
				// This frame's move sideways: walked (its own walk in still on) or shoved (standing: the crowd packing).
				const FVector2D Step = P - Track->Last;
				const double LastEdge = DistanceToBox(Track->Last, Box) - Unit->Radius;
				const double StepIn = FMath::Clamp(LastEdge - Edge, 0., Step.Size());
				const double Sideways = FMath::Sqrt(FMath::Max(0., Step.SizeSquared() - StepIn * StepIn));
				const FMassEntityHandle Entity = UnitSubsystem ? UnitSubsystem->FindEntity(Unit->UnitId) : FMassEntityHandle();
				const FHOLCombatFragment* Combat = EntitySubsystem && EntitySubsystem->GetEntityManager().IsEntityValid(Entity)
					? EntitySubsystem->GetEntityManager().GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
				(Combat && Combat->bApproaching ? Track->Walked : Track->Shoved) += Sideways;
				Track->Lapses += bStrikesCount && Unit->Activity != EHOLUnitActivity::Attacking ? 1 : 0;
			}
			Track->Last = P;
		}
	}

	void Measure(FRun& Run, const TArray<FHOLUnitView>& Units, const AHOLBuilding& House, const double Elapsed, const bool bTimedOut)
	{
		const FBox2D Box = House.GetBodyBounds();
		TArray<const FHOLUnitView*> Alive;
		for (const FHOLUnitView& Unit : Units)
		{
			if (Run.Group.Contains(Unit.UnitId))
			{
				Alive.Add(&Unit);
			}
		}
		int32 NotStriking = 0;
		int32 OutOfReach = 0;
		double FarthestEdge = 0.;
		for (const FHOLUnitView* Unit : Alive)
		{
			const FVector2D P(Unit->Location);
			const double Edge = DistanceToBox(P, Box) - Unit->Radius;
			FarthestEdge = FMath::Max(FarthestEdge, Edge);
			OutOfReach += Edge > Run.Reach + ReachSlack ? 1 : 0;
			NotStriking += Unit->Activity != EHOLUnitActivity::Attacking ? 1 : 0;
		}
		const double Radius = Alive.IsEmpty() ? 60. : Alive[0]->Radius;
		double MinPair = Alive.Num() < 2 ? 0. : TNumericLimits<double>::Max();
		int32 Piled = 0;
		for (int32 A = 0; A < Alive.Num(); ++A)
		{
			for (int32 B = A + 1; B < Alive.Num(); ++B)
			{
				const double D = FVector2D::Distance(FVector2D(Alive[A]->Location), FVector2D(Alive[B]->Location));
				MinPair = FMath::Min(MinPair, D);
				Piled += D < MinPairFactor * Radius ? 1 : 0;
			}
		}
		// (3) The bodies against the visible wall: each one's signed edge distance to it (negative: inside).
		const FBox2D Visible = GetVisibleBox(House);
		TArray<double> Signed;
		double WorstVisible = Alive.IsEmpty() ? 0. : TNumericLimits<double>::Max();
		double FarthestVisible = 0.;
		int32 InVisibleWall = 0;
		for (const FHOLUnitView* Unit : Alive)
		{
			const double S = SignedDistanceToBox(FVector2D(Unit->Location), Visible) - Unit->Radius;
			Signed.Add(S);
			WorstVisible = FMath::Min(WorstVisible, S);
			FarthestVisible = FMath::Max(FarthestVisible, S);
			InVisibleWall += S < -VisibleWallSlack ? 1 : 0;
		}
		// (4) and (5) from the units' tracks.
		int32 Detours = 0, Wanders = 0, Late = 0, Slow = 0, FreeRoom = 0;
		double WorstDetour = 0., WorstDetourRatio = 0., WorstWander = 0., WorstShoved = 0., WorstDelay = 0., WorstSlow = 0., WorstSlowRatio = 0.;
		double SumStrikeEdge = 0.;
		int32 Struck = 0, Lapsed = 0;
		for (const FHOLUnitView* Unit : Alive)
		{
			const FTrack* Track = Run.Tracks.Find(Unit->UnitId);
			if (!Track || Track->StrikeAt < 0.)
			{
				continue; // (1) already fails it
			}
			++Struck;
			SumStrikeEdge += Track->StrikeEdge;
			Lapsed += Track->Lapses > 0 ? 1 : 0;
			const double Straight = FVector2D::Distance(Track->Start, Track->StrikePos);
			const double Detour = Track->Path - (DetourFactor * Straight + DetourSlackRadii * Radius);
			WorstDetour = FMath::Max(WorstDetour, Track->Path - Straight);
			WorstDetourRatio = FMath::Max(WorstDetourRatio, Track->Path / FMath::Max(Straight, 1.));
			// O.7b loop 9: judged for a unit with free room only; one that met the crowd before coming in reach goes round it
			// to a free column by the rule (the crowd wraps round): its path is logged, not judged.
			Detours += Detour > 1. && !Track->bCrowded ? 1 : 0;
			WorstWander = FMath::Max(WorstWander, Track->Walked);
			WorstShoved = FMath::Max(WorstShoved, Track->Shoved);
			Wanders += Track->Walked > WanderRadii * Radius + 1. ? 1 : 0;
			const double Delay = Track->StrikeAt - FMath::Max(0., Track->InReachAt < 0. ? Track->StrikeAt : Track->InReachAt);
			WorstDelay = FMath::Max(WorstDelay, Delay);
			Late += Delay > StrikeDelaySeconds + (Run.Mode == EMode::Shift ? ShiftStrikeDelaySeconds : 0.) ? 1 : 0;
			if (!Track->bCrowded && Run.Speed > 0.)
			{
				++FreeRoom;
				const double WalkTime = Straight / Run.Speed;
				const double Took = Track->StrikeAt - FMath::Clamp(Track->MoveAt, 0., Track->StrikeAt);
				WorstSlow = FMath::Max(WorstSlow, Took - WalkTime);
				WorstSlowRatio = FMath::Max(WorstSlowRatio, Took / FMath::Max(WalkTime, 0.1));
				Slow += Took > QuickFactor * WalkTime + QuickSlackSeconds ? 1 : 0;
			}
		}
		// (6) Packed: a unit leaned on from behind stands at the wall or leans on a friend nearer it.
		const double Lean = (2. + PackSlackRadii) * Radius;
		int32 Loose = 0;
		double WorstLoose = 0.;
		for (int32 A = 0; A < Alive.Num(); ++A)
		{
			if (Signed[A] <= PackedGap)
			{
				continue;
			}
			bool bLeanedOn = false, bLeans = false;
			for (int32 B = 0; B < Alive.Num(); ++B)
			{
				if (B == A || FVector2D::Distance(FVector2D(Alive[A]->Location), FVector2D(Alive[B]->Location)) > Lean)
				{
					continue;
				}
				bLeanedOn |= Signed[B] >= Signed[A] + LeanRadii * Radius;
				bLeans |= Signed[B] <= Signed[A] - LeanRadii * Radius;
			}
			if (bLeanedOn && !bLeans)
			{
				++Loose;
				WorstLoose = FMath::Max(WorstLoose, Signed[A]);
			}
		}
		// (7) Rows off the visible wall: row 1's edges within a radius of it, each next row 2 radii farther.
		const double Contact = 2. * Radius;
		const int32 Rows = Alive.IsEmpty() ? 0 : 1 + FMath::FloorToInt32(FMath::Max(0., FarthestVisible + Radius) / Contact);
		int32 AtWall = 0;
		TArray<double> Along;
		for (int32 A = 0; A < Alive.Num(); ++A)
		{
			AtWall += Signed[A] <= PackedGap ? 1 : 0;
			Along.Add(RoundCoordinate(FVector2D(Alive[A]->Location), Visible, Radius));
		}
		// The crowd's arc along the round (the round less its largest void), for the log: it grows with the group.
		Along.Sort();
		const double Round = RoundLength(Visible, Radius);
		double Void = Along.IsEmpty() ? Round : Round - Along.Last() + Along[0];
		for (int32 Index = 1; Index < Along.Num(); ++Index)
		{
			Void = FMath::Max(Void, Along[Index] - Along[Index - 1]);
		}
		const double Arc = Along.Num() < 2 ? 0. : Round - Void;
		TArray<FString> Why;
		if (NotStriking > 0)
		{
			Why.Add(FString::Printf(TEXT("(1) %d of %d not striking"), NotStriking, Alive.Num()));
		}
		if (OutOfReach > 0)
		{
			Why.Add(FString::Printf(TEXT("(1) %d out of reach (farthest %.0f cm from the wall, reach %.0f)"), OutOfReach, FarthestEdge, Run.Reach));
		}
		if (Piled > 0)
		{
			Why.Add(FString::Printf(TEXT("(2) %d pairs closer than %.0f cm (closest %.0f cm)"), Piled, MinPairFactor * Radius, MinPair));
		}
		if (InVisibleWall > 0)
		{
			Why.Add(FString::Printf(TEXT("(3) %d bodies in the visible wall (worst %.0f cm, at least %.0f)"), InVisibleWall, WorstVisible, -VisibleWallSlack));
		}
		if (Detours > 0)
		{
			Why.Add(FString::Printf(TEXT("(4a) %d detours (worst path %.0f cm longer than the straight line, ratio up to %.2f; at most x%.2f + %.0f cm)"), Detours, WorstDetour, WorstDetourRatio, DetourFactor, DetourSlackRadii * Radius));
		}
		if (Wanders > 0)
		{
			Why.Add(FString::Printf(TEXT("(4b) %d walked off once striking (worst %.0f cm, at most %.0f)"), Wanders, WorstWander, WanderRadii * Radius));
		}
		if (Late > 0)
		{
			Why.Add(FString::Printf(TEXT("(5a) %d struck late once in reach (worst %.2f s, at most %.2f)"), Late, WorstDelay, StrikeDelaySeconds + (Run.Mode == EMode::Shift ? ShiftStrikeDelaySeconds : 0.)));
		}
		if (Slow > 0)
		{
			Why.Add(FString::Printf(TEXT("(5b) %d of %d with free room slow (worst %.1f s over the straight walk, ratio up to %.2f; at most x%.2f + %.1f s)"), Slow, FreeRoom, WorstSlow, WorstSlowRatio, QuickFactor, QuickSlackSeconds));
		}
		if (Loose > 0)
		{
			Why.Add(FString::Printf(TEXT("(6) %d leaned on but not pushed in (worst %.0f cm off the visible wall, nothing nearer to lean on)"), Loose, WorstLoose));
		}
		// (7) is logged, not judged (O.7b loop 9): a staggered crowd packs a 4th row by this count at ~310 cm off the visible
		// wall, in reach (315); (1) fails a unit out of reach.
		Run.SumMinPair += MinPair;
		UE_LOG(LogHOLMatch, Display, TEXT("BuildingCrowd %d/%d: mode %s, %d stopped striking a while once struck"), Run.Trip, Run.Count, ModeName(Run.Mode), Lapsed);
		UE_LOG(LogHOLMatch, Display, TEXT("BuildingCrowd %d/%d: %d %s, angle %.0f deg, start %.0f cm: %s after %.1f s, striking %d/%d, closest pair %.0f cm, %d piled, farthest %.0f cm from the wall, worst %.0f cm from the visible wall, path up to %.0f cm over straight (x%.2f), walked off up to %.0f cm, shoved up to %.0f cm, strike delay in reach up to %.2f s, free room %d slow up to %.1f s (x%.2f), first strikes %.0f cm from the wall on average, %d at the wall, %d loose, %d rows deep (farthest %.0f cm off the visible wall), arc %.0f of %.0f cm, %s%s"),
			Run.Trip, Run.Count, Alive.Num(), *Run.Type, Run.Angle, Run.StartDistance, bTimedOut ? TEXT("timed out") : TEXT("measured"), Elapsed,
			Alive.Num() - NotStriking, Alive.Num(), MinPair, Piled, FarthestEdge, WorstVisible, WorstDetour, WorstDetourRatio, WorstWander, WorstShoved, WorstDelay,
			FreeRoom, WorstSlow, WorstSlowRatio, Struck > 0 ? SumStrikeEdge / Struck : 0., AtWall, Loose, Rows, FarthestVisible, Arc, Round,
			Why.IsEmpty() ? TEXT("OK") : TEXT("FAIL: "), *FString::Join(Why, TEXT(", ")));
		if (!Why.IsEmpty())
		{
			++Run.Failed;
			Run.Failures.Add(FString::FromInt(Run.Trip));
			LogUnits(Run, Alive, House, Box);
		}
	}

	/** A failed repetition, a line per unit: where it stands, its track ((4), (5)), whether it strikes, whether it walks in (O.7, FHOLCombatFragment). */
	void LogUnits(const FRun& Run, const TArray<const FHOLUnitView*>& Alive, const AHOLBuilding& House, const FBox2D& Box)
	{
		UWorld* World = House.GetWorld();
		const UHOLUnitSubsystem* UnitSubsystem = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
		UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
		if (!UnitSubsystem || !EntitySubsystem)
		{
			return;
		}
		FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
		const FBox2D Visible = GetVisibleBox(House);
		for (const FHOLUnitView* Unit : Alive)
		{
			const FMassEntityHandle Entity = UnitSubsystem->FindEntity(Unit->UnitId);
			const FHOLCombatFragment* Combat = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
			const FVector2D P(Unit->Location);
			const bool bWalkingIn = Combat && Combat->bApproaching;
			const FTrack* Track = Run.Tracks.Find(Unit->UnitId);
			UE_LOG(LogHOLMatch, Display, TEXT("BuildingCrowd %d/%d:   unit %u at (%.0f, %.0f), %.0f cm from the wall, %.0f cm from the visible wall, %s; first seen at %.2f s %.0f cm from the wall; path %.0f cm for %.0f straight, in reach at %.2f s, struck at %.2f s %.0f cm from the wall, walked off %.0f cm, shoved %.0f cm (%.0f cm off its strike point), %s; %s"),
				Run.Trip, Run.Count, Unit->UnitId, P.X, P.Y, DistanceToBox(P, Box) - Unit->Radius, SignedDistanceToBox(P, Visible) - Unit->Radius,
				Unit->Activity == EHOLUnitActivity::Attacking ? TEXT("striking") : TEXT("NOT striking"),
				Track ? Track->FirstAt : -1., Track ? DistanceToBox(Track->Start, Box) - Unit->Radius : -1.,
				Track ? Track->Path : -1., Track && Track->StrikeAt >= 0. ? FVector2D::Distance(Track->Start, Track->StrikePos) : -1.,
				Track ? Track->InReachAt : -1., Track ? Track->StrikeAt : -1., Track ? Track->StrikeEdge : -1., Track ? Track->Walked : -1., Track ? Track->Shoved : -1., Track ? Track->Wander : -1.,
				Track && Track->bCrowded ? TEXT("crowded") : TEXT("free room"), bWalkingIn ? TEXT("walking in (O.7)") : TEXT("standing"));
		}
	}

	bool Tick(const TSharedRef<FRun>& RunRef)
	{
		FRun& Run = *RunRef;
		UWorld* World = GetGameWorld();
		AHOLPlayerController* Controller = World ? Cast<AHOLPlayerController>(World->GetFirstPlayerController()) : nullptr;
		if (!World || !Controller || Current != RunRef)
		{
			return false;
		}
		const double Now = World->GetTimeSeconds();
		TArray<FHOLUnitView> Units;
		HOLUnitView::Gather(*World, Units);
		if (AHOLBuilding* House = Run.House.Get())
		{
			House->RestoreHealth(); // it stands until the measure
		}
		if (AHOLBuilding* First = Run.FirstHouse.Get(); First && !Run.bFirstFelled)
		{
			First->RestoreHealth(); // Shift: it stands until all strike it
		}
		switch (Run.Phase)
		{
		case EPhase::Spawn:
		{
			if (Run.Trip >= Run.Count)
			{
				Finish(Run, World);
				return false;
			}
			++Run.Trip;
			Run.GroupSize = Run.Mode == EMode::Order ? Run.Random.RandRange(MinGroup, MaxGroup) : Run.Random.RandRange(MinBigGroup, MaxBigGroup);
			Run.GroupSize = Run.FixedGroup > 0 ? Run.FixedGroup : Run.GroupSize; // drawn anyway: the same angles and starts for a seed
			Run.Angle = Run.Random.FRandRange(0., 360.);
			// Idle: born within its aggression radius of the wall (the search at rest), else beyond the House's reach.
			Run.StartDistance = Run.Mode == EMode::Idle ? Run.Random.FRandRange(MinIdleStart, FMath::Max(MinIdleStart + 100., 0.5 * Run.Aggression + 300.))
				: Run.Random.FRandRange(MinStart, MaxStart);
			const FVector2D Place = Places[(Run.Trip - 1) % UE_ARRAY_COUNT(Places)];
			Run.HousePoint = Run.ToWorld(Place);
			Run.FirstHousePoint = Run.ToWorld(Place - FVector2D(0., FMath::Sign(Place.Y) * SecondHouseOffset));
			Run.House.Reset();
			Run.FirstHouse.Reset();
			Run.bFirstFelled = false;
			Run.FollowStart = -1.;
			Run.Tracks.Reset();
			Run.Group.Reset();
			Run.Before.Reset();
			for (const FHOLUnitView& Unit : Units)
			{
				Run.Before.Add(Unit.UnitId);
			}
			Controller->RequestCheatSpawnBuilding(FName(TEXT("House")), Run.EnemySlot, FVector(Run.HousePoint, Run.CentreZ));
			if (Run.Mode == EMode::Shift)
			{
				Controller->RequestCheatSpawnBuilding(FName(TEXT("House")), Run.EnemySlot, FVector(Run.FirstHousePoint, Run.CentreZ));
			}
			const double Radians = FMath::DegreesToRadians(Run.Angle);
			const FVector2D Birth = Run.HousePoint + FVector2D(FMath::Cos(Radians), FMath::Sin(Radians)) * Run.StartDistance;
			GEngine->Exec(World, *FString::Printf(TEXT("hol.Units.Spawn %d %.0f %.0f %d %s %.0f"), Run.GroupSize, Birth.X, Birth.Y, Run.Slot, *Run.Type, BirthSpread));
			Run.Phase = EPhase::WaitBirth;
			Run.PhaseStart = Now;
			return true;
		}
		case EPhase::WaitBirth:
		{
			auto FindHouse = [&Run, World](const FVector2D& Point)
			{
				AHOLBuilding* Found = nullptr;
				double Best = 0.5 * SecondHouseOffset;
				for (TActorIterator<AHOLBuilding> It(World); It; ++It)
				{
					const double D = FVector2D::Distance(FVector2D(It->GetActorLocation()), Point);
					if (It->GetOwnerPlayer() == Run.EnemySlot && It->GetHealth() > 0 && D < Best)
					{
						Best = D;
						Found = *It;
					}
				}
				return Found;
			};
			if (!Run.House.IsValid())
			{
				Run.House = FindHouse(Run.HousePoint);
			}
			if (Run.Mode == EMode::Shift && !Run.FirstHouse.IsValid())
			{
				Run.FirstHouse = FindHouse(Run.FirstHousePoint);
			}
			Run.Group.Reset();
			for (const FHOLUnitView& Unit : Units)
			{
				if (!Run.Before.Contains(Unit.UnitId) && Unit.OwnerPlayer == Run.Slot && Unit.TypeName == FName(*Run.Type))
				{
					Run.Group.Add(Unit.UnitId);
				}
			}
			// (4) and (5): followed from the birth on (Shift: from the fall).
			if (Run.Mode != EMode::Shift && Run.House.IsValid() && Run.Group.Num() >= Run.GroupSize)
			{
				Run.FollowStart = Run.FollowStart < 0. ? Now : Run.FollowStart;
				FollowUnits(Run, Units, *Run.House, Now - Run.FollowStart);
			}
			if (Run.House.IsValid() && (Run.Mode != EMode::Shift || Run.FirstHouse.IsValid()) && Run.Group.Num() >= Run.GroupSize && Now - Run.PhaseStart >= OrderDelay)
			{
				// The player's input, as the controller's (EtapeDefini O): a right click, an attack-move, nothing, or a right
				// click then a Shift + right click.
				switch (Run.Mode)
				{
				case EMode::Order:
					Controller->RequestAttackBuilding(Run.Group, Run.House.Get());
					break;
				case EMode::AttackMove:
					Controller->RequestAttackMove(Run.Group, FVector(Run.HousePoint, Run.CentreZ));
					break;
				case EMode::Idle:
					break; // at rest: the search takes the House on by itself
				case EMode::Shift:
					Controller->RequestAttackBuilding(Run.Group, Run.FirstHouse.Get());
					Controller->RequestAttackBuilding(Run.Group, Run.House.Get(), /*bQueue*/ true);
					break;
				}
				Run.ArrivalFrom = GroupMiddle(Run, Units);
				Run.Phase = EPhase::Walk;
				Run.PhaseStart = Now;
				Run.AllStrikingSince = -1.;
			}
			else if (Now - Run.PhaseStart > 10.)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("BuildingCrowd %d/%d: not set up (house %s, %d of %d units)"), Run.Trip, Run.Count,
					Run.House.IsValid() ? TEXT("found") : TEXT("missing"), Run.Group.Num(), Run.GroupSize);
				Finish(Run, World);
				return false;
			}
			return true;
		}
		case EPhase::Walk:
		{
			AHOLBuilding* House = Run.House.Get();
			int32 Alive = 0;
			int32 Striking = 0;
			for (const FHOLUnitView& Unit : Units)
			{
				if (Run.Group.Contains(Unit.UnitId))
				{
					++Alive;
					Striking += Unit.Activity == EHOLUnitActivity::Attacking ? 1 : 0;
				}
			}
			const double Elapsed = Now - Run.PhaseStart;
			// Shift: the strikes on the felled House still show for a few frames, before the queued order starts (next step):
			// counted from ShiftSettleDelay after the fall only (the 1st red run measured the group 2 s after it, half still walking).
			if (Alive > 0 && Striking == Alive && (Run.Mode != EMode::Shift || Elapsed >= ShiftSettleDelay))
			{
				Run.AllStrikingSince = Run.AllStrikingSince < 0. ? Now : Run.AllStrikingSince;
			}
			const bool bSettled = Run.AllStrikingSince >= 0. && Now - Run.AllStrikingSince >= SettleSeconds;
			const bool bTimedOut = Elapsed >= TimeoutSeconds;
			// Shift: the first House felled once the group strikes it (as a fall in a match: its health to 0), the second
			// one (queued) measured from then on.
			if (Run.Mode == EMode::Shift && !Run.bFirstFelled && (Run.AllStrikingSince >= 0. || bTimedOut))
			{
				if (AHOLBuilding* First = Run.FirstHouse.Get())
				{
					UE_LOG(LogHOLMatch, Display, TEXT("BuildingCrowd %d/%d: the first House felled after %.1f s, the queued one next"), Run.Trip, Run.Count, Elapsed);
					Controller->RequestCheatDamageBuilding(First->GetActorLocation(), 1000000, static_cast<uint8>(EHOLAttackType::Melee));
				}
				Run.bFirstFelled = true;
				Run.ArrivalFrom = GroupMiddle(Run, Units);
				Run.Tracks.Reset(); // followed from the fall on
				Run.FollowStart = Now;
				Run.PhaseStart = Now;
				Run.AllStrikingSince = -1.;
				return true;
			}
			// (4) and (5): each unit followed every frame (Shift: from the fall on, the queued House).
			if (House && (Run.Mode != EMode::Shift || Run.bFirstFelled))
			{
				FollowUnits(Run, Units, *House, Now - Run.FollowStart);
			}
			if (!House || (!bSettled && !bTimedOut))
			{
				if (!House)
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("BuildingCrowd %d/%d: the House is gone"), Run.Trip, Run.Count);
					++Run.Failed;
					Run.Failures.Add(FString::FromInt(Run.Trip));
					Run.Phase = EPhase::Pause;
					Run.PhaseStart = Now;
				}
				return true;
			}
			Measure(Run, Units, *House, Elapsed, bTimedOut && !bSettled);
			// Cleared for the next one: the group removed, the House destroyed.
			if (UHOLUnitSubsystem* UnitSubsystem = World->GetSubsystem<UHOLUnitSubsystem>())
			{
				UnitSubsystem->RequestDestroyUnits(Run.Group);
			}
			House->Destroy();
			Run.House.Reset();
			if (AHOLBuilding* First = Run.FirstHouse.Get())
			{
				First->Destroy();
			}
			Run.FirstHouse.Reset();
			Run.Phase = EPhase::Pause;
			Run.PhaseStart = Now;
			return true;
		}
		case EPhase::Pause:
			if (Now - Run.PhaseStart >= 1.)
			{
				Run.Phase = EPhase::Spawn;
			}
			return true;
		}
		return true;
	}

	void Start(const TArray<FString>& Args)
	{
		UWorld* World = GetGameWorld();
		AHOLPlayerController* Controller = World ? Cast<AHOLPlayerController>(World->GetFirstPlayerController()) : nullptr;
		const AHOLPlayerState* PlayerState = Controller ? Controller->GetHOLPlayerState() : nullptr;
		const AHOLStartSlot* StartSlot = PlayerState ? AHOLStartSlot::FindForSlot(World, PlayerState->GetPlayerIndex()) : nullptr;
		if (Args.IsEmpty() || !StartSlot)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Test.BuildingCrowd <Count> [Seed=1] [Type=ManAtArms] [Mode=Order|AttackMove|Idle|Shift] [group=N] [quit], in a match"));
			return;
		}
		TSharedRef<FRun> Run = MakeShared<FRun>();
		Run->Count = FMath::Max(1, FCString::Atoi(*Args[0]));
		Run->Random.Initialize(Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 1);
		Run->Type = Args.Num() > 2 && Args[2] != TEXT("quit") ? Args[2] : TEXT("ManAtArms");
		Run->bQuit = Args.Contains(TEXT("quit"));
		for (const FString& Arg : Args)
		{
			Run->FixedGroup = Arg.StartsWith(TEXT("group=")) ? FMath::Max(1, FCString::Atoi(*Arg.RightChop(6))) : Run->FixedGroup;
		}
		const FString ModeArg = Args.Num() > 3 && Args[3] != TEXT("quit") ? Args[3] : TEXT("Order");
		Run->Mode = ModeArg == TEXT("AttackMove") ? EMode::AttackMove : ModeArg == TEXT("Idle") ? EMode::Idle : ModeArg == TEXT("Shift") ? EMode::Shift : EMode::Order;
		Run->Slot = PlayerState->GetPlayerIndex();
		Run->EnemySlot = Run->Slot == 0 ? 1 : 0;
		const FVector CentreLocation = StartSlot->GetMainCentreLocation();
		Run->Centre = FVector2D(CentreLocation);
		Run->CentreZ = CentreLocation.Z;
		Run->Radial = (-Run->Centre).GetSafeNormal();
		Run->Across = FVector2D(-Run->Radial.Y, Run->Radial.X);
		const UDataTable* Table = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		const FHOLUnitTypeRow* Row = Table ? Table->FindRow<FHOLUnitTypeRow>(FName(*Run->Type), TEXT("BuildingCrowd"), false) : nullptr;
		if (!Row)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("BuildingCrowd: no unit type %s"), *Run->Type);
			return;
		}
		Run->Reach = Row->BuildingAttackRange;
		Run->Aggression = Row->AggressionRadius;
		Run->Speed = Row->Speed;
		UE_LOG(LogHOLMatch, Display, TEXT("BuildingCrowd: %d runs, slot %d's %s (building reach %.0f cm, aggression %.0f cm, speed %.0f cm/s) on slot %d's Houses, mode %s, seed %s"), Run->Count, Run->Slot,
			*Run->Type, Run->Reach, Run->Aggression, Run->Speed, Run->EnemySlot, ModeName(Run->Mode), Args.Num() > 1 ? *Args[1] : TEXT("1"));
		Current = Run;
		Run->Ticker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Run](float) { return Tick(Run); }));
	}

	FAutoConsoleCommand BuildingCrowdCommand(
		TEXT("hol.Test.BuildingCrowd"),
		TEXT("hol.Test.BuildingCrowd <Count> [Seed=1] [Type=ManAtArms] [Mode=Order|AttackMove|Idle|Shift] [group=N] [quit]: a group of random size sent on an enemy House from a random side, again and again; each time checked once all strike (EtapeDefini O.7, a crowd: in reach, no pile, out of the visible wall, no detour nor walk off once striking, strikes as soon as in reach, packed against the wall, three rows deep at most)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&Start));
}
