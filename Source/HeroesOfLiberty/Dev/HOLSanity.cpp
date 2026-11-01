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
		};
		FSanityWatch Watch;

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
				for (const int32 Neutral : Neutrals)
				{
					const FColor NeutralColor = HOLPlayerColors::GetColor(DisplayColorIndex(*World, Neutral));
					for (const int32 Slot : PlayerSlots)
					{
						if (HOLPlayerColors::GetColor(DisplayColorIndex(*World, Slot)) == NeutralColor)
						{
							Failures.Add(FString::Printf(TEXT("neutral units (owner %d, colour index %d) are drawn in the colour of slot %d (%s)"),
								Neutral, DisplayColorIndex(*World, Neutral), Slot, *HOLPlayerColors::GetColorName(DisplayColorIndex(*World, Slot)).ToString()));
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
		FAutoConsoleCommand SanityShakeCommand(TEXT("hol.Sanity.Shake"),
			TEXT("hol.Sanity.Shake <cm>: test fixture, pushes every idle unit +-cm at each step of the watch (0: off)"),
			FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) { Watch.Shake = Args.Num() > 0 ? FCString::Atod(*Args[0]) : 0.; }));
	}
}
