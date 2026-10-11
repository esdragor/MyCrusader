// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/MCSimulationProcessors.h"

#include "Engine/World.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavMeshNavigationFragments.h"
#include "NavCorridor.h"
#include "NavigationSystem.h"
#include "Simulation/MCSimulationFragments.h"

//----------------------------------------------------------------------//
// UMCStorePreviousTransformProcessor
//----------------------------------------------------------------------//

UMCStorePreviousTransformProcessor::UMCStorePreviousTransformProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Tasks);
}

void UMCStorePreviousTransformProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCPreviousTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UMCStorePreviousTransformProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMCPreviousTransformFragment> PreviousTransforms = Context.GetMutableFragmentView<FMCPreviousTransformFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			PreviousTransforms[EntityIt].Transform = Transforms[EntityIt].GetTransform();
		}
	});
}

//----------------------------------------------------------------------//
// UMCWanderProcessor
//----------------------------------------------------------------------//

namespace UE::MyCrusader::Wander
{
	// Same corridor settings as Mass' navmesh path follow task.
	constexpr float CorridorWidth = 600.f;
	constexpr float OffsetFromBoundaries = 10.f;
	constexpr float EndDistanceThreshold = 20.f;

	bool RequestPath(const UWorld& World, const UNavigationSystemV1& NavSystem, const ANavigationData& NavData, const FMassEntityHandle Entity,
		const FVector& Start, const FVector& Destination, const FMassMovementParameters& MovementParams, const FMassDesiredMovementFragment& DesiredMovement,
		FMassNavMeshCachedPathFragment& CachedPath, FMassNavMeshShortPathFragment& ShortPath, FMassMoveTargetFragment& MoveTarget)
	{
		const FPathFindingQuery Query(&NavSystem, NavData, Start, Destination);
		const FPathFindingResult Result = NavData.FindPath(Query.NavAgentProperties, Query);
		if (!Result.IsSuccessful() || Result.Path->GetPathPoints().Num() < 2)
		{
			return false;
		}
		Result.Path->RemoveOverlappingPoints(FNavCorridor::OverlappingPointTolerance);
		if (Result.Path->GetPathPoints().Num() < 2)
		{
			return false;
		}

		CachedPath.NavPath = Result.Path;
		CachedPath.PathSource = EMassNavigationPathSource::NavMesh;
		CachedPath.Corridor = MakeShared<FNavCorridor>();

		FNavCorridorParams CorridorParams;
		CorridorParams.SetFromWidth(CorridorWidth);
		CorridorParams.PathOffsetFromBoundaries = OffsetFromBoundaries;
		CachedPath.Corridor->BuildFromPath(*CachedPath.NavPath, NavData.GetDefaultQueryFilter(), CorridorParams);

		ShortPath.RequestShortPath(CachedPath.Corridor, /*NavCorridorStartIndex*/0, /*NumLeadingPoints*/0, EndDistanceThreshold);
		CachedPath.NavPathNextStartIndex = static_cast<uint16>(FMath::Max(ShortPath.NumPoints - FMassNavMeshShortPathFragment::NumPointsBeyondUpdate - FMassNavMeshCachedPathFragment::NumLeadingPoints, 0));

		const float DesiredSpeed = FMath::Min3(
			MovementParams.GenerateDesiredSpeed(FMassMovementStyleRef(), Entity.Index),
			MovementParams.MaxSpeed,
			DesiredMovement.DesiredMaxSpeedOverride);
		MoveTarget.DesiredSpeed.Set(DesiredSpeed);
		MoveTarget.CreateNewAction(EMassMovementAction::Move, World);
		return true;
	}

	void UpdateShortPath(FMassNavMeshCachedPathFragment& CachedPath, FMassNavMeshShortPathFragment& ShortPath)
	{
		ShortPath.RequestShortPath(CachedPath.Corridor, CachedPath.NavPathNextStartIndex, FMassNavMeshCachedPathFragment::NumLeadingPoints, EndDistanceThreshold);
		CachedPath.NavPathNextStartIndex += static_cast<uint16>(FMath::Max(ShortPath.NumPoints - FMassNavMeshShortPathFragment::NumPointsBeyondUpdate - FMassNavMeshCachedPathFragment::NumLeadingPoints, 0));
	}
}

UMCWanderProcessor::UMCWanderProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavMeshPathFollowProcessor"));
	bRequiresGameThreadExecution = true; // synchronous navmesh queries
}

void UMCWanderProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassNavMeshCachedPathFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassNavMeshShortPathFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>();
	EntityQuery.AddTagRequirement<FMCWanderTag>(EMassFragmentPresence::All);
}

void UMCWanderProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	ANavigationData* NavData = NavSystem ? NavSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) : nullptr;
	if (!NavData)
	{
		return;
	}

	int32 PathRequestsLeft = MaxPathRequestsPerStep;

	EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassDesiredMovementFragment> DesiredMovements = Context.GetFragmentView<FMassDesiredMovementFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const TArrayView<FMassNavMeshCachedPathFragment> CachedPaths = Context.GetMutableFragmentView<FMassNavMeshCachedPathFragment>();
		const TArrayView<FMassNavMeshShortPathFragment> ShortPaths = Context.GetMutableFragmentView<FMassNavMeshShortPathFragment>();
		const FMassMovementParameters& MovementParams = Context.GetConstSharedFragment<FMassMovementParameters>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FMassNavMeshShortPathFragment& ShortPath = ShortPaths[EntityIt];
			if (!ShortPath.IsDone())
			{
				continue;
			}

			FMassNavMeshCachedPathFragment& CachedPath = CachedPaths[EntityIt];
			if (ShortPath.bPartialResult && CachedPath.Corridor.IsValid())
			{
				// The short path only covers part of the corridor: move on to the next part.
				UE::MyCrusader::Wander::UpdateShortPath(CachedPath, ShortPath);
				continue;
			}

			if (PathRequestsLeft <= 0)
			{
				continue;
			}
			--PathRequestsLeft;

			const FVector Location = Transforms[EntityIt].GetTransform().GetLocation();
			FNavLocation Destination;
			if (NavSystem->GetRandomReachablePointInRadius(Location, WanderRadius, Destination, NavData))
			{
				UE::MyCrusader::Wander::RequestPath(*World, *NavSystem, *NavData, Context.GetEntity(EntityIt), Location, Destination.Location,
					MovementParams, DesiredMovements[EntityIt], CachedPath, ShortPath, MoveTargets[EntityIt]);
			}
		}
	});
}
