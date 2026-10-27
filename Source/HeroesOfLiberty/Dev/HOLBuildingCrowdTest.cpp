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
 * movement as in a match). The House keeps its health (RestoreHealth each frame) so it stands until the measure. Once
 * every unit of the group strikes, plus SettleSeconds (or at TimeoutSeconds), the measure:
 *   (1) every unit strikes (Attacking) and stands within its type's building reach of the wall (+ ReachSlack);
 *   (2) no two units closer than MinPairFactor x radius (no pile);
 *   (3) the arc of the wall the group covers at least the room a row of it takes (80 % of 2 radii a unit, the round
 *       at most): a group in one row along the wall, not a knot at one point. Loop 12: measured along the round of (5)
 *       (same round, same projection), not on the body (an arc through a corner: 2 or 3 units in front of one point).
 *   (4) no body in the visible wall (EtapeDefini J, trial of 2026-10-26: half the bodies on the House's red slab): the
 *       signed distance from each body's edge to the visible shape (the body widened by BodyInset - VisualInset, D324) at
 *       least -VisibleWallSlack (negative: inside the House);
 *   (5) one arc at contact, corners included (the user's rule of 2026-10-26, replacing "the whole round, evenly"): the
 *       first rank (the units nearest the wall, as many as the round holds at contact) projected on the round (the visible
 *       shape widened by a radius, quarter circles at the corners) and sorted along it; every gap between neighbours but
 *       the largest one (the void out of the arc) between ContactMinFactor and ContactMaxFactor x 2 radii (no hole, no
 *       pile), and the arc (round less that void) at most ArcFactor x (units - 1) x 2 radii + ArcSlackRadii radii. A first
 *       rank that fills the round has no void: every gap in bounds (the largest one may also hold the round's remainder).
 *   hol.Test.BuildingCrowd <Count> [Seed=1] [Type=ManAtArms] [quit]
 * One line per repetition ("BuildingCrowd N/M: ... OK" or "... FAIL: why"), then "BuildingCrowd: done, M runs, F failed".
 * Positions in the frame of the player's centre: X towards the map's middle, Y across (as hol.Test.Crossings).
 */
namespace UE::HeroesOfLiberty::Dev::BuildingCrowd
{
	/** The Houses' places (frame, cm), one per repetition in turn: 30 m apart, beyond the ridge (X 3000). */
	const FVector2D Places[] = { FVector2D(6500., -2500.), FVector2D(9500., 2500.), FVector2D(6500., 2500.), FVector2D(9500., -2500.) };
	constexpr int32 MinGroup = 6;
	constexpr int32 MaxGroup = 12;
	/** The group's birth: from the House's middle, at this distance (cm), and its spread (radius). */
	constexpr double MinStart = 1200.;
	constexpr double MaxStart = 2200.;
	constexpr double BirthSpread = 250.;
	constexpr double OrderDelay = 1.;
	constexpr double SettleSeconds = 2.;
	constexpr double TimeoutSeconds = 25.;
	constexpr double ReachSlack = 10.;
	constexpr double MinPairFactor = 1.5;
	constexpr double RowFactor = 0.8;
	/**
	 * (4): a body may stand this far (cm) in the visible wall: the guard's 1 cm and a crowd's few cm of push, a sixth of a
	 * radius, unseen from the camera; the trial's bodies stood 40 to 85 cm in it.
	 */
	constexpr double VisibleWallSlack = 10.;
	/**
	 * (5) one arc at contact (the user's rule of 2026-10-26, EtapeDefini J loop 11a). Contact = 2 radii between neighbours
	 * along the round. Each gap of the arc at most ContactMaxFactor x 2 radii (180 cm for a Man-at-arms): room for one body
	 * stopped up to 50 cm off its place (OrderArrivalTolerance) and a crowd's 10 cm of push, while an empty place (240 cm)
	 * fails. At least ContactMinFactor x 2 radii (90 cm = criterion (2)'s pile, 1.5 radii): bodies may press a little, not
	 * stand on each other. The arc (the round less its one void) at most ArcFactor x (units - 1) x 2 radii, plus
	 * ArcSlackRadii radii (each end of the arc a body off its place): grouped, not spread round the House.
	 */
	constexpr double ContactMaxFactor = 1.5;
	constexpr double ContactMinFactor = 0.75;
	constexpr double ArcFactor = 1.3;
	constexpr double ArcSlackRadii = 2.;

	enum class EPhase : uint8 { Spawn, WaitBirth, Walk, Pause };

	struct FRun
	{
		int32 Count = 0;
		FString Type = TEXT("ManAtArms");
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
		// (4) The bodies against the visible wall: the worst signed distance of a body's edge to it (negative: inside).
		const FBox2D Visible = GetVisibleBox(House);
		double WorstVisible = TNumericLimits<double>::Max();
		int32 InVisibleWall = 0;
		TArray<TPair<double, const FHOLUnitView*>> ByWall;
		for (const FHOLUnitView* Unit : Alive)
		{
			const double Signed = SignedDistanceToBox(FVector2D(Unit->Location), Visible) - Unit->Radius;
			WorstVisible = FMath::Min(WorstVisible, Signed);
			InVisibleWall += Signed < -VisibleWallSlack ? 1 : 0;
			ByWall.Emplace(Signed, Unit);
		}
		WorstVisible = Alive.IsEmpty() ? 0. : WorstVisible;
		// (5) The round: the first rank (as many as the round holds at contact, nearest the wall first) along it.
		const double Round = RoundLength(Visible, Radius);
		// (3) The arc covered (loop 12: along the round, the same round and projection as (5), not on the body: an arc at
		// contact through a corner puts 2 or 3 units in front of it, one point on the body): the round less its largest gap
		// between neighbours along it, every unit of the group.
		TArray<double> Coordinates;
		for (const FHOLUnitView* Unit : Alive)
		{
			Coordinates.Add(RoundCoordinate(FVector2D(Unit->Location), Visible, Radius));
		}
		Coordinates.Sort();
		double LargestGap = Coordinates.IsEmpty() ? Round : Round - Coordinates.Last() + Coordinates[0];
		for (int32 Index = 1; Index < Coordinates.Num(); ++Index)
		{
			LargestGap = FMath::Max(LargestGap, Coordinates[Index] - Coordinates[Index - 1]);
		}
		const double Covered = Round - LargestGap;
		const double Needed = FMath::Min(Round, FMath::Max(0, Alive.Num() - 1) * 2. * Radius * RowFactor);
		const int32 Holds = FMath::Max(1, FMath::FloorToInt32(Round / (2. * Radius)));
		ByWall.Sort([](const TPair<double, const FHOLUnitView*>& A, const TPair<double, const FHOLUnitView*>& B) { return A.Key < B.Key; });
		const int32 FirstRank = FMath::Min(Holds, ByWall.Num());
		TArray<double> Along;
		for (int32 Index = 0; Index < FirstRank; ++Index)
		{
			Along.Add(RoundCoordinate(FVector2D(ByWall[Index].Value->Location), Visible, Radius));
		}
		Along.Sort();
		// Gaps between neighbours along the round, the closing one (last to first) included; the largest is the void.
		TArray<double> RoundGaps;
		for (int32 Index = 1; Index < Along.Num(); ++Index)
		{
			RoundGaps.Add(Along[Index] - Along[Index - 1]);
		}
		if (!Along.IsEmpty())
		{
			RoundGaps.Add(Round - Along.Last() + Along[0]);
		}
		int32 VoidIndex = INDEX_NONE;
		for (int32 Index = 0; Index < RoundGaps.Num(); ++Index)
		{
			VoidIndex = VoidIndex == INDEX_NONE || RoundGaps[Index] > RoundGaps[VoidIndex] ? Index : VoidIndex;
		}
		const double Contact = 2. * Radius;
		const bool bFullRound = FirstRank >= Holds;
		const double VoidGap = bFullRound || VoidIndex == INDEX_NONE ? 0. : RoundGaps[VoidIndex];
		const double Arc = Along.Num() < 2 ? 0. : Round - (VoidIndex == INDEX_NONE ? 0. : RoundGaps[VoidIndex]);
		const double MaxArc = ArcFactor * FMath::Max(0, FirstRank - 1) * Contact + ArcSlackRadii * Radius;
		// A full round: the largest gap may also hold what is left of the round once it is shared at contact.
		const double Remainder = Round - Holds * Contact;
		double SmallestArcGap = 0., LargestArcGap = 0.;
		int32 Holes = 0, Pressed = 0, Counted = 0;
		for (int32 Index = 0; Index < RoundGaps.Num(); ++Index)
		{
			if (Along.Num() < 2 || (!bFullRound && Index == VoidIndex))
			{
				continue;
			}
			const double Gap = RoundGaps[Index];
			SmallestArcGap = Counted == 0 ? Gap : FMath::Min(SmallestArcGap, Gap);
			LargestArcGap = Counted == 0 ? Gap : FMath::Max(LargestArcGap, Gap);
			++Counted;
			const double Bound = ContactMaxFactor * Contact + (bFullRound && Index == VoidIndex ? Remainder : 0.);
			Holes += Gap > Bound + 1. ? 1 : 0;
			Pressed += Gap + 1. < ContactMinFactor * Contact ? 1 : 0;
		}
		TArray<FString> Why;
		if (NotStriking > 0)
		{
			Why.Add(FString::Printf(TEXT("%d of %d not striking"), NotStriking, Alive.Num()));
		}
		if (OutOfReach > 0)
		{
			Why.Add(FString::Printf(TEXT("%d out of reach (farthest %.0f cm from the wall, reach %.0f)"), OutOfReach, FarthestEdge, Run.Reach));
		}
		if (Piled > 0)
		{
			Why.Add(FString::Printf(TEXT("%d pairs closer than %.0f cm (closest %.0f cm)"), Piled, MinPairFactor * Radius, MinPair));
		}
		if (Covered + 1. < Needed)
		{
			Why.Add(FString::Printf(TEXT("spread %.0f cm of the wall, %.0f needed"), Covered, Needed));
		}
		if (InVisibleWall > 0)
		{
			Why.Add(FString::Printf(TEXT("%d bodies in the visible wall (worst %.0f cm, at least %.0f)"), InVisibleWall, WorstVisible, -VisibleWallSlack));
		}
		if (Holes > 0)
		{
			Why.Add(FString::Printf(TEXT("%d holes in the arc (largest gap %.0f cm, at most %.0f)"), Holes, LargestArcGap, ContactMaxFactor * Contact));
		}
		if (Pressed > 0)
		{
			Why.Add(FString::Printf(TEXT("%d pressed neighbours in the arc (smallest gap %.0f cm, at least %.0f)"), Pressed, SmallestArcGap, ContactMinFactor * Contact));
		}
		if (!bFullRound && Arc > MaxArc + 1.)
		{
			Why.Add(FString::Printf(TEXT("arc of %.0f cm, at most %.0f"), Arc, MaxArc));
		}
		Run.SumMinPair += MinPair;
		UE_LOG(LogHOLMatch, Display, TEXT("BuildingCrowd %d/%d: %d %s, angle %.0f deg, start %.0f cm: %s after %.1f s, striking %d/%d, closest pair %.0f cm, %d piled pairs, farthest %.0f cm from the wall, spread %.0f of %.0f cm (needed %.0f), worst %.0f cm from the visible wall, round %.0f cm, first rank %d%s, arc gaps %.0f to %.0f cm (contact %.0f), arc %.0f cm (at most %.0f), void %.0f cm, %s%s"),
			Run.Trip, Run.Count, Alive.Num(), *Run.Type, Run.Angle, Run.StartDistance, bTimedOut ? TEXT("timed out") : TEXT("measured"), Elapsed,
			Alive.Num() - NotStriking, Alive.Num(), MinPair, Piled, FarthestEdge, Covered, Round, Needed, WorstVisible, Round, FirstRank,
			bFullRound ? TEXT(" (full round)") : TEXT(""), SmallestArcGap, LargestArcGap, Contact, Arc, MaxArc, VoidGap,
			Why.IsEmpty() ? TEXT("OK") : TEXT("FAIL: "), *FString::Join(Why, TEXT(", ")));
		if (!Why.IsEmpty())
		{
			++Run.Failed;
			Run.Failures.Add(FString::FromInt(Run.Trip));
			LogUnits(Run, Alive, House, Box);
		}
	}

	/** A failed repetition, a line per unit: where it stands, its place (FHOLCombatFragment), its walk, whether it strikes. */
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
			const FHOLMoveOrderFragment* Move = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity) : nullptr;
			const FVector2D P(Unit->Location);
			const bool bHasPlace = Combat && Combat->PlaceBuilding.Get() == &House;
			UE_LOG(LogHOLMatch, Display, TEXT("BuildingCrowd %d/%d:   unit %u at (%.0f, %.0f), %.0f cm from the wall, %.0f cm from the visible wall, round at %.0f cm, %s, place (%.0f, %.0f) %.0f cm away, %s, blocked %d ms, best %.0f cm, walk phase %d, approaching %d, shifted %d"),
				Run.Trip, Run.Count, Unit->UnitId, P.X, P.Y, DistanceToBox(P, Box) - Unit->Radius, SignedDistanceToBox(P, Visible) - Unit->Radius,
				RoundCoordinate(P, Visible, Unit->Radius),
				Unit->Activity == EHOLUnitActivity::Attacking ? TEXT("striking") : TEXT("NOT striking"),
				Combat ? Combat->BuildingPlace.X : 0., Combat ? Combat->BuildingPlace.Y : 0., Combat ? FVector2D::Distance(P, Combat->BuildingPlace) : -1.,
				bHasPlace ? TEXT("has its place") : TEXT("place forgotten"), Combat ? Combat->PlaceBlockedMs : -1,
				Combat && Combat->PlaceBestDistance < MAX_flt ? Combat->PlaceBestDistance : -1.f, Move ? int32(Move->Phase) : -1,
				Combat ? int32(Combat->bApproaching) : -1, Combat ? int32(Combat->bPlaceShifted) : -1);
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
			Run.GroupSize = Run.Random.RandRange(MinGroup, MaxGroup);
			Run.Angle = Run.Random.FRandRange(0., 360.);
			Run.StartDistance = Run.Random.FRandRange(MinStart, MaxStart);
			Run.HousePoint = Run.ToWorld(Places[(Run.Trip - 1) % UE_ARRAY_COUNT(Places)]);
			Run.House.Reset();
			Run.Group.Reset();
			Run.Before.Reset();
			for (const FHOLUnitView& Unit : Units)
			{
				Run.Before.Add(Unit.UnitId);
			}
			Controller->RequestCheatSpawnBuilding(FName(TEXT("House")), Run.EnemySlot, FVector(Run.HousePoint, Run.CentreZ));
			const double Radians = FMath::DegreesToRadians(Run.Angle);
			const FVector2D Birth = Run.HousePoint + FVector2D(FMath::Cos(Radians), FMath::Sin(Radians)) * Run.StartDistance;
			GEngine->Exec(World, *FString::Printf(TEXT("hol.Units.Spawn %d %.0f %.0f %d %s %.0f"), Run.GroupSize, Birth.X, Birth.Y, Run.Slot, *Run.Type, BirthSpread));
			Run.Phase = EPhase::WaitBirth;
			Run.PhaseStart = Now;
			return true;
		}
		case EPhase::WaitBirth:
		{
			if (!Run.House.IsValid())
			{
				double Best = 2000.;
				for (TActorIterator<AHOLBuilding> It(World); It; ++It)
				{
					const double D = FVector2D::Distance(FVector2D(It->GetActorLocation()), Run.HousePoint);
					if (It->GetOwnerPlayer() == Run.EnemySlot && It->GetHealth() > 0 && D < Best)
					{
						Best = D;
						Run.House = *It;
					}
				}
			}
			Run.Group.Reset();
			for (const FHOLUnitView& Unit : Units)
			{
				if (!Run.Before.Contains(Unit.UnitId) && Unit.OwnerPlayer == Run.Slot && Unit.TypeName == FName(*Run.Type))
				{
					Run.Group.Add(Unit.UnitId);
				}
			}
			if (Run.House.IsValid() && Run.Group.Num() >= Run.GroupSize && Now - Run.PhaseStart >= OrderDelay)
			{
				Controller->RequestAttackBuilding(Run.Group, Run.House.Get());
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
			if (Alive > 0 && Striking == Alive)
			{
				Run.AllStrikingSince = Run.AllStrikingSince < 0. ? Now : Run.AllStrikingSince;
			}
			const bool bSettled = Run.AllStrikingSince >= 0. && Now - Run.AllStrikingSince >= SettleSeconds;
			const bool bTimedOut = Elapsed >= TimeoutSeconds;
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
		const AHOLStartSlot* StartSlot = PlayerState ? AHOLStartSlot::Find(World, PlayerState->GetPlayerIndex()) : nullptr;
		if (Args.IsEmpty() || !StartSlot)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Test.BuildingCrowd <Count> [Seed=1] [Type=ManAtArms] [quit], in a match"));
			return;
		}
		TSharedRef<FRun> Run = MakeShared<FRun>();
		Run->Count = FMath::Max(1, FCString::Atoi(*Args[0]));
		Run->Random.Initialize(Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 1);
		Run->Type = Args.Num() > 2 && Args[2] != TEXT("quit") ? Args[2] : TEXT("ManAtArms");
		Run->bQuit = Args.Contains(TEXT("quit"));
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
		UE_LOG(LogHOLMatch, Display, TEXT("BuildingCrowd: %d runs, slot %d's %s (building reach %.0f cm) on slot %d's Houses, seed %s"), Run->Count, Run->Slot,
			*Run->Type, Run->Reach, Run->EnemySlot, Args.Num() > 1 ? *Args[1] : TEXT("1"));
		Current = Run;
		Run->Ticker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Run](float) { return Tick(Run); }));
	}

	FAutoConsoleCommand BuildingCrowdCommand(
		TEXT("hol.Test.BuildingCrowd"),
		TEXT("hol.Test.BuildingCrowd <Count> [Seed=1] [Type=ManAtArms] [quit]: a group of random size sent on an enemy House from a random side, again and again; each time checked once all strike (in reach of the wall, no pile, spread along it, out of the visible wall, one arc at contact round the House)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&Start));
}
