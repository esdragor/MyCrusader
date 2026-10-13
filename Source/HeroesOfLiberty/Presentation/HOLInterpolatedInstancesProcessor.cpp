// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLInterpolatedInstancesProcessor.h"

#include "Engine/World.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassLODFragments.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassUpdateISMProcessor.h"
#include "MassUpdateSkinnedMeshProcessor.h"
#include "MassVisualizationComponent.h"
#include "Presentation/HOLPresentationFragments.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

namespace UE::HeroesOfLiberty::Presentation
{
	// Walk/idle choice from the drawn speed, as a share of the type's speed (D234). Two thresholds so a unit
	// hovering around one does not flicker between the two animations. Provisional, to tune in test.
	constexpr float WalkStartSpeedShare = 0.15f;
	constexpr float WalkStopSpeedShare = 0.10f;
	constexpr float AnimationBlendSeconds = 0.2f;

	/**
	 * Player colours (D233), chosen by the client: the units only carry their owner's index. Index 8 and above
	 * wrap. Written as linear base colour into the instance's custom data 0-2 (M_UnitPlayerColor).
	 */
	const FVector3f& GetPlayerColor(const uint8 OwnerPlayer)
	{
		static const TArray<FVector3f> Palette = []
		{
			const FColor Colors[] = {
				FColor(40, 90, 220),	// blue
				FColor(210, 40, 40),	// red
				FColor(40, 170, 60),	// green
				FColor(230, 200, 40),	// yellow
				FColor(40, 190, 200),	// teal
				FColor(150, 60, 190),	// purple
				FColor(230, 120, 30),	// orange
				FColor(230, 120, 170),	// pink
			};
			TArray<FVector3f> Linear;
			for (const FColor& Color : Colors)
			{
				const FLinearColor Converted(Color);
				Linear.Add(FVector3f(Converted.R, Converted.G, Converted.B));
			}
			return Linear;
		}();
		return Palette[OwnerPlayer % Palette.Num()];
	}

	/** Start offset (s) of a unit in a sequence: stable per unit and sequence, so 2 000 units do not step in time. */
	float GetStartOffset(const uint32 UnitId, const int32 SequenceIndex, const float SequenceLength)
	{
		const uint32 Hash = HashCombineFast(GetTypeHash(UnitId), GetTypeHash(SequenceIndex));
		return SequenceLength * static_cast<float>(Hash & 0xFFFF) / 65536.f;
	}
}

UHOLInterpolatedInstancesProcessor::UHOLInterpolatedInstancesProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
	bRequiresGameThreadExecution = true;
}

void UHOLInterpolatedInstancesProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLPreviousTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationAnimationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLUnitAnimationStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>();
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.SetChunkFilter(&FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UMassRepresentationSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMassStaticRepresentationTag>(EMassFragmentPresence::None);
}

void UHOLInterpolatedInstancesProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Presentation;

	const UWorld* World = EntityManager.GetWorld();
	const UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	const float Alpha = Simulation ? Simulation->GetInterpolationAlpha() : 1.f;
	const float StepSeconds = Simulation ? Simulation->GetStepSeconds() : 0.f;

	EntityQuery.ForEachEntityChunk(Context, [Alpha, StepSeconds](FMassExecutionContext& Context)
	{
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		FMassInstancedStaticMeshInfoArrayView ISMInfos = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		FMassInstancedSkinnedMeshInfoArrayView SkinnedMeshInfos = RepresentationSubsystem->GetMutableInstancedSkinnedMeshInfos();

		const FHOLUnitTypeConstants& Constants = Context.GetConstSharedFragment<FHOLUnitTypeConstants>();
		const float WalkStartSpeed = Constants.Speed * WalkStartSpeedShare;
		const float WalkStopSpeed = Constants.Speed * WalkStopSpeedShare;

		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FHOLPreviousTransformFragment> PreviousTransforms = Context.GetFragmentView<FHOLPreviousTransformFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TArrayView<FMassRepresentationFragment> Representations = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODs = Context.GetFragmentView<FMassRepresentationLODFragment>();
		const TArrayView<FMassRepresentationAnimationFragment> Animations = Context.GetMutableFragmentView<FMassRepresentationAnimationFragment>();
		const TArrayView<FHOLUnitAnimationStateFragment> AnimationStates = Context.GetMutableFragmentView<FHOLUnitAnimationStateFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FMassRepresentationFragment& Representation = Representations[EntityIt];
			const bool bStatic = Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance;
			const bool bSkinned = Representation.CurrentRepresentation == EMassRepresentationType::SkinnedMeshInstance;
			if (!bStatic && !bSkinned)
			{
				continue;
			}

			const FTransform& From = PreviousTransforms[EntityIt].Transform;
			const FTransform& To = Transforms[EntityIt].GetTransform();
			const FTransform Drawn(
				FQuat::Slerp(From.GetRotation(), To.GetRotation(), Alpha),
				FMath::Lerp(From.GetLocation(), To.GetLocation(), Alpha),
				To.GetScale3D());

			const FMassEntityHandle Entity = Context.GetEntity(EntityIt);
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODs[EntityIt];
			// PrevTransform is last frame's drawn transform (motion vectors), not the previous fixed step.
			if (bStatic)
			{
				const int32 ISMInfoIndex = Representation.StaticMeshDescHandle.ToIndex();
				if (ensureMsgf(ISMInfos.IsValidIndex(ISMInfoIndex), TEXT("Invalid handle index %u for ISMInfosView"), ISMInfoIndex))
				{
					UMassUpdateISMProcessor::UpdateISMTransform(Entity, ISMInfos[ISMInfoIndex], Drawn, Representation.PrevTransform,
						RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
				}
			}
			else
			{
				const int32 SkinnedMeshInfoIndex = Representation.SkinnedMeshDescHandle.ToIndex();
				if (ensureMsgf(SkinnedMeshInfos.IsValidIndex(SkinnedMeshInfoIndex), TEXT("Invalid handle index %u for SkinnedMeshInfosView"), SkinnedMeshInfoIndex))
				{
					FMassInstancedSkinnedMeshInfo& MeshInfo = SkinnedMeshInfos[SkinnedMeshInfoIndex];

					// Walk or idle from the speed the client draws (D234): the step's displacement over its duration.
					const float DrawnSpeed = StepSeconds > 0.f ? FVector::Dist2D(From.GetLocation(), To.GetLocation()) / StepSeconds : 0.f;
					FHOLUnitAnimationStateFragment& AnimationState = AnimationStates[EntityIt];
					FAnimSequenceTrackAutoPlayData& AnimData = Animations[EntityIt].AnimData;
					const bool bWalking = AnimationState.bStarted && AnimData.SequenceIndex == Constants.WalkSequence;
					const bool bWantsWalk = bWalking ? DrawnSpeed >= WalkStopSpeed : DrawnSpeed > WalkStartSpeed;
					const int32 WantedSequence = bWantsWalk ? Constants.WalkSequence : Constants.IdleSequence;
					if (!AnimationState.bStarted || AnimData.SequenceIndex != WantedSequence)
					{
						// Mass hands the data to the instance only when the sequence changes, so the play rate is set here.
						AnimData.SequenceIndex = WantedSequence;
						AnimData.PlayRate = bWantsWalk ? Constants.WalkPlayRate : 1.f;
						AnimData.Position = GetStartOffset(States[EntityIt].UnitId, WantedSequence, bWantsWalk ? Constants.WalkLength : Constants.IdleLength);
						AnimData.BlendTime = AnimationState.bStarted ? AnimationBlendSeconds : 0.f;
						AnimData.LoopMode = EAnimSequenceTrackLoopMode::Loop;
						AnimationState.bStarted = true;
					}

					// Transform, animation and colour in this order for each entity: Mass pairs them by position.
					UMassUpdateInstancedSkinnedMeshProcessor::UpdateMeshTransform(Entity, MeshInfo, Drawn, Representation.PrevTransform,
						RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
					MeshInfo.AddBatchedAnimationData(Entity, AnimData, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
					MeshInfo.AddBatchedCustomData(GetPlayerColor(States[EntityIt].OwnerPlayer), RepresentationLOD.LODSignificance,
						Representation.PrevLODSignificance);
				}
			}

			Representation.PrevTransform = Drawn;
			Representation.PrevLODSignificance = RepresentationLOD.LODSignificance;
		}
	});
}
