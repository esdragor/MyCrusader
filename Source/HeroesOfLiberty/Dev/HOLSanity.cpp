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
		};
		FSanityWatch Watch;
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
							UE_LOG(LogHOLTest, Display, TEXT("SanityDump #%u combat mode %d approaching %d attacking %d target %d detour %d approach (%.0f, %.0f) goal (%.0f, %.0f) slot dist %.0f"),
								UnitId, static_cast<int32>(DumpCombat->Mode), DumpCombat->bApproaching ? 1 : 0, DumpCombat->bAttacking ? 1 : 0, DumpCombat->Target.IsSet() ? 1 : 0,
								DumpCombat->bAttackMoveDetour ? 1 : 0, DumpCombat->ApproachPoint.X, DumpCombat->ApproachPoint.Y, DumpCombat->AttackMoveGoal.X, DumpCombat->AttackMoveGoal.Y,
								DumpOrder ? FVector2D::Distance(FVector2D(Where), FVector2D(DumpOrder->Slot)) : -1.);
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
				if (Window.Places.Num() > WindowSteps)
				{
					Window.Places.RemoveAt(0, Window.Places.Num() - WindowSteps, EAllowShrinking::No);
					Window.Ordered.RemoveAt(0, Window.Ordered.Num() - WindowSteps, EAllowShrinking::No);
					Window.Flanking.RemoveAt(0, Window.Flanking.Num() - WindowSteps, EAllowShrinking::No);
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
						for (int32 Index = 0; Index < Window.Places.Num(); Index += 4)
						{
							Trail += FString::Printf(TEXT(" (%.0f,%.0f)"), Window.Places[Index].X, Window.Places[Index].Y);
						}
						UE_LOG(LogHOLTest, Display, TEXT("SanityTrail #%u at step %lld, every 4th step:%s"), UnitId, Watch.Steps, *Trail);
					}
					if (Result.bSpin || Result.bShake)
					{
						Watch.Faults.Add(UnitId, FString::Printf(TEXT("unit #%u (slot %d) at (%.0f, %.0f) %s over 8 s: path %.0f cm, net %.0f cm, turn %.1f rad, %d reversals, ordered %.0f%%"),
							UnitId, State->OwnerPlayer, Where.X, Where.Y, Result.bSpin ? TEXT("spins") : TEXT("shakes"), Result.Path, Result.Net, Result.Turn, Result.Reversals, Result.OrderedShare * 100.f));
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
		 * The colour index a unit of Owner is drawn with. Mirrors UHOLInterpolatedInstancesProcessor's FSlotColors::Get (the slot's
		 * colour from its buildings and its player state, the owner's own number for a slot not known, and for a neutral owner,
		 * which is no slot, the number itself: HOLPlayerColors wraps it). The rule lives in the processor's .cpp, out of reach: when the
		 * display colour of neutrals gets its own function, this one must call it (Auto_Journey_Retours.md).
		 */
		int32 DisplayColorIndex(const UWorld& World, const int32 Owner)
		{
			if (Owner < 0 || Owner >= FHOLMatchConfig::MaxSlots)
			{
				return Owner;
			}
			int32 Index = Owner;
			for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
			{
				if (It->GetOwnerPlayer() == Owner)
				{
					Index = It->GetColorIndex();
				}
			}
			if (const AGameStateBase* GameState = World.GetGameState())
			{
				for (const APlayerState* PlayerState : GameState->PlayerArray)
				{
					const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
					if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() == Owner)
					{
						Index = HOLPlayerState->GetColorIndex();
					}
				}
			}
			return Index;
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
				for (TActorIterator<AHOLBuilding> It(World); It; ++It)
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
				// The colours the renderer uses (6be90ce): HOLPlayerColors::GetOwnerLinearColor over the slots' colour indices; a neutral
				// owner is no slot and gets the neutral grey, so it is compared as the renderer draws it.
				TArray<uint8> SlotColors;
				for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
				{
					SlotColors.Add(static_cast<uint8>(DisplayColorIndex(*World, Slot)));
				}
				for (const int32 Neutral : Neutrals)
				{
					const FVector3f NeutralColor = HOLPlayerColors::GetOwnerLinearColor(Neutral, SlotColors);
					for (const int32 Slot : PlayerSlots)
					{
						if (HOLPlayerColors::GetOwnerLinearColor(Slot, SlotColors) == NeutralColor)
						{
							Failures.Add(FString::Printf(TEXT("neutral units (owner %d) are drawn in the colour of slot %d (%s)"),
								Neutral, Slot, *HOLPlayerColors::GetColorName(DisplayColorIndex(*World, Slot)).ToString()));
						}
					}
				}
				// 2 and 3. The units' steps.
				if (Watch.World.Get() != World || Watch.Steps == 0)
				{
					Failures.Add(TEXT("no unit steps recorded: hol.Sanity.Watch 1 must come first in the scenario (spinning and trembling not checked)"));
				}
				for (const TPair<uint32, FString>& Fault : Watch.Faults)
				{
					Failures.Add(Fault.Value);
				}
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
