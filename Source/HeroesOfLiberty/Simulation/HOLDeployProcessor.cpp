// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLDeployProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Siege/HOLDeployRules.h"
#include "Simulation/HOLCombatRules.h"

// Palier3InProgress.md G6 step b: which unit carries the fragment is not decided (Retours/Q-palier3-deploy-wiring.md). Until it is, the Mangonel
// stands in for the engine behind this cvar: 0 (default) no unit of the game deploys, 1 every Mangonel is born packed with the fragment.
static TAutoConsoleVariable<int32> CVarSiegeDeployable(
	TEXT("hol.Siege.Deployable"), 0,
	TEXT("Test stand-in for the deployable engine: 1 = every Mangonel is born packed with the deploy fragment (G6). Default 0."),
	ECVF_Default);

UHOLDeployProcessor::UHOLDeployProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	// Before the combat of the step: a unit that has just deployed fires in this step's combat.
	ExecutionOrder.ExecuteBefore.Add(TEXT("HOLCombatProcessor"));
}

void UHOLDeployProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLDeployFragment>(EMassFragmentAccess::ReadWrite);
}

void UHOLDeployProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const int32 StepMs = HOLCombat::ToMilliseconds(Context.GetDeltaTimeSeconds());
	// Until the unit table carries the columns (Q-trebuchet-deploy.md), every deployable engine has the Trebuchet's profile.
	const HOLDeploy::FProfile Profile = HOLDeploy::DefaultProfile();
	EntityQuery.ForEachEntityChunk(Context, [StepMs, &Profile](FMassExecutionContext& Chunk)
	{
		const TArrayView<FHOLDeployFragment> Deploys = Chunk.GetMutableFragmentView<FHOLDeployFragment>();
		for (FMassExecutionContext::FEntityIterator It = Chunk.CreateEntityIterator(); It; ++It)
		{
			FHOLDeployFragment& Fragment = Deploys[It];
			HOLDeploy::FState State;
			State.Phase = HOLDeploy::PhaseAt(Fragment.Phase);
			State.ElapsedMs = Fragment.ElapsedMs;
			if (Fragment.PendingOrder != 0)
			{
				// The order is read once: decided, applied, then forgotten.
				const HOLDeploy::EOrder Order = Fragment.PendingOrder == 1 ? HOLDeploy::EOrder::Attack : HOLDeploy::EOrder::Move;
				const HOLDeploy::FDecision Decision = HOLDeploy::Decide(State, Order, Fragment.PendingDistanceCm, Profile);
				Fragment.LastAction = static_cast<uint8>(Decision.Action);
				Fragment.LastReason = Decision.Reason;
				State = HOLDeploy::Apply(State, Decision);
				Fragment.PendingOrder = 0;
			}
			State = HOLDeploy::Advance(State, StepMs, Profile, Fragment.CounterweightPercent);
			Fragment.Phase = static_cast<uint8>(State.Phase);
			Fragment.ElapsedMs = State.ElapsedMs;
		}
	});
}
