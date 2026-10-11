// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/MCInterpolatedISMProcessor.h"

#include "Engine/World.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassLODFragments.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassUpdateISMProcessor.h"
#include "MassVisualizationComponent.h"
#include "Simulation/MCSimulationFragments.h"
#include "Simulation/MCSimulationSubsystem.h"

UMCInterpolatedISMProcessor::UMCInterpolatedISMProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
	bRequiresGameThreadExecution = true;
}

void UMCInterpolatedISMProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCPreviousTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.SetChunkFilter(&FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UMassRepresentationSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMassStaticRepresentationTag>(EMassFragmentPresence::None);
}

void UMCInterpolatedISMProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const UWorld* World = EntityManager.GetWorld();
	const UMCSimulationSubsystem* Simulation = World ? World->GetSubsystem<UMCSimulationSubsystem>() : nullptr;
	const float Alpha = Simulation ? Simulation->GetInterpolationAlpha() : 1.f;

	EntityQuery.ForEachEntityChunk(Context, [Alpha](FMassExecutionContext& Context)
	{
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		FMassInstancedStaticMeshInfoArrayView ISMInfos = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();

		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMCPreviousTransformFragment> PreviousTransforms = Context.GetFragmentView<FMCPreviousTransformFragment>();
		const TArrayView<FMassRepresentationFragment> Representations = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODs = Context.GetFragmentView<FMassRepresentationLODFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FMassRepresentationFragment& Representation = Representations[EntityIt];
			if (Representation.CurrentRepresentation != EMassRepresentationType::StaticMeshInstance)
			{
				continue;
			}

			const int32 ISMInfoIndex = Representation.StaticMeshDescHandle.ToIndex();
			if (!ensureMsgf(ISMInfos.IsValidIndex(ISMInfoIndex), TEXT("Invalid handle index %u for ISMInfosView"), ISMInfoIndex))
			{
				continue;
			}

			const FTransform& From = PreviousTransforms[EntityIt].Transform;
			const FTransform& To = Transforms[EntityIt].GetTransform();
			const FTransform Drawn(
				FQuat::Slerp(From.GetRotation(), To.GetRotation(), Alpha),
				FMath::Lerp(From.GetLocation(), To.GetLocation(), Alpha),
				To.GetScale3D());

			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODs[EntityIt];
			// PrevTransform is last frame's drawn transform (motion vectors), not the previous fixed step.
			UMassUpdateISMProcessor::UpdateISMTransform(Context.GetEntity(EntityIt), ISMInfos[ISMInfoIndex], Drawn, Representation.PrevTransform,
				RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);

			Representation.PrevTransform = Drawn;
			Representation.PrevLODSignificance = RepresentationLOD.LODSignificance;
		}
	});
}
