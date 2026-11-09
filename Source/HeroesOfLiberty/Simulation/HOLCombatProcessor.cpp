// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLCombatProcessor.h"

#include "Async/ParallelFor.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLVictoryRules.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Presentation/HOLArrowSubsystem.h"
#include "Simulation/HOLBuildFragments.h"
#include "Simulation/HOLCampSubsystem.h"
#include "Simulation/HOLDuelSubsystem.h"
#include "Simulation/HOLHeroAbilityRules.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLOrderChainRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Siege/HOLSiegeRules.h"
#include "Technologies/HOLTechnologyRules.h"

int64 UHOLCombatProcessor::TotalHits = 0;
int64 UHOLCombatProcessor::TotalKills = 0;
int64 UHOLCombatProcessor::TotalArrowsReleased = 0;
int64 UHOLCombatProcessor::TotalArrowsLanded = 0;
int64 UHOLCombatProcessor::TotalArrowsLost = 0;

namespace UE::HeroesOfLiberty::Combat
{
	/** A unit looks for an enemy one step of SearchSteps (about 5 times a second at 20 Hz), staggered by its identifier. */
	constexpr int32 SearchSteps = 4;
	/** Cells of the search grid (cm), and its buckets (a power of two). */
	constexpr double CellSize = 400.0;
	constexpr int32 Buckets = 4096;
	/** A new walk to the target once it stands this far (cm) from where the last one went. */
	constexpr double ReapproachDistance = 100.0;
	/** An attack-move is over this close (cm) to the unit's slot (J7b.3). */
	constexpr double AttackMoveArrival = 150.0;
	/** A place around an enemy is taken by a body whose edge stands this close (cm) to its own (a melee's reach). */
	constexpr double PlaceGap = 50.0;
	/** J7c.4, loop 18: an attacker of the enemy takes a place up to its weapon's reach and this margin (cm), at most MaxPlaceReach. */
	constexpr double PlaceReachMargin = 20.0;
	constexpr double MaxPlaceReach = 200.0;
	constexpr double BuildingApproachGap = HOLCombat::BuildingApproachGap;

	/** D581 (TQ2 = A): the speed of a unit under Serrez les rangs, percent of its walk; 0 holds the formation in place. */
	int32 CloseRanksSpeed = HOLHeroAbility::CloseRanksSpeedPercent;
	FAutoConsoleVariableRef CVarCloseRanksSpeed(TEXT("hol.Hero.CloseRanksSpeed"), CloseRanksSpeed,
		TEXT("Speed of the units under the cry Serrez les rangs, in percent of their walk (default 0: held in place, 1-100: slowed; Pas de l'Aube lets them advance at 50 % at least)"));
	int32 LogHits = 0;
	FAutoConsoleVariableRef CVarLogHits(TEXT("hol.Combat.LogHits"), LogHits, TEXT("1: logs every hit of the melee and every arrow released, landed or lost (server or solo)"));
	int32 Enabled = 1;
	FAutoConsoleVariableRef CVarEnabled(TEXT("hol.Combat.Enabled"), Enabled, TEXT("0: no unit fights (server or solo)"));
	/** D733: a unit on an attack-move takes on the Neutrals it meets (a Neutral player's units, in Diplomacy). 0: it leaves them alone, as before. */
	int32 AttackMoveEngagesNeutral = 1;
	FAutoConsoleVariableRef CVarAttackMoveEngagesNeutral(TEXT("hol.Combat.AttackMoveEngagesNeutral"), AttackMoveEngagesNeutral,
		TEXT("1 (default, D733): the attack-move (Q then a click, on the ground too) engages the Neutral players' units it meets, the stance unchanged; 0: it passes them by (a click on one still orders the attack)"));
	/** J7c.4, S3: the same outcome by cheaper ways (a flat entity index, a key sort, no search when the target is in reach). */
	int32 FastPaths = 1;
	FAutoConsoleVariableRef CVarFastPaths(TEXT("hol.Combat.FastPaths"), FastPaths,
		TEXT("1 (default): the combat's fast paths (J7c.4, S3); 0: the former ways (TMap index, full sort, a search at every search step), the same outcome"));
	/** J7c.4, S2: a step where no unit can search, strike or be struck ends after a single pass (the same outcome). */
	int32 FastIdle = 1;
	FAutoConsoleVariableRef CVarFastIdle(TEXT("hol.Combat.FastIdle"), FastIdle,
		TEXT("1 (default): a step where no unit can fight (no target, no attacker, no arrow, nobody dying, nobody free to search this step, no attack-move) only ticks the units' idle state, without the sort, the grid and the searches (J7c.4, S2); 0: the full pass, the same outcome"));
	/** J7c.4, S2: in the full pass, a unit whose outcome is its own idle tick gets it at once (the same outcome). */
	int32 FastIdleUnit = 1;
	FAutoConsoleVariableRef CVarFastIdleUnit(TEXT("hol.Combat.FastIdleUnit"), FastIdleUnit,
		TEXT("1 (default): in the full pass, a unit with no target, no attacker, no approach, no attack, no attack-move and no search this step, checked at its own turn, only ticks its idle state (J7c.4, S2); 0: its whole turn, the same outcome"));
	/** J7c.4, S2: no attacker to count when no unit has a target (the same outcome: all counts 0). */
	int32 SkipAttackerCount = 1;
	FAutoConsoleVariableRef CVarSkipAttackerCount(TEXT("hol.Combat.SkipAttackerCount"), SkipAttackerCount,
		TEXT("1 (default): the attackers' counts zeroed at the gathering and counted only when a unit has a target (J7c.4, S2); 0: zeroed and counted every step, the same outcome"));
	/** Perf (2026-10-24): more of the serial pass's lists and counts made ahead, in parallel (the same outcome). */
	int32 ParallelAhead = 1;
	FAutoConsoleVariableRef CVarParallelAhead(TEXT("hol.Combat.ParallelAhead"), ParallelAhead,
		TEXT("1 (default): the contact lists of the units walking to fight and the places taken around the enemies the searchers will see, made ahead in parallel (a count read only while no unit has died since); 0: in the serial pass, as before, the same outcome"));
	/** Perf (2026-10-24): the target searches made ahead in parallel, the serial pass weighing only what changed since (the same outcome). */
	int32 ParallelSearch = 1;
	FAutoConsoleVariableRef CVarParallelSearch(TEXT("hol.Combat.ParallelSearch"), ParallelSearch,
		TEXT("1 (default): the searches of the units gathered ahead made in parallel once the arrows have landed; at its turn, a unit whose target and last attacker are the same takes that choice weighed against the candidates changed since (target, attackers, death, places), else searches as before; 2: both, a warning for each choice that differs (a check); 0: in the serial pass, as before, the same outcome"));
	/** hol.Combat.ParallelSearch 2: choices made ahead that differed from the serial search's (0 expected). */
	int64 SearchMismatches = 0;
	/** J7c.4, trial feedback: a melee out of reach leaves an enemy with no place left around it for one with a place. */
	int32 PreferFreeTarget = 1;
	FAutoConsoleVariableRef CVarPreferFreeTarget(TEXT("hol.Combat.PreferFreeTarget"), PreferFreeTarget,
		TEXT("1 (default): a melee out of reach takes the nearest enemy with a free place around it (MaxAttackers bodies of any side touching it: full), its target kept when none; 0: as before"));
	/** J7c.4, trial feedback: a walking melee stops at the first enemy in its reach, not only at its target. */
	int32 StopAtContact = 1;
	FAutoConsoleVariableRef CVarStopAtContact(TEXT("hol.Combat.StopAtContact"), StopAtContact,
		TEXT("1 (default): a melee walking to fight, its target out of reach, takes at every step the enemy in its reach of least unit id (an ordered target kept; dense grid only); 0: as before"));
	/** J7c.4, loop 12: a walking melee does not pass by an enemy this close (cm, edge to edge): it takes it on. */
	int32 ClaimedSlots = 1;
	FAutoConsoleVariableRef CVarClaimedSlots(TEXT("hol.Combat.ClaimedSlots"), ClaimedSlots,
		TEXT("1 (default, with hol.Combat.ClaimPlace): the places the friends of the same target claimed count as bodies standing on their spots when a melee chooses its place (the nearer holder keeps its spot, then the lower id), so two friends never walk to the same spot; 0: off, the rule as before (BUG-melee-stuck-attack)"));
	float StuckRetargetSeconds = 3.f;
	FAutoConsoleVariableRef CVarStuckRetargetSeconds(TEXT("hol.Combat.StuckRetargetSeconds"), StuckRetargetSeconds,
		TEXT("s (default 3, with hol.Combat.ClaimPlace): a melee out of reach with no headway to its target for this long takes the nearest enemy with a free place it can reach (no going round); a target the player ordered is kept; 0: off (BUG-melee-stuck-attack)"));
	float StuckLooseSeconds = 0.f; // OFF by default: the lot of 2026-11-06 17:00 gave 3/8 red with 4 (see Retours/BUG-melee-stuck-attack.md, Correcteur 2)
	FAutoConsoleVariableRef CVarStuckLooseSeconds(TEXT("hol.Combat.StuckLooseSeconds"), StuckLooseSeconds,
		TEXT("s (default 0 = off, tried at 4: not better; with hol.Combat.StuckRetargetSeconds): a melee out of reach with no headway for this long, no enemy with a free place it can reach, takes the nearest other enemy whatever the places and the friends in the way (a target the player ordered is kept); 0: off (BUG-melee-stuck-attack)"));
	int32 SameWalkGoal = 1;
	FAutoConsoleVariableRef CVarSameWalkGoal(TEXT("hol.Combat.SameWalkGoal"), SameWalkGoal,
		TEXT("1 (default; Celeborn-14): a unit already walking to the place of the enemy it seeks (within 2 radii + 50 cm) is not given the same walk again at each search step (it restarted the final leg's wait of FinalLegStuckSteps: a walk held by its friends never ended); 0: as before"));
	int32 WalkRiposteSteps = 10;
	FAutoConsoleVariableRef CVarWalkRiposteSteps(TEXT("hol.Combat.WalkRiposteSteps"), WalkRiposteSteps,
		TEXT("steps (default 10 = 0.5 s; Q5 = C): a unit on a player's walk (not an attack-move) whose walk made no headway for this many steps (its place barred, a friend in the way) takes on the enemy that strikes it (HOLCombat::MaySeek); a normal walk is never cut; 0: off (as before: no riposte during a walk)"));
	float MeleeLock = 60.f;
	FAutoConsoleVariableRef CVarMeleeLock(TEXT("hol.Combat.MeleeLock"), MeleeLock,
		TEXT("cm (default 60): a melee soldier that struck last step keeps its target while it lives and stays within its reach plus this (the target nudged out of reach by a few cm no longer makes the soldier take another enemy and begin its blow again: BUG-melee-stuck-attack); 0: off, as before"));
	int32 ZoneSkipFull = 1;
	FAutoConsoleVariableRef CVarZoneSkipFull(TEXT("hol.Combat.ZoneSkipFull"), ZoneSkipFull,
		TEXT("1 (default, with hol.Combat.ZoneOfControl and hol.Combat.PreferFreeTarget): the zone of control does not take an enemy with every place around it taken (other than the target already held): the search and the zone no longer swing the soldier between two enemies; 0: as before (BUG-melee-stuck-attack)"));
	float ZoneOfControl = 150.f;
	FAutoConsoleVariableRef CVarZoneOfControl(TEXT("hol.Combat.ZoneOfControl"), ZoneOfControl,
		TEXT("cm edge to edge (default 150): a melee walking to fight (hol.Combat.StopAtContact), no enemy in its reach, takes the nearest enemy this close and walks to it alone (its target kept while in the zone); 0: as before"));
	/** J7c.4, loop 19: a melee out of reach keeps its target unless the new one is clearly nearer (no swing at each search). */
	float RearTargetKeep = 200.f;
	FAutoConsoleVariableRef CVarRearTargetKeep(TEXT("hol.Combat.RearTargetKeep"), RearTargetKeep,
		TEXT("cm (default 200, with hol.Combat.PreferFreeTarget): a melee out of reach of its target and of the new one found keeps its target, alive with a free place around it, unless the new one is this much nearer (edge to edge); 0: as before"));
	/** J8 close (decision A of 2026-10-30, the J8a trial: Men-at-arms idle round the hero): a surplus attacker of an encircled ordered target takes another enemy. */
	int32 SurplusReassign = 1;
	FAutoConsoleVariableRef CVarSurplusReassign(TEXT("hol.Combat.SurplusReassign"), SurplusReassign,
		TEXT("1 (default, with hol.Combat.PreferFreeTarget): a melee ordered at a target it cannot reach, every place around it taken (MaxAttackers bodies), takes the nearest enemy within its aggression radius that has a place left, its order over; 0: it waits behind the crowd, as before"));
	/** D641 (the user's retour on the neutrals trial): a melee that makes no headway to its target, with a place free round it, walks to that place and keeps its side. */
	int32 ClaimPlace = 1;
	FAutoConsoleVariableRef CVarClaimPlace(TEXT("hol.Combat.ClaimPlace"), ClaimPlace,
		TEXT("1 (default): a melee out of reach of its target that has gained no hol.Combat.ClaimProgress cm of edge distance for hol.Combat.ClaimStuck s, with a place free round the target, walks to that place (the free spot nearest to the side it was on, kept while free) instead of the target's centre; 0: off"));
	/** D641 (the walk-round trace of 2026-11-05): a claimed place that is taken is replaced by the free spot on the soldier's own side, not the one nearest the old place. */
	int32 PlaceBySide = 1;
	FAutoConsoleVariableRef CVarPlaceBySide(TEXT("hol.Combat.PlaceBySide"), PlaceBySide,
		TEXT("1 (default, with hol.Combat.ClaimPlace): a claimed place that gets taken is replaced by the free spot nearest to the side of the target the soldier stands on now (the shortest way round the ring); 0: the spot nearest to the old place (it could lie across the whole melee)"));
	float ClaimStuckSeconds = 0.15f;
	FAutoConsoleVariableRef CVarClaimStuck(TEXT("hol.Combat.ClaimStuck"), ClaimStuckSeconds,
		TEXT("s (default 0.15, with hol.Combat.ClaimPlace): how long a melee out of reach may make no headway before it claims a free place round its target"));
	float ClaimProgress = 25.f;
	FAutoConsoleVariableRef CVarClaimProgress(TEXT("hol.Combat.ClaimProgress"), ClaimProgress,
		TEXT("cm (default 25, with hol.Combat.ClaimPlace): the edge distance to its target a melee must gain to count as making headway"));
	/** Celeborn-24 (BUG-melee-spin-sanity): a claimed place walled in by the bodies round the way to it is not claimed (HOLCombat::IsPlaceWalledIn). */
	int32 PlaceBoxedIn = 0;
	FAutoConsoleVariableRef CVarPlaceBoxedIn(TEXT("hol.Combat.PlaceBoxedIn"), PlaceBoxedIn,
		TEXT("1 (default, with hol.Combat.ClaimPlace): a melee does not claim a free place round its target when the bodies round its way to it wall it in (a gap of less than its body): it walks to the target and waits behind the crowd instead of a U round its friends (the spin of melee_stuck); 0: as before"));
	int32 KeepStuckCount = 1;
	FAutoConsoleVariableRef CVarKeepStuckCount(TEXT("hol.Combat.KeepStuckCount"), KeepStuckCount,
		TEXT("1 (default, with hol.Combat.ClaimPlace): a place claimed again after being lost does not set the no-headway count back to 0 (the count that starts hol.Combat.StuckRetargetSeconds); 0: as before"));
	float NeighbourGap = 150.f;
	FAutoConsoleVariableRef CVarNeighbourGap(TEXT("hol.Combat.NeighbourGap"), NeighbourGap,
		TEXT("cm (default 600; was 150): a new target within this of the last one keeps the no-headway count (the soldier that swings between two enemies 450 cm apart, np back to 0 at each swing, never reached StuckRetargetSeconds)"));
	/** Q-neutrals-second-rank = C (the user, 2026-11-05): a soldier walled in between its friends changes target instead of turning back. */
	int32 BoxedRetarget = 0;
	FAutoConsoleVariableRef CVarBoxedRetarget(TEXT("hol.Combat.BoxedRetarget"), BoxedRetarget,
		TEXT("1 (default 0 since the neutrals_trace measure of 2026-11-05: 3 green of 8 with it on, no better than without; needs hol.Combat.ClaimPlace): a melee out of reach of its target, walled in between two friends (HOLCombat::IsBoxedIn) with no headway for hol.Combat.BoxedRetargetSeconds, takes the nearest enemy (within 500 cm) with a free place it can reach without going round; a target the player ordered is kept; 0: off"));
	float BoxedRetargetSeconds = 1.f;
	FAutoConsoleVariableRef CVarBoxedRetargetSeconds(TEXT("hol.Combat.BoxedRetargetSeconds"), BoxedRetargetSeconds,
		TEXT("s (default 1, with hol.Combat.BoxedRetarget): how long a walled-in melee makes no headway before it changes target"));
	int32 FarPlaceRetarget = 1;
	FAutoConsoleVariableRef CVarFarPlaceRetarget(TEXT("hol.Combat.FarPlaceRetarget"), FarPlaceRetarget,
		TEXT("1 (needs hol.Combat.ClaimPlace): a melee that would claim a NEW place more than hol.Combat.FarPlaceGap rad from the side of its target it stands on (the free spot is across the melee, a walk round the crowd that reverses on its steps while the target dies) takes instead the nearest enemy with a free place on its own side; none: it waits up to hol.Combat.FarPlaceWaitSeconds of no headway, then takes the far place as before; a target the player ordered is kept; 0: off"));
	float FarPlaceGap = 1.5f;
	FAutoConsoleVariableRef CVarFarPlaceGap(TEXT("hol.Combat.FarPlaceGap"), FarPlaceGap,
		TEXT("rad (default 1.5, about 90 degrees, with hol.Combat.FarPlaceRetarget): the largest gap between the place claimed and the side of the target the soldier is on"));
	float FarPlaceWaitSeconds = 2.f;
	FAutoConsoleVariableRef CVarFarPlaceWaitSeconds(TEXT("hol.Combat.FarPlaceWaitSeconds"), FarPlaceWaitSeconds,
		TEXT("s (default 2, with hol.Combat.FarPlaceRetarget): how long a soldier with no enemy on its side waits (no headway) before it takes the far place"));
	constexpr double BoxedRetargetReach = 900.0;
	constexpr double BoxedRetargetEnemyReach = 500.0;
	/** MV.4, choice C (AoE4): an idle melee soldier comes to the aid of a friend fighting within this radius (HOLCombat::ShouldComeToAid). */
	float AidRadius = HOLCombat::DefaultAidRadius;
	FAutoConsoleVariableRef CVarAidRadius(TEXT("hol.Combat.AidRadius"), AidRadius,
		TEXT("cm (default 2000): an idle melee soldier with no enemy within its aggression comes to the aid of a friend fighting within this radius (edge to edge), taking the nearest enemy engaged with such a friend; 0: off"));
}

namespace UE::HeroesOfLiberty::Simulation
{
	// J7c.4 loop 21 (hol.Combat.RearFlank, HOLSimulationProcessors.cpp): each side's line of contact, of the last step.
	extern int32 RearFlank;
	extern float RearFlankDelay;
	extern TMap<uint8, FHOLFrontLine> FrontLines;
	// J7c.4 loop 22 (HOLSimulationProcessors.cpp).
	extern float RearFlankStagger;
	extern int32 RearFlankNearest;
	extern int32 FlankToTarget;
	extern float FlankStillSeconds;
	extern float RearFlankMaxShift;
	extern float RearFlankBack;
	extern float RearTargetMaxShift;
}

namespace UE::HeroesOfLiberty::Combat
{
	/** J7c.4 loop 21 (hol.Combat.RearFlank): the flank's goal beyond the line's end and ahead of it (cm). */
	constexpr double RearFlankOut = 200.0;
	constexpr double RearFlankAhead = 100.0;
	/** Held this close behind its own line (cm, along the charge) it counts as held; a free enemy this close (edge) ends the flank. */
	constexpr double RearFlankZone = 800.0;
	constexpr double RearFlankFreeReach = 400.0;
	/** A free enemy ending the flank stands at most this much inside the line's end (cm); the goal reached at this distance. */
	constexpr double RearFlankEndSlack = 100.0;
	constexpr double RearFlankArrival = 100.0;
	/** The flank's walk given again once its goal moved this much (cm); the flank given up after this long (s). */
	constexpr double RearFlankRewalk = 100.0;
	constexpr float RearFlankGiveUp = 8.f;
}

UHOLCombatProcessor::UHOLCombatProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	// Last of the tasks: the step's orders, gathering and building applied (a player's order seen first), before the
	// movement, so that a unit stopped in reach does not move this step.
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLUnitOrderProcessor"));
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLGatherProcessor"));
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLBuildProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavMeshPathFollowProcessor"));
	bRequiresGameThreadExecution = true; // the order queue, the unit subsystem
}

void UHOLCombatProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLCombatFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLGatherFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLBuildFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

int32 UHOLCombatProcessor::BucketOf(const FIntPoint& Cell)
{
	return static_cast<int32>((static_cast<uint32>(Cell.X) * 73856093u ^ static_cast<uint32>(Cell.Y) * 19349663u) & (UE::HeroesOfLiberty::Combat::Buckets - 1));
}

