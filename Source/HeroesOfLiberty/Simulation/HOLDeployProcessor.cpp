// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLDeployProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "HeroesOfLibertyLog.h"
#include "Siege/HOLDeployRules.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationFragments.h"

// Palier3InProgress.md G6 step b: which unit carries the fragment is not decided (Retours/Q-palier3-deploy-wiring.md). Until it is, the Mangonel
// stands in for the engine behind this cvar: 0 (default) no unit of the game deploys, 1 every Mangonel is born packed with the fragment.
static TAutoConsoleVariable<int32> CVarSiegeDeployable(
	TEXT("hol.Siege.Deployable"), 0,
	TEXT("Test stand-in for the deployable engine: 1 = every Mangonel is born packed with the deploy fragment (G6). Default 0."),
	ECVF_Default);

bool HOLDeployStandInEnabled()
{
	return CVarSiegeDeployable.GetValueOnAnyThread() >= 1;
}

UHOLDeployProcessor::UHOLDeployProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	// After the player's orders (a march order is noted here), before the combat of the step: a unit that has just deployed fires in this step's combat.
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLUnitOrderProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("HOLCombatProcessor"));
}

void UHOLDeployProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLDeployFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FHOLCombatFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
}

void UHOLDeployProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const int32 StepMs = HOLCombat::ToMilliseconds(Context.GetDeltaTimeSeconds());
	// Until the unit table carries the columns (Q-trebuchet-deploy.md), every deployable engine has the Trebuchet's profile.
	const HOLDeploy::FProfile Profile = HOLDeploy::DefaultProfile();
	EntityQuery.ForEachEntityChunk(Context, [StepMs, &Profile](FMassExecutionContext& Chunk)
	{
		const TArrayView<FHOLDeployFragment> Deploys = Chunk.GetMutableFragmentView<FHOLDeployFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = Chunk.GetMutableFragmentView<FMassMoveTargetFragment>();
		const TArrayView<FMassDesiredMovementFragment> Movements = Chunk.GetMutableFragmentView<FMassDesiredMovementFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Chunk.GetFragmentView<FHOLMoveOrderFragment>();
		const TConstArrayView<FHOLCombatFragment> Combats = Chunk.GetFragmentView<FHOLCombatFragment>();
		const TConstArrayView<FHOLUnitStateFragment> UnitStates = Chunk.GetFragmentView<FHOLUnitStateFragment>();
		for (FMassExecutionContext::FEntityIterator It = Chunk.CreateEntityIterator(); It; ++It)
		{
			FHOLDeployFragment& Fragment = Deploys[It];
			HOLDeploy::FState State;
			State.Phase = HOLDeploy::PhaseAt(Fragment.Phase);
			State.ElapsedMs = Fragment.ElapsedMs;
			// A walk the player ordered (not the combat's own approach) while the engine is deployed or deploying: a march order, noted for
			// HOLDeploy::Decide (it packs the engine up, or cancels the deploying). The attack order's note is left by the combat.
			if (!Orders.IsEmpty() && !Combats.IsEmpty() && Orders[It].Phase != EHOLMovePhase::None && !Combats[It].bApproaching
				&& (State.Phase == HOLDeploy::EPhase::Deploying || State.Phase == HOLDeploy::EPhase::Deployed))
			{
				Fragment.PendingOrder = 2;
			}
			// A deployed, deploying or packing engine does not walk: only a packed one has a speed.
			if (!HOLDeploy::CanMove(State) && !Movements.IsEmpty() && !MoveTargets.IsEmpty())
			{
				Movements[It].DesiredVelocity = FVector::ZeroVector;
				MoveTargets[It].DesiredSpeed.Set(0.f);
			}
			if (Fragment.PendingOrder != 0)
			{
				// The order is read once: decided, applied, then forgotten.
				const HOLDeploy::EOrder Order = Fragment.PendingOrder == 1 ? HOLDeploy::EOrder::Attack : HOLDeploy::EOrder::Move;
				// The Mount / Dismount button (PendingOrder 3, G6 d) is the manual toggle; an order is decided by where its target stands.
				const HOLDeploy::FDecision Decision = Fragment.PendingOrder == 3 ? HOLDeploy::Toggle(State)
					: HOLDeploy::Decide(State, Order, Fragment.PendingDistanceCm, Profile);
				Fragment.LastAction = static_cast<uint8>(Decision.Action);
				Fragment.LastReason = Decision.Reason;
				// Retours/E-deploy.md: the decisions that change the phase, or come from the button, are written (a scenario reads them).
				if (Fragment.PendingOrder == 3 || Decision.NextPhase != State.Phase)
				{
					UE_LOG(LogHOLSimulation, Display, TEXT("Deploy: #%u phase %d -> %d (order %d, action %d, reason %d, distance %d cm, walk %d)"),
						UnitStates.IsEmpty() ? 0u : UnitStates[It].UnitId, static_cast<int32>(State.Phase), static_cast<int32>(Decision.NextPhase),
						Fragment.PendingOrder, static_cast<int32>(Decision.Action), Decision.Reason, Fragment.PendingDistanceCm,
						Orders.IsEmpty() ? -1 : static_cast<int32>(Orders[It].Phase));
				}
				State = HOLDeploy::Apply(State, Decision);
				Fragment.PendingOrder = 0;
			}
			const HOLDeploy::EPhase PhaseBefore = State.Phase;
			State = HOLDeploy::Advance(State, StepMs, Profile, Fragment.CounterweightPercent);
			if (State.Phase != PhaseBefore)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Deploy: #%u phase %d -> %d (delay over)"), UnitStates.IsEmpty() ? 0u : UnitStates[It].UnitId,
					static_cast<int32>(PhaseBefore), static_cast<int32>(State.Phase));
			}
			Fragment.Phase = static_cast<uint8>(State.Phase);
			Fragment.ElapsedMs = State.ElapsedMs;
		}
	});
}
