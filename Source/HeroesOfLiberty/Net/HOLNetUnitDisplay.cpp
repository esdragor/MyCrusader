// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLNetUnitDisplay.h"

#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntityTemplateRegistry.h"
#include "MassExecutionContext.h"
#include "MassLODTypes.h"
#include "MassRepresentationTypes.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Simulation/HOLSimulationFragments.h"

namespace UE::HeroesOfLiberty::NetDisplay
{
	/** Below this speed (cm/s) a unit keeps its facing. */
	constexpr float MinTurnSpeed = 20.f;
}

void UHOLNetUnitDisplayTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FHOLNetUnitDisplayFragment>();
}

UHOLNetUnitDisplayProcessor::UHOLNetUnitDisplayProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::LODCollector);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Representation);
}

void UHOLNetUnitDisplayProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FHOLNetUnitDisplayFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLPreviousTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UHOLNetUnitDisplayProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::NetDisplay;

	const UWorld* World = EntityManager.GetWorld();
	const UHOLUnitReceptionSubsystem* Reception = World ? World->GetSubsystem<UHOLUnitReceptionSubsystem>() : nullptr;
	if (Reception == nullptr || !Reception->GetClock().IsValid())
	{
		return;
	}
	const double Now = FPlatformTime::Seconds();
	const double ClockStep = Reception->GetClock().GetStep(Now);
	const float StepSeconds = Reception->GetStepSeconds();

	EntityQuery.ForEachEntityChunk(Context, [Now, ClockStep, StepSeconds, Reception, &EntityManager](FMassExecutionContext& Context)
	{
		const TArrayView<FHOLNetUnitDisplayFragment> Displays = Context.GetMutableFragmentView<FHOLNetUnitDisplayFragment>();
		const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FHOLPreviousTransformFragment> PreviousTransforms = Context.GetMutableFragmentView<FHOLPreviousTransformFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FHOLNetUnitDisplayFragment& Display = Displays[EntityIt];
			// Facing its walk at once, as the simulation does (UHOLInstantOrientationProcessor); a unit at rest pushed aside
			// (its sent velocity is the step's move, D294) does not turn.
			if (IsWalkingActivity(Display.State.Activity) &&Display.Velocity.SizeSquared2D() > FMath::Square(MinTurnSpeed))
			{
				Display.Yaw = FMath::RadiansToDegrees(FMath::Atan2(Display.Velocity.Y, Display.Velocity.X));
			}
			else if (Display.State.Activity == EHOLUnitActivity::Attacking)
			{
				// Striking (J7b.4, J7-Q5): turned towards its target, where this client draws it.
				const FMassEntityHandle Target = Reception->GetTargetEntity(Display.State);
				const FTransformFragment* TargetTransform = EntityManager.IsEntityValid(Target) ? EntityManager.GetFragmentDataPtr<FTransformFragment>(Target) : nullptr;
				const FVector ToTarget = TargetTransform ? TargetTransform->GetTransform().GetLocation() - Transforms[EntityIt].GetTransform().GetLocation() : FVector::ZeroVector;
				if (ToTarget.SizeSquared2D() > 1.0)
				{
					Display.Yaw = FMath::RadiansToDegrees(FMath::Atan2(ToTarget.Y, ToTarget.X));
				}
			}
			else if (Display.State.bFacesFront)
			{
				// At rest at its formation place (J7b.4, J7a.4's deferred item): its front, as the server holds it.
				Display.Yaw = Display.State.GetFrontYaw();
			}
			// The drawing interpolates from the previous transform with the simulation's alpha, which is 1 on a client
			// (no fixed step there): both get the present.
			const FTransform Drawn(FRotator(0.f, Display.Yaw, 0.f), Display.GetDisplayedPosition(ClockStep, Now, StepSeconds));
			Transforms[EntityIt].SetTransform(Drawn);
			PreviousTransforms[EntityIt].Transform = Drawn;
		}
	});
}
