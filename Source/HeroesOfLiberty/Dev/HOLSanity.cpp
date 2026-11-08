// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Dev/HOLSanityRules.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLPlayerState.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassMovementFragments.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

/**
 * hol.Test.Sanity: what a trial must never show, written to the log for the pre-flight (Tools/Scenarios/preflight.ps1):
 * "Sanity: OK" or one "Sanity: FAIL <reason>" line per fault found (the pre-flight fails on any FAIL line). A scenario calls
 * it at its end. Three checks:
 *   1. a neutral unit's display colour is no player's colour of the game (units of the neutral camps drawn in a player's colour);
 *   2. no unit with an order spins without getting anywhere (HOLSanity::FUnitTrack, the walking units' steps);
 *   3. no unit without an order trembles (the same, the idle units' steps).
 * Checks 2 and 3 need the units' steps: hol.Sanity.Watch 1 (a scenario puts it first) records them from the simulation's steps;
 * hol.Test.Sanity says FAIL when nothing was recorded, so a scenario that forgot it does not pass for sound.
 * hol.Sanity.Shake <cm> is the test fixture of check 3 (the idle units' real simulation transforms are pushed +-cm each step).
 */
namespace UE::HeroesOfLiberty::Dev
{
	namespace
	{
		struct FSanityWatch
		{
			TWeakObjectPtr<UWorld> World;
			FDelegateHandle Handle;
			TMap<uint32, HOLSanity::FUnitTrack> Tracks;
			/** First fault of each unit, as the line the report prints. */
			TMap<uint32, FString> Faults;
			int64 Steps = 0;
			double Shake = 0.;
			HOLSanity::FSanityRules Rules;
			/** Each unit's last 8 s (places and orders) for the whole-window rules (HOLSanity::Analyse, merged from hero-j8-close). */
			TMap<uint32, HOLSanity::FTrace> Windows;
			/** hol.Test.SanityMotion <Seconds>: the verdict (hol.Test.Sanity) is given once this many steps are watched (0: none). */
			int64 VerdictAtSteps = 0;
			/** Check 4: each unit's run of steps standing still out of reach of its target while a place was free round it, and the faults found. */
			TMap<uint32, HOLSanity::FStall> Stalls;
			TMap<uint32, FString> StallFaults;
			/** Check 5: each unit's run of steps in reach of a living target without a blow of its own landing, and the faults found. */
			TMap<uint32, HOLSanity::FNoStrike> NoStrikes;
			TMap<uint32, FString> NoStrikeFaults;
		};
		FSanityWatch Watch;
		/** hol.Test.SanityStall <Seconds>: check 4, a soldier standing still this long out of reach of its target with a free place round it fails (0: off). */
		float StallSeconds = 0.5f;
		FAutoConsoleVariableRef CVarStallSeconds(TEXT("hol.Test.SanityStall"), StallSeconds,
			TEXT("Seconds (default 0.5; 0 = off) a melee may stand still out of reach of its target while a place is free round that target: longer, hol.Test.Sanity says FAIL (the user's retour on the neutrals trial, D641)"));
		/** hol.Test.SanityStallFinalLeg <Seconds>: Q-melee-stall = A, the margin added to the wait of a FinalLeg walk (FinalLegStuckSteps) before check 4 judges (-1: no tolerance). */
		float StallFinalLegMargin = 0.2f;
		FAutoConsoleVariableRef CVarStallFinalLegMargin(TEXT("hol.Test.SanityStallFinalLeg"), StallFinalLegMargin,
			TEXT("Seconds (default 0.2; -1 = off, the limit of hol.Test.SanityStall always) added to FinalLegStuckSteps (2 s): a soldier on the FinalLeg of a walk held by its friends may stand that long before check 4 judges it, the walk ends by itself and it fights (Q-melee-stall = A)"));
		/** hol.Test.SanityNoStrike <Seconds>: check 5, a melee in reach of its living target landing no blow this long fails (0: off). */
		float NoStrikeSeconds = 3.f;
		FAutoConsoleVariableRef CVarNoStrikeSeconds(TEXT("hol.Test.SanityNoStrike"), NoStrikeSeconds,
			TEXT("Seconds (default 3; 0 = off) a melee soldier may stay in reach of its living target without one blow of its own landing (idle in front of it, or the start of the blow begun again): longer, hol.Test.Sanity says FAIL (BUG-melee-stuck-attack, L8-2)"));
		/** hol.Test.SanityPlaceWalk: Q-sanity-circling (Q-sanity-circling = C, default 1): the walk round ONE side to the free place a soldier claimed round its target (D641) is tolerated by the spin rules; a turn against the one before is not (see FTrace::PlaceWalk). */
		int32 PlaceWalkIsManoeuvre = 1;
		FAutoConsoleVariableRef CVarPlaceWalkIsManoeuvre(TEXT("hol.Test.SanityPlaceWalk"), PlaceWalkIsManoeuvre,
			TEXT("1 (default, Q-sanity-circling = C): a soldier walking to the free place it claimed round its target may go round ONE side (turns in one direction, each under a right angle, are not summed); a turn back on its steps is still a fault. 0: every step is judged"));
		/** hol.Sanity.DumpUnit <UnitId>: the log line of that unit at every step (position, velocity, order), -1 none. */
		int32 DumpUnit = -1;
		FAutoConsoleVariableRef CVarDumpUnit(TEXT("hol.Sanity.DumpUnit"), DumpUnit, TEXT("Log the position, velocity and order of this unit at every watched step (-1: none)."));