bool UHOLCombatProcessor::BuildDenseGrid()
{
	// J7c.4, S3. The box of the step's cells (Cells filled), and the sides (an owner each, few).
	FIntPoint Min = Cells[0];
	FIntPoint Max = Cells[0];
	for (const FIntPoint& Cell : Cells)
	{
		Min = FIntPoint(FMath::Min(Min.X, Cell.X), FMath::Min(Min.Y, Cell.Y));
		Max = FIntPoint(FMath::Max(Max.X, Cell.X), FMath::Max(Max.Y, Cell.Y));
	}
	Sides.Reset();
	SideOfFighter.SetNumUninitialized(Fighters.Num());
	int32 LastOwner = INDEX_NONE;
	int32 LastSide = INDEX_NONE;
	for (int32 Index = 0; Index < Fighters.Num(); ++Index)
	{
		const int32 Owner = Fighters[Index].Camp;
		if (LastSide == INDEX_NONE || Owner != LastOwner)
		{
			LastSide = Sides.Find(Owner);
			if (LastSide == INDEX_NONE)
			{
				LastSide = Sides.Add(Owner);
			}
			LastOwner = Owner;
		}
		SideOfFighter[Index] = LastSide;
	}
	// Too many cells (units spread over a huge box): the hashed grid, as before.
	constexpr int64 MaxKeys = 1 << 22;
	const int64 Width = static_cast<int64>(Max.X) - Min.X + 1;
	const int64 Height = static_cast<int64>(Max.Y) - Min.Y + 1;
	if (Width * Height * Sides.Num() > MaxKeys)
	{
		return false;
	}
	GridMin = Min;
	GridWidth = static_cast<int32>(Width);
	GridHeight = static_cast<int32>(Height);
	const int32 NumCells = GridWidth * GridHeight;
	const int32 NumKeys = NumCells * Sides.Num();
	const auto KeyOf = [this, NumCells](const int32 Index)
	{
		return SideOfFighter[Index] * NumCells + (Cells[Index].Y - GridMin.Y) * GridWidth + (Cells[Index].X - GridMin.X);
	};
	// Counting sort, stable: within a (side, cell), the units stay in identifier order.
	GridStarts.Init(0, NumKeys + 1);
	for (int32 Index = 0; Index < Fighters.Num(); ++Index)
	{
		++GridStarts[KeyOf(Index) + 1];
	}
	for (int32 Key = 0; Key < NumKeys; ++Key)
	{
		GridStarts[Key + 1] += GridStarts[Key];
	}
	GridCursor = GridStarts;
	GridUnits.SetNumUninitialized(Fighters.Num());
	for (int32 Index = 0; Index < Fighters.Num(); ++Index)
	{
		const FFighter& Fighter = Fighters[Index];
		FGridUnit& Unit = GridUnits[GridCursor[KeyOf(Index)]++];
		Unit.X = Fighter.Location.X;
		Unit.Y = Fighter.Location.Y;
		Unit.Radius = Fighter.Constants->Radius;
		Unit.Fighter = Index;
	}
	return true;
}

void UHOLCombatProcessor::GatherNearby(const int32 Index, const double Reach, TArray<int32>& Out) const
{
	// J7c.4, S3: the same cells as the hashed walk (clamped to the box: no unit outside), the other sides only, their units
	// of a row of cells contiguous. A squared distance with a 1 cm margin (MaxRadius bounds every radius) only spares the
	// exact test, the hashed walk's, on the same values.
	using namespace UE::HeroesOfLiberty::Combat;
	Out.Reset();
	const FVector2D Position(Fighters[Index].Location);
	const float Radius = Fighters[Index].Constants->Radius;
	const FIntPoint MinCell(FMath::FloorToInt32((Position.X - Reach) / CellSize), FMath::FloorToInt32((Position.Y - Reach) / CellSize));
	const FIntPoint MaxCell(FMath::FloorToInt32((Position.X + Reach) / CellSize), FMath::FloorToInt32((Position.Y + Reach) / CellSize));
	const int32 X0 = FMath::Max(MinCell.X, GridMin.X) - GridMin.X;
	const int32 X1 = FMath::Min(MaxCell.X, GridMin.X + GridWidth - 1) - GridMin.X;
	const int32 Y0 = FMath::Max(MinCell.Y, GridMin.Y) - GridMin.Y;
	const int32 Y1 = FMath::Min(MaxCell.Y, GridMin.Y + GridHeight - 1) - GridMin.Y;
	const double Prefilter = FMath::Square(Reach + static_cast<double>(Radius) + static_cast<double>(MaxRadius) + 1.0);
	const int32 MySide = SideOfFighter[Index];
	const int32 NumCells = GridWidth * GridHeight;
	for (int32 Side = 0; Side < Sides.Num() && X0 <= X1; ++Side)
	{
		if (Side == MySide)
		{
			continue;
		}
		for (int32 Y = Y0; Y <= Y1; ++Y)
		{
			const int32 Row = Side * NumCells + Y * GridWidth;
			const int32 End = GridStarts[Row + X1 + 1];
			for (int32 Slot = GridStarts[Row + X0]; Slot < End; ++Slot)
			{
				const FGridUnit& Unit = GridUnits[Slot];
				const double DX = Unit.X - Position.X;
				const double DY = Unit.Y - Position.Y;
				if (DX * DX + DY * DY <= Prefilter && HOLCombat::GetEdgeDistance(Position, Radius, FVector2D(Unit.X, Unit.Y), Unit.Radius) <= Reach)
				{
					Out.Add(Unit.Fighter);
				}
			}
		}
	}
}

void UHOLCombatProcessor::GatherFriends(const int32 Index, const double Reach, TArray<int32>& Out) const
{
	// MV.4 (coming to aid): GatherNearby's walk on its own side only (the dense grid), itself left out.
	using namespace UE::HeroesOfLiberty::Combat;
	Out.Reset();
	const FVector2D Position(Fighters[Index].Location);
	const float Radius = Fighters[Index].Constants->Radius;
	const FIntPoint MinCell(FMath::FloorToInt32((Position.X - Reach) / CellSize), FMath::FloorToInt32((Position.Y - Reach) / CellSize));
	const FIntPoint MaxCell(FMath::FloorToInt32((Position.X + Reach) / CellSize), FMath::FloorToInt32((Position.Y + Reach) / CellSize));
	const int32 X0 = FMath::Max(MinCell.X, GridMin.X) - GridMin.X;
	const int32 X1 = FMath::Min(MaxCell.X, GridMin.X + GridWidth - 1) - GridMin.X;
	const int32 Y0 = FMath::Max(MinCell.Y, GridMin.Y) - GridMin.Y;
	const int32 Y1 = FMath::Min(MaxCell.Y, GridMin.Y + GridHeight - 1) - GridMin.Y;
	const double Prefilter = FMath::Square(Reach + static_cast<double>(Radius) + static_cast<double>(MaxRadius) + 1.0);
	const int32 Side = SideOfFighter[Index];
	const int32 NumCells = GridWidth * GridHeight;
	for (int32 Y = Y0; Y <= Y1 && X0 <= X1; ++Y)
	{
		const int32 Row = Side * NumCells + Y * GridWidth;
		const int32 End = GridStarts[Row + X1 + 1];
		for (int32 Slot = GridStarts[Row + X0]; Slot < End; ++Slot)
		{
			const FGridUnit& Unit = GridUnits[Slot];
			const double DX = Unit.X - Position.X;
			const double DY = Unit.Y - Position.Y;
			if (Unit.Fighter != Index && DX * DX + DY * DY <= Prefilter && HOLCombat::GetEdgeDistance(Position, Radius, FVector2D(Unit.X, Unit.Y), Unit.Radius) <= Reach)
			{
				Out.Add(Unit.Fighter);
			}
		}
	}
}

int32 UHOLCombatProcessor::CountBodiesAround(const int32 Index, const double Gap, const int32 Limit) const
{
	const FVector2D Position(Fighters[Index].Location);
	const float Radius = Fighters[Index].Constants->Radius;
	// J7c.4, loop 18: an attacker (a body of another side than the enemy's) takes a place up to its weapon's reach (a
	// Spearman 130 cm away fights it as one touching it), a friend of the enemy up to Gap: a ring of Spearmen is full.
	const int32 EnemySide = SideOfFighter[Index];
	const double Reach = FMath::Max(Gap, UE::HeroesOfLiberty::Combat::MaxPlaceReach) + static_cast<double>(Radius) + static_cast<double>(MaxRadius);
	const int32 X0 = FMath::Max(FMath::FloorToInt32((Position.X - Reach) / UE::HeroesOfLiberty::Combat::CellSize), GridMin.X) - GridMin.X;
	const int32 X1 = FMath::Min(FMath::FloorToInt32((Position.X + Reach) / UE::HeroesOfLiberty::Combat::CellSize), GridMin.X + GridWidth - 1) - GridMin.X;
	const int32 Y0 = FMath::Max(FMath::FloorToInt32((Position.Y - Reach) / UE::HeroesOfLiberty::Combat::CellSize), GridMin.Y) - GridMin.Y;
	const int32 Y1 = FMath::Min(FMath::FloorToInt32((Position.Y + Reach) / UE::HeroesOfLiberty::Combat::CellSize), GridMin.Y + GridHeight - 1) - GridMin.Y;
	const int32 NumCells = GridWidth * GridHeight;
	int32 Count = 0;
	for (int32 Side = 0; Side < Sides.Num() && X0 <= X1; ++Side)
	{
		for (int32 Y = Y0; Y <= Y1; ++Y)
		{
			const int32 Row = Side * NumCells + Y * GridWidth;
			const int32 End = GridStarts[Row + X1 + 1];
			for (int32 Slot = GridStarts[Row + X0]; Slot < End; ++Slot)
			{
				const FGridUnit& Unit = GridUnits[Slot];
				// Perf: a body beyond any place's reach (MaxPlaceReach or Gap, and 1 cm) first, before its health is read.
				const double DX = Unit.X - Position.X;
				const double DY = Unit.Y - Position.Y;
				if (DX * DX + DY * DY > FMath::Square(Reach + 1.0))
				{
					continue;
				}
				if (Unit.Fighter == Index || Fighters[Unit.Fighter].State->CurrentHealth <= 0)
				{
					continue;
				}
				double PlaceReach = Gap;
				if (Side != EnemySide)
				{
					const float WeaponReach = Fighters[Unit.Fighter].Constants->AttackRange;
					if (!HOLCombat::IsRangedAttack(WeaponReach))
					{
						PlaceReach = FMath::Clamp(static_cast<double>(WeaponReach) + UE::HeroesOfLiberty::Combat::PlaceReachMargin, Gap, UE::HeroesOfLiberty::Combat::MaxPlaceReach);
					}
				}
				if (HOLCombat::GetEdgeDistance(Position, Radius, FVector2D(Unit.X, Unit.Y), Unit.Radius) <= PlaceReach && ++Count >= Limit)
				{
					return Count;
				}
			}
		}
	}
	return Count;
}

bool UHOLCombatProcessor::IsAutoFoe(const FFighter& A, const FFighter& B) const
{
	if (!StepCamps.IsDiplomacy() || StepCamps.FiresAtOnce(A.State->OwnerPlayer, B.State->OwnerPlayer))
	{
		return true;
	}
	// D733 (the user, 2026-11-05): a unit on an attack-move (Q then a click, even on the ground) takes on the Neutrals it meets, though nobody takes a
	// Neutral on by itself (D369, D401); the stance is not touched.
	return UE::HeroesOfLiberty::Combat::AttackMoveEngagesNeutral != 0 && A.Combat->Mode == EHOLCombatMode::AttackMove
		&& StepCamps.IsPassiveAdversary(A.State->OwnerPlayer, B.State->OwnerPlayer);
}

void UHOLCombatProcessor::StopWalk(const UWorld& World, FFighter& Fighter) const
{
	// As the order processor's arrival (D290): stands dead where it is. Its order serial cleared, a path still to come for
	// its walk is not applied to it (the order processor drops a request whose units follow another serial).
	FMassMoveTargetFragment& MoveTarget = *Fighter.MoveTarget;
	MoveTarget.Center = Fighter.Location;
	MoveTarget.DistanceToGoal = 0.f;
	MoveTarget.CreateNewAction(EMassMovementAction::Stand, World);
	Fighter.Movement->DesiredVelocity = FVector::ZeroVector;
	Fighter.Order->Phase = EHOLMovePhase::None;
	Fighter.Order->OrderSerial = 0;
	Fighter.Order->bFormation = false;
	Fighter.Order->MarchSpeed = 0.f;
	Fighter.State->Activity = EHOLUnitActivity::Idle;
	Fighter.Combat->bApproaching = false;
}

void UHOLCombatProcessor::KillUnit(const UWorld* World, UHOLUnitSubsystem& Units, FFighter& Victim, const int64 StepIndex, const int32 KillerPlayer) const
{
	// Dying (J7b.4, J7-Q5): stopped dead, out of the fight and of its task, destroyed DyingSteps later.
	if (World)
	{
		StopWalk(*World, Victim);
	}
	Victim.Combat->Disengage();
	Victim.Combat->DeathStep = StepIndex;
	Victim.State->Activity = EHOLUnitActivity::Dying;
	// Its population places free at once (the user's trial of J7b), not at its destruction; a kill of KillerPlayer (C2).
	Units.MarkUnitDying(Victim.UnitId, KillerPlayer);
	// V5.2 (Regicide, D355): a King dead defeats its slot at the end of the step (none again for a defeated slot's collapse).
	if (World && HOLIsKing(Victim.Constants->ArmorCategoryMask))
	{
		if (AHOLGameState* GameState = World->GetGameState<AHOLGameState>())
		{
			GameState->HandleKingDeath(Victim.State->OwnerPlayer);
		}
	}
}

void UHOLCombatProcessor::FinishDying(const UWorld* World, UHOLUnitSubsystem& Units, FFighter& Fighter, const int64 StepIndex)
{
	// Dying (J7b.4): it stands ~1 s for its fall, then is destroyed at the end of a step (D224).
	if (Fighter.Combat->DeathStep < 0)
	{
		// At 0 health by another way than a hit or an arrow of this processor: the same dying path all the same.
		KillUnit(World, Units, Fighter, StepIndex);
	}
	if (StepIndex - Fighter.Combat->DeathStep >= HOLCombat::DyingSteps)
	{
		Deaths.Add(Fighter.UnitId);
	}
}

int32 UHOLCombatProcessor::FindStepBuilding(const AHOLBuilding* Building) const
{
	if (!Building)
	{
		return INDEX_NONE;
	}
	for (int32 Index = 0; Index < StepBuildings.Num(); ++Index)
	{
		if (StepBuildings[Index].Building == Building)
		{
			return StepBuildings[Index].bGone ? INDEX_NONE : Index;
		}
	}
	return INDEX_NONE;
}

uint64 UHOLCombatProcessor::HashArrows() const
{
	uint64 Hash = 14695981039346656037ull;
	const auto Mix = [&Hash](const uint64 Value)
	{
		for (int32 Byte = 0; Byte < 8; ++Byte)
		{
			Hash = (Hash ^ ((Value >> (Byte * 8)) & 0xFFull)) * 1099511628211ull;
		}
	};
	for (const FArrow& Arrow : Arrows)
	{
		Mix(Arrow.ShooterId);
		Mix(Arrow.TargetId);
		Mix(static_cast<uint64>(Arrow.ShooterSlot));
		Mix(static_cast<uint64>(BitCast<uint32>(Arrow.Damage)));
		Mix(static_cast<uint64>(Arrow.ReleaseStep));
		Mix(static_cast<uint64>(Arrow.ArrivalStep));
	}
	return Hash;
}

bool UHOLCombatProcessor::IsInAura(const FFighter& Victim) const
{
	bool bInAura = false;
	for (const FAuraSource& Source : AuraSources)
	{
		bInAura |= Source.Owner == Victim.State->OwnerPlayer
			&& HOLHeroAbility::IsWithinAura(static_cast<float>(FVector2D::Distance(Source.Location, FVector2D(Victim.Location))));
	}
	return bInAura;
}

int32 UHOLCombatProcessor::GetDamageTakenPercent(const FFighter& Victim) const
{
	return HOLHeroAbility::GetDamageTakenPercent(IsInAura(Victim), Victim.Combat->CloseRanksMs > 0);
}

int32 UHOLCombatProcessor::GetTalentArmorBonus(const FFighter& Victim) const
{
	const bool bShieldsUp = (ShieldsUpOwners & (1u << Victim.State->OwnerPlayer)) != 0;
	return bShieldsUp && IsInAura(Victim) ? HOLHeroAbility::ShieldsUpArmorBonus : 0;
}

int32 UHOLCombatProcessor::GetTechnologyAttackPercent(const FFighter& Attacker) const
{
	const int32 Slot = Attacker.State->OwnerPlayer;
	if (Slot < 0 || Slot >= FHOLMatchConfig::MaxSlots)
	{
		return 0;
	}
	// S5: a type a technology names itself (the Mangonel, Heavy Munitions) reads that one, whatever its attack's type (siege: none).
	if (!HOLSiege::GetDamageStat(Attacker.Constants->TypeName).IsNone())
	{
		return MangonelDamagePercent[Slot];
	}
	switch (Attacker.Constants->AttackType)
	{
	case EHOLAttackType::Melee: return MeleeAttackPercent[Slot];
	case EHOLAttackType::Ranged: return RangedAttackPercent[Slot];
	default: return 0;
	}
}

int32 UHOLCombatProcessor::GetTechnologyArmorPercent(const FFighter& Victim, const EHOLAttackType AttackType) const
{
	const int32 Slot = Victim.State->OwnerPlayer;
	if (Slot < 0 || Slot >= FHOLMatchConfig::MaxSlots || !HOLTechnology::ArmorCoversAttack(AttackType))
	{
		return 0;
	}
	const FName Stat = HOLTechnology::GetArmorStat(Victim.Constants->ArmorCategoryMask);
	if (Stat == HOLTechnology::InfantryArmorStat)
	{
		return InfantryArmorPercent[Slot];
	}
	return Stat == HOLTechnology::CavalryArmorStat ? CavalryArmorPercent[Slot] : 0;
}

int32 UHOLCombatProcessor::GetBuildingAttackWithTechnology(const FFighter& Attacker) const
{
	// S5: only a type a technology names itself (the Mangonel) hits buildings harder; the attack technologies of the soldiers stay on units.
	const int32 Slot = Attacker.State->OwnerPlayer;
	if (Slot < 0 || Slot >= FHOLMatchConfig::MaxSlots || HOLSiege::GetDamageStat(Attacker.Constants->TypeName).IsNone())
	{
		return Attacker.Constants->BuildingAttack;
	}
	return HOLTechnology::ScaleByPercent(Attacker.Constants->BuildingAttack, MangonelDamagePercent[Slot]);
}

void UHOLCombatProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Combat;
	UWorld* World = EntityManager.GetWorld();
	UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	const UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (!Units || !Commands)
	{
		return;
	}
	// Off (hol.Combat.Enabled 0): no unit fights and the arrows in flight wait, but the dying still fall and go.
	const bool bFighting = Enabled != 0;
	const int64 StepIndex = Simulation ? Simulation->GetStepIndex() : 0;
	const int32 StepMs = HOLCombat::ToMilliseconds(Context.GetDeltaTimeSeconds());
	// F5 (FogInProgress.md point 3): a unit takes on by itself only what its camp sees (the fog of the last step's end).
	const UHOLVisionSubsystem* Vision = World ? World->GetSubsystem<UHOLVisionSubsystem>() : nullptr;
	if (Vision && (!Vision->IsStarted() || Vision->GetMode() == EHOLFogMode::Disabled))
	{
		Vision = nullptr;
	}
	const auto IsSeenBy = [Vision](const FFighter& Seeker, const FFighter& Other)
	{
		return !Vision || Vision->IsVisible(Seeker.State->OwnerPlayer, Other.Location);
	};
	// D5 (DuelInProgress.md; GDD 10.2, D18): while a duel is on, a hero in it is hit by his adversary only and hits nobody else.
	// The script book (HOLDuel::BookCanHit) answers, once a step, for every pair (attacker hero slot, victim hero slot), -1 for
	// anything that is not a hero; the pass below only reads that table. A unit never aims at, nor strikes, a protected hero.
	constexpr int32 DuelStride = FHOLMatchConfig::MaxSlots + 1;
	TArray<uint8> DuelHits;
	// D6: the percent of his damage each duellist deals (his posture), by hero slot; 100 for anyone else.
	TArray<int32> DuelDamagePercent;
	// D120: each duellist's adversary by hero slot (-1 for anyone else): the contact is automatic.
	TArray<int32> DuelAdversary;
	UHOLDuelSubsystem* Book = UHOLDuelSubsystem::Find(World);
	const bool bDuelOn = Book && Book->IsDuelOn();
	if (bDuelOn)
	{
		DuelHits.SetNumZeroed(DuelStride * DuelStride);
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			DuelDamagePercent.Add(Book->GetDamagePercentNow(Slot));
			DuelAdversary.Add(Book->GetAdversaryNow(Slot));
		}
		for (int32 Attacker = -1; Attacker < FHOLMatchConfig::MaxSlots; ++Attacker)
		{
			for (int32 Victim = -1; Victim < FHOLMatchConfig::MaxSlots; ++Victim)
			{
				DuelHits[(Attacker + 1) * DuelStride + Victim + 1] = Book->CanHitNow(Attacker, Victim) ? 1 : 0;
			}
		}
	}
	const auto HeroSlotOf = [](const FFighter& Fighter)
	{
		return HOLIsHero(Fighter.Constants->ArmorCategoryMask) ? static_cast<int32>(Fighter.State->OwnerPlayer) : INDEX_NONE;
	};
	const auto CanHitSlots = [&DuelHits, bDuelOn](const int32 AttackerHero, const int32 VictimHero)
	{
		return !bDuelOn || DuelHits[(AttackerHero + 1) * DuelStride + VictimHero + 1] != 0;
	};
	const auto CanTouch = [&](const FFighter& Attacker, const FFighter& Victim)
	{
		return !bDuelOn || CanHitSlots(HeroSlotOf(Attacker), HeroSlotOf(Victim));
	};
	// D6: a duellist's damage follows his posture (HOLDuelKit); whole percents, the same on every machine.
	const auto DuelScaled = [&](const FFighter& Attacker, const float Damage)
	{
		const int32 HeroSlot = bDuelOn ? HeroSlotOf(Attacker) : INDEX_NONE;
		return HeroSlot != INDEX_NONE && DuelDamagePercent.IsValidIndex(HeroSlot) ? Damage * static_cast<float>(DuelDamagePercent[HeroSlot]) * 0.01f : Damage;
	};

	// The step's units, in identifier order (the outcome does not depend on the chunks). Each one's camp read once here.
	const FHOLCampTable CampTable = FHOLCampTable::FromWorld(World);
	StepCamps = CampTable;
	Fighters.Reset();
	MaxRadius = 1.f;
	// J7c.4, S2 (hol.Combat.SkipAttackerCount): the counts zeroed here (the attackers' count below starts with it; nothing
	// reads them before), and whether any unit has a target (none: no attacker to count).
	const bool bZeroAtGather = bFighting && SkipAttackerCount != 0;
	bool bAnyTarget = false;
	// J8c.2: the living heroes are aura sources (J8c-Q1); the cries count down on every unit, whether combat is on or not.
	AuraSources.Reset();
	ShieldsUpOwners = 0;
	// T6: each slot's attack technologies, from the player states (the server has them all; a technology is read as the gather's).
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		MeleeAttackPercent[Slot] = 0;
		RangedAttackPercent[Slot] = 0;
		InfantryArmorPercent[Slot] = 0;
		CavalryArmorPercent[Slot] = 0;
		MangonelDamagePercent[Slot] = 0;
	}
	if (const AGameStateBase* GameState = World ? World->GetGameState() : nullptr)
	{
		for (const APlayerState* PlayerState : GameState->PlayerArray)
		{
			const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
			const int32 Slot = HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE;
			if (Slot >= 0 && Slot < FHOLMatchConfig::MaxSlots)
			{
				MeleeAttackPercent[Slot] = HOLPlayerState->GetTechnologyBonusPercent(HOLTechnology::GetAttackStat(EHOLAttackType::Melee));
				RangedAttackPercent[Slot] = HOLPlayerState->GetTechnologyBonusPercent(HOLTechnology::GetAttackStat(EHOLAttackType::Ranged));
				InfantryArmorPercent[Slot] = HOLPlayerState->GetTechnologyBonusPercent(HOLTechnology::InfantryArmorStat);
				CavalryArmorPercent[Slot] = HOLPlayerState->GetTechnologyBonusPercent(HOLTechnology::CavalryArmorStat);
				MangonelDamagePercent[Slot] = HOLPlayerState->GetTechnologyBonusPercent(HOLSiege::GetDamageStat(FName(TEXT("Mangonel"))));
			}
		}
	}
	DawnStepOwners = 0;
	EntityQuery.ForEachEntityChunk(Context, [this, bZeroAtGather, &bAnyTarget, &CampTable, StepMs](FMassExecutionContext& Context)
	{
		const TArrayView<FHOLCombatFragment> Combats = Context.GetMutableFragmentView<FHOLCombatFragment>();
		const TArrayView<FHOLUnitStateFragment> States = Context.GetMutableFragmentView<FHOLUnitStateFragment>();
		const TArrayView<FHOLMoveOrderFragment> Orders = Context.GetMutableFragmentView<FHOLMoveOrderFragment>();
		const TConstArrayView<FHOLGatherFragment> Gathers = Context.GetFragmentView<FHOLGatherFragment>();
		const TConstArrayView<FHOLBuildFragment> Builds = Context.GetFragmentView<FHOLBuildFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const TArrayView<FMassDesiredMovementFragment> Movements = Context.GetMutableFragmentView<FMassDesiredMovementFragment>();
		const FHOLUnitTypeConstants& Constants = Context.GetConstSharedFragment<FHOLUnitTypeConstants>();
		MaxRadius = FMath::Max(MaxRadius, Constants.Radius);
		const bool bHeroes = HOLIsHero(Constants.ArmorCategoryMask);
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			HOLHeroAbility::Tick(Combats[EntityIt].CloseRanksMs, StepMs);
			if (bHeroes && States[EntityIt].CurrentHealth > 0.f && Combats[EntityIt].DeathStep < 0)
			{
				const FVector Location = Transforms[EntityIt].GetTransform().GetLocation();
				AuraSources.Add(FAuraSource{ FVector2D(Location), States[EntityIt].OwnerPlayer, true });
			}
			FFighter& Fighter = Fighters.AddDefaulted_GetRef();
			Fighter.Entity = Context.GetEntity(EntityIt);
			Fighter.UnitId = States[EntityIt].UnitId;
			Fighter.Location = Transforms[EntityIt].GetTransform().GetLocation();
			Fighter.Combat = &Combats[EntityIt];
			Fighter.State = &States[EntityIt];
			Fighter.Camp = CampTable.Get(States[EntityIt].OwnerPlayer);
			Fighter.Order = &Orders[EntityIt];
			Fighter.MoveTarget = &MoveTargets[EntityIt];
			Fighter.Movement = &Movements[EntityIt];
			Fighter.Constants = &Constants;
			// N4: a neutral camp's member at its post or walking back is left out of the fight as a worker at its task is (it answers a blow by its camp's thought).
			Fighter.bBusy = Gathers[EntityIt].IsGathering() || Builds[EntityIt].IsBuilding()
				|| (States[EntityIt].OwnerPlayer == FHOLMatchConfig::NeutralOwner && HOLCamps::IsPassive(Fighter.UnitId));
			if (bZeroAtGather)
			{
				bAnyTarget |= Combats[EntityIt].Target.IsSet();
				Combats[EntityIt].AttackerCount = 0;
			}
		}
	});
	if (Fighters.Num() == 0)
	{
		TotalArrowsLost += Arrows.Num(); // no target left for any
		Arrows.Reset();
		return;
	}
	// J8c.2: the banners standing (J8c-Q3, even once their hero is dead), and the cries cast since the last step (J8c-Q2):
	// each crying hero's living units within his aura's reach now are under it for its whole duration (refreshed).
	if (UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>())
	{
		const int32 HeroSources = AuraSources.Num();
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			// T4: the talents that change a hit (Boucliers levees), from the hero sheets of this step.
			ShieldsUpOwners |= HOLHeroTalent::GrantsAuraArmor(Heroes->GetSheets(Slot).Private.TalentPicks) ? (1u << Slot) : 0u;
			DawnStepOwners |= HOLHeroTalent::GrantsDawnStep(Heroes->GetSheets(Slot).Private.TalentPicks) ? (1u << Slot) : 0u;
			FVector BannerLocation;
			if (Heroes->GetBanner(Slot, BannerLocation))
			{
				AuraSources.Add(FAuraSource{ FVector2D(BannerLocation), static_cast<uint8>(Slot), false });
			}
			if (!Heroes->ConsumeCloseRanks(Slot))
			{
				continue;
			}
			for (int32 Source = 0; Source < HeroSources; ++Source)
			{
				if (AuraSources[Source].Owner != Slot)
				{
					continue;
				}
				int32 Cried = 0;
				for (FFighter& Fighter : Fighters)
				{
					if (Fighter.State->OwnerPlayer == Slot && Fighter.State->CurrentHealth > 0.f
						&& HOLHeroAbility::IsWithinAura(static_cast<float>(FVector2D::Distance(AuraSources[Source].Location, FVector2D(Fighter.Location)))))
					{
						Fighter.Combat->CloseRanksMs = HOLHeroAbility::CloseRanksDurationMs;
						++Cried;
					}
				}
				UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld slot %d cries Serrez les rangs, %d unit(s) under it"), StepIndex, Slot, Cried);
			}
		}
	}
	// D581: the speed of each unit under a cry, for the movement processor (a formation held in place, or slowed with Pas de l'Aube).
	for (FFighter& Fighter : Fighters)
	{
		Fighter.Combat->CloseRanksSpeedPercent = static_cast<uint8>(HOLHeroAbility::GetCloseRanksSpeedPercent(
			Fighter.Combat->CloseRanksMs > 0, (DawnStepOwners & (1u << Fighter.State->OwnerPlayer)) != 0, CloseRanksSpeed));
	}
	if (!bFighting)
	{
		Deaths.Reset();
		for (FFighter& Fighter : Fighters)
		{
			// Nobody strikes (EtapeDefini I): a unit caught striking is no longer Attacking (its gesture, the HUD, the clients),
			// its target kept for when combat is back (then looked up as at any step: gone, it is let go).
			Fighter.Combat->bAttacking = false;
			if (Fighter.State->CurrentHealth <= 0)
			{
				FinishDying(World, *Units, Fighter, StepIndex);
			}
		}
		if (Deaths.Num() > 0)
		{
			Units->RequestDestroyUnits(Deaths);
		}
		return;
	}
	// J7c.4, S2 (hol.Combat.FastIdle): a step where every unit's outcome below is its own idle tick, whatever the others.
	// Such a unit stands, has no target and no last attacker (nothing to look up, no attacker to count: 0), is not
	// approaching nor attacking (no walk to stop, nothing to disengage), is not on an attack-move (no contact search, no end
	// of attack-move) and does not search this step (busy, walking an order, an ordered target, or not its search step).
	// With no arrow in flight, the pass below then only sets AttackerCount 0 and, for each unit: busy, TickIdle; else the
	// branch without target (bFlanking false, FlankWaitSeconds 0, TickIdle). Checked first, then applied: no unit changes
	// another, so the order does not matter.
	if (FastIdle != 0 && Arrows.Num() == 0)
	{
		bool bIdle = true;
		for (int32 Index = 0; Index < Fighters.Num() && bIdle; ++Index)
		{
			const FFighter& Fighter = Fighters[Index];
			const FHOLCombatFragment& Combat = *Fighter.Combat;
			const bool bWalkingOrder = Fighter.Order->Phase != EHOLMovePhase::None && !Combat.bApproaching;
			const bool bMaySeek = HOLCombat::MaySeek(bWalkingOrder, Combat.Mode == EHOLCombatMode::AttackMove, Combat.bOrderedTarget,
				!Combat.OrderedBuilding.IsExplicitlyNull(), HOLCombat::IsWalkStalled(Fighter.Order->StuckSteps, WalkRiposteSteps));
			const bool bSearchStep = ((static_cast<int64>(Fighter.UnitId) + StepIndex) % SearchSteps) == 0;
			bIdle = Fighter.State->CurrentHealth > 0 && !Combat.Target.IsSet() && Combat.TargetBuilding.IsExplicitlyNull() && Combat.OrderedBuilding.IsExplicitlyNull() && !Combat.LastAttacker.IsSet() && !Combat.bApproaching
				&& !Combat.bAttacking && Combat.Mode != EHOLCombatMode::AttackMove && (Fighter.bBusy || !bMaySeek || !bSearchStep);
		}
		if (bIdle)
		{
			for (FFighter& Fighter : Fighters)
			{
				FHOLCombatFragment& Combat = *Fighter.Combat;
				Combat.AttackerCount = 0;
				if (!Fighter.bBusy)
				{
					Combat.bFlanking = false;
					Combat.FlankWaitSeconds = 0.f;
				}
				HOLCombat::TickIdle(Combat.CooldownMs, StepMs);
			}
			Deaths.Reset();
			return;
		}
	}
	const bool bFast = FastPaths != 0;
	if (!bFast)
	{
		Fighters.Sort([](const FFighter& A, const FFighter& B) { return A.UnitId < B.UnitId; });
	}
	else
	{
		// J7c.4, S3: the identifiers are unique (the unit subsystem's table), so sorting small (identifier, index) keys and
		// moving the fighters once gives the very order of the sort above; nothing to do when the chunks gave it already.
		bool bSorted = true;
		for (int32 Index = 1; Index < Fighters.Num() && bSorted; ++Index)
		{
			bSorted = Fighters[Index - 1].UnitId < Fighters[Index].UnitId;
		}
		if (!bSorted)
		{
			SortKeys.Reset(Fighters.Num());
			for (int32 Index = 0; Index < Fighters.Num(); ++Index)
			{
				SortKeys.Add(static_cast<uint64>(Fighters[Index].UnitId) << 32 | static_cast<uint32>(Index));
			}
			SortKeys.Sort();
			SortedFighters.Reset(Fighters.Num());
			for (const uint64 Key : SortKeys)
			{
				SortedFighters.Add(Fighters[static_cast<int32>(Key & 0xFFFFFFFFull)]);
			}
			Swap(Fighters, SortedFighters);
		}
		// A new stamp makes every entry of the steps before stale (all of them cleared once, at its wrap).
		if (++LookupStamp == 0)
		{
			FighterOfEntity.Reset();
			LookupStamp = 1;
		}
	}
	IndexOfEntity.Reset();
	Cells.SetNumUninitialized(Fighters.Num());
	for (int32 Index = 0; Index < Fighters.Num(); ++Index)
	{
		Cells[Index] = FIntPoint(FMath::FloorToInt32(Fighters[Index].Location.X / CellSize), FMath::FloorToInt32(Fighters[Index].Location.Y / CellSize));
	}
	// J7c.4, S3: the dense grid of the same cells in place of the hashed one, when the box allows it.
	bDenseGrid = bFast && BuildDenseGrid();
	if (bFast)
	{
		SeenStamp.Init(0, Fighters.Num());
		SearchStamp = 0;
	}
	const bool bPreferFree = PreferFreeTarget != 0 && bDenseGrid;
	const bool bStopAtContact = StopAtContact != 0 && bDenseGrid;
	if (bPreferFree)
	{
		PlacesTaken.Init(-1, Fighters.Num());
	}
	if (!bDenseGrid)
	{
		CellHeads.Init(INDEX_NONE, Buckets);
		NextInCell.SetNumUninitialized(Fighters.Num());
	}
	// Inserted from the last, so each bucket lists its units in identifier order.
	for (int32 Index = Fighters.Num() - 1; Index >= 0; --Index)
	{
		const FFighter& Fighter = Fighters[Index];
		if (bFast)
		{
			const int32 EntityIndex = Fighter.Entity.Index;
			if (EntityIndex >= FighterOfEntity.Num())
			{
				FighterOfEntity.SetNum(EntityIndex + 1);
			}
			FighterOfEntity[EntityIndex] = FEntitySlot{ Fighter.Entity.SerialNumber, Index, LookupStamp };
		}
		else
		{
			IndexOfEntity.Add(Fighter.Entity, Index);
		}
		if (!bDenseGrid)
		{
			int32& Head = CellHeads[BucketOf(Cells[Index])];
			NextInCell[Index] = Head;
			Head = Index;
		}
	}
	const auto FindFighter = [this, bFast](const FMassEntityHandle Entity) -> int32
	{
		if (!Entity.IsSet())
		{
			return INDEX_NONE;
		}
		if (bFast)
		{
			// The entry of this step for that entity index, and that very entity (not a former one of the same index).
			if (!FighterOfEntity.IsValidIndex(Entity.Index))
			{
				return INDEX_NONE;
			}
			const FEntitySlot& Slot = FighterOfEntity[Entity.Index];
			return Slot.Stamp == LookupStamp && Slot.SerialNumber == Entity.SerialNumber ? Slot.Fighter : INDEX_NONE;
		}
		const int32* Index = IndexOfEntity.Find(Entity);
		return Index ? *Index : INDEX_NONE;
	};
	// Attackers of each unit at the step's start (HOLCombat::FCandidate::AttackerCount).
	if (!bZeroAtGather) // else zeroed at the gathering
	{
		for (FFighter& Fighter : Fighters)
		{
			Fighter.Combat->AttackerCount = 0;
		}
	}
	if (!bZeroAtGather || bAnyTarget) // else no target: no attacker
	{
		for (const FFighter& Fighter : Fighters)
		{
			const int32 Target = FindFighter(Fighter.Combat->Target);
			if (Target != INDEX_NONE)
			{
				++Fighters[Target].Combat->AttackerCount;
			}
		}
	}
	// J7c.4, S3: the lists of the units likely to search this step (by the step's start: a search step, free to seek, no
	// target standing in reach), gathered ahead in parallel. GatherNearby reads only what no unit changes during the
	// step; the serial pass below reads the health, the targets and the attackers then, as before, and a unit not
	// foreseen (or with another reach) gathers there. A wrong guess costs time only.
	// The contact search's reach (hol.Combat.StopAtContact), the same ahead and in the serial pass.
	const auto ContactReachOf = [this](const FHOLUnitTypeConstants& Constants)
	{
		const double Zone = FMath::Max(static_cast<double>(ZoneOfControl), 0.0);
		const double ContactReach = Constants.Radius + Constants.AttackRange + MaxRadius;
		return Zone > 0.0 ? FMath::Max(ContactReach, Zone) : ContactReach;
	};
	const bool bParallelAhead = ParallelAhead != 0 && bDenseGrid;
	if (bDenseGrid)
	{
		AheadSlotOfFighter.Init(INDEX_NONE, Fighters.Num());
		AheadFighters.Reset();
		AheadReach.Reset();
		ContactSlotOfFighter.Init(INDEX_NONE, Fighters.Num());
		ContactFighters.Reset();
		for (int32 Index = 0; Index < Fighters.Num(); ++Index)
		{
			const FFighter& Fighter = Fighters[Index];
			const FHOLCombatFragment& Combat = *Fighter.Combat;
			const FHOLUnitTypeConstants& Constants = *Fighter.Constants;
			const bool bWalkingOrder = Fighter.Order->Phase != EHOLMovePhase::None && !Combat.bApproaching;
			const bool bMaySeekNow = HOLCombat::MaySeek(bWalkingOrder, Combat.Mode == EHOLCombatMode::AttackMove, Combat.bOrderedTarget, !Combat.OrderedBuilding.IsExplicitlyNull(),
				HOLCombat::IsWalkStalled(Fighter.Order->StuckSteps, WalkRiposteSteps));
			// Perf: a melee walking to fight will likely search for contact (the serial pass's test, by the step's start).
			if (bParallelAhead && bStopAtContact && bMaySeekNow && Fighter.State->CurrentHealth > 0 && !Fighter.bBusy && Fighter.Order->Phase != EHOLMovePhase::None
				&& !HOLCombat::IsRangedAttack(Constants.AttackRange) && (Combat.TargetBuilding.IsExplicitlyNull() || Constants.AggressionRadius > 0.f))
			{
				ContactSlotOfFighter[Index] = ContactFighters.Add(Index);
			}
			if (Fighter.State->CurrentHealth <= 0 || Fighter.bBusy || ((static_cast<int64>(Fighter.UnitId) + StepIndex) % SearchSteps) != 0
				|| !bMaySeekNow)
			{
				continue;
			}
			HOLCombat::FSeeker Seeker;
			Seeker.AttackRange = Constants.AttackRange;
			Seeker.AggressionRadius = Constants.AggressionRadius;
			Seeker.bCanAttackUnits = Constants.Attack > 0; // S4 (SQ1 = B): the Ram takes no unit on by itself
			Seeker.Mode = Combat.Mode == EHOLCombatMode::Hold ? HOLCombat::ESeekMode::Hold
				: Combat.Mode == EHOLCombatMode::AttackMove ? HOLCombat::ESeekMode::AttackMove : HOLCombat::ESeekMode::Auto;
			const float SearchRadius = HOLCombat::GetSearchRadius(Seeker);
			const int32 Target = FindFighter(Combat.Target);
			if (SearchRadius <= 0.f || (Target != INDEX_NONE && Fighters[Target].State->CurrentHealth > 0
				&& HOLCombat::IsInRange(FVector2D(Fighter.Location), Constants.Radius, FVector2D(Fighters[Target].Location), Fighters[Target].Constants->Radius, Constants.AttackRange)))
			{
				continue;
			}
			AheadSlotOfFighter[Index] = AheadFighters.Add(Index);
			AheadReach.Add(SearchRadius + Constants.Radius + MaxRadius);
		}
		if (AheadLists.Num() < AheadFighters.Num())
		{
			AheadLists.SetNum(AheadFighters.Num());
		}
		if (ContactLists.Num() < ContactFighters.Num())
		{
			ContactLists.SetNum(ContactFighters.Num());
		}
		ParallelFor(TEXT("HOLCombatAhead"), AheadFighters.Num() + ContactFighters.Num(), 32, [this, &ContactReachOf](const int32 Slot)
		{
			if (Slot < AheadFighters.Num())
			{
				GatherNearby(AheadFighters[Slot], AheadReach[Slot], AheadLists[Slot]);
				return;
			}
			const int32 Contact = Slot - AheadFighters.Num();
			GatherNearby(ContactFighters[Contact], ContactReachOf(*Fighters[ContactFighters[Contact]].Constants), ContactLists[Contact]);
		});
	}
	// Perf (hol.Combat.ParallelAhead): a unit died since the places were counted ahead (they may count it).
	bool bDeathSincePlaces = false;
	// Perf (hol.Combat.ParallelSearch): a fighter whose target, attackers or life changes, once the searches are made ahead.
	bool bTouchSearches = false;
	const auto Touch = [&](const int32 Other)
	{
		if (bTouchSearches && Other != INDEX_NONE)
		{
			SearchTouched[Other] = 1;
		}
	};
	// J8b.2: the XP of a death for the killer side's hero (J8b-Q1, Q2, Q5): killed by him (combat), by one of his troops
	// near him (command, the victim within HOLHeroXP::CommandRadius of him), nothing while he is dead (D06).
	UHOLHeroSubsystem* Heroes = World ? World->GetSubsystem<UHOLHeroSubsystem>() : nullptr;
	const auto AwardKillXP = [&](const FFighter& Victim, const int32 KillerPlayer, const uint32 KillerId)
	{
		const uint32 HeroId = Heroes && KillerPlayer >= 0 ? Units->FindHeroUnit(KillerPlayer) : 0;
		const FHOLUnitTypeRow* VictimRow = HeroId != 0 ? Units->FindTypeRow(Victim.Constants->TypeName) : nullptr;
		if (!VictimRow)
		{
			return;
		}
		HOLHeroXP::FKillFacts Facts;
		Facts.VictimValue = HOLHeroXP::GetValue(VictimRow->Cost);
		Facts.bVictimHero = HOLIsHero(Victim.Constants->ArmorCategoryMask);
		Facts.VictimLevel = Victim.Combat->Level;
		Facts.bKilledByHero = KillerId == HeroId;
		const int32 HeroIndex = FindFighter(Units->FindEntity(HeroId));
		if (HeroIndex != INDEX_NONE)
		{
			const FFighter& HeroFighter = Fighters[HeroIndex];
			Facts.HeroLevel = HeroFighter.Combat->Level;
			Facts.bHeroAlive = HeroFighter.State->CurrentHealth > 0.f && HeroFighter.Combat->DeathStep < 0;
			Facts.DistanceToHero = static_cast<float>(FVector::Dist2D(Victim.Location, HeroFighter.Location));
		}
		TOptional<EHOLXPSource> Source;
		const int32 XP = HOLHeroXP::GetKillXP(Facts, Source);
		if (Source.IsSet())
		{
			Heroes->AddXP(KillerPlayer, XP, Source.GetValue());
		}
	};
	// A unit killed in the step: its own target loses an attacker at once (KillUnit disengages it), not at the next count.
	const auto Kill = [&](FFighter& Victim, const int32 KillerPlayer, const uint32 KillerId)
	{
		bDeathSincePlaces = true;
		Touch(static_cast<int32>(&Victim - Fighters.GetData()));
		const int32 FormerTarget = FindFighter(Victim.Combat->Target);
		if (FormerTarget != INDEX_NONE)
		{
			--Fighters[FormerTarget].Combat->AttackerCount;
			Touch(FormerTarget);
		}
		KillUnit(World, *Units, Victim, StepIndex, KillerPlayer);
		AwardKillXP(Victim, KillerPlayer, KillerId);
	};

	// D345.2b: the buildings that can be attacked this step (a bare foundation is unseen by the enemy: never a target).
	StepBuildings.Reset();
	for (FHOLBuildingIterator It(World); It; ++It)
	{
		AHOLBuilding* Building = *It;
		if (!IsValid(Building) || Building->IsActorBeingDestroyed() || Building->IsFoundation() || !Building->GetType() || Building->GetHealth() <= 0)
		{
			continue;
		}
		FStepBuilding& Step = StepBuildings.AddDefaulted_GetRef();
		Step.Building = Building;
		Step.Box = Building->GetBodyBounds();
		Step.VisibleBox = Building->GetVisibleBodyBounds();
		Step.Id = Building->GetUniqueID();
		Step.OwnerPlayer = Building->GetOwnerPlayer();
		Step.Camp = CampTable.Get(Step.OwnerPlayer);
	}
	// A building hit to 0 this step: no longer a target of anyone (true when it is gone).
	const auto MarkBuildingGone = [this](const AHOLBuilding* Building)
	{
		if (!IsValid(Building) || Building->IsActorBeingDestroyed() || Building->GetHealth() <= 0)
		{
			for (FStepBuilding& Step : StepBuildings)
			{
				Step.bGone |= Step.Building == Building;
			}
			return true;
		}
		return false;
	};

	// The arrows that land this step (J7c.2), in release order, before the units are taken: a unit killed by one dies as
	// one killed in melee, its attackers look for another this step.
	int32 Kept = 0;
	for (int32 ArrowIndex = 0; ArrowIndex < Arrows.Num(); ++ArrowIndex)
	{
		const FArrow Arrow = Arrows[ArrowIndex];
		if (Arrow.ArrivalStep > StepIndex)
		{
			Arrows[Kept++] = Arrow;
			continue;
		}
		// D345.2b: an arrow at a building, its damage through DamageBuilding (the building's ranged armour off), lost if gone.
		if (!Arrow.TargetBuilding.IsExplicitlyNull())
		{
			AHOLBuilding* Building = Arrow.TargetBuilding.Get();
			if (Building && FindStepBuilding(Building) != INDEX_NONE)
			{
				++TotalArrowsLanded;
				// A building's arrow carries the shooter's whole BuildingAttack (buildings keep integer health).
				const int32 Dealt = UHOLConstructionSubsystem::DamageBuilding(*Building, FMath::RoundToInt(Arrow.Damage), static_cast<EHOLAttackType>(Arrow.BuildingAttackType));
				const int32 Shooter = FindFighter(Arrow.Shooter);
				if (Shooter != INDEX_NONE)
				{
					Fighters[Shooter].Combat->DamageDealt += Dealt;
				}
				if (LogHits != 0)
				{
					UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld arrow of #%u %s lands on a building for %d"), StepIndex, Arrow.ShooterId,
						*Arrow.ShooterType.ToString(), Dealt);
				}
				MarkBuildingGone(Building);
			}
			else
			{
				++TotalArrowsLost;
			}
			continue;
		}
		int32 Victim = FindFighter(Arrow.Target);
		if (bDuelOn && Victim != INDEX_NONE)
		{
			// D5: an arrow that reaches a protected hero (the duel began in its flight, or it comes from another hero's side) is lost.
			const int32 ShooterIndex = FindFighter(Arrow.Shooter);
			if (!CanHitSlots(ShooterIndex != INDEX_NONE ? HeroSlotOf(Fighters[ShooterIndex]) : INDEX_NONE, HeroSlotOf(Fighters[Victim])))
			{
				Victim = INDEX_NONE;
			}
		}
		float Health = Victim != INDEX_NONE ? Fighters[Victim].State->CurrentHealth : 0.f;
		// J8c.2: the morale effects where it lands, now (aura, cry), not where it was released.
		const float Landed = Victim != INDEX_NONE ? HOLHeroAbility::GetReducedDamage(Arrow.Damage, GetDamageTakenPercent(Fighters[Victim])) : Arrow.Damage;
		const HOLCombat::EArrowOutcome Outcome = HOLCombat::ResolveArrow(Victim != INDEX_NONE, Health, Landed);
		if (Outcome == HOLCombat::EArrowOutcome::Lost)
		{
			++TotalArrowsLost;
			if (LogHits != 0)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld arrow of #%u %s at #%u lost (target dead), released step %lld"), StepIndex, Arrow.ShooterId,
					*Arrow.ShooterType.ToString(), Arrow.TargetId, Arrow.ReleaseStep);
			}
			continue;
		}
		++TotalArrowsLanded;
		FFighter& Enemy = Fighters[Victim];
		Enemy.State->CurrentHealth = Health;
		const int32 Shooter = FindFighter(Arrow.Shooter);
		if (Shooter != INDEX_NONE)
		{
			Enemy.Combat->LastAttacker = Arrow.Shooter;
			Enemy.Combat->LastHitStep = StepIndex;
			if (Enemy.State->OwnerPlayer == FHOLMatchConfig::NeutralOwner)
			{
				HOLCamps::NoteHit(Enemy.UnitId, Fighters[Shooter].State->OwnerPlayer, Landed);
			}
			Fighters[Shooter].Combat->DamageDealt += Landed;
		}
		if (LogHits != 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld arrow of #%u %s lands on #%u %s for %g, %g HP left (released step %lld, %lld steps)"), StepIndex,
				Arrow.ShooterId, *Arrow.ShooterType.ToString(), Enemy.UnitId, *Enemy.Constants->TypeName.ToString(), static_cast<double>(Landed), static_cast<double>(Health), Arrow.ReleaseStep,
				StepIndex - Arrow.ReleaseStep);
		}
		if (Outcome == HOLCombat::EArrowOutcome::Kill)
		{
			++TotalKills;
			if (Shooter != INDEX_NONE)
			{
				++Fighters[Shooter].Combat->Kills;
			}
			Kill(Enemy, Arrow.ShooterSlot, Arrow.ShooterId);
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld #%u %s of slot %d killed by #%u %s of slot %d (%d hits struck, by arrow)"), StepIndex, Enemy.UnitId,
				*Enemy.Constants->TypeName.ToString(), Enemy.State->OwnerPlayer, Arrow.ShooterId, *Arrow.ShooterType.ToString(), Arrow.ShooterSlot,
				Shooter != INDEX_NONE ? Fighters[Shooter].Combat->Hits : 0);
		}
		// S4 (SQ1 = B, D614): an area (the Mangonel's 250 cm) on a unit hit: the enemies of the shooter's camp around the target (centre to
		// centre, the circle's edge in) take the same blow, none of its own camp (no friendly fire), in the fighters' order (the hits come
		// off one after another, D468). The target itself was struck above.
		if (Arrow.AreaRadius > 0.f)
		{
			const FVector2D Centre(Enemy.Location);
			const int32 ShooterHero = Shooter != INDEX_NONE ? HeroSlotOf(Fighters[Shooter]) : INDEX_NONE;
			int32 NumSplashed = 0;
			for (int32 Other = 0; Other < Fighters.Num(); ++Other)
			{
				FFighter& Around = Fighters[Other];
				if (Other == Victim || Around.Camp == Arrow.ShooterCamp || Around.State->CurrentHealth <= 0
					|| (StepCamps.IsDiplomacy() && !StepCamps.FiresAtOnce(Arrow.ShooterSlot, Around.State->OwnerPlayer)) // allies and Neutrals take no splash (D369)
					|| !HOLCombat::IsInArea(Centre, FVector2D(Around.Location), Arrow.AreaRadius) || !CanHitSlots(ShooterHero, HeroSlotOf(Around)))
				{
					continue;
				}
				const float Splash = HOLHeroAbility::GetReducedDamage(Arrow.Damage, GetDamageTakenPercent(Around));
				Around.State->CurrentHealth -= Splash;
				++NumSplashed;
				Around.Combat->LastAttacker = Arrow.Shooter;
				Around.Combat->LastHitStep = StepIndex;
				if (Shooter != INDEX_NONE)
				{
					Fighters[Shooter].Combat->DamageDealt += Splash;
				}
				if (Around.State->CurrentHealth <= 0)
				{
					++TotalKills;
					if (Shooter != INDEX_NONE)
					{
						++Fighters[Shooter].Combat->Kills;
					}
					Kill(Around, Arrow.ShooterSlot, Arrow.ShooterId);
				}
			}
			if (LogHits != 0)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld area of #%u %s around #%u: %d other units hit within %.0f cm"), StepIndex, Arrow.ShooterId,
					*Arrow.ShooterType.ToString(), Arrow.TargetId, NumSplashed, static_cast<double>(Arrow.AreaRadius));
			}
		}
	}
	Arrows.SetNum(Kept, EAllowShrinking::No);

	// Perf (hol.Combat.ParallelAhead): the places around the living enemies the melee searchers gathered ahead will see
	// (and their targets), counted in parallel now, the arrows landed: what CountBodiesAround gives in the serial pass
	// while no unit dies (it reads only the positions, the constants and whether each body still stands).
	const bool bPlacesAhead = bParallelAhead && bPreferFree && AheadFighters.Num() > 0;
	if (bPlacesAhead)
	{
		AheadPlaces.Init(-1, Fighters.Num());
		AheadPlaceFighters.Reset();
		const auto Mark = [this](const int32 Other)
		{
			if (Other != INDEX_NONE && AheadPlaces[Other] == -1 && Fighters[Other].State->CurrentHealth > 0)
			{
				AheadPlaces[Other] = -2; // to count
				AheadPlaceFighters.Add(Other);
			}
		};
		// Lists longer than the fighters together (a dense battle): every living fighter counted, rather than each marked.
		int32 Listed = 0;
		for (int32 Slot = 0; Slot < AheadFighters.Num(); ++Slot)
		{
			Listed += AheadLists[Slot].Num();
		}
		if (Listed > Fighters.Num())
		{
			for (int32 Other = 0; Other < Fighters.Num(); ++Other)
			{
				Mark(Other);
			}
		}
		for (int32 Slot = 0; Slot < AheadFighters.Num() && Listed <= Fighters.Num(); ++Slot)
		{
			const FFighter& Seeker = Fighters[AheadFighters[Slot]];
			if (HOLCombat::IsRangedAttack(Seeker.Constants->AttackRange))
			{
				continue; // a shooter's candidates are never counted
			}
			for (const int32 Other : AheadLists[Slot])
			{
				Mark(Other);
			}
			Mark(FindFighter(Seeker.Combat->Target));
		}
		ParallelFor(TEXT("HOLCombatPlaces"), AheadPlaceFighters.Num(), 32, [this](const int32 Slot)
		{
			const int32 Other = AheadPlaceFighters[Slot];
			AheadPlaces[Other] = CountBodiesAround(Other, UE::HeroesOfLiberty::Combat::PlaceGap, HOLCombat::MaxAttackers);
		});
		bDeathSincePlaces = false;
	}
	// The places taken around a fighter, counted once a step, the first time needed.
	const auto PlacesAround = [&](const int32 Other)
	{
		if (PlacesTaken[Other] < 0)
		{
			PlacesTaken[Other] = bPlacesAhead && !bDeathSincePlaces && AheadPlaces[Other] >= 0 ? AheadPlaces[Other]
				: CountBodiesAround(Other, PlaceGap, HOLCombat::MaxAttackers);
		}
		return PlacesTaken[Other];
	};
	// D641: whether a body of SelfRadius could stand touching Target (a free place round it, the bodies of any side read by HOLCombat::HasFreePlaceAround).
	const auto GatherRoomBodies = [&](const int32 Self, const int32 Enemy, TArray<HOLCombat::FBody>& OutBodies)
	{
		TArray<int32> RoomNearby;
		TArray<int32> RoomFriends;
		GatherNearby(Enemy, 400., RoomNearby);
		GatherFriends(Enemy, 400., RoomFriends);
		RoomNearby.Append(RoomFriends);
		for (const int32 Body : RoomNearby)
		{
			if (Body != Self && Body != Enemy && Fighters[Body].State->CurrentHealth > 0)
			{
				OutBodies.Add({ FVector2D(Fighters[Body].Location), Fighters[Body].Constants->Radius });
			}
		}
	};
	const auto RoomAtTarget = [&](const int32 Self, const int32 Enemy, const float SelfRadius)
	{
		TArray<HOLCombat::FBody> RoomBodies;
		GatherRoomBodies(Self, Enemy, RoomBodies);
		return HOLCombat::HasFreePlaceAround(FVector2D(Fighters[Enemy].Location), Fighters[Enemy].Constants->Radius, SelfRadius, RoomBodies);
	};

	// Perf (hol.Combat.ParallelSearch): the searches of the units gathered ahead, made now in parallel, the arrows landed:
	// the serial pass's search as it would run if nothing it reads changed before the unit's turn (the same candidates,
	// read the same way, the place counts it would read now). At its turn, the serial pass weighs against that choice the
	// candidates changed since (Touch); the others rank as they did, behind it.
	const bool bSearchAhead = bParallelAhead && ParallelSearch != 0 && AheadFighters.Num() > 0;
	if (bSearchAhead)
	{
		SearchTouched.Init(0, Fighters.Num());
		if (AheadSearches.Num() < AheadFighters.Num())
		{
			AheadSearches.SetNum(AheadFighters.Num());
		}
		ParallelFor(TEXT("HOLCombatSearchAhead"), AheadFighters.Num(), 16, [&](const int32 Slot)
		{
			FAheadSearch& Search = AheadSearches[Slot];
			Search.bDone = false;
			Search.PlaceFighters.Reset();
			Search.PlaceFull.Reset();
			const int32 Index = AheadFighters[Slot];
			const FFighter& Fighter = Fighters[Index];
			const FHOLCombatFragment& Combat = *Fighter.Combat;
			const FHOLUnitTypeConstants& Constants = *Fighter.Constants;
			const FVector2D Position(Fighter.Location);
			// Its target and last attacker as its turn reads them (a dead one: none; EtapeDefini S: a last attacker whose hit is
			// HOLCombat::LastAttackerMemoryMs old, none).
			int32 Target = FindFighter(Combat.Target);
			if (Target != INDEX_NONE && Fighters[Target].State->CurrentHealth <= 0)
			{
				Target = INDEX_NONE;
			}
			int32 LastAttacker = FindFighter(Combat.LastAttacker);
			if (LastAttacker != INDEX_NONE && (Fighters[LastAttacker].State->CurrentHealth <= 0 || !HOLCombat::KeepsLastAttacker(StepIndex, Combat.LastHitStep, StepMs)))
			{
				LastAttacker = INDEX_NONE;
			}
			const FMassEntityHandle LastAttackerEntity = LastAttacker != INDEX_NONE ? Combat.LastAttacker : FMassEntityHandle();
			HOLCombat::FSeeker Seeker;
			Seeker.Position = Position;
			Seeker.Radius = Constants.Radius;
			Seeker.AttackRange = Constants.AttackRange;
			Seeker.AggressionRadius = Constants.AggressionRadius;
			Seeker.bCanAttackUnits = Constants.Attack > 0; // S4 (SQ1 = B): the Ram takes no unit on by itself
			Seeker.bRanged = HOLCombat::IsRangedAttack(Constants.AttackRange);
			Seeker.Mode = Combat.Mode == EHOLCombatMode::Hold ? HOLCombat::ESeekMode::Hold
				: Combat.Mode == EHOLCombatMode::AttackMove ? HOLCombat::ESeekMode::AttackMove : HOLCombat::ESeekMode::Auto;
			if (HOLCombat::GetSearchRadius(Seeker) <= 0.f || (bFast && Target != INDEX_NONE && Fighters[Target].Camp != Fighter.Camp
				&& HOLCombat::IsInRange(Position, Seeker.Radius, FVector2D(Fighters[Target].Location), Fighters[Target].Constants->Radius, Seeker.AttackRange)))
			{
				return; // no search, or its target kept (bKeepTarget)
			}
			const FHOLFrontLine* const ShiftLine = UE::HeroesOfLiberty::Simulation::RearFlank != 0 && UE::HeroesOfLiberty::Simulation::RearTargetMaxShift > 0.f
				&& bPreferFree && !Seeker.bRanged && Combat.Mode == EHOLCombatMode::AttackMove
				? UE::HeroesOfLiberty::Simulation::FrontLines.Find(Fighter.State->OwnerPlayer) : nullptr;
			TArray<HOLCombat::FCandidate, TInlineAllocator<128>> Found;
			TArray<int32, TInlineAllocator<128>> FoundFighters;
			const auto Add = [&](const int32 Other)
			{
				const FFighter& Enemy = Fighters[Other];
				HOLCombat::FCandidate& Candidate = Found.AddDefaulted_GetRef();
				Candidate.Id = Enemy.UnitId;
				Candidate.Position = FVector2D(Enemy.Location);
				Candidate.Radius = Enemy.Constants->Radius;
				Candidate.bAttacksMe = Enemy.Combat->Target == Fighter.Entity || Enemy.Entity == LastAttackerEntity;
				Candidate.bIsCurrentTarget = Other == Target;
				Candidate.AttackerCount = Enemy.Combat->AttackerCount - (Other == Target ? 1 : 0);
				if (bPreferFree && !Seeker.bRanged
					&& !HOLCombat::IsInRange(Position, Seeker.Radius, Candidate.Position, Candidate.Radius, Seeker.AttackRange))
				{
					const bool bFull = (AheadPlaces[Other] >= 0 ? AheadPlaces[Other] : CountBodiesAround(Other, PlaceGap, HOLCombat::MaxAttackers))
						>= HOLCombat::MaxAttackers;
					Candidate.bNoPlace = bFull || (ShiftLine && ShiftLine->Count > 0
						&& FMath::Abs(FVector2D::DotProduct(Candidate.Position - Position, ShiftLine->Axis)) > UE::HeroesOfLiberty::Simulation::RearTargetMaxShift);
					Search.PlaceFighters.Add(Other);
					Search.PlaceFull.Add(bFull);
				}
				FoundFighters.Add(Other);
			};
			for (const int32 Other : AheadLists[Slot])
			{
				if (Fighters[Other].State->CurrentHealth > 0 && IsAutoFoe(Fighter, Fighters[Other]))
				{
					Add(Other);
				}
			}
			for (const int32 Extra : { Target, LastAttacker })
			{
				if (Extra != INDEX_NONE && !FoundFighters.Contains(Extra) && Fighters[Extra].Camp != Fighter.Camp)
				{
					Add(Extra);
				}
			}
			const int32 Chosen = HOLCombat::ChooseTarget(Seeker, Found);
			Search.Chosen = Chosen == INDEX_NONE ? INDEX_NONE : FoundFighters[Chosen];
			Search.Target = Target;
			Search.LastAttacker = LastAttacker;
			Search.bDone = true;
		});
		bTouchSearches = true;
	}

	// EtapeDefini O.7b loop 2: the melee striking a building at the step's start, for a closing-in one to see whether a
	// striking friend bars its way in (only a building's attackers are looked at: a handful against all the units).
	BuildingStrikers.Reset();
	for (int32 Index = 0; Index < Fighters.Num(); ++Index)
	{
		const FHOLCombatFragment& Combat = *Fighters[Index].Combat;
		if (Combat.bAttacking && !Combat.Target.IsSet() && Combat.TargetBuilding.IsValid() && !Combat.BuildingInward.IsZero()
			&& Fighters[Index].State->CurrentHealth > 0)
		{
			BuildingStrikers.Add(Index);
		}
	}

	Deaths.Reset();
	for (int32 Index = 0; Index < Fighters.Num(); ++Index)
	{
		FFighter& Fighter = Fighters[Index];
		FHOLCombatFragment& Combat = *Fighter.Combat;
		const FHOLUnitTypeConstants& Constants = *Fighter.Constants;
		if (Fighter.State->CurrentHealth <= 0)
		{
			FinishDying(World, *Units, Fighter, StepIndex);
			continue;
		}
		// D120 (GDD 10.3): contact automatique. A hero in a duel has no free movement: he aims at his adversary, whatever his
		// player ordered (a walk, an attack on another, a hold), and walks to him as to an ordered target; the pair stay together
		// in their circle. Before the fast idle test below, which would let a far, idle hero stand.
		if (bDuelOn && HOLIsHero(Constants.ArmorCategoryMask))
		{
			const int32 AdversarySlot = DuelAdversary.IsValidIndex(Fighter.State->OwnerPlayer) ? DuelAdversary[Fighter.State->OwnerPlayer] : INDEX_NONE;
			const uint32 AdversaryId = AdversarySlot >= 0 && Units ? Units->FindHeroUnit(AdversarySlot) : 0u;
			const FMassEntityHandle AdversaryEntity = AdversaryId != 0 ? Units->FindEntity(AdversaryId) : FMassEntityHandle();
			const int32 Adversary = AdversaryEntity.IsSet() ? FindFighter(AdversaryEntity) : INDEX_NONE;
			if (Adversary != INDEX_NONE && Fighters[Adversary].State->CurrentHealth > 0)
			{
				if (Combat.Target != AdversaryEntity)
				{
					const int32 Former = FindFighter(Combat.Target);
					if (Former != INDEX_NONE)
					{
						--Fighters[Former].Combat->AttackerCount;
						Touch(Former);
					}
					Combat.Disengage();
					Combat.Target = AdversaryEntity;
					++Fighters[Adversary].Combat->AttackerCount;
					Touch(Adversary);
					Touch(Index);
				}
				Combat.bOrderedTarget = true;
				Combat.Mode = EHOLCombatMode::Normal;
				// A walk of his player's (neither the combat's own approach nor over): stopped, the approach below takes over.
				if (World && Fighter.Order->Phase != EHOLMovePhase::None && !Combat.bApproaching)
				{
					StopWalk(*World, Fighter);
				}
			}
		}
		// J7c.4, S2 (hol.Combat.FastIdleUnit): hol.Combat.FastIdle's test, on this unit alone and by its state now (a unit
		// before it may have taken it as target or struck it: then a last attacker, the whole turn). Such a unit's turn below
		// is its idle tick only: busy, nothing to disengage (TickIdle); else no target to look up nor lose, no last attacker
		// to clear, no search (busy, walking an order, an ordered target, or not its search step; bLost false), no contact
		// search (it needs bMaySeek on a walk, so an attack-move), then the branch without target: bFlanking false,
		// FlankWaitSeconds 0, TickIdle, no walk to stop (not approaching), no end of attack-move. Its AttackerCount is left
		// as is there too.
		if (FastIdleUnit != 0 && !Combat.Target.IsSet() && Combat.TargetBuilding.IsExplicitlyNull() && Combat.OrderedBuilding.IsExplicitlyNull() && !Combat.LastAttacker.IsSet() && !Combat.bApproaching && !Combat.bAttacking
			&& Combat.Mode != EHOLCombatMode::AttackMove)
		{
			const bool bWalkingOrder = Fighter.Order->Phase != EHOLMovePhase::None; // not approaching
			const bool bMaySeek = (!bWalkingOrder || HOLCombat::IsWalkStalled(Fighter.Order->StuckSteps, WalkRiposteSteps)) && !Combat.bOrderedTarget; // not on an attack-move
			if (Fighter.bBusy || !bMaySeek || ((static_cast<int64>(Fighter.UnitId) + StepIndex) % SearchSteps) != 0)
			{
				if (!Fighter.bBusy)
				{
					Combat.bFlanking = false;
					Combat.FlankWaitSeconds = 0.f;
				}
				HOLCombat::TickIdle(Combat.CooldownMs, StepMs);
				continue;
			}
		}
		const FVector2D Position(Fighter.Location);

		// At its gathering or building (a player's order of this step included, applied before this processor): out of
		// the fight, its walk left to its task (J7b.2: a player's order prevails).
		if (Fighter.bBusy)
		{
			if (Combat.Target.IsSet() || Combat.bApproaching || Combat.bAttacking)
			{
				const int32 Former = FindFighter(Combat.Target);
				if (Former != INDEX_NONE)
				{
					--Fighters[Former].Combat->AttackerCount;
					Touch(Former);
				}
				Combat.Disengage();
				Touch(Index);
			}
			HOLCombat::TickIdle(Combat.CooldownMs, StepMs);
			continue;
		}

		// Its target, if it still stands.
		int32 Target = FindFighter(Combat.Target);
		bool bLost = false;
		// D5: a target that became a protected hero (a duel began) is let go; the hero's own aim at anything but his adversary too.
		if (bDuelOn && Target != INDEX_NONE && Fighters[Target].State->CurrentHealth > 0 && !CanTouch(Fighter, Fighters[Target]))
		{
			--Fighters[Target].Combat->AttackerCount;
			Touch(Target);
			Combat.Disengage();
			Touch(Index);
			Target = INDEX_NONE;
		}
		// D6 (D369, D370, D401): the alliance made since it chose its target: no more blows at an ally; a target gone Neutral is let go too
		// unless the player ordered it (a Neutral stays one that can be attacked on order, but nobody takes it on by itself).
		if (StepCamps.IsDiplomacy() && Target != INDEX_NONE && Fighters[Target].State->CurrentHealth > 0
			&& (!StepCamps.AreHostile(Fighter.State->OwnerPlayer, Fighters[Target].State->OwnerPlayer)
				|| (!Combat.bOrderedTarget && !IsAutoFoe(Fighter, Fighters[Target]) && Fighters[Target].Combat->Target != Fighter.Entity)))
		{
			--Fighters[Target].Combat->AttackerCount;
			Touch(Target);
			Combat.Disengage();
			Touch(Index);
			Target = INDEX_NONE;
		}
		if (Combat.Target.IsSet() && (Target == INDEX_NONE || Fighters[Target].State->CurrentHealth <= 0))
		{
			Touch(Index);
			Combat.Target = FMassEntityHandle();
			Combat.bOrderedTarget = false;
			Combat.bAttacking = false;
			Target = INDEX_NONE;
			bLost = true;
		}
		// Looked up once (J7c.4, S3): nothing below changes it before the search reads it. Dead or gone, or its hit
		// HOLCombat::LastAttackerMemoryMs old (EtapeDefini S: an attacker that walked off out of the search radius is no
		// longer chased across the map, and this unit no longer counts as fighting): forgotten.
		int32 LastAttacker = FindFighter(Combat.LastAttacker);
		if (LastAttacker == INDEX_NONE || Fighters[LastAttacker].State->CurrentHealth <= 0 || !HOLCombat::KeepsLastAttacker(StepIndex, Combat.LastHitStep, StepMs))
		{
			Combat.LastAttacker = FMassEntityHandle();
			Combat.LastHitStep = -1;
			LastAttacker = INDEX_NONE;
		}

		// A unit walking a player's order takes on nobody by itself, but on an attack-move (J7b.3); an ordered target is
		// kept until it dies, an ordered building until it falls (D345: no enemy unit makes it drop it).
		const bool bWalkingOrder = Fighter.Order->Phase != EHOLMovePhase::None && !Combat.bApproaching;
		const bool bMaySeek = HOLCombat::MaySeek(bWalkingOrder, Combat.Mode == EHOLCombatMode::AttackMove, Combat.bOrderedTarget,
			!Combat.OrderedBuilding.IsExplicitlyNull(), HOLCombat::IsWalkStalled(Fighter.Order->StuckSteps, WalkRiposteSteps));
		const bool bSearchStep = ((static_cast<int64>(Fighter.UnitId) + StepIndex) % SearchSteps) == 0;
		const int32 TargetBeforeSearch = Target;
		// BUG-melee-stuck-attack (hol.Combat.MeleeLock, the user's rule: a soldier at grips with a target keeps it until it dies): a melee that struck
		// last step (bAttacking) keeps its living target while it stays within its reach plus MeleeLock; no search, no zone of control, no retarget
		// takes it away. (The target nudged out of reach by a few cm made the search take a nearer enemy and the blow begin again, in a loop.)
		bool bMeleeLocked = false;
		if (MeleeLock > 0.f && Target != INDEX_NONE && Combat.bAttacking && !HOLCombat::IsRangedAttack(Constants.AttackRange)
			&& Fighters[Target].State->CurrentHealth > 0 && Fighters[Target].Camp != Fighter.Camp && CanTouch(Fighter, Fighters[Target]))
		{
			bMeleeLocked = HOLCombat::IsInRange(Position, Constants.Radius, FVector2D(Fighters[Target].Location), Fighters[Target].Constants->Radius, Constants.AttackRange + MeleeLock);
		}
		if (bMaySeek && !bMeleeLocked && (bSearchStep || bLost))
		{
			HOLCombat::FSeeker Seeker;
			Seeker.Position = Position;
			Seeker.Radius = Constants.Radius;
			Seeker.AttackRange = Constants.AttackRange;
			Seeker.AggressionRadius = Constants.AggressionRadius;
			Seeker.bCanAttackUnits = Constants.Attack > 0; // S4 (SQ1 = B): the Ram takes no unit on by itself
			Seeker.bRanged = HOLCombat::IsRangedAttack(Constants.AttackRange);
			Seeker.bRipostes = Constants.bRipostes;
			Seeker.Mode = Combat.Mode == EHOLCombatMode::Hold ? HOLCombat::ESeekMode::Hold
				: Combat.Mode == EHOLCombatMode::AttackMove ? HOLCombat::ESeekMode::AttackMove : HOLCombat::ESeekMode::Auto;
			const float SearchRadius = HOLCombat::GetSearchRadius(Seeker);
			// J7c.4, S3: its target standing in reach, ChooseTarget keeps it whatever the others. With a search radius, the
			// candidates hold it once (the grid, or the extras below: alive, of another side); ChooseTarget then does not
			// give up at its start, and returns at it (in reach and current), the only candidate that can return early, the
			// ones before it only ranked. So the scan is spared; it changes nothing (the candidates are the step's scratch).
			const bool bKeepTarget = bFast && Target != INDEX_NONE && SearchRadius > 0.f
				&& Fighters[Target].State->CurrentHealth > 0 && Fighters[Target].Camp != Fighter.Camp && IsSeenBy(Fighter, Fighters[Target]) && CanTouch(Fighter, Fighters[Target])
				&& HOLCombat::IsInRange(Position, Seeker.Radius, FVector2D(Fighters[Target].Location), Fighters[Target].Constants->Radius, Seeker.AttackRange);
			int32 NewTarget = Target;
			if (!bKeepTarget)
			{
				Candidates.Reset();
				CandidateFighters.Reset();
				++SearchStamp; // only read on the fast paths
				// J7c.4 loop 22 (hol.Combat.RearTargetMaxShift): a melee of an attack-move out of reach does not take as free an
				// enemy further along its own line than the cap (no free target at the other end of the battle).
				const FHOLFrontLine* const ShiftLine = UE::HeroesOfLiberty::Simulation::RearFlank != 0 && UE::HeroesOfLiberty::Simulation::RearTargetMaxShift > 0.f
					&& bPreferFree && !Seeker.bRanged && Combat.Mode == EHOLCombatMode::AttackMove
					? UE::HeroesOfLiberty::Simulation::FrontLines.Find(Fighter.State->OwnerPlayer) : nullptr;
				const auto IsFarAlong = [&](const FVector2D& Other)
				{
					return ShiftLine && ShiftLine->Count > 0
						&& FMath::Abs(FVector2D::DotProduct(Other - Position, ShiftLine->Axis)) > UE::HeroesOfLiberty::Simulation::RearTargetMaxShift;
				};
				const auto AddCandidate = [&](const int32 Other)
				{
					const FFighter& Enemy = Fighters[Other];
					if (!IsSeenBy(Fighter, Enemy) || !CanTouch(Fighter, Enemy))
					{
						return;
					}
					HOLCombat::FCandidate& Candidate = Candidates.AddDefaulted_GetRef();
					Candidate.Id = Enemy.UnitId;
					Candidate.Position = FVector2D(Enemy.Location);
					Candidate.Radius = Enemy.Constants->Radius;
					Candidate.bAttacksMe = Enemy.Combat->Target == Fighter.Entity || Enemy.Entity == Combat.LastAttacker;
					Candidate.bIsCurrentTarget = Other == Target;
					Candidate.AttackerCount = Enemy.Combat->AttackerCount - (Other == Target ? 1 : 0);
					// Counted once a step, the first time needed (a melee out of its reach only: in reach, it is hit now).
					if (bPreferFree && !Seeker.bRanged
						&& !HOLCombat::IsInRange(Position, Seeker.Radius, Candidate.Position, Candidate.Radius, Seeker.AttackRange))
					{
						Candidate.bNoPlace = PlacesAround(Other) >= HOLCombat::MaxAttackers || IsFarAlong(Candidate.Position);
					}
					CandidateFighters.Add(Other);
					if (bFast)
					{
						SeenStamp[Other] = SearchStamp;
					}
				};
				// Perf (hol.Combat.ParallelSearch): the choice made ahead, its target and last attacker the same (the same
				// candidates but the dead, read the same way). Those unchanged since rank as they did, behind it; it and those
				// changed (Touch, or a place count other now) are ranked again, the same way. Its own changed: searched as before.
				int32 Weighed = INDEX_NONE;
				bool bWeighed = false;
				const int32 SearchSlot = bSearchAhead ? AheadSlotOfFighter[Index] : INDEX_NONE;
				if (SearchSlot != INDEX_NONE && AheadSearches[SearchSlot].bDone && AheadReach[SearchSlot] == SearchRadius + Constants.Radius + MaxRadius
					&& AheadSearches[SearchSlot].Target == Target && AheadSearches[SearchSlot].LastAttacker == LastAttacker)
				{
					const FAheadSearch& Search = AheadSearches[SearchSlot];
					const int32 Chosen = Search.Chosen;
					bWeighed = Chosen == INDEX_NONE || (SearchTouched[Chosen] == 0 && Fighters[Chosen].State->CurrentHealth > 0);
					// The place counts it read, read now (the very calls of the search below, in the same step's counts).
					for (int32 Read = 0; Read < Search.PlaceFighters.Num() && bWeighed; ++Read)
					{
						const int32 Other = Search.PlaceFighters[Read];
						if (Fighters[Other].State->CurrentHealth > 0 && SearchTouched[Other] == 0
							&& (PlacesAround(Other) >= HOLCombat::MaxAttackers) != Search.PlaceFull[Read])
						{
							bWeighed = Other != Chosen;
							if (bWeighed)
							{
								AddCandidate(Other);
							}
						}
					}
					if (bWeighed)
					{
						if (Chosen != INDEX_NONE)
						{
							AddCandidate(Chosen);
						}
						for (const int32 Other : AheadLists[SearchSlot])
						{
							if (SearchTouched[Other] != 0 && Fighters[Other].State->CurrentHealth > 0 && IsAutoFoe(Fighter, Fighters[Other]))
							{
								AddCandidate(Other);
							}
						}
						for (const int32 Extra : { Target, LastAttacker })
						{
							if (Extra != INDEX_NONE && SearchTouched[Extra] != 0 && Fighters[Extra].Camp != Fighter.Camp
								&& !CandidateFighters.Contains(Extra))
							{
								AddCandidate(Extra);
							}
						}
						const int32 Best = HOLCombat::ChooseTarget(Seeker, Candidates);
						Weighed = Best == INDEX_NONE ? INDEX_NONE : CandidateFighters[Best];
						NewTarget = Weighed;
					}
				}
				// Searched as before (or for the check): the candidates the weighing left taken off.
				if (Candidates.Num() > 0 && (!bWeighed || ParallelSearch == 2))
				{
					Candidates.Reset();
					CandidateFighters.Reset();
					++SearchStamp;
				}
				if (SearchRadius > 0.f && (!bWeighed || ParallelSearch == 2))
				{
					const double Reach = SearchRadius + Constants.Radius + MaxRadius;
					const FIntPoint MinCell(FMath::FloorToInt32((Position.X - Reach) / CellSize), FMath::FloorToInt32((Position.Y - Reach) / CellSize));
					const FIntPoint MaxCell(FMath::FloorToInt32((Position.X + Reach) / CellSize), FMath::FloorToInt32((Position.Y + Reach) / CellSize));
					if (bDenseGrid)
					{
						// J7c.4, S3: the hashed walk's tests on the dense grid (GatherNearby), the list gathered ahead when it
						// was for this very reach (it depends on the step's constants only), the health read now as before:
						// the same candidates. Their order differs (side first); ChooseTarget does not depend on it
						// (HOLCombatRules.h: the current target in reach returned wherever it stands, else the least of a strict
						// order ending on the unique Id).
						const int32 AheadSlot = AheadSlotOfFighter[Index];
						const bool bAhead = AheadSlot != INDEX_NONE && AheadReach[AheadSlot] == Reach;
						if (!bAhead)
						{
							GatherNearby(Index, Reach, Nearby);
						}
						for (const int32 Other : bAhead ? AheadLists[AheadSlot] : Nearby)
						{
							if (Fighters[Other].State->CurrentHealth > 0 && IsAutoFoe(Fighter, Fighters[Other]))
							{
								AddCandidate(Other);
							}
						}
					}
					for (int32 Y = MinCell.Y; Y <= MaxCell.Y && !bDenseGrid; ++Y)
					{
						for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
						{
							const FIntPoint Cell(X, Y);
							for (int32 Other = CellHeads[BucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
							{
								const FFighter& Enemy = Fighters[Other];
								if (Cells[Other] != Cell || Enemy.Camp == Fighter.Camp || !IsAutoFoe(Fighter, Enemy) || Enemy.State->CurrentHealth <= 0
									|| HOLCombat::GetEdgeDistance(Position, Constants.Radius, FVector2D(Enemy.Location), Enemy.Constants->Radius) > Reach)
								{
									continue;
								}
								AddCandidate(Other);
							}
						}
					}
					// The current target and the last attacker, wherever they stand (ChooseTarget weighs them).
					for (const int32 Extra : { Target, LastAttacker })
					{
						const bool bListed = Extra != INDEX_NONE && (bFast ? SeenStamp[Extra] == SearchStamp : CandidateFighters.Contains(Extra));
						if (Extra != INDEX_NONE && !bListed && Fighters[Extra].Camp != Fighter.Camp)
						{
							AddCandidate(Extra);
						}
					}
				}
				else if (SearchRadius <= 0.f && Seeker.bRipostes && Seeker.Mode != HOLCombat::ESeekMode::Hold && (!bWeighed || ParallelSearch == 2))
				{
					// V5.1 (the King): no search, only its riposte: its target and its last attacker, ChooseTarget keeping one attacking it.
					for (const int32 Extra : { Target, LastAttacker })
					{
						if (Extra != INDEX_NONE && Fighters[Extra].Camp != Fighter.Camp && !CandidateFighters.Contains(Extra))
						{
							AddCandidate(Extra);
						}
					}
				}
				if (!bWeighed || ParallelSearch == 2)
				{
					const int32 Chosen = HOLCombat::ChooseTarget(Seeker, Candidates);
					NewTarget = Chosen == INDEX_NONE ? INDEX_NONE : CandidateFighters[Chosen];
					if (bWeighed && NewTarget != Weighed)
					{
						++SearchMismatches;
						UE_LOG(LogHOLSimulation, Warning, TEXT("hol.Combat.ParallelSearch: step %lld #%u chose #%u ahead, #%u searching (%lld so far)"), StepIndex, Fighter.UnitId,
							Weighed == INDEX_NONE ? 0u : Fighters[Weighed].UnitId, NewTarget == INDEX_NONE ? 0u : Fighters[NewTarget].UnitId, SearchMismatches);
					}
				}
				// J7c.4 loop 19 (hol.Combat.RearTargetKeep): out of reach of both, its target alive and not full kept unless the new
				// one is clearly nearer: the ranks behind no longer turn from one enemy to the next at each search.
				if (RearTargetKeep > 0.f && bPreferFree && !Seeker.bRanged && Target != INDEX_NONE && NewTarget != INDEX_NONE && NewTarget != Target
					&& Fighters[Target].State->CurrentHealth > 0 && Fighters[Target].Camp != Fighter.Camp)
				{
					const FVector2D TargetPosition(Fighters[Target].Location);
					const FVector2D NewPosition(Fighters[NewTarget].Location);
					const float TargetRadius = Fighters[Target].Constants->Radius;
					const float NewRadius = Fighters[NewTarget].Constants->Radius;
					if (!HOLCombat::IsInRange(Position, Seeker.Radius, TargetPosition, TargetRadius, Seeker.AttackRange)
						&& !HOLCombat::IsInRange(Position, Seeker.Radius, NewPosition, NewRadius, Seeker.AttackRange))
					{
						if (PlacesAround(Target) < HOLCombat::MaxAttackers && !IsFarAlong(TargetPosition)
							&& HOLCombat::GetEdgeDistance(Position, Seeker.Radius, NewPosition, NewRadius)
								> HOLCombat::GetEdgeDistance(Position, Seeker.Radius, TargetPosition, TargetRadius) - RearTargetKeep)
						{
							NewTarget = Target;
						}
					}
				}
			}
			// MV.4, choice C (hol.Combat.AidRadius): no enemy found within its own aggression, an idle melee soldier comes to the
			// aid of a friend fighting within the radius: the nearest enemy (edge to edge, least id on a tie) that such a friend
			// strikes or is hit by. Walked to as any target (below); the next searches keep it while no other is in reach.
			if (NewTarget == INDEX_NONE && AidRadius > 0.f && bDenseGrid &&Combat.TargetBuilding.IsExplicitlyNull()
				&& HOLCombat::ShouldComeToAid(Seeker.Mode, Seeker.bRanged, Seeker.AggressionRadius, false, 0., AidRadius))
			{
				GatherFriends(Index, AidRadius, Friends);
				double FriendDistance = -1.;
				int32 Aided = INDEX_NONE;
				double AidedDistance = 0.;
				for (const int32 Friend : Friends)
				{
					const FFighter& Near = Fighters[Friend];
					const FHOLCombatFragment& NearCombat = *Near.Combat;
					if (Near.State->CurrentHealth <= 0 || Near.Camp != Fighter.Camp
						|| !HOLCombat::IsFriendFighting(NearCombat.bAttacking, NearCombat.AttackerCount, NearCombat.LastAttacker.IsSet()))
					{
						continue;
					}
					const double Distance = HOLCombat::GetEdgeDistance(Position, Constants.Radius, FVector2D(Near.Location), Near.Constants->Radius);
					FriendDistance = FriendDistance < 0. ? Distance : FMath::Min(FriendDistance, Distance);
					for (const FMassEntityHandle Foe : { NearCombat.Target, NearCombat.LastAttacker })
					{
						const int32 Other = FindFighter(Foe);
						if (Other == INDEX_NONE || Fighters[Other].State->CurrentHealth <= 0 || Fighters[Other].Camp == Fighter.Camp || !CanTouch(Fighter, Fighters[Other]))
						{
							continue;
						}
						const double FoeDistance = HOLCombat::GetEdgeDistance(Position, Constants.Radius, FVector2D(Fighters[Other].Location), Fighters[Other].Constants->Radius);
						if (Aided == INDEX_NONE || FoeDistance < AidedDistance || (FoeDistance == AidedDistance && Fighters[Other].UnitId < Fighters[Aided].UnitId))
						{
							Aided = Other;
							AidedDistance = FoeDistance;
						}
					}
				}
				if (Aided != INDEX_NONE && HOLCombat::ShouldComeToAid(Seeker.Mode, Seeker.bRanged, Seeker.AggressionRadius, false, FriendDistance, AidRadius))
				{
					NewTarget = Aided;
				}
			}
			if (NewTarget != Target)
			{
				if (Target != INDEX_NONE)
				{
					--Fighters[Target].Combat->AttackerCount;
				}
				if (NewTarget != INDEX_NONE)
				{
					++Fighters[NewTarget].Combat->AttackerCount;
				}
				Touch(Target);
				Touch(NewTarget);
				Touch(Index);
				Combat.Target = NewTarget == INDEX_NONE ? FMassEntityHandle() : Fighters[NewTarget].Entity;
				Combat.bAttacking = false;
				Target = NewTarget;
			}
		}

		// J7c.4, trial feedback: a melee walking to fight (an attack-move or an approach), its target out of reach, takes
		// the enemy it already touches, of least unit id (no draw), and stops below: it no longer slides past a body to its
		// target. Every step, the full search at its rate unchanged; an ordered target is never replaced (bMaySeek).
		bool bZoneTaken = false; // a target taken by the zone of control: walked to at once, not to the former one
		if (bStopAtContact && bMaySeek && !bMeleeLocked && Fighter.Order->Phase != EHOLMovePhase::None && !HOLCombat::IsRangedAttack(Constants.AttackRange)
			&& (Combat.TargetBuilding.IsExplicitlyNull() || Constants.AggressionRadius > 0.f) // D345.2b: not a Peasant sent to a building
			&& (Target == INDEX_NONE || !HOLCombat::IsInRange(Position, Constants.Radius, FVector2D(Fighters[Target].Location), Fighters[Target].Constants->Radius, Constants.AttackRange)))
		{
			const double Zone = FMath::Max(static_cast<double>(ZoneOfControl), 0.0);
			// Perf: the list gathered ahead when this unit was foreseen (the same reach, the positions unchanged).
			const int32 ContactSlot = bParallelAhead ? ContactSlotOfFighter[Index] : INDEX_NONE;
			if (ContactSlot == INDEX_NONE)
			{
				GatherNearby(Index, ContactReachOf(Constants), Nearby);
			}
			const TArray<int32>& Contacts = ContactSlot != INDEX_NONE ? ContactLists[ContactSlot] : Nearby;
			int32 Touched = INDEX_NONE;
			int32 Nearest = INDEX_NONE; // the zone of control: the nearest living enemy within Zone, least unit id on a tie
			double NearestDistance = 0.0;
			bool bTargetInZone = false;
			for (const int32 Other : Contacts)
			{
				const FFighter& Enemy = Fighters[Other];
				if (Enemy.State->CurrentHealth <= 0 || !CanTouch(Fighter, Enemy))
				{
					continue;
				}
				if ((Touched == INDEX_NONE || Enemy.UnitId < Fighters[Touched].UnitId)
					&& HOLCombat::IsInRange(Position, Constants.Radius, FVector2D(Enemy.Location), Enemy.Constants->Radius, Constants.AttackRange))
				{
					Touched = Other;
				}
				if (Zone > 0.0)
				{
					const double Distance = HOLCombat::GetEdgeDistance(Position, Constants.Radius, FVector2D(Enemy.Location), Enemy.Constants->Radius);
					if (Distance <= Zone)
					{
						bTargetInZone |= Other == Target;
						// BUG-melee-stuck-attack (hol.Combat.ZoneSkipFull): the search (PreferFreeTarget) takes an enemy with a place left, the zone took back the
						// nearest one with none at the next step, and the soldier swung between the two for seconds, its attack never begun (#5: #12 <-> #9 over 160 steps).
						// An enemy with every place taken (other than its target) is no choice of the zone: it cannot be struck anyway.
						if (ZoneSkipFull != 0 && bPreferFree && Other != Target && PlacesAround(Other) >= HOLCombat::MaxAttackers)
						{
							continue;
						}
						if (Nearest == INDEX_NONE || Distance < NearestDistance || (Distance == NearestDistance && Enemy.UnitId < Fighters[Nearest].UnitId))
						{
							Nearest = Other;
							NearestDistance = Distance;
						}
					}
				}
			}
			// None in reach: the nearest in the zone, its target kept while in the zone (no swing between two).
			if (Touched == INDEX_NONE && Nearest != INDEX_NONE && !bTargetInZone)
			{
				Touched = Nearest;
				bZoneTaken = Nearest != TargetBeforeSearch; // the one it already walked to, taken back from the search: no new walk
			}
			if (Touched != INDEX_NONE && Touched != Target)
			{
				if (Target != INDEX_NONE)
				{
					--Fighters[Target].Combat->AttackerCount;
				}
				++Fighters[Touched].Combat->AttackerCount;
				Touch(Target);
				Touch(Touched);
				Touch(Index);
				Combat.Target = Fighters[Touched].Entity;
				Combat.bAttacking = false;
				Target = Touched;
			}
		}

		// J8 close (decision A of 2026-10-30, the J8a trial: Men-at-arms idle round the hero): a melee ordered at a target it cannot
		// reach, every place around it taken (MaxAttackers bodies touching it), leaves it for the nearest enemy within its aggression
		// radius that has a place left; its order is then over (as when its target dies) and it fights as a free unit. An unordered
		// one already does so in its search (PreferFreeTarget).
		if (SurplusReassign != 0 && bPreferFree && bSearchStep && Combat.bOrderedTarget && Target != INDEX_NONE && Constants.AggressionRadius > 0.f
			&& !HOLCombat::IsRangedAttack(Constants.AttackRange) && Combat.TargetBuilding.IsExplicitlyNull() && Combat.OrderedBuilding.IsExplicitlyNull()
			&& !HOLCombat::IsInRange(Position, Constants.Radius, FVector2D(Fighters[Target].Location), Fighters[Target].Constants->Radius, Constants.AttackRange)
			&& PlacesAround(Target) >= HOLCombat::MaxAttackers)
		{
			GatherNearby(Index, Constants.AggressionRadius + Constants.Radius + MaxRadius, Nearby);
			int32 Spare = INDEX_NONE;
			double SpareDistance = 0.0;
			for (const int32 Other : Nearby)
			{
				const FFighter& Enemy = Fighters[Other];
				if (Other == Target || Enemy.Camp == Fighter.Camp || Enemy.State->CurrentHealth <= 0 || !CanTouch(Fighter, Enemy))
				{
					continue;
				}
				const double Distance = HOLCombat::GetEdgeDistance(Position, Constants.Radius, FVector2D(Enemy.Location), Enemy.Constants->Radius);
				if (Distance > Constants.AggressionRadius || PlacesAround(Other) >= HOLCombat::MaxAttackers)
				{
					continue;
				}
				if (Spare == INDEX_NONE || Distance < SpareDistance || (Distance == SpareDistance && Enemy.UnitId < Fighters[Spare].UnitId))
				{
					Spare = Other;
					SpareDistance = Distance;
				}
			}
			if (Spare != INDEX_NONE)
			{
				--Fighters[Target].Combat->AttackerCount;
				++Fighters[Spare].Combat->AttackerCount;
				Touch(Target);
				Touch(Spare);
				Touch(Index);
				Combat.Target = Fighters[Spare].Entity;
				Combat.bOrderedTarget = false;
				Combat.bAttacking = false;
				Target = Spare;
				bZoneTaken = true; // walked to at once
			}
		}

		// D345.2b: an enemy unit taken on makes it drop the building it attacked; only one taken on by itself (at rest, on an
		// attack-move): with an ordered one it takes on no enemy unit (D345, bMaySeek).
		if (Target != INDEX_NONE && !Combat.TargetBuilding.IsExplicitlyNull())
		{
			// Rare (once per drop): a scenario waits for it.
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld #%u %s of slot %d drops a building for #%u %s"), StepIndex, Fighter.UnitId,
				*Constants.TypeName.ToString(), Fighter.State->OwnerPlayer, Fighters[Target].UnitId, *Fighters[Target].Constants->TypeName.ToString());
			Combat.TargetBuilding.Reset();
		}
		const int32 IntervalMs = HOLCombat::ToMilliseconds(Constants.AttackInterval);
		if (Target == INDEX_NONE)
		{
			// D345.2b: no enemy unit. Its building (ordered, or taken on before), else one it takes on by itself: on an
			// attack-move, at rest within its aggression radius (none for the Peasant), on hold within its reach.
			int32 BuildingIndex = INDEX_NONE;
			if (!Combat.TargetBuilding.IsExplicitlyNull())
			{
				BuildingIndex = FindStepBuilding(Combat.TargetBuilding.Get());
				if (BuildingIndex != INDEX_NONE && (StepBuildings[BuildingIndex].Camp == Fighter.Camp
						|| !StepCamps.AreHostile(Fighter.State->OwnerPlayer, StepBuildings[BuildingIndex].OwnerPlayer))) // an ally's, since the alliance (D6)
				{
					BuildingIndex = INDEX_NONE;
				}
				if (BuildingIndex == INDEX_NONE)
				{
					// Its building gone (fallen, or no longer an enemy's): an ordered one's order is over, its queue goes on (D345).
					HOLOrderChain::EndBuildingTarget(Combat.TargetBuilding, Combat.OrderedBuilding);
					Combat.bAttacking = false;
				}
			}
			if (BuildingIndex == INDEX_NONE && Constants.BuildingAttack > 0 && bMaySeek && (bSearchStep || bLost) && StepBuildings.Num() > 0)
			{
				HOLCombat::FSeeker Seeker;
				Seeker.AggressionRadius = Constants.AggressionRadius;
				Seeker.Mode = Combat.Mode == EHOLCombatMode::Hold ? HOLCombat::ESeekMode::Hold
					: Combat.Mode == EHOLCombatMode::AttackMove ? HOLCombat::ESeekMode::AttackMove : HOLCombat::ESeekMode::Auto;
				const float BuildingSearch = HOLCombat::GetBuildingSearchRadius(Seeker, Constants);
				if (BuildingSearch > 0.f)
				{
					BuildingCandidates.Reset();
					BuildingCandidateIndices.Reset();
					for (int32 Other = 0; Other < StepBuildings.Num(); ++Other)
					{
						const FStepBuilding& Step = StepBuildings[Other];
						if (!Step.bGone && Step.Camp != Fighter.Camp && (!StepCamps.IsDiplomacy() || StepCamps.FiresAtOnce(Fighter.State->OwnerPlayer, Step.OwnerPlayer)))
						{
							HOLCombat::FBuildingCandidate& Candidate = BuildingCandidates.AddDefaulted_GetRef();
							Candidate.Id = Step.Id;
							Candidate.Box = Step.Box;
							BuildingCandidateIndices.Add(Other);
						}
					}
					const int32 Chosen = HOLCombat::ChooseBuilding(Position, Constants.Radius, BuildingSearch, BuildingCandidates);
					if (Chosen != INDEX_NONE)
					{
						BuildingIndex = BuildingCandidateIndices[Chosen];
						Combat.TargetBuilding = StepBuildings[BuildingIndex].Building;
						Combat.bAttacking = false;
					}
				}
			}
			// D5: a hero in a duel touches nothing but his adversary: no building (its target kept for after the duel).
			if (BuildingIndex != INDEX_NONE && !CanHitSlots(HeroSlotOf(Fighter), INDEX_NONE))
			{
				Combat.bAttacking = false;
				BuildingIndex = INDEX_NONE;
			}
			if (BuildingIndex != INDEX_NONE && World)
			{
				Combat.bFlanking = false;
				Combat.FlankWaitSeconds = 0.f;
				FStepBuilding& Step = StepBuildings[BuildingIndex];
				const bool bHold = Combat.Mode == EHOLCombatMode::Hold;
				const bool bShot = HOLCombat::IsBuildingShot(Constants);
				const double Edge = HOLCombat::GetEdgeDistanceToBox(Position, Constants.Radius, Step.Box);
				// EtapeDefini O.7 (the user's rule of 2026-10-28, from an AoE4 clip; J's and O.5's places dropped): within its
				// building reach (D497, 400 cm) it strikes at once, wherever it stands, whatever its walk. A melee walks to the
				// visible wall's point nearest it (off it by its radius and BuildingWallGap, never into the House: a straight
				// line), a shooter to the body's (it stops at its reach anyway).
				const bool bInReach = HOLCombat::IsBuildingInReach(Constants, Edge);
				const FVector2D WallPoint = bShot ? HOLCombat::GetBuildingApproachPoint(Position, Constants.Radius, Step.Box, BuildingApproachGap)
					: HOLCombat::GetBuildingApproachPoint(Position, Constants.Radius, Step.VisibleBox, HOLCombat::BuildingWallGap);
				if (!bInReach)
				{
					Combat.bAttacking = false;
					Combat.BuildingInward = FVector2f::ZeroVector;
					HOLCombat::TickIdle(Combat.CooldownMs, StepMs);
					if (bHold)
					{
						Combat.TargetBuilding.Reset(); // on hold, never a walk to it
						continue;
					}
					// A walk to its wall: the first; again at the search rate once the last one ended short, or (a melee) once the
					// wall's point nearest it moved a radius off the one it walks to: slid sideways round the crowd's back
					// (UHOLFullSpeedProcessor::SlideAlongFriends), it walks in from where it now stands, never back to the first one.
					// O.7b loop 3: a melee walks to the wall's point nearest the one facing it that no striking friend covers (the
					// crowd's nearer free end): blocked behind the crowd, it goes round it, its walk re-aimed from where it stands.
					FVector2D WalkPoint = WallPoint;
					if (!bShot && (!Combat.bApproaching || bSearchStep))
					{
						TArray<FVector2D, TInlineAllocator<64>> Bodies;
						TArray<float, TInlineAllocator<64>> BodyRadii;
						for (const int32 Striker : BuildingStrikers)
						{
							const FFighter& Friend = Fighters[Striker];
							if (Striker != Index && Friend.Camp == Fighter.Camp && Friend.Combat->TargetBuilding == Combat.TargetBuilding)
							{
								Bodies.Add(FVector2D(Friend.Location));
								BodyRadii.Add(Friend.Constants->Radius);
							}
						}
						// The reach is measured from the body box (IsBuildingInReach), the walk aimed at the visible one, wider: the
						// reach off the visible box is that much shorter (loop 8).
						const double VisualGap = Step.Box.Min.X - Step.VisibleBox.Min.X;
						WalkPoint = HOLCombat::GetFreeBuildingWallPoint(Position, Constants.Radius, Step.VisibleBox, HOLCombat::BuildingWallGap, Constants.BuildingAttackRange - VisualGap, Bodies, BodyRadii);
					}
					if (!Combat.bApproaching || (bSearchStep && (Fighter.Order->Phase == EHOLMovePhase::None
						|| (!bShot && FVector2D::Distance(WalkPoint, Combat.ApproachPoint) > Constants.Radius))))
					{
						Commands->IssueCombatWalk(Fighter.UnitId, FVector(WalkPoint.X, WalkPoint.Y, Fighter.Location.Z), Combat.EngageSerial, /*bAttackMove*/ false);
						Combat.ApproachPoint = WalkPoint;
						Combat.bAttackMoveDetour |= Combat.Mode == EHOLCombatMode::AttackMove;
						Combat.bApproaching = true;
					}
					continue;
				}
				// In reach: it strikes, with its building attack.
				if (bShot || bHold)
				{
					if (Fighter.Order->Phase != EHOLMovePhase::None)
					{
						Combat.bAttackMoveDetour |= Combat.Mode == EHOLCombatMode::AttackMove;
						StopWalk(*World, Fighter);
					}
					Combat.bApproaching = false;
				}
				else
				{
					// EtapeDefini O.7: a melee closes in to the visible wall while it strikes (the crowd packs against it, AoE4): its
					// walk there goes on, or one starts as it comes in reach (another walk, an attack-move's, stopped first); it
					// stops at the wall, or once BuildingCloseInStopMs without coming BuildingCloseInProgress nearer (friends in
					// front: it strikes where it stands, pushed in by those behind, only along BuildingInward: the separation).
					const double WallEdge = HOLCombat::GetEdgeDistanceToBox(Position, Constants.Radius, Step.VisibleBox);
					const FVector2D Nearest(FMath::Clamp(Position.X, Step.VisibleBox.Min.X, Step.VisibleBox.Max.X), FMath::Clamp(Position.Y, Step.VisibleBox.Min.Y, Step.VisibleBox.Max.Y));
					Combat.BuildingInward = FVector2f((Nearest - Position).GetSafeNormal());
					Combat.BuildingRoom = float(FMath::Max(0., WallEdge - HOLCombat::BuildingWallGap));
					const bool bAtWall = WallEdge <= HOLCombat::BuildingWallGap + HOLCombat::BuildingCloseInProgress;
					if (!Combat.bAttacking)
					{
						Combat.CloseInBest = MAX_flt;
						Combat.CloseInBlockedMs = 0;
					}
					// O.7b loop 2: a striking friend touching it ahead (within 60 degrees of its way in) bars the way: it stops at
					// once (waiting BuildingCloseInStopMs, it shoved into the crowd and was pushed off sideways by it).
					bool bBarred = false;
					bool bTouchedAhead = false; // a striking friend touching it anywhere in front (loop 7: no walk in again then)
					for (int32 Slot = 0; Slot < BuildingStrikers.Num() && !bBarred; ++Slot)
					{
						const FFighter& Friend = Fighters[BuildingStrikers[Slot]];
						if (BuildingStrikers[Slot] == Index || Friend.Camp != Fighter.Camp || Friend.Combat->TargetBuilding != Combat.TargetBuilding)
						{
							continue;
						}
						const FVector2D Ahead = FVector2D(Friend.Location) - Position;
						const double Contact = Constants.Radius + Friend.Constants->Radius + HOLCombat::BuildingCloseInProgress;
						const double In = FVector2D::DotProduct(Ahead, FVector2D(Combat.BuildingInward));
						const bool bTouching = Ahead.SizeSquared() <= FMath::Square(Contact);
						bTouchedAhead |= bTouching && In > 0.;
						bBarred = bTouching && In >= 0.5 * Ahead.Size();
					}
					// O.7b loop 4: coming in reach on a walk aimed more than a radius off its nearest wall point (to the crowd's free
					// end, along it), that walk is dropped: it closes in straight to the wall (never along the crowd once striking).
					const bool bSideWalk = !Combat.bAttacking && Combat.bApproaching && Fighter.Order->Phase != EHOLMovePhase::None
						&& FVector2D::Distance(Combat.ApproachPoint, WallPoint) > Constants.Radius;
					if (Combat.bApproaching && Fighter.Order->Phase != EHOLMovePhase::None && !bSideWalk)
					{
						if (WallEdge + HOLCombat::BuildingCloseInProgress <= Combat.CloseInBest)
						{
							Combat.CloseInBest = float(WallEdge);
							Combat.CloseInBlockedMs = 0;
						}
						else
						{
							Combat.CloseInBlockedMs += StepMs;
						}
						if (bAtWall || bBarred || Combat.CloseInBlockedMs >= HOLCombat::BuildingCloseInStopMs)
						{
							StopWalk(*World, Fighter);
						}
					}
					else if (!Combat.bAttacking)
					{
						if (Fighter.Order->Phase != EHOLMovePhase::None)
						{
							Combat.bAttackMoveDetour |= Combat.Mode == EHOLCombatMode::AttackMove;
							StopWalk(*World, Fighter);
						}
						Combat.bApproaching = false;
						if (!bAtWall && !bBarred)
						{
							Commands->IssueCombatWalk(Fighter.UnitId, FVector(WallPoint.X, WallPoint.Y, Fighter.Location.Z), Combat.EngageSerial, /*bAttackMove*/ false);
							Combat.ApproachPoint = WallPoint;
							Combat.bAttackMoveDetour |= Combat.Mode == EHOLCombatMode::AttackMove;
							Combat.bApproaching = true;
						}
					}
					else if (Combat.bApproaching && Combat.CloseInBest < MAX_flt)
					{
						Combat.bApproaching = false; // its walk in ended (at the wall's point, or short of it)
					}
					else if (!Combat.bApproaching && Fighter.Order->Phase == EHOLMovePhase::None && bSearchStep && !bAtWall && !bTouchedAhead)
					{
						// O.7b loop 7 (the crowd packs against the wall, AoE4): stopped short of the wall (held by the friends in front),
						// it walks in again, straight, at the search rate, once no striking friend touches it in front any more
						// (those ahead went on to the wall): the rows close up from the wall outwards.
						Commands->IssueCombatWalk(Fighter.UnitId, FVector(WallPoint.X, WallPoint.Y, Fighter.Location.Z), Combat.EngageSerial, /*bAttackMove*/ false);
						Combat.ApproachPoint = WallPoint;
						Combat.bAttackMoveDetour |= Combat.Mode == EHOLCombatMode::AttackMove;
						Combat.bApproaching = true;
						Combat.CloseInBest = MAX_flt;
						Combat.CloseInBlockedMs = 0;
					}
					else if (!Combat.bApproaching && Fighter.Order->Phase != EHOLMovePhase::None)
					{
						StopWalk(*World, Fighter); // never another walk while it strikes
					}
				}

				const int32 BuildingIntervalMs = HOLCombat::ToMilliseconds(Constants.BuildingAttackInterval);
				if (!Combat.bAttacking)
				{
					Combat.CooldownMs = HOLCombat::StartAttack(Combat.CooldownMs, BuildingIntervalMs, Constants.AttackWindUpPercent);
					Combat.bAttacking = true;
				}
				if (!HOLCombat::TickAttack(Combat.CooldownMs, StepMs, BuildingIntervalMs))
				{
					continue;
				}
				++Combat.Hits;
				++TotalHits;
				if (Combat.FirstHitStep < 0)
				{
					Combat.FirstHitStep = StepIndex;
				}
				const FVector2D Aim(FMath::Clamp(Position.X, Step.Box.Min.X, Step.Box.Max.X), FMath::Clamp(Position.Y, Step.Box.Min.Y, Step.Box.Max.Y));
				if (HOLCombat::IsBuildingShot(Constants))
				{
					// An arrow, as at a unit: it lands after its flight to the wall's nearest point.
					const int32 FlightMs = HOLCombat::GetArrowFlightMilliseconds(Position, Aim, HOLCombat::GetArrowSpeed());
					FArrow& Arrow = Arrows.AddDefaulted_GetRef();
					Arrow.Shooter = Fighter.Entity;
					Arrow.ShooterId = Fighter.UnitId;
					Arrow.ShooterType = Constants.TypeName;
					Arrow.ShooterSlot = Fighter.State->OwnerPlayer;
					Arrow.Damage = static_cast<float>(GetBuildingAttackWithTechnology(Fighter));
					Arrow.ReleaseStep = StepIndex;
					Arrow.ArrivalStep = StepIndex + HOLCombat::GetArrowFlightSteps(FlightMs, StepMs);
					Arrow.TargetBuilding = Step.Building;
					Arrow.BuildingAttackType = static_cast<uint8>(Constants.BuildingAttackType);
					++TotalArrowsReleased;
					if (UHOLArrowSubsystem* ArrowDisplay = World->GetSubsystem<UHOLArrowSubsystem>())
					{
						FHOLArrowLaunch Launch;
						Launch.From = Fighter.Location;
						Launch.To = FVector(Aim.X, Aim.Y, Fighter.Location.Z);
						Launch.FlightMs = FlightMs;
						Launch.ShooterType = Constants.TypeName;
						ArrowDisplay->Launch(Launch);
					}
					continue;
				}
				const int32 Dealt = UHOLConstructionSubsystem::DamageBuilding(*Step.Building, GetBuildingAttackWithTechnology(Fighter), Constants.BuildingAttackType);
				Combat.DamageDealt += Dealt;
				if (LogHits != 0)
				{
					UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld #%u %s hits a building for %d"), StepIndex, Fighter.UnitId, *Constants.TypeName.ToString(), Dealt);
				}
				if (MarkBuildingGone(Step.Building))
				{
					HOLOrderChain::EndBuildingTarget(Combat.TargetBuilding, Combat.OrderedBuilding); // D345: an ordered one's order over, its queue goes on
					Combat.bAttacking = false;
				}
				continue;
			}
			Combat.bFlanking = false; // J7c.4 loop 21
			Combat.FlankWaitSeconds = 0.f;
			HOLCombat::TickIdle(Combat.CooldownMs, StepMs);
			if (Combat.bApproaching && World)
			{
				StopWalk(*World, Fighter); // its target gone: it stands where it is
			}
			// An attack-move with no enemy left (J7b.3): on to its slot, at a search step with nothing found; there, the
			// attack-move is over.
			else if (Combat.Mode == EHOLCombatMode::AttackMove && Fighter.Order->Phase == EHOLMovePhase::None && bSearchStep)
			{
				// Only after a fight took it off its way (a walk ended short of its slot otherwise ends the attack-move).
				if (Combat.bAttackMoveDetour && FVector2D::DistSquared(Position, FVector2D(Combat.AttackMoveGoal)) > FMath::Square(AttackMoveArrival))
				{
					Combat.bAttackMoveDetour = false;
					Commands->IssueCombatWalk(Fighter.UnitId, Combat.AttackMoveGoal, Combat.EngageSerial, /*bAttackMove*/ true);
				}
				else
				{
					Combat.Mode = EHOLCombatMode::Normal;
				}
			}
			continue;
		}

		// J7c.4 loop 21 (hol.Combat.RearFlank): a melee of an attack-move held behind its own line goes round its end (on its
		// slide's side, else the nearer end) on a walk of its own, until a free enemy near that end or the goal is reached.
		if (UE::HeroesOfLiberty::Simulation::RearFlank != 0 && bPreferFree && Combat.Mode == EHOLCombatMode::AttackMove && World
			&& !HOLCombat::IsRangedAttack(Constants.AttackRange))
		{
			const FHOLFrontLine* const Line = UE::HeroesOfLiberty::Simulation::FrontLines.Find(Fighter.State->OwnerPlayer);
			const bool bInReach = HOLCombat::IsInRange(Position, Constants.Radius, FVector2D(Fighters[Target].Location), Fighters[Target].Constants->Radius,
				Constants.AttackRange);
			const float StepSeconds = static_cast<float>(StepMs) / 1000.f;
			// D641 (hol.Combat.ClaimPlace): holding a claimed place within 400 cm, it does not go round the line (that tour of 600 cm was the 3 s loop).
			bool bPlaceNear = false;
			if (Combat.bHasPlace && Combat.PlaceTarget == Combat.Target)
			{
				const double PlaceReach = Fighters[Target].Constants->Radius + Constants.Radius + 2.;
				const FVector2D PlaceAt = FVector2D(Fighters[Target].Location) + PlaceReach * FVector2D(FMath::Cos(static_cast<double>(Combat.PlaceAngle)), FMath::Sin(static_cast<double>(Combat.PlaceAngle)));
				bPlaceNear = HOLCombat::ShouldSuspendRearFlank(true, FVector2D::Distance(Position, PlaceAt));
			}
			if (bInReach || bPlaceNear || !Line || Line->Count <= 0)
			{
				Combat.bFlanking = false;
				Combat.FlankWaitSeconds = 0.f;
			}
			else
			{
				const double Along = FVector2D::DotProduct(Position - Line->Origin, Line->Axis);
				const double Behind = -FVector2D::DotProduct(Position - Line->Origin, Line->Charge);
				// D641 (hol.Combat.FlankToTarget): how long it has stood within 3 cm of one place.
				if (FVector2D::DistSquared(Position, FVector2D(Combat.StillFrom)) > 9.)
				{
					Combat.StillFrom = FVector2f(Position);
					Combat.StillSeconds = 0.f;
				}
				else
				{
					Combat.StillSeconds += StepSeconds;
				}
				if (!Combat.bFlanking)
				{
					Combat.FlankWaitSeconds = Behind > -RearFlankArrival && Behind < RearFlankZone ? Combat.FlankWaitSeconds + StepSeconds : 0.f;
					// Loop 22: the departures spread by id (deterministic), not all on the same step.
					const float Delay = UE::HeroesOfLiberty::Simulation::RearFlankDelay
						+ UE::HeroesOfLiberty::Simulation::RearFlankStagger * static_cast<float>(Fighter.UnitId % 8) / 8.f;
					if (Combat.FlankWaitSeconds >= Delay && bSearchStep)
					{
						const double SlideSide = FVector2D::DotProduct(FVector2D(Combat.SlideWay.X, Combat.SlideWay.Y), Line->Axis);
						const int8 NearSide = Along >= 0.5 * (Line->Min + Line->Max) ? 1 : -1;
						const int8 Side = UE::HeroesOfLiberty::Simulation::RearFlankNearest != 0 ? NearSide
							: (FMath::Abs(SlideSide) > 0.3 ? (SlideSide > 0. ? 1 : -1) : NearSide);
						// Loop 22: its goal further along the line than the cap, it does not go round (it waits or slides).
						const double Shift = FMath::Abs((Side > 0 ? Line->Max : Line->Min) + Side * RearFlankOut - Along);
						const float MaxShift = UE::HeroesOfLiberty::Simulation::RearFlankMaxShift;
						// L114 (the tours): only if an enemy with a free place lies at or beyond that end (else it came back from the end
							// to the same blocked place, and set off again: a tour of the camp every 3 s).
							const double SideEnd = Side > 0 ? Line->Max : Line->Min;
							bool bFreeBeyondEnd = false;
							GatherNearby(Index, RearFlankZone, Nearby);
							for (const int32 Other : Nearby)
							{
								const FFighter& Near = Fighters[Other];
								if (Other != Target && Near.State->CurrentHealth > 0 && Near.Camp != Fighter.Camp && CanTouch(Fighter, Near)
									&& HOLCombat::IsBeyondLineEnd(Side, FVector2D::DotProduct(FVector2D(Near.Location) - Line->Origin, Line->Axis), SideEnd, RearFlankEndSlack)
									&& PlacesAround(Other) < HOLCombat::MaxAttackers)
								{
									bFreeBeyondEnd = true;
									break;
								}
							}
							// D641 (hol.Combat.FlankToTarget, the user's retour on the neutrals trial): held still behind its line with a place free round
							// its own target (the bodies round it, of any side, read by HOLCombat::HasFreePlaceAround), it goes round the end to that place:
							// it waits only when there is no place to strike from.
							bool bRoomAtTarget = false;
							if (!bFreeBeyondEnd && UE::HeroesOfLiberty::Simulation::FlankToTarget != 0 && Combat.StillSeconds >= UE::HeroesOfLiberty::Simulation::FlankStillSeconds)
							{
								bRoomAtTarget = RoomAtTarget(Index, Target, Constants.Radius);
							}
							if (HOLCombat::ShouldGoRound(bFreeBeyondEnd || bRoomAtTarget, Shift, MaxShift))
						{
							Combat.bFlanking = true;
							Combat.FlankWaitSeconds = 0.f;
							Combat.FlankSide = Side;
							Combat.FlankWalkGoal = FVector2f::ZeroVector;
							Combat.FlankBack = UE::HeroesOfLiberty::Simulation::RearFlankBack > 0.f
								? static_cast<float>(FMath::Max(Behind, 0.) + UE::HeroesOfLiberty::Simulation::RearFlankBack) : 0.f;
						}
					}
				}
				if (Combat.bFlanking)
				{
					Combat.FlankWaitSeconds += StepSeconds;
					const double End = Combat.FlankSide > 0 ? Line->Max : Line->Min;
					const FVector2D EndGoal = Line->Origin + Line->Axis * (End + Combat.FlankSide * RearFlankOut) + Line->Charge * RearFlankAhead;
					// Loop 22 (hol.Combat.RearFlankBack): first a point behind its rank at the end's level (not along the crowd),
					// passed once near it or once level with the end.
					const FVector2D BackPoint = Line->Origin + Line->Axis * (End + Combat.FlankSide * RearFlankOut) - Line->Charge * Combat.FlankBack;
					if (Combat.FlankBack > 0.f && (Combat.FlankSide * (Along - End) >= 0.
						|| FVector2D::DistSquared(Position, BackPoint) <= FMath::Square(2. * RearFlankArrival)))
					{
						Combat.FlankBack = 0.f;
					}
					const FVector2D Goal = Combat.FlankBack > 0.f ? BackPoint : EndGoal;
					const bool bArrived = FVector2D::DistSquared(Position, EndGoal) <= FMath::Square(RearFlankArrival)
						|| Combat.FlankWaitSeconds >= RearFlankGiveUp;
					// On a search step, the nearest free enemy close by, at or beyond the line's end on its side (least id on a tie).
					int32 Free = INDEX_NONE;
					double FreeDistance = 0.0;
					if (!bArrived && bSearchStep)
					{
						GatherNearby(Index, RearFlankFreeReach, Nearby);
						for (const int32 Other : Nearby)
						{
							const FFighter& Near = Fighters[Other];
							if (Near.State->CurrentHealth <= 0 || !CanTouch(Fighter, Near))
							{
								continue;
							}
							// L114 (the Spearman that goes round): its own target is no "free enemy" before it is round the end. It was held
							// from that very enemy by its friends in front of it; taking it again ended the flank 0.2 s after it began, the
							// unit walked back to the same blocked place, waited the delay, set off again: a to-and-fro for the whole fight.
							if (!HOLCombat::CanFlankTakeEnemy(Other == Target, Combat.FlankSide, Along, End))
							{
								continue;
							}
							const FVector2D OtherPosition(Near.Location);
							const double Distance = HOLCombat::GetEdgeDistance(Position, Constants.Radius, OtherPosition, Near.Constants->Radius);
							if (Distance > RearFlankFreeReach
								|| Combat.FlankSide * (FVector2D::DotProduct(OtherPosition - Line->Origin, Line->Axis) - End) < -RearFlankEndSlack
								|| (Free != INDEX_NONE && (Distance > FreeDistance || (Distance == FreeDistance && Near.UnitId > Fighters[Free].UnitId))))
							{
								continue;
							}
							if (PlacesAround(Other) < HOLCombat::MaxAttackers)
							{
								Free = Other;
								FreeDistance = Distance;
							}
						}
					}
					if (bArrived || Free != INDEX_NONE)
					{
						// Back to the fight: walks to its target at once (below).
						Combat.bFlanking = false;
						Combat.FlankWaitSeconds = 0.f;
						Combat.bApproaching = false;
						if (Free != INDEX_NONE && Free != Target)
						{
							--Fighters[Target].Combat->AttackerCount;
							++Fighters[Free].Combat->AttackerCount;
							Touch(Target);
							Touch(Free);
							Touch(Index);
							Combat.Target = Fighters[Free].Entity;
							Target = Free;
						}
					}
					else
					{
						Combat.bAttacking = false;
						HOLCombat::TickIdle(Combat.CooldownMs, StepMs);
						const FVector2f Goal2f(Goal);
						if (Combat.FlankWalkGoal.IsZero()
							|| (bSearchStep && (Fighter.Order->Phase == EHOLMovePhase::None || FVector2f::DistSquared(Goal2f, Combat.FlankWalkGoal) > FMath::Square(RearFlankRewalk))))
						{
							Commands->IssueCombatWalk(Fighter.UnitId, FVector(Goal.X, Goal.Y, Fighter.Location.Z), Combat.EngageSerial, /*bAttackMove*/ false);
							Combat.FlankWalkGoal = Goal2f;
							Combat.ApproachPoint = FVector2D(Fighters[Target].Location);
							Combat.bAttackMoveDetour = true;
							Combat.bApproaching = true;
						}
						continue;
					}
				}
			}
		}

		FFighter& Enemy = Fighters[Target];
		if (!CanTouch(Fighter, Enemy))
		{
			// D5: the last gate before a blow or an arrow (the target was taken above in the step): nothing is struck.
			--Enemy.Combat->AttackerCount;
			Touch(Target);
			Combat.Disengage();
			Touch(Index);
			continue;
		}
		const FVector2D EnemyPosition(Enemy.Location);
		if (!HOLCombat::IsInRange(Position, Constants.Radius, EnemyPosition, Enemy.Constants->Radius, Constants.AttackRange))
		{
			Combat.bAttacking = false;
			HOLCombat::TickIdle(Combat.CooldownMs, StepMs);
			// D641 (hol.Combat.LoopHoldOnlyFull): whether a place is free round its target, read at the search rate; UHOLFullSpeedProcessor holds a looping soldier only if none.
			if (bSearchStep && bPreferFree && !HOLCombat::IsRangedAttack(Constants.AttackRange))
			{
				Combat.bRoomAtTarget = RoomAtTarget(Index, Target, Constants.Radius);
			}
			// D641 (hol.Combat.ClaimPlace): no headway for a while with a place free round the target: it claims the free spot nearest to
			// the side it is on and walks to that spot, keeping the same one (the same side of the target) while it stays free.
			bool bPlaceNew = false;
			bool bWaitFarPlace = false;
			if (ClaimPlace != 0 && bPreferFree && !HOLCombat::IsRangedAttack(Constants.AttackRange) && !Combat.bFlanking && Combat.Mode != EHOLCombatMode::Hold)
			{
				const double Edge = HOLCombat::GetEdgeDistance(Position, Constants.Radius, EnemyPosition, Enemy.Constants->Radius);
				if (Combat.PlaceTarget != Combat.Target)
				{
					// A new target next to the last one (the enemies strike in turn, the quarrel is the same crowd): the count of no headway goes on.
					const bool bNeighbour = Combat.PlaceTarget.IsSet() && HOLCombat::IsNeighbourTarget(FVector2D(Combat.PlaceTargetWhere), EnemyPosition, NeighbourGap);
					Combat.PlaceTarget = Combat.Target;
					Combat.bHasPlace = false;
					if (!bNeighbour)
					{
						Combat.BestEdge = static_cast<float>(Edge);
						Combat.NoProgressSeconds = 0.f;
					}
					else
					{
						Combat.BestEdge = FMath::Min(Combat.BestEdge, static_cast<float>(Edge));
					}
				}
				else if (Edge < static_cast<double>(Combat.BestEdge) - ClaimProgress)
				{
					Combat.BestEdge = static_cast<float>(Edge);
					Combat.NoProgressSeconds = 0.f;
				}
				else
				{
					Combat.NoProgressSeconds += static_cast<float>(StepMs) / 1000.f;
				}
				Combat.PlaceTargetWhere = FVector2f(EnemyPosition);
					// Q-neutrals-second-rank = C (hol.Combat.BoxedRetarget) and D641 (hol.Combat.FarPlaceRetarget): a soldier that cannot get at its target takes the
					// nearest enemy with a free place it can reach (no going round; with MaxPlaceGap, a place on its own side) instead. True when it did (the target changed).
					const auto TryRetarget = [&](const bool bRequireBoxed, const double MaxPlaceGap, const bool bLoose = false) -> bool
					{
						if (bMeleeLocked)
						{
							return false; // hol.Combat.MeleeLock: at grips with its target, it keeps it
						}
						TArray<int32> BoxEnemies;
						TArray<int32> BoxFriends;
						GatherNearby(Index, BoxedRetargetReach, BoxEnemies);
						GatherFriends(Index, BoxedRetargetReach, BoxFriends);
						TArray<HOLCombat::FBody> BoxBodies;
						TArray<int32> BoxFighters;
						TArray<int32> BoxEnemyBodies;
						TArray<uint64> BoxEnemyIds;
						int32 TargetBody = INDEX_NONE;
						for (const int32 Other : BoxFriends)
						{
							if (Other != Index && Fighters[Other].State->CurrentHealth > 0)
							{
								BoxBodies.Add({ FVector2D(Fighters[Other].Location), Fighters[Other].Constants->Radius });
								BoxFighters.Add(Other);
							}
						}
						for (const int32 Other : BoxEnemies)
						{
							if (Fighters[Other].State->CurrentHealth <= 0)
							{
								continue;
							}
							if (Other == Target)
							{
								TargetBody = BoxBodies.Num();
							}
							else if (!CanTouch(Fighter, Fighters[Other]) || HOLCombat::GetEdgeDistance(Position, Constants.Radius, FVector2D(Fighters[Other].Location), Fighters[Other].Constants->Radius) > BoxedRetargetEnemyReach)
							{
								BoxBodies.Add({ FVector2D(Fighters[Other].Location), Fighters[Other].Constants->Radius });
								BoxFighters.Add(Other);
								continue;
							}
							BoxEnemyBodies.Add(BoxBodies.Num());
							BoxEnemyIds.Add(Fighters[Other].UnitId);
							BoxBodies.Add({ FVector2D(Fighters[Other].Location), Fighters[Other].Constants->Radius });
							BoxFighters.Add(Other);
						}
						if (bRequireBoxed)
						{
							TArray<HOLCombat::FBody> BoxOthers;
							for (int32 BodyIndex = 0; BodyIndex < BoxBodies.Num(); ++BodyIndex)
							{
								if (BodyIndex != TargetBody)
								{
									BoxOthers.Add(BoxBodies[BodyIndex]);
								}
							}
							if (!HOLCombat::ShouldRetargetWhenBoxedIn(Combat.NoProgressSeconds, HOLCombat::IsBoxedIn(Position, Constants.Radius, EnemyPosition, BoxOthers), Combat.bOrderedTarget, BoxedRetargetSeconds))
							{
								return false;
							}
						}
						const int32 Chosen = HOLCombat::FindReachableEnemy(Position, Constants.Radius, BoxBodies, BoxEnemyBodies, BoxEnemyIds, TargetBody, MaxPlaceGap, bLoose);
						if (Chosen == INDEX_NONE)
						{
							return false;
						}
						const int32 NewTarget = BoxFighters[BoxEnemyBodies[Chosen]];
						--Enemy.Combat->AttackerCount;
						++Fighters[NewTarget].Combat->AttackerCount;
						Touch(Target);
						Touch(NewTarget);
						Touch(Index);
						Combat.Target = Fighters[NewTarget].Entity;
						Combat.bAttacking = false;
						Combat.bApproaching = false; // the walk to the new target is given at the next step
						Combat.PlaceTarget = Combat.Target;
						Combat.PlaceTargetWhere = FVector2f(FVector2D(Fighters[NewTarget].Location));
						Combat.bHasPlace = false;
						Combat.NoProgressSeconds = 0.f;
						Combat.BestEdge = static_cast<float>(HOLCombat::GetEdgeDistance(Position, Constants.Radius, FVector2D(Fighters[NewTarget].Location), Fighters[NewTarget].Constants->Radius));
						UE_LOG(LogHOLSimulation, Verbose, TEXT("Combat: step %lld #%u retargets (walled in or place across), takes #%u for #%u"), StepIndex, Fighter.UnitId, Fighters[NewTarget].UnitId, Enemy.UnitId);
						return true;
					};
					if (BoxedRetarget != 0 && bSearchStep && !Combat.bOrderedTarget && Combat.NoProgressSeconds >= BoxedRetargetSeconds && TryRetarget(true, -1.))
					{
						continue;
					}
					// BUG-melee-stuck-attack (hol.Combat.StuckRetargetSeconds): a soldier that has made no headway to its target for this long (held behind friends
					// that strike, its place claimed and never reached: #5 waited 10 s, np 12 s) takes the nearest enemy with a free place it can reach; the walled-in test
					// above (off by default) asks to be boxed in, this one only waits. A target the player ordered is kept.
					if (StuckRetargetSeconds > 0.f && bSearchStep && !Combat.bOrderedTarget && Combat.NoProgressSeconds >= StuckRetargetSeconds && TryRetarget(false, -1.))
					{
						continue;
					}
				// BUG-melee-stuck-attack (hol.Combat.StuckLooseSeconds): still no headway after this long and no enemy with a free place it can reach: it turns to the
				// nearest other enemy whatever the places and the friends in the way (the walk to a new goal frees it from the friends that bar the old one).
				if (StuckLooseSeconds > 0.f && bSearchStep && !Combat.bOrderedTarget && Combat.NoProgressSeconds >= StuckLooseSeconds && TryRetarget(false, -1., true))
				{
					continue;
				}
				if (bSearchStep && (Combat.bHasPlace || (Combat.bRoomAtTarget && Combat.NoProgressSeconds >= ClaimStuckSeconds)))
				{
					TArray<HOLCombat::FBody> PlaceBodies;
					GatherRoomBodies(Index, Target, PlaceBodies);
						const int32 RoomBodyCount = PlaceBodies.Num(); // before the places the friends claimed (they are no walls)
						// Celeborn-24 (hol.Combat.PlaceBoxedIn): the spot chosen is not claimed when the bodies round the way to it wall the soldier in.
						const auto PlaceWalledIn = [&](const double ForAngle) -> bool
						{
							if (PlaceBoxedIn == 0)
							{
								return false;
							}
							const double Reach = Enemy.Constants->Radius + Constants.Radius + 2.;
							const FVector2D PlaceWhere = EnemyPosition + Reach * FVector2D(FMath::Cos(ForAngle), FMath::Sin(ForAngle));
							return HOLCombat::IsPlaceWalledIn(Position, Constants.Radius, PlaceWhere, TConstArrayView<HOLCombat::FBody>(PlaceBodies.GetData(), RoomBodyCount));
						};
					if (ClaimedSlots != 0)
					{
						// BUG-melee-stuck-attack: the places the friends of the same target claimed stand as bodies on their spots.
						TArray<int32> ClaimFriends;
						GatherFriends(Index, 800., ClaimFriends);
						TArray<HOLCombat::FPlaceClaim> Claims;
						for (const int32 Friend : ClaimFriends)
						{
							const FFighter& Mate = Fighters[Friend];
							if (Friend != Index && Mate.Camp == Fighter.Camp && Mate.State->CurrentHealth > 0 && Mate.Combat->bHasPlace && Mate.Combat->Target == Combat.Target)
							{
								Claims.Add({ FVector2D(Mate.Location), Mate.Constants->Radius, static_cast<double>(Mate.Combat->PlaceAngle), Mate.UnitId });
							}
						}
						HOLCombat::AppendClaimedPlaceBodies(EnemyPosition, Enemy.Constants->Radius, Position, Fighter.UnitId, static_cast<double>(Combat.PlaceAngle), Combat.bHasPlace,
							Constants.Radius, Claims, PlaceBodies);
					}
					const FVector2D FromTarget = Position - EnemyPosition;
					const double SelfSide = FromTarget.IsNearlyZero() ? 0. : FMath::Atan2(FromTarget.Y, FromTarget.X) + (FromTarget.Y < 0. ? 2. * UE_DOUBLE_PI : 0.);
					double Angle = 0.;
					if (HOLCombat::ChooseFreePlace(EnemyPosition, Enemy.Constants->Radius, Constants.Radius, PlaceBodies, Combat.bHasPlace, static_cast<double>(Combat.PlaceAngle), SelfSide, PlaceBySide != 0, Angle) && !PlaceWalledIn(Angle))
					{
						// D641 (hol.Combat.FarPlaceRetarget): a NEW place across the melee (more than FarPlaceGap from the soldier's side) is refused.
						const bool bNewClaim = !Combat.bHasPlace || FMath::Abs(Angle - static_cast<double>(Combat.PlaceAngle)) > 1e-3;
						if (FarPlaceRetarget != 0 && bNewClaim && !Combat.bOrderedTarget && HOLCombat::IsPlaceTooFar(Angle, SelfSide, FarPlaceGap))
						{
							if (TryRetarget(false, FarPlaceGap))
							{
								continue;
							}
							if (Combat.NoProgressSeconds < FarPlaceWaitSeconds)
							{
								bWaitFarPlace = true; // no enemy on its side: it waits where it stands, the place is looked at again at the next search step
							}
						}
						if (!bWaitFarPlace)
						{
							bPlaceNew = bNewClaim;
							if (!Combat.bHasPlace && KeepStuckCount == 0)
							{
								Combat.BestEdge = static_cast<float>(Edge);
								Combat.NoProgressSeconds = 0.f;
							}
							Combat.bHasPlace = true;
							Combat.PlaceAngle = static_cast<float>(Angle);
						}
					}
					else
					{
						Combat.bHasPlace = false;
					}
				}
			}
			else
			{
				Combat.bHasPlace = false;
			}
			FVector WalkGoal = Enemy.Location;
			if (Combat.bHasPlace)
			{
				const double PlaceReach = Enemy.Constants->Radius + Constants.Radius + 2.;
				WalkGoal.X = EnemyPosition.X + PlaceReach * FMath::Cos(static_cast<double>(Combat.PlaceAngle));
				WalkGoal.Y = EnemyPosition.Y + PlaceReach * FMath::Sin(static_cast<double>(Combat.PlaceAngle));
			}
			// A walk to it at most at the search rate: the first, or once it moved off, or once the last one ended short.
			const bool bWalkOver = Fighter.Order->Phase == EHOLMovePhase::None;
			const bool bMoved = FVector2D::DistSquared(EnemyPosition, Combat.ApproachPoint) > FMath::Square(ReapproachDistance);
			// Celeborn-14 (diplomacy_qground_watch, #12: ph 3 held 150 steps within 10 cm, 24 reversals): a walk already on its way to that very goal
			// is not given again. Every search step the same walk was re-issued (a new order: the final leg's wait of FinalLegStuckSteps restarted
			// from 0 and the unit blipped 10 cm towards the enemy and back), so a walk held by its friends never ended. 0: as before.
			const bool bSameWalk = SameWalkGoal != 0 && Fighter.Order->Phase != EHOLMovePhase::None
				&& FVector2D::DistSquared(FVector2D(WalkGoal), FVector2D(Fighter.Order->Slot)) <= FMath::Square(static_cast<double>(Constants.Radius) * 2.0 + 50.0);
			if (Combat.Mode != EHOLCombatMode::Hold && !bWaitFarPlace && !bSameWalk
				&& (bZoneTaken || bPlaceNew || ((bSearchStep || bLost || !Combat.bApproaching) && (!Combat.bApproaching || bMoved || bWalkOver))))
			{
				// Stamped (J7b.3): a player's order given meanwhile drops it.
				Commands->IssueCombatWalk(Fighter.UnitId, WalkGoal, Combat.EngageSerial, /*bAttackMove*/ false);
				Combat.ApproachPoint = EnemyPosition;
				Combat.bAttackMoveDetour |= Combat.Mode == EHOLCombatMode::AttackMove;
				Combat.bApproaching = true;
			}
			continue;
		}

		// S4 (SQ1 = B, D614): an engine with no attack on units (the Ram) never strikes one, even ordered: it drops the target.
		if (Constants.Attack <= 0)
		{
			--Enemy.Combat->AttackerCount;
			Touch(Target);
			Combat.Disengage();
			Combat.bOrderedTarget = false;
			Touch(Index);
			continue;
		}
		// In reach: it stands and strikes.
		if (Fighter.Order->Phase != EHOLMovePhase::None && World)
		{
			Combat.bAttackMoveDetour |= Combat.Mode == EHOLCombatMode::AttackMove;
			StopWalk(*World, Fighter);
		}
		Combat.bApproaching = false;
		if (!Combat.bAttacking)
		{
			Combat.CooldownMs = HOLCombat::StartAttack(Combat.CooldownMs, IntervalMs, Constants.AttackWindUpPercent);
			Combat.bAttacking = true;
		}
		if (!HOLCombat::TickAttack(Combat.CooldownMs, StepMs, IntervalMs))
		{
			continue;
		}
		// An arrow in flight for a long reach; the armour taken off comes from the attack's type (D345), not from the reach.
		const bool bShot = HOLCombat::IsRangedAttack(Constants.AttackRange);
		const float Damage = HOLCombat::GetDamage(Constants, *Enemy.Constants, Combat.Level, GetTalentArmorBonus(Enemy), GetTechnologyAttackPercent(Fighter), GetTechnologyArmorPercent(Enemy, Constants.AttackType));
		if (bShot)
		{
			// An arrow (J7c.2): released now, at the strike's moment of the gesture; it lands on its target after its flight
			// (the distance at release over the arrow speed), whatever the shooter does meanwhile.
			const int32 FlightMs = HOLCombat::GetArrowFlightMilliseconds(Position, EnemyPosition, HOLCombat::GetArrowSpeed());
			FArrow& Arrow = Arrows.AddDefaulted_GetRef();
			Arrow.Shooter = Fighter.Entity;
			Arrow.Target = Enemy.Entity;
			Arrow.ShooterId = Fighter.UnitId;
			Arrow.TargetId = Enemy.UnitId;
			Arrow.ShooterType = Constants.TypeName;
			Arrow.ShooterSlot = Fighter.State->OwnerPlayer;
			Arrow.Damage = DuelScaled(Fighter, Damage);
			Arrow.AreaRadius = Constants.AreaRadius; // S4 (SQ1 = B): the Mangonel's blow reaches the enemies around the target
			Arrow.ShooterCamp = Fighter.Camp;
			Arrow.ReleaseStep = StepIndex;
			Arrow.ArrivalStep = StepIndex + HOLCombat::GetArrowFlightSteps(FlightMs, StepMs);
			++Combat.Hits;
			++TotalHits;
			++TotalArrowsReleased;
			// Drawn here in solo / host (J7c.2b, the same process; a dedicated server has no arrow display).
			if (UHOLArrowSubsystem* ArrowDisplay = World->GetSubsystem<UHOLArrowSubsystem>())
			{
				FHOLArrowLaunch Launch;
				Launch.From = Fighter.Location;
				Launch.To = Enemy.Location;
				Launch.Target = Enemy.Entity;
				Launch.FlightMs = FlightMs;
				Launch.ShooterType = Constants.TypeName;
				ArrowDisplay->Launch(Launch);
			}
			if (Combat.FirstHitStep < 0)
			{
				Combat.FirstHitStep = StepIndex;
			}
			if (LogHits != 0)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld #%u %s releases an arrow at #%u %s: %g damage, %.0f cm, %d ms, lands step %lld"), StepIndex,
					Fighter.UnitId, *Constants.TypeName.ToString(), Enemy.UnitId, *Enemy.Constants->TypeName.ToString(), static_cast<double>(Damage), FVector2D::Distance(Position, EnemyPosition),
					FlightMs, Arrow.ArrivalStep);
			}
			continue;
		}
		// D468: the hits come off one after another in the fighters' order (this loop is sequential): with float health the
		// order of the subtractions matters, so applying them from a parallel loop would break the reproducibility.
		// J8c.2: less under the morale effects of its side (aura, cry; J8c-Q1, Q2).
		const float Dealt = HOLHeroAbility::GetReducedDamage(DuelScaled(Fighter, Damage), GetDamageTakenPercent(Enemy));
		Enemy.State->CurrentHealth -= Dealt;
		if (Enemy.State->OwnerPlayer == FHOLMatchConfig::NeutralOwner)
		{
			HOLCamps::NoteHit(Enemy.UnitId, Fighter.State->OwnerPlayer, Dealt);
		}
		Enemy.Combat->LastAttacker = Fighter.Entity;
		Enemy.Combat->LastHitStep = StepIndex;
		++Combat.Hits;
		Combat.DamageDealt += Dealt;
		++TotalHits;
		if (Combat.FirstHitStep < 0)
		{
			Combat.FirstHitStep = StepIndex;
		}
		if (LogHits != 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld #%u %s hits #%u %s for %g, %g HP left"), StepIndex, Fighter.UnitId, *Constants.TypeName.ToString(),
				Enemy.UnitId, *Enemy.Constants->TypeName.ToString(), static_cast<double>(Dealt), static_cast<double>(Enemy.State->CurrentHealth));
		}
		if (Enemy.State->CurrentHealth <= 0)
		{
			++Combat.Kills;
			++TotalKills;
			Kill(Enemy, Fighter.State->OwnerPlayer, Fighter.UnitId);
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld #%u %s of slot %d killed by #%u %s of slot %d (%d hits struck)"), StepIndex, Enemy.UnitId,
				*Enemy.Constants->TypeName.ToString(), Enemy.State->OwnerPlayer, Fighter.UnitId, *Constants.TypeName.ToString(), Fighter.State->OwnerPlayer, Combat.Hits);
			// Its target dead, an ordered one too (the review of J7b): it looks for enemies again and its queue goes on.
			Touch(Index);
			Combat.Target = FMassEntityHandle();
			Combat.bOrderedTarget = false;
			Combat.bAttacking = false;
		}
	}
	// Destroyed at the end of the step (D224), once Dying for DyingSteps: the identifier table and the selection let it go
	// there (its population went at its death, MarkUnitDying).
	if (Deaths.Num() > 0)
	{
		Units->RequestDestroyUnits(Deaths);
	}
}

