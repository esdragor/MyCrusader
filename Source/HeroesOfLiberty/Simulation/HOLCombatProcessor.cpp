// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLCombatProcessor.h"

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Presentation/HOLArrowSubsystem.h"
#include "Simulation/HOLBuildFragments.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

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

	int32 LogHits = 0;
	FAutoConsoleVariableRef CVarLogHits(TEXT("hol.Combat.LogHits"), LogHits, TEXT("1: logs every hit of the melee and every arrow released, landed or lost (server or solo)"));
	int32 Enabled = 1;
	FAutoConsoleVariableRef CVarEnabled(TEXT("hol.Combat.Enabled"), Enabled, TEXT("0: no unit fights (server or solo)"));
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

void UHOLCombatProcessor::KillUnit(const UWorld* World, UHOLUnitSubsystem& Units, FFighter& Victim, const int64 StepIndex) const
{
	// Dying (J7b.4, J7-Q5): stopped dead, out of the fight and of its task, destroyed DyingSteps later.
	if (World)
	{
		StopWalk(*World, Victim);
	}
	Victim.Combat->Disengage();
	Victim.Combat->DeathStep = StepIndex;
	Victim.State->Activity = EHOLUnitActivity::Dying;
	// Its population places free at once (the user's trial of J7b), not at its destruction.
	Units.MarkUnitDying(Victim.UnitId);
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

	// The step's units, in identifier order (the outcome does not depend on the chunks).
	Fighters.Reset();
	MaxRadius = 1.f;
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
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
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FFighter& Fighter = Fighters.AddDefaulted_GetRef();
			Fighter.Entity = Context.GetEntity(EntityIt);
			Fighter.UnitId = States[EntityIt].UnitId;
			Fighter.Location = Transforms[EntityIt].GetTransform().GetLocation();
			Fighter.Combat = &Combats[EntityIt];
			Fighter.State = &States[EntityIt];
			Fighter.Order = &Orders[EntityIt];
			Fighter.MoveTarget = &MoveTargets[EntityIt];
			Fighter.Movement = &Movements[EntityIt];
			Fighter.Constants = &Constants;
			Fighter.bBusy = Gathers[EntityIt].IsGathering() || Builds[EntityIt].IsBuilding();
		}
	});
	if (Fighters.Num() == 0)
	{
		TotalArrowsLost += Arrows.Num(); // no target left for any
		Arrows.Reset();
		return;
	}
	if (!bFighting)
	{
		Deaths.Reset();
		for (FFighter& Fighter : Fighters)
		{
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
	Fighters.Sort([](const FFighter& A, const FFighter& B) { return A.UnitId < B.UnitId; });
	IndexOfEntity.Reset();
	CellHeads.Init(INDEX_NONE, Buckets);
	NextInCell.SetNumUninitialized(Fighters.Num());
	Cells.SetNumUninitialized(Fighters.Num());
	// Inserted from the last, so each bucket lists its units in identifier order.
	for (int32 Index = Fighters.Num() - 1; Index >= 0; --Index)
	{
		const FFighter& Fighter = Fighters[Index];
		IndexOfEntity.Add(Fighter.Entity, Index);
		Cells[Index] = FIntPoint(FMath::FloorToInt32(Fighter.Location.X / CellSize), FMath::FloorToInt32(Fighter.Location.Y / CellSize));
		int32& Head = CellHeads[BucketOf(Cells[Index])];
		NextInCell[Index] = Head;
		Head = Index;
	}
	const auto FindFighter = [this](const FMassEntityHandle Entity) -> int32
	{
		const int32* Index = Entity.IsSet() ? IndexOfEntity.Find(Entity) : nullptr;
		return Index ? *Index : INDEX_NONE;
	};
	// Attackers of each unit at the step's start (HOLCombat::FCandidate::AttackerCount).
	for (FFighter& Fighter : Fighters)
	{
		Fighter.Combat->AttackerCount = 0;
	}
	for (const FFighter& Fighter : Fighters)
	{
		const int32 Target = FindFighter(Fighter.Combat->Target);
		if (Target != INDEX_NONE)
		{
			++Fighters[Target].Combat->AttackerCount;
		}
	}

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
		const int32 Victim = FindFighter(Arrow.Target);
		int32 Health = Victim != INDEX_NONE ? Fighters[Victim].State->CurrentHealth : 0;
		const HOLCombat::EArrowOutcome Outcome = HOLCombat::ResolveArrow(Victim != INDEX_NONE, Health, Arrow.Damage);
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
			Fighters[Shooter].Combat->DamageDealt += Arrow.Damage;
		}
		if (LogHits != 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld arrow of #%u %s lands on #%u %s for %d, %d HP left (released step %lld, %lld steps)"), StepIndex,
				Arrow.ShooterId, *Arrow.ShooterType.ToString(), Enemy.UnitId, *Enemy.Constants->TypeName.ToString(), Arrow.Damage, Health, Arrow.ReleaseStep,
				StepIndex - Arrow.ReleaseStep);
		}
		if (Outcome == HOLCombat::EArrowOutcome::Kill)
		{
			++TotalKills;
			if (Shooter != INDEX_NONE)
			{
				++Fighters[Shooter].Combat->Kills;
			}
			KillUnit(World, *Units, Enemy, StepIndex);
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld #%u %s of slot %d killed by #%u %s of slot %d (%d hits struck, by arrow)"), StepIndex, Enemy.UnitId,
				*Enemy.Constants->TypeName.ToString(), Enemy.State->OwnerPlayer, Arrow.ShooterId, *Arrow.ShooterType.ToString(), Arrow.ShooterSlot,
				Shooter != INDEX_NONE ? Fighters[Shooter].Combat->Hits : 0);
		}
	}
	Arrows.SetNum(Kept, EAllowShrinking::No);

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
				}
				Combat.Disengage();
			}
			HOLCombat::TickIdle(Combat.CooldownMs, StepMs);
			continue;
		}

		// Its target, if it still stands.
		int32 Target = FindFighter(Combat.Target);
		bool bLost = false;
		if (Combat.Target.IsSet() && (Target == INDEX_NONE || Fighters[Target].State->CurrentHealth <= 0))
		{
			Combat.Target = FMassEntityHandle();
			Combat.bOrderedTarget = false;
			Combat.bAttacking = false;
			Target = INDEX_NONE;
			bLost = true;
		}
		if (FindFighter(Combat.LastAttacker) == INDEX_NONE || Fighters[FindFighter(Combat.LastAttacker)].State->CurrentHealth <= 0)
		{
			Combat.LastAttacker = FMassEntityHandle();
		}

		// A unit walking a player's order takes on nobody by itself, but on an attack-move (J7b.3); an ordered target is
		// kept until it dies.
		const bool bWalkingOrder = Fighter.Order->Phase != EHOLMovePhase::None && !Combat.bApproaching;
		const bool bMaySeek = (!bWalkingOrder || Combat.Mode == EHOLCombatMode::AttackMove) && !Combat.bOrderedTarget;
		const bool bSearchStep = ((static_cast<int64>(Fighter.UnitId) + StepIndex) % SearchSteps) == 0;
		if (bMaySeek && (bSearchStep || bLost))
		{
			HOLCombat::FSeeker Seeker;
			Seeker.Position = Position;
			Seeker.Radius = Constants.Radius;
			Seeker.AttackRange = Constants.AttackRange;
			Seeker.AggressionRadius = Constants.AggressionRadius;
			Seeker.bRanged = HOLCombat::IsRangedAttack(Constants.AttackRange);
			Seeker.Mode = Combat.Mode == EHOLCombatMode::Hold ? HOLCombat::ESeekMode::Hold
				: Combat.Mode == EHOLCombatMode::AttackMove ? HOLCombat::ESeekMode::AttackMove : HOLCombat::ESeekMode::Auto;
			const float SearchRadius = HOLCombat::GetSearchRadius(Seeker);
			Candidates.Reset();
			CandidateFighters.Reset();
			const auto AddCandidate = [&](const int32 Other)
			{
				const FFighter& Enemy = Fighters[Other];
				HOLCombat::FCandidate& Candidate = Candidates.AddDefaulted_GetRef();
				Candidate.Id = Enemy.UnitId;
				Candidate.Position = FVector2D(Enemy.Location);
				Candidate.Radius = Enemy.Constants->Radius;
				Candidate.bAttacksMe = Enemy.Combat->Target == Fighter.Entity || Enemy.Entity == Combat.LastAttacker;
				Candidate.bIsCurrentTarget = Other == Target;
				Candidate.AttackerCount = Enemy.Combat->AttackerCount - (Other == Target ? 1 : 0);
				CandidateFighters.Add(Other);
			};
			if (SearchRadius > 0.f)
			{
				const double Reach = SearchRadius + Constants.Radius + MaxRadius;
				const FIntPoint MinCell(FMath::FloorToInt32((Position.X - Reach) / CellSize), FMath::FloorToInt32((Position.Y - Reach) / CellSize));
				const FIntPoint MaxCell(FMath::FloorToInt32((Position.X + Reach) / CellSize), FMath::FloorToInt32((Position.Y + Reach) / CellSize));
				for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
				{
					for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
					{
						const FIntPoint Cell(X, Y);
						for (int32 Other = CellHeads[BucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
						{
							const FFighter& Enemy = Fighters[Other];
							if (Cells[Other] != Cell || Enemy.State->OwnerPlayer == Fighter.State->OwnerPlayer || Enemy.State->CurrentHealth <= 0
								|| HOLCombat::GetEdgeDistance(Position, Constants.Radius, FVector2D(Enemy.Location), Enemy.Constants->Radius) > Reach)
							{
								continue;
							}
							AddCandidate(Other);
						}
					}
				}
				// The current target and the last attacker, wherever they stand (ChooseTarget weighs them).
				for (const int32 Extra : { Target, FindFighter(Combat.LastAttacker) })
				{
					if (Extra != INDEX_NONE && !CandidateFighters.Contains(Extra) && Fighters[Extra].State->OwnerPlayer != Fighter.State->OwnerPlayer)
					{
						AddCandidate(Extra);
					}
				}
			}
			const int32 Chosen = HOLCombat::ChooseTarget(Seeker, Candidates);
			const int32 NewTarget = Chosen == INDEX_NONE ? INDEX_NONE : CandidateFighters[Chosen];
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
				Combat.Target = NewTarget == INDEX_NONE ? FMassEntityHandle() : Fighters[NewTarget].Entity;
				Combat.bAttacking = false;
				Target = NewTarget;
			}
		}

		const int32 IntervalMs = HOLCombat::ToMilliseconds(Constants.AttackInterval);
		if (Target == INDEX_NONE)
		{
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

		FFighter& Enemy = Fighters[Target];
		const FVector2D EnemyPosition(Enemy.Location);
		if (!HOLCombat::IsInRange(Position, Constants.Radius, EnemyPosition, Enemy.Constants->Radius, Constants.AttackRange))
		{
			Combat.bAttacking = false;
			HOLCombat::TickIdle(Combat.CooldownMs, StepMs);
			// A walk to it at most at the search rate: the first, or once it moved off, or once the last one ended short.
			const bool bWalkOver = Fighter.Order->Phase == EHOLMovePhase::None;
			const bool bMoved = FVector2D::DistSquared(EnemyPosition, Combat.ApproachPoint) > FMath::Square(ReapproachDistance);
			if (Combat.Mode != EHOLCombatMode::Hold && (bSearchStep || bLost || !Combat.bApproaching) && (!Combat.bApproaching || bMoved || bWalkOver))
			{
				// Stamped (J7b.3): a player's order given meanwhile drops it.
				Commands->IssueCombatWalk(Fighter.UnitId, Enemy.Location, Combat.EngageSerial, /*bAttackMove*/ false);
				Combat.ApproachPoint = EnemyPosition;
				Combat.bAttackMoveDetour |= Combat.Mode == EHOLCombatMode::AttackMove;
				Combat.bApproaching = true;
			}
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
			Combat.CooldownMs = HOLCombat::StartAttack(Combat.CooldownMs, IntervalMs);
			Combat.bAttacking = true;
		}
		if (!HOLCombat::TickAttack(Combat.CooldownMs, StepMs, IntervalMs))
		{
			continue;
		}
		const bool bShot = HOLCombat::IsRangedAttack(Constants.AttackRange);
		const int32 Damage = HOLCombat::GetDamage(Constants, *Enemy.Constants, bShot);
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
			Arrow.Damage = Damage;
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
				ArrowDisplay->Launch(Launch);
			}
			if (Combat.FirstHitStep < 0)
			{
				Combat.FirstHitStep = StepIndex;
			}
			if (LogHits != 0)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld #%u %s releases an arrow at #%u %s: %d damage, %.0f cm, %d ms, lands step %lld"), StepIndex,
					Fighter.UnitId, *Constants.TypeName.ToString(), Enemy.UnitId, *Enemy.Constants->TypeName.ToString(), Damage, FVector2D::Distance(Position, EnemyPosition),
					FlightMs, Arrow.ArrivalStep);
			}
			continue;
		}
		Enemy.State->CurrentHealth -= Damage;
		Enemy.Combat->LastAttacker = Fighter.Entity;
		++Combat.Hits;
		Combat.DamageDealt += Damage;
		++TotalHits;
		if (Combat.FirstHitStep < 0)
		{
			Combat.FirstHitStep = StepIndex;
		}
		if (LogHits != 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld #%u %s hits #%u %s for %d, %d HP left"), StepIndex, Fighter.UnitId, *Constants.TypeName.ToString(),
				Enemy.UnitId, *Enemy.Constants->TypeName.ToString(), Damage, Enemy.State->CurrentHealth);
		}
		if (Enemy.State->CurrentHealth <= 0)
		{
			++Combat.Kills;
			++TotalKills;
			KillUnit(World, *Units, Enemy, StepIndex);
			UE_LOG(LogHOLSimulation, Display, TEXT("Combat: step %lld #%u %s of slot %d killed by #%u %s of slot %d (%d hits struck)"), StepIndex, Enemy.UnitId,
				*Enemy.Constants->TypeName.ToString(), Enemy.State->OwnerPlayer, Fighter.UnitId, *Constants.TypeName.ToString(), Fighter.State->OwnerPlayer, Combat.Hits);
			// Its target dead, an ordered one too (the review of J7b): it looks for enemies again and its queue goes on.
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
				Lines.Add(FString::Printf(TEXT("#%u %s slot %d hp %d/%d target %u attacking %d hits %d damage %d kills %d first %lld attackers %d at %.0f %.0f"), UnitId,
					*Constants.TypeName.ToString(), State->OwnerPlayer, State->CurrentHealth, Constants.MaxHealth, TargetId, Combat->bAttacking ? 1 : 0, Combat->Hits,
					Combat->DamageDealt, Combat->Kills, Combat->FirstHitStep, Combat->AttackerCount, Location.X, Location.Y));
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
}
