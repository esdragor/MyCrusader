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
#include "Presentation/HOLArrowSubsystem.h"
#include "Presentation/HOLUnitAnimation.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLUnitTypes.h"

namespace UE::HeroesOfLiberty::NetDisplay
{
	/** Below this speed (cm/s) a unit keeps its facing. */
	constexpr float MinTurnSpeed = 20.f;
	/** At most this many steps replayed at once for a shooter's arrows (a long stall: the rest skipped, not a burst). */
	constexpr int64 MaxShotStepsAtOnce = 20;
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
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>();
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
	const int32 StepMs = FMath::Max(FMath::RoundToInt32(StepSeconds * 1000.f), 1);
	UHOLArrowSubsystem* ArrowDisplay = World->GetSubsystem<UHOLArrowSubsystem>();

	EntityQuery.ForEachEntityChunk(Context, [Now, ClockStep, StepSeconds, StepMs, Reception, ArrowDisplay, &EntityManager](FMassExecutionContext& Context)
	{
		const FHOLUnitTypeConstants& Constants = Context.GetConstSharedFragment<FHOLUnitTypeConstants>();
		const bool bShooter = ArrowDisplay && HOLCombat::IsRangedAttack(Constants.AttackRange);
		const int32 IntervalMs = HOLCombat::ToMilliseconds(Constants.AttackInterval);
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

			// Its arrows (J7c.2b2, Q4): deduced, nothing more received. The server's cadence replayed on the clock from the
			// step its Attacking state was taken at (that step already ticked, HOLCombat::StartAttack then TickAttack), an
			// arrow at each step the replayed cool-down runs out, from where it is drawn to where its target is drawn, as
			// long as it stays Attacking (a new target keeps the cadence, as on the server). Not Attacking, the cool-down
			// keeps running down (hit and run, J7c, retour d'essai 8): attacking again, it resumes from what is left.
			if (!bShooter || Display.State.Activity != EHOLUnitActivity::Attacking)
			{
				Display.StopShots();
				continue;
			}
			if (Display.ShotStep < 0)
			{
				Display.StartShots(Display.StateStep, StepMs, IntervalMs, Constants.AttackWindUpPercent);
			}
			const int64 ClockWhole = FMath::FloorToInt64(ClockStep);
			if (ClockWhole - Display.ShotStep > MaxShotStepsAtOnce)
			{
				Display.ShotStep = ClockWhole - MaxShotStepsAtOnce;
			}
			while (Display.ShotStep < ClockWhole)
			{
				++Display.ShotStep;
				if (!HOLCombat::TickAttack(Display.ShotCooldownMs, StepMs, IntervalMs))
				{
					continue;
				}
				const FMassEntityHandle Target = Reception->GetTargetEntity(Display.State);
				const FTransformFragment* TargetTransform = EntityManager.IsEntityValid(Target) ? EntityManager.GetFragmentDataPtr<FTransformFragment>(Target) : nullptr;
				if (TargetTransform == nullptr)
				{
					continue;
				}
				FHOLArrowLaunch Launch;
				Launch.From = Drawn.GetLocation();
				Launch.To = TargetTransform->GetTransform().GetLocation();
				Launch.Target = Target;
				Launch.FlightMs = HOLCombat::GetArrowFlightMilliseconds(FVector2D(Launch.From), FVector2D(Launch.To), HOLCombat::GetArrowSpeed());
				ArrowDisplay->Launch(Launch);
			}
			// Its gesture's phase (UHOLInterpolatedInstancesProcessor): its next release, from the clock's present.
			Display.ShotSecondsToHit = HOLUnitAnimation::GetSecondsToHit(Display.ShotCooldownMs, StepMs)
				+ static_cast<float>((static_cast<double>(Display.ShotStep) - ClockStep) * StepSeconds);
		}
	});
}

void FHOLNetUnitDisplayFragment::StartShots(const int64 AttackStep, const int32 StepMs, const int32 IntervalMs, const int32 WindUpPercent)
{
	int32 LeftMs = 0;
	if (ShotIdleStep >= 0)
	{
		const int64 IdleSteps = FMath::Max<int64>(AttackStep - 1 - ShotIdleStep, 0);
		LeftMs = static_cast<int32>(FMath::Max<int64>(static_cast<int64>(ShotCooldownMs) - IdleSteps * StepMs, 0));
	}
	ShotStep = AttackStep;
	ShotCooldownMs = HOLCombat::StartAttack(LeftMs, IntervalMs, WindUpPercent) - StepMs;
}

void FHOLNetUnitDisplayFragment::StopShots()
{
	if (ShotStep >= 0)
	{
		ShotIdleStep = ShotStep;
		ShotStep = -1;
	}
}