namespace UE::HeroesOfLiberty::Combat
{
	FAutoConsoleCommandWithWorldAndArgs ReportCommand(TEXT("hol.Combat.Report"),
		TEXT("hol.Combat.Report [Slot]: logs each unit's health, target, hits struck, damage, kills and first hit step, and the totals (server or solo)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* InWorld)
		{
			UHOLUnitSubsystem* Units = InWorld ? InWorld->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
			UMassEntitySubsystem* EntitySubsystem = InWorld ? InWorld->GetSubsystem<UMassEntitySubsystem>() : nullptr;
			if (!Units || !EntitySubsystem || InWorld->GetNetMode() == NM_Client)
			{
				UE_LOG(LogHOLSimulation, Warning, TEXT("hol.Combat.Report: on the server or in solo"));
				return;
			}
			const int32 Slot = Args.Num() > 0 ? FCString::Atoi(*Args[0]) : INDEX_NONE;
			const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
			const UHOLSimulationSubsystem* Simulation = InWorld->GetSubsystem<UHOLSimulationSubsystem>();
			TArray<uint32> UnitIds;
			Units->GetAllUnitIds(UnitIds);
			TArray<int32> Population;
			Units->GetPopulationByOwner(Population);
			TArray<FString> Lines;
			int32 Fighting = 0;
			for (const uint32 UnitId : UnitIds)
			{
				const FMassEntityHandle Entity = Units->FindEntity(UnitId);
				const FHOLCombatFragment* Combat = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
				const FHOLUnitStateFragment* State = Combat ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
				if (!State || (Slot != INDEX_NONE && State->OwnerPlayer != Slot))
				{
					continue;
				}
				const FHOLUnitTypeConstants& Constants = EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity);
				const uint32 TargetId = Units->FindUnitId(Combat->Target);
				const FVector Location = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform().GetLocation();
				Fighting += TargetId != 0 ? 1 : 0;
				Lines.Add(FString::Printf(TEXT("#%u %s slot %d hp %g/%d target %u attacking %d hits %d damage %g kills %d first %lld attackers %d at %.0f %.0f"), UnitId,
					*Constants.TypeName.ToString(), State->OwnerPlayer, static_cast<double>(State->CurrentHealth), HOLCombat::GetMaxHealth(Constants, 1, Units->GetTechnologyHealthPercent(Constants, State->OwnerPlayer)), TargetId, Combat->bAttacking ? 1 : 0, Combat->Hits,
					static_cast<double>(Combat->DamageDealt), Combat->Kills, Combat->FirstHitStep, Combat->AttackerCount, Location.X, Location.Y));
			}
			TArray<FString> PopulationText;
			for (int32 Owner = 0; Owner < Population.Num(); ++Owner)
			{
				PopulationText.Add(FString::Printf(TEXT("%d=%d"), Owner, Population[Owner]));
			}
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat report: step %lld units %d fighting %d hits %lld kills %lld population %s"),
				Simulation ? Simulation->GetStepIndex() : 0, Lines.Num(), Fighting, UHOLCombatProcessor::GetTotalHits(), UHOLCombatProcessor::GetTotalKills(),
				*FString::Join(PopulationText, TEXT(" ")));
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat report arrows: released %lld landed %lld lost %lld"), UHOLCombatProcessor::GetTotalArrowsReleased(),
				UHOLCombatProcessor::GetTotalArrowsLanded(), UHOLCombatProcessor::GetTotalArrowsLost());
			for (const FString& Line : Lines)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("  %s"), *Line);
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs AidReportCommand(TEXT("hol.Combat.AidReport"),
		TEXT("hol.Combat.AidReport Slot [Radius] [X Y Area]: logs the slot's idle melee soldiers (only those within Area cm of X Y if given) (Auto, no target, no order walked) and how many stand within Radius (default hol.Combat.AidRadius, else 2000 cm) of a fighting friend, with the closest enemy to them (server or solo)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* InWorld)
		{
			UHOLUnitSubsystem* Units = InWorld ? InWorld->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
			UMassEntitySubsystem* EntitySubsystem = InWorld ? InWorld->GetSubsystem<UMassEntitySubsystem>() : nullptr;
			if (!Units || !EntitySubsystem || InWorld->GetNetMode() == NM_Client || Args.Num() < 1)
			{
				UE_LOG(LogHOLSimulation, Warning, TEXT("hol.Combat.AidReport Slot [Radius]: on the server or in solo"));
				return;
			}
			const int32 Slot = FCString::Atoi(*Args[0]);
			const double Radius = Args.Num() > 1 ? FCString::Atod(*Args[1]) : (AidRadius > 0.f ? AidRadius : HOLCombat::DefaultAidRadius);
			const bool bArea = Args.Num() > 4;
			const FVector2D AreaCentre = bArea ? FVector2D(FCString::Atod(*Args[2]), FCString::Atod(*Args[3])) : FVector2D::ZeroVector;
			const double AreaRadius = bArea ? FCString::Atod(*Args[4]) : 0.;
			const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
			struct FUnit { FVector2D Position; float Radius; int32 Owner; bool bFighting; bool bIdleMelee; uint32 Id; };
			TArray<FUnit> All;
			TArray<uint32> UnitIds;
			Units->GetAllUnitIds(UnitIds);
			for (const uint32 UnitId : UnitIds)
			{
				const FMassEntityHandle Entity = Units->FindEntity(UnitId);
				const FHOLCombatFragment* Combat = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
				const FHOLUnitStateFragment* State = Combat ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
				const FHOLMoveOrderFragment* Order = Combat ? EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity) : nullptr;
				if (!State || !Order || State->CurrentHealth <= 0)
				{
					continue;
				}
				const FHOLUnitTypeConstants& Constants = EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity);
				FUnit& Unit = All.AddDefaulted_GetRef();
				Unit.Position = FVector2D(EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform().GetLocation());
				Unit.Radius = Constants.Radius;
				Unit.Owner = State->OwnerPlayer;
				Unit.Id = UnitId;
				Unit.bFighting = Combat->bAttacking || Combat->AttackerCount > 0 || Combat->LastAttacker.IsSet();
				Unit.bIdleMelee = Combat->Mode == EHOLCombatMode::Normal &&!Combat->Target.IsSet() && Combat->TargetBuilding.IsExplicitlyNull()
					&& Order->Phase == EHOLMovePhase::None && !HOLCombat::IsRangedAttack(Constants.AttackRange) && Constants.AggressionRadius > 0.f;
			}
			int32 Idle = 0;
			int32 Near = 0;
			double ClosestEnemy = -1.;
			TArray<FString> NearIds;
			for (const FUnit& Unit : All)
			{
				if (Unit.Owner != Slot || !Unit.bIdleMelee || (bArea && FVector2D::Distance(Unit.Position, AreaCentre) > AreaRadius))
				{
					continue;
				}
				++Idle;
				bool bNear = false;
				double Enemy = -1.;
				for (const FUnit& Other : All)
				{
					const double Distance = HOLCombat::GetEdgeDistance(Unit.Position, Unit.Radius, Other.Position, Other.Radius);
					if (Other.Owner == Slot)
					{
						bNear |= &Other != &Unit && Other.bFighting && Distance <= Radius;
					}
					else
					{
						Enemy = Enemy < 0. ? Distance : FMath::Min(Enemy, Distance);
					}
				}
				if (bNear)
				{
					++Near;
					NearIds.Add(FString::Printf(TEXT("#%u"), Unit.Id));
					ClosestEnemy = Enemy < 0. ? ClosestEnemy : (ClosestEnemy < 0. ? Enemy : FMath::Min(ClosestEnemy, Enemy));
				}
			}
			UE_LOG(LogHOLSimulation, Display, TEXT("Aid report: slot %d idle melee %d near a fighting friend %d (within %.0f cm) closest enemy %.0f cm %s"), Slot, Idle, Near, Radius,
				ClosestEnemy, *FString::Join(NearIds, TEXT(" ")));
		}));
}