		void Sanity(const TArray<FString>& Args);

		UWorld* FindGameWorld()
		{
			UWorld* GameWorld = nullptr;
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.World() && Context.World()->IsGameWorld() && Context.World()->GetNetMode() != NM_Client)
				{
					GameWorld = Context.World();
				}
			}
			return GameWorld;
		}

		void OnStepEnded(const int64 StepIndex)
		{
			UWorld* World = Watch.World.Get();
			UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
			UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
			UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
			if (!Units || !Simulation || !EntitySubsystem)
			{
				return;
			}
			FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
			const double Time = static_cast<double>(StepIndex) * Simulation->GetStepSeconds();
			++Watch.Steps;
			const int32 WindowSteps = FMath::Max(3, FMath::RoundToInt32(8.f / FMath::Max(static_cast<float>(Simulation->GetStepSeconds()), 0.001f)));
			const int32 JudgeEvery = FMath::Max(1, FMath::RoundToInt32(1.f / FMath::Max(static_cast<float>(Simulation->GetStepSeconds()), 0.001f)));
			TArray<uint32> UnitIds;
			Units->GetAllUnitIds(UnitIds);
			// Check 4's bodies: every living unit's place and width this step (and whether it shoots, a shooter stands off on purpose).
			TMap<uint32, TPair<HOLSanity::FBody, bool>> StallBodies;
			if (StallSeconds > 0.f || NoStrikeSeconds > 0.f)
			{
				for (const uint32 BodyId : UnitIds)
				{
					const FMassEntityHandle BodyEntity = Units->FindEntity(BodyId);
					const FTransformFragment* BodyTransform = EntityManager.IsEntityValid(BodyEntity) ? EntityManager.GetFragmentDataPtr<FTransformFragment>(BodyEntity) : nullptr;
					const FHOLUnitStateFragment* BodyState = BodyTransform ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(BodyEntity) : nullptr;
					FHOLUnitView BodyView;
					if (BodyState && BodyState->Activity != EHOLUnitActivity::Dying && Units->GetUnitView(BodyId, BodyView))
					{
						const FVector BodyWhere = BodyTransform->GetTransform().GetLocation();
						StallBodies.Add(BodyId, TPair<HOLSanity::FBody, bool>(HOLSanity::FBody{ FVector2D(BodyWhere.X, BodyWhere.Y), BodyView.Radius }, BodyView.bRanged));
					}
				}
			}
			for (const uint32 UnitId : UnitIds)
			{
				const FMassEntityHandle Entity = Units->FindEntity(UnitId);
				if (!EntityManager.IsEntityValid(Entity))
				{
					continue;
				}
				FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
				const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity);
				if (!Transform || !State || State->Activity == EHOLUnitActivity::Dying)
				{
					continue;
				}
				const HOLSanity::EUnitMotion Motion = IsWalkingActivity(State->Activity) ? HOLSanity::EUnitMotion::Ordered
					: State->Activity == EHOLUnitActivity::Idle ? HOLSanity::EUnitMotion::Idle : HOLSanity::EUnitMotion::Busy;
				if (Watch.Shake != 0. && Motion == HOLSanity::EUnitMotion::Idle)
				{
					FVector Location = Transform->GetTransform().GetLocation();
					Location.X += (StepIndex & 1) ? Watch.Shake : -Watch.Shake;
					Transform->GetMutableTransform().SetLocation(Location);
				}
				const FVector Where = Transform->GetTransform().GetLocation();
				if (DumpUnit == -3 && UnitId <= 16)
				{
					// hol.Sanity.DumpUnit -3 (D641, the loops at the melee's front): units 1 to 16, one line each: position, velocity, its slot (the
					// walk's goal), its combat (approaching, attacking, target and where it is, slide way), the bodies within 2.5 m as
					// #id(owner)(x,y) v(vx,vy), so that the path of a loop can be read against its neighbours step by step.
					const FMassVelocityFragment* Velocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity);
					const FHOLMoveOrderFragment* DumpOrder = EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity);
					const FHOLCombatFragment* DumpCombat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity);
					int32 TargetId = -1;
					FVector TargetWhere = FVector::ZeroVector;
					if (DumpCombat && DumpCombat->Target.IsSet() && EntityManager.IsEntityValid(DumpCombat->Target))
					{
						if (const FTransformFragment* TargetTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(DumpCombat->Target))
						{
							TargetWhere = TargetTransform->GetTransform().GetLocation();
						}
						if (const FHOLUnitStateFragment* TargetState = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(DumpCombat->Target))
						{
							TargetId = static_cast<int32>(TargetState->UnitId);
						}
					}
					FString Near;
					for (const uint32 OtherId : UnitIds)
					{
						const FMassEntityHandle OtherEntity = Units->FindEntity(OtherId);
						const FTransformFragment* OtherTransform = OtherId != UnitId && EntityManager.IsEntityValid(OtherEntity) ? EntityManager.GetFragmentDataPtr<FTransformFragment>(OtherEntity) : nullptr;
						const FHOLUnitStateFragment* OtherState = OtherTransform ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(OtherEntity) : nullptr;
						if (OtherState && OtherState->Activity != EHOLUnitActivity::Dying && FVector::Dist2D(OtherTransform->GetTransform().GetLocation(), Where) < 250.)
						{
							const FMassVelocityFragment* OtherVelocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(OtherEntity);
							Near += FString::Printf(TEXT(" #%u(%d)(%.0f,%.0f)v(%.0f,%.0f)"), OtherId, OtherState->OwnerPlayer, OtherTransform->GetTransform().GetLocation().X, OtherTransform->GetTransform().GetLocation().Y,
								OtherVelocity ? OtherVelocity->Value.X : 0., OtherVelocity ? OtherVelocity->Value.Y : 0.);
						}
					}
					UE_LOG(LogHOLTest, Display, TEXT("SanityDump3 #%u step %lld at (%.0f, %.0f) vel (%.0f, %.0f) ph %d slot (%.0f, %.0f) ap%d at%d tgt #%d (%.0f, %.0f) slide (%.2f, %.2f) flank %d place %d/%.2f np %.1f room %d loop %.1f near:%s"), UnitId, StepIndex, Where.X, Where.Y,
						Velocity ? Velocity->Value.X : 0., Velocity ? Velocity->Value.Y : 0., DumpOrder ? static_cast<int32>(DumpOrder->Phase) : -1, DumpOrder ? DumpOrder->Slot.X : 0., DumpOrder ? DumpOrder->Slot.Y : 0.,
						DumpCombat && DumpCombat->bApproaching ? 1 : 0, DumpCombat && DumpCombat->bAttacking ? 1 : 0, TargetId, TargetWhere.X, TargetWhere.Y,
						DumpCombat ? DumpCombat->SlideWay.X : 0.f, DumpCombat ? DumpCombat->SlideWay.Y : 0.f, DumpCombat && DumpCombat->bFlanking ? 1 : 0,
							DumpCombat && DumpCombat->bHasPlace ? 1 : 0, DumpCombat ? DumpCombat->PlaceAngle : 0.f, DumpCombat ? DumpCombat->NoProgressSeconds : 0.f, DumpCombat && DumpCombat->bRoomAtTarget ? 1 : 0,
							DumpCombat ? DumpCombat->LoopWaitSeconds : 0.f, *Near);
				}
				if (DumpUnit == -2 && UnitId >= 8 && UnitId <= 13)
				{
					// hol.Sanity.DumpUnit -2: units 8 to 13 on one compact line each (position, velocity, the slot and the march slot, the nearest bodies).
					const FMassVelocityFragment* Velocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity);
					const FHOLMoveOrderFragment* DumpOrder = EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity);
					FString Near;
					for (const uint32 OtherId : UnitIds)
					{
						const FMassEntityHandle OtherEntity = Units->FindEntity(OtherId);
						const FTransformFragment* OtherTransform = OtherId != UnitId && EntityManager.IsEntityValid(OtherEntity) ? EntityManager.GetFragmentDataPtr<FTransformFragment>(OtherEntity) : nullptr;
						if (OtherTransform && FVector::Dist2D(OtherTransform->GetTransform().GetLocation(), Where) < 200.)
						{
							Near += FString::Printf(TEXT(" #%u d%.0f"), OtherId, FVector::Dist2D(OtherTransform->GetTransform().GetLocation(), Where));
						}
					}
					UE_LOG(LogHOLTest, Display, TEXT("SanityDump2 #%u step %lld at (%.1f, %.1f) vel (%.0f, %.0f) phase %d slot (%.0f, %.0f) march (%.0f, %.0f) hold %d near:%s"), UnitId, StepIndex, Where.X, Where.Y,
						Velocity ? Velocity->Value.X : 0., Velocity ? Velocity->Value.Y : 0., DumpOrder ? static_cast<int32>(DumpOrder->Phase) : -1, DumpOrder ? DumpOrder->Slot.X : 0., DumpOrder ? DumpOrder->Slot.Y : 0.,
						DumpOrder ? DumpOrder->MarchSlot.X : 0., DumpOrder ? DumpOrder->MarchSlot.Y : 0., DumpOrder ? static_cast<int32>(DumpOrder->ObstacleHold) : 0, *Near);
				}
				if (static_cast<int32>(UnitId) == DumpUnit)
				{
					const FMassVelocityFragment* Velocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity);
					const FHOLMoveOrderFragment* DumpOrder = EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity);
					UE_LOG(LogHOLTest, Display, TEXT("SanityDump #%u step %lld at (%.1f, %.1f) vel (%.0f, %.0f) yaw %.1f phase %d slot (%.0f, %.0f) march (%.0f, %.0f) marchspeed %.0f"),
						UnitId, StepIndex, Where.X, Where.Y, Velocity ? Velocity->Value.X : 0., Velocity ? Velocity->Value.Y : 0., Transform->GetTransform().Rotator().Yaw,
						DumpOrder ? static_cast<int32>(DumpOrder->Phase) : -1, DumpOrder ? DumpOrder->Slot.X : 0., DumpOrder ? DumpOrder->Slot.Y : 0.,
						DumpOrder ? DumpOrder->MarchSlot.X : 0., DumpOrder ? DumpOrder->MarchSlot.Y : 0., DumpOrder ? DumpOrder->MarchSpeed : 0.);
						if (const FHOLCombatFragment* DumpCombat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity))
						{
							FVector TargetWhere = FVector::ZeroVector;
							int32 TargetId = -1;
							if (DumpCombat->Target.IsSet() && EntityManager.IsEntityValid(DumpCombat->Target))
							{
								if (const FTransformFragment* TargetTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(DumpCombat->Target))
								{
									TargetWhere = TargetTransform->GetTransform().GetLocation();
								}
								if (const FHOLUnitStateFragment* TargetState = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(DumpCombat->Target))
								{
									TargetId = static_cast<int32>(TargetState->UnitId);
								}
							}
							UE_LOG(LogHOLTest, Display, TEXT("SanityDump #%u target #%d at (%.0f, %.0f)"), UnitId, TargetId, TargetWhere.X, TargetWhere.Y);
							if (StepIndex % 4 == 0)
							{
								FString Around;
								for (const uint32 OtherId : UnitIds)
								{
									const FMassEntityHandle OtherEntity = Units->FindEntity(OtherId);
									const FTransformFragment* OtherTransform = OtherId != UnitId && EntityManager.IsEntityValid(OtherEntity) ? EntityManager.GetFragmentDataPtr<FTransformFragment>(OtherEntity) : nullptr;
									const FHOLUnitStateFragment* OtherState = OtherTransform ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(OtherEntity) : nullptr;
									if (OtherState && OtherState->Activity != EHOLUnitActivity::Dying && FVector::Dist2D(OtherTransform->GetTransform().GetLocation(), Where) < 500.)
									{
										Around += FString::Printf(TEXT(" #%u(p%d)(%.0f,%.0f)a%d"), OtherId, OtherState->OwnerPlayer, OtherTransform->GetTransform().GetLocation().X, OtherTransform->GetTransform().GetLocation().Y, static_cast<int32>(OtherState->Activity));
									}
								}
								UE_LOG(LogHOLTest, Display, TEXT("SanityDump #%u around:%s"), UnitId, *Around);
							}
							UE_LOG(LogHOLTest, Display, TEXT("SanityDump #%u combat mode %d approaching %d attacking %d target %d detour %d approach (%.0f, %.0f) goal (%.0f, %.0f) slot dist %.0f flank %d"),
								UnitId, static_cast<int32>(DumpCombat->Mode), DumpCombat->bApproaching ? 1 : 0, DumpCombat->bAttacking ? 1 : 0, DumpCombat->Target.IsSet() ? 1 : 0,
								DumpCombat->bAttackMoveDetour ? 1 : 0, DumpCombat->ApproachPoint.X, DumpCombat->ApproachPoint.Y, DumpCombat->AttackMoveGoal.X, DumpCombat->AttackMoveGoal.Y,
								DumpOrder ? FVector2D::Distance(FVector2D(Where), FVector2D(DumpOrder->Slot)) : -1., DumpCombat->bFlanking ? 1 : 0);
						}
				}
				HOLSanity::FUnitTrack& Track = Watch.Tracks.FindOrAdd(UnitId);
				Track.Add(Time, FVector2D(Where.X, Where.Y), Motion, Watch.Rules);
				if (!Watch.Faults.Contains(UnitId))
				{
					const FString Fault = Track.Judge(Watch.Rules);
					if (!Fault.IsEmpty())
					{
						Watch.Faults.Add(UnitId, FString::Printf(TEXT("unit #%u (slot %d) at (%.0f, %.0f) %s"), UnitId, State->OwnerPlayer, Where.X, Where.Y, *Fault));
					}
				}
				// The whole-window rules over the last 8 s, judged once a second: a spin met in part of the window, a heading that
				// wobbles at each step (the units round the hero in duel_challenge). "Ordered" as the combat sees it (move order,
				// approach, target), the way these rules were tuned.
				const FHOLMoveOrderFragment* Order = EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity);
				const FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity);
				const bool bOrdered = (Order && Order->Phase != EHOLMovePhase::None) || (Combat && (Combat->bApproaching || Combat->Target.IsSet()));
				HOLSanity::FTrace& Window = Watch.Windows.FindOrAdd(UnitId);
				Window.UnitId = UnitId;
				Window.Places.Add(FVector2D(Where.X, Where.Y));
				Window.Ordered.Add(bOrdered ? 1 : 0);
				Window.Flanking.Add(Combat && Combat->bFlanking ? 1 : 0);
					// Q-sanity-circling = C: the walk round one side to the claimed place is tolerated, a turn back on its steps is not (HOLSanity::FTrace::PlaceWalk).
					Window.PlaceWalk.Add(Combat && PlaceWalkIsManoeuvre != 0 && Combat->bHasPlace && Combat->bApproaching ? 1 : 0);
				{
					const FMassVelocityFragment* StepVelocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity);
					int32 TargetUnit = -1;
					if (Combat && Combat->Target.IsSet() && EntityManager.IsEntityValid(Combat->Target))
					{
						if (const FHOLUnitStateFragment* TargetState = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Combat->Target))
						{
							TargetUnit = static_cast<int32>(TargetState->UnitId);
						}
					}
					Window.States.Add(FString::Printf(TEXT("%d:ap%d at%d t%d ph%d v%.0f,%.0f"), Combat ? static_cast<int32>(Combat->Mode) : -1, Combat && Combat->bApproaching ? 1 : 0, Combat && Combat->bAttacking ? 1 : 0,
						TargetUnit, Order ? static_cast<int32>(Order->Phase) : -1, StepVelocity ? StepVelocity->Value.X : 0., StepVelocity ? StepVelocity->Value.Y : 0.));
				}
				if (StallSeconds > 0.f)
				{
					// Check 4: a melee standing still (under 0.3 cm this step) more than 40 cm short of its target's edge, with a free place round it.
					bool bStalledWithRoom = false;
					FString StallText;
					const FHOLUnitStateFragment* TargetState = Combat && Combat->Target.IsSet() && EntityManager.IsEntityValid(Combat->Target) && !Combat->bAttacking && Combat->Mode != EHOLCombatMode::Hold && Window.Places.Num() >= 2
						? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Combat->Target) : nullptr;
					const TPair<HOLSanity::FBody, bool>* SelfBody = TargetState ? StallBodies.Find(UnitId) : nullptr;
					const TPair<HOLSanity::FBody, bool>* TargetBody = TargetState ? StallBodies.Find(TargetState->UnitId) : nullptr;
					if (SelfBody && TargetBody && !SelfBody->Value)
					{
						const bool bStill = FVector2D::Distance(Window.Places[Window.Places.Num() - 2], FVector2D(Where.X, Where.Y)) < 0.3;
						const double Gap = FVector2D::Distance(SelfBody->Key.Where, TargetBody->Key.Where) - SelfBody->Key.Radius - TargetBody->Key.Radius;
						if (bStill && Gap > 40.)
						{
							TArray<HOLSanity::FBody> Others;
							for (const TPair<uint32, TPair<HOLSanity::FBody, bool>>& Body : StallBodies)
							{
								if (Body.Key != UnitId && Body.Key != TargetState->UnitId)
								{
									Others.Add(Body.Value.Key);
								}
							}
							bStalledWithRoom = HOLSanity::HasFreePlaceAround(TargetBody->Key.Where, TargetBody->Key.Radius, SelfBody->Key.Radius, Others)
									// Q-neutrals-hold = C: a place that bodies close on both sides keep out of reach is not room to strike.
									&& !HOLSanity::IsBoxedIn(SelfBody->Key.Where, SelfBody->Key.Radius, TargetBody->Key.Where, Others);
							StallText = FString::Printf(TEXT("out of reach of #%u at (%.0f, %.0f), %.0f cm short of its edge, with a free place round it (mode %d, approaching %d, flanking %d, flank wait %.1f s, loop wait %.1f s, phase %d, claimed place %d, no headway %.1f s)"),
								TargetState->UnitId, TargetBody->Key.Where.X, TargetBody->Key.Where.Y, Gap, static_cast<int32>(Combat->Mode), Combat->bApproaching ? 1 : 0, Combat->bFlanking ? 1 : 0,
								Combat->FlankWaitSeconds, Combat->LoopWaitSeconds, Order ? static_cast<int32>(Order->Phase) : -1, Combat->bHasPlace ? 1 : 0, Combat->NoProgressSeconds);
						}
					}
					HOLSanity::FStall& Stall = Watch.Stalls.FindOrAdd(UnitId);
					// Q-melee-stall = A: the wait at the end of a FinalLeg walk (phase 3) is the game's own (FinalLegStuckSteps); the limit follows it.
					// The whole run keeps the tolerance once it began on a FinalLeg walk: the step the walk ends is still a stand-still, the soldier strikes after.
					if (!bStalledWithRoom)
					{
						Stall.bFinalLegRun = false;
					}
					else if (Order && Order->Phase == EHOLMovePhase::FinalLeg)
					{
						Stall.bFinalLegRun = true;
					}
					const double StallLimit = HOLSanity::StallLimitSeconds(StallSeconds, Stall.bFinalLegRun,
						GetDefault<UHOLSimulationSettings>()->FinalLegStuckSteps * static_cast<double>(Simulation->GetStepSeconds()), StallFinalLegMargin);
					if (Stall.Step(bStalledWithRoom, Simulation->GetStepSeconds(), StallLimit))
					{
						const FString StallLine = FString::Printf(TEXT("unit #%u (slot %d) at (%.0f, %.0f) stalls over %.1f s %s"), UnitId, State->OwnerPlayer, Where.X, Where.Y, StallLimit, *StallText);
						UE_LOG(LogHOLTest, Display, TEXT("SanityStall step %lld: %s"), Watch.Steps, *StallLine);
						if (!Watch.StallFaults.Contains(UnitId))
						{
							Watch.StallFaults.Add(UnitId, StallLine);
						}
					}
				}
				if (NoStrikeSeconds > 0.f)
				{
					// Check 5: a melee within 40 cm (edge to edge) of its living target. Its own count of blows (Hits) must go up within NoStrikeSeconds:
					// the target's health falls with it, so the damage is the effect read. A shooter stands off on purpose and is not judged.
					bool bInReach = false;
					FString NoStrikeText;
					const FHOLUnitStateFragment* AimState = Combat && Combat->Target.IsSet() && EntityManager.IsEntityValid(Combat->Target)
						? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Combat->Target) : nullptr;
					const TPair<HOLSanity::FBody, bool>* SelfBodyN = AimState ? StallBodies.Find(UnitId) : nullptr;
					const TPair<HOLSanity::FBody, bool>* AimBody = AimState ? StallBodies.Find(AimState->UnitId) : nullptr;
					if (SelfBodyN && AimBody && !SelfBodyN->Value && AimState->CurrentHealth > 0.f && AimState->Activity != EHOLUnitActivity::Dying)
					{
						const double AimGap = FVector2D::Distance(SelfBodyN->Key.Where, AimBody->Key.Where) - SelfBodyN->Key.Radius - AimBody->Key.Radius;
						bInReach = AimGap <= 40.;
						NoStrikeText = FString::Printf(TEXT("in reach of #%u (health %.0f), %.0f cm from its edge, no blow struck (blows %d, mode %d, attacking %d, approaching %d, flanking %d, phase %d, claimed place %d, ordered target %d, no headway %.1f s)"),
							AimState->UnitId, static_cast<double>(AimState->CurrentHealth), AimGap, Combat->Hits, static_cast<int32>(Combat->Mode), Combat->bAttacking ? 1 : 0, Combat->bApproaching ? 1 : 0,
							Combat->bFlanking ? 1 : 0, Order ? static_cast<int32>(Order->Phase) : -1, Combat->bHasPlace ? 1 : 0, Combat->bOrderedTarget ? 1 : 0, Combat->NoProgressSeconds);
					}
					HOLSanity::FNoStrike& NoStrike = Watch.NoStrikes.FindOrAdd(UnitId);
					if (NoStrike.Step(bInReach, Combat ? Combat->Hits : 0, Simulation->GetStepSeconds(), NoStrikeSeconds))
					{
						const FString NoStrikeLine = FString::Printf(TEXT("unit #%u (slot %d) at (%.0f, %.0f) strikes nothing over %.1f s %s"), UnitId, State->OwnerPlayer, Where.X, Where.Y, NoStrikeSeconds, *NoStrikeText);
						UE_LOG(LogHOLTest, Display, TEXT("SanityNoStrike step %lld: %s"), Watch.Steps, *NoStrikeLine);
						if (!Watch.NoStrikeFaults.Contains(UnitId))
						{
							Watch.NoStrikeFaults.Add(UnitId, NoStrikeLine);
						}
					}
				}
				if (Window.Places.Num() > WindowSteps)
				{
					Window.Places.RemoveAt(0, Window.Places.Num() - WindowSteps, EAllowShrinking::No);
					Window.Ordered.RemoveAt(0, Window.Ordered.Num() - WindowSteps, EAllowShrinking::No);
					Window.Flanking.RemoveAt(0, Window.Flanking.Num() - WindowSteps, EAllowShrinking::No);
						Window.PlaceWalk.RemoveAt(0, Window.PlaceWalk.Num() - WindowSteps, EAllowShrinking::No);
					Window.States.RemoveAt(0, Window.States.Num() - WindowSteps, EAllowShrinking::No);
				}
				if (Window.Places.Num() == WindowSteps && Watch.Steps % JudgeEvery == 0 && !Watch.Faults.Contains(UnitId))
				{
					FHOLUnitView View;
					Window.Radius = Units->GetUnitView(UnitId, View) ? View.Radius : 60.f;
					const HOLSanity::FResult Result = HOLSanity::Analyse(Window, static_cast<float>(Simulation->GetStepSeconds()));
					if (Result.bSpin || Result.bShake)
					{
						// The window's places, every 4th step, so that the fault can be read without a dump (hol.Sanity.DumpUnit).
						FString Trail;
						for (int32 Index = 0; Index < Window.Places.Num(); Index += 1)
						{
							Trail += FString::Printf(TEXT(" (%.0f,%.0f)"), Window.Places[Index].X, Window.Places[Index].Y);
						}
						UE_LOG(LogHOLTest, Display, TEXT("SanityTrail #%u at step %lld, every step:%s"), UnitId, Watch.Steps, *Trail);
						for (int32 Index = 0; Index < Window.States.Num(); ++Index)
						{
							UE_LOG(LogHOLTest, Display, TEXT("SanityState #%u [%d] (%.0f,%.0f) %s"), UnitId, Index, Window.Places[Index].X, Window.Places[Index].Y, *Window.States[Index]);
						}
					}
					if (Result.bSpin || Result.bShake)
					{
						Watch.Faults.Add(UnitId, FString::Printf(TEXT("unit #%u (slot %d) at (%.0f, %.0f) %s over 8 s: path %.0f cm, net %.0f cm, turn %.1f rad (net %.1f, to and fro %.1f), %d reversals, ordered %.0f%%"),
							UnitId, State->OwnerPlayer, Where.X, Where.Y, Result.bSpin ? TEXT("spins") : TEXT("shakes"), Result.Path, Result.Net, Result.Turn, Result.SignedTurn, Result.Zigzag, Result.Reversals, Result.OrderedShare * 100.f));
					}
				}
			}
			if (Watch.VerdictAtSteps > 0 && Watch.Steps >= Watch.VerdictAtSteps)
			{
				Watch.VerdictAtSteps = 0;
				Sanity(TArray<FString>());
			}
		}

		void StopWatching()
		{
			if (UWorld* World = Watch.World.Get())
			{
				if (UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>())
				{
					Simulation->GetOnStepEnded().Remove(Watch.Handle);
				}
			}
			Watch.Handle.Reset();
			Watch.World.Reset();
			Watch.Tracks.Reset();
			Watch.Faults.Reset();
			Watch.Windows.Reset();
			Watch.Stalls.Reset();
			Watch.StallFaults.Reset();
			Watch.NoStrikes.Reset();
			Watch.NoStrikeFaults.Reset();
			Watch.VerdictAtSteps = 0;
			Watch.Steps = 0;
		}

		void SetWatch(const TArray<FString>& Args)
		{
			const bool bOn = Args.Num() < 1 || FCString::Atoi(*Args[0]) != 0;
			StopWatching();
			if (!bOn)
			{
				UE_LOG(LogHOLTest, Display, TEXT("Sanity watch: off"));
				return;
			}
			UWorld* World = FindGameWorld();
			UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
			if (!Simulation)
			{
				UE_LOG(LogHOLTest, Display, TEXT("Sanity watch: no game world with a simulation"));
				return;
			}
			Watch.World = World;
			Watch.Handle = Simulation->GetOnStepEnded().AddStatic(&OnStepEnded);
			UE_LOG(LogHOLTest, Display, TEXT("Sanity watch: on"));
		}

		/**
		 * The colour each slot chose, as this machine knows it (the slot's own index until its buildings or its player state say
		 * otherwise): the data the drawing processor gathers each frame (UHOLInterpolatedInstancesProcessor's FSlotColors). The RULE that
		 * turns an owner into a colour is not here: HOLSanity::FindColourClashes calls HOLPlayerColors::GetOwnerLinearColor, the same
		 * function as the processor's (SAN-Q1 = A, D616), so the neutral grey cannot be forgotten by one side only.
		 */
		TArray<uint8> GatherSlotColors(const UWorld& World)
		{
			TArray<uint8> Colors;
			for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
			{
				Colors.Add(static_cast<uint8>(Slot));
			}
			for (FHOLBuildingIterator It(&World); It; ++It)
			{
				if (It->GetOwnerPlayer() < FHOLMatchConfig::MaxSlots)
				{
					Colors[It->GetOwnerPlayer()] = static_cast<uint8>(It->GetColorIndex());
				}
			}
			if (const AGameStateBase* GameState = World.GetGameState())
			{
				for (const APlayerState* PlayerState : GameState->PlayerArray)
				{
					const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
					if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() >= 0 && HOLPlayerState->GetPlayerIndex() < FHOLMatchConfig::MaxSlots)
					{
						Colors[HOLPlayerState->GetPlayerIndex()] = static_cast<uint8>(HOLPlayerState->GetColorIndex());
					}
				}
			}
			return Colors;
		}

		void Sanity(const TArray<FString>& Args)
		{
			UWorld* World = FindGameWorld();
			TArray<FString> Failures;
			if (!World)
			{
				Failures.Add(TEXT("no game world"));
			}
			else
			{
				// 1. The neutrals' colour against the colour of every player of the game.
				TArray<FHOLUnitView> Views;
				HOLUnitView::Gather(*World, Views);
				TSet<int32> PlayerSlots;
				TSet<int32> Neutrals;
				for (const FHOLUnitView& View : Views)
				{
					(FHOLMatchConfig::IsNeutralOwner(View.OwnerPlayer) ? Neutrals : PlayerSlots).Add(View.OwnerPlayer);
				}
				for (FHOLBuildingIterator It(World); It; ++It)
				{
					if (It->GetOwnerPlayer() < FHOLMatchConfig::MaxSlots)
					{
						PlayerSlots.Add(It->GetOwnerPlayer());
					}
				}
				if (const AGameStateBase* GameState = World->GetGameState())
				{
					for (const APlayerState* PlayerState : GameState->PlayerArray)
					{
						if (const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState))
						{
							PlayerSlots.Add(HOLPlayerState->GetPlayerIndex());
						}
					}
				}
				Failures.Append(HOLSanity::FindColourClashes(Neutrals.Array(), PlayerSlots.Array(), GatherSlotColors(*World)));
				// 2 and 3. The units' steps.
				if (Watch.World.Get() != World || Watch.Steps == 0)
				{
					Failures.Add(TEXT("no unit steps recorded: hol.Sanity.Watch 1 must come first in the scenario (spinning and trembling not checked)"));
				}
				for (const TPair<uint32, FString>& Fault : Watch.Faults)
				{
					Failures.Add(Fault.Value);
				}
				for (const TPair<uint32, FString>& Fault : Watch.StallFaults)
				{
					Failures.Add(Fault.Value);
				}
				for (const TPair<uint32, FString>& Fault : Watch.NoStrikeFaults)
				{
					Failures.Add(Fault.Value);
				}
				int32 NoStrikeRuns = 0;
				double NoStrikeLongest = 0.;
				for (const TPair<uint32, HOLSanity::FNoStrike>& NoStrike : Watch.NoStrikes)
				{
					NoStrikeRuns += NoStrike.Value.Runs;
					NoStrikeLongest = FMath::Max(NoStrikeLongest, NoStrike.Value.Longest);
				}
				UE_LOG(LogHOLTest, Display, TEXT("Sanity no-strike: %d runs over %.1f s, longest %.1f s (limit hol.Test.SanityNoStrike)"), NoStrikeRuns, NoStrikeSeconds, NoStrikeLongest);
				int32 StallRuns = 0;
				double StallLongest = 0.;
				for (const TPair<uint32, HOLSanity::FStall>& Stall : Watch.Stalls)
				{
					StallRuns += Stall.Value.Runs;
					StallLongest = FMath::Max(StallLongest, Stall.Value.Longest);
				}
				UE_LOG(LogHOLTest, Display, TEXT("Sanity stalls: %d runs over %.1f s, longest %.1f s (limit hol.Test.SanityStall)"), StallRuns, StallSeconds, StallLongest);
			}
			if (Failures.IsEmpty())
			{
				UE_LOG(LogHOLTest, Display, TEXT("Sanity: OK (%lld steps, %d units followed)"), Watch.Steps, Watch.Tracks.Num());
			}
			for (const FString& Failure : Failures)
			{
				UE_LOG(LogHOLTest, Display, TEXT("Sanity: FAIL %s"), *Failure);
			}
		}

		FAutoConsoleCommand SanityCommand(TEXT("hol.Test.Sanity"),
			TEXT("hol.Test.Sanity: logs Sanity: OK or one Sanity: FAIL <reason> per fault (neutral colour = a player's, a unit that spins, a unit that shakes); read by Tools/Scenarios/preflight.ps1"),
			FConsoleCommandWithArgsDelegate::CreateStatic(&Sanity));
		FAutoConsoleCommand SanityWatchCommand(TEXT("hol.Sanity.Watch"),
			TEXT("hol.Sanity.Watch [0|1]: records every unit's steps for hol.Test.Sanity (spinning, trembling); 1 restarts the record"),
			FConsoleCommandWithArgsDelegate::CreateStatic(&SetWatch));
		/** The old name of the motion audit (hero-j8-close's duel_challenge scenarios): starts the watch, gives the verdict of hol.Test.Sanity Seconds later. */
		FAutoConsoleCommand SanityMotionCommand(TEXT("hol.Test.SanityMotion"),
			TEXT("hol.Test.SanityMotion [Seconds=8]: alias of hol.Sanity.Watch 1, then hol.Test.Sanity once Seconds of game time have been watched"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
			{
				SetWatch(TArray<FString>());
				UWorld* World = Watch.World.Get();
				UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
				if (Simulation)
				{
					const double Seconds = Args.Num() >= 1 ? FCString::Atod(*Args[0]) : 8.;
					Watch.VerdictAtSteps = FMath::Max<int64>(3, FMath::RoundToInt64(Seconds / FMath::Max(static_cast<double>(Simulation->GetStepSeconds()), 0.001)));
				}
			}));
		FAutoConsoleCommand SanityShakeCommand(TEXT("hol.Sanity.Shake"),
			TEXT("hol.Sanity.Shake <cm>: test fixture, pushes every idle unit +-cm at each step of the watch (0: off)"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) { Watch.Shake = Args.Num() > 0 ? FCString::Atod(*Args[0]) : 0.; }));
	}
}
