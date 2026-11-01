// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSanitySubsystem.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

namespace { int32 SanityVerbose = 0; FAutoConsoleVariableRef CVarSanityVerbose(TEXT("hol.Test.SanityVerbose"), SanityVerbose, TEXT("1: hol.Test.SanityMotion logs the numbers of every unit")); }

bool UHOLSanitySubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLSanitySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}
	if (UHOLSimulationSubsystem* Simulation = InWorld.GetSubsystem<UHOLSimulationSubsystem>())
	{
		StepEndedHandle = Simulation->GetOnStepEnded().AddUObject(this, &UHOLSanitySubsystem::OnStepEnded);
	}
}

void UHOLSanitySubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		if (UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>())
		{
			Simulation->GetOnStepEnded().Remove(StepEndedHandle);
		}
	}
	StepEndedHandle.Reset();
	Super::Deinitialize();
}

void UHOLSanitySubsystem::StartMotion(const float Seconds)
{
	const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	Traces.Reset();
	StepsToRecord = FMath::Max(3, FMath::RoundToInt32(Seconds / FMath::Max(StepSeconds, KINDA_SMALL_NUMBER)));
	bRecording = true;
	UE_LOG(LogHOLSimulation, Display, TEXT("Sanity: motion audit started for %.1f s (%d steps)"), Seconds, StepsToRecord);
}

void UHOLSanitySubsystem::OnStepEnded(const int64 StepIndex)
{
	if (!bRecording)
	{
		return;
	}
	UWorld* World = GetWorld();
	UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!Units || !EntitySubsystem)
	{
		return;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TArray<uint32> Ids;
	Units->GetAllUnitIds(Ids);
	for (const uint32 Id : Ids)
	{
		const FMassEntityHandle Entity = Units->FindEntity(Id);
		const FTransformFragment* Transform = Entity.IsSet() && EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr;
		const FHOLUnitStateFragment* State = Transform ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		if (!State || State->CurrentHealth <= 0)
		{
			continue;
		}
		const FHOLMoveOrderFragment* Order = EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity);
		const FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity);
		HOLSanity::FTrace& Trace = Traces.FindOrAdd(Id);
		Trace.UnitId = Id;
		Trace.Places.Add(FVector2D(Transform->GetTransform().GetLocation()));
		const bool bOrdered = (Order && Order->Phase != EHOLMovePhase::None) || (Combat && (Combat->bApproaching || Combat->Target.IsSet()));
		Trace.Ordered.Add(bOrdered ? 1 : 0);
		if (SanityVerbose >= 2 && Combat && Order)
		{
			double DT = -1., DA = -1.;
			if (Combat->Target.IsSet() && EntityManager.IsEntityValid(Combat->Target))
			{
				if (const FTransformFragment* TT = EntityManager.GetFragmentDataPtr<FTransformFragment>(Combat->Target))
				{
					DT = FVector2D::Distance(FVector2D(TT->GetTransform().GetLocation()), FVector2D(Transform->GetTransform().GetLocation()));
				}
			}
			DA = FVector2D::Distance(FVector2D(Combat->ApproachPoint), FVector2D(Transform->GetTransform().GetLocation()));
			Trace.States.Add(FString::Printf(TEXT("DT=%.0f DA=%.0f T=%d A=%d M=%d P=%d OT=%d TB=%d Atk=%d Ph=%d"), DT, DA, Combat->Target.IsSet() ? Combat->Target.Index : -1, Combat->bApproaching, static_cast<int32>(Combat->Mode),
				static_cast<int32>(Order->Phase), Combat->bOrderedTarget, !Combat->TargetBuilding.IsExplicitlyNull(), Combat->bAttacking, static_cast<int32>(Order->Phase)));
		}
	}
	if (--StepsToRecord <= 0)
	{
		bRecording = false;
		Judge();
	}
}

int32 UHOLSanitySubsystem::Judge()
{
	UWorld* World = GetWorld();
	UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	int32 Bad = 0;
	int32 Judged = 0;
	for (const TPair<uint32, HOLSanity::FTrace>& Pair : Traces)
	{
		// A unit that lived the whole window only (a trace shorter than the longest is a unit born or killed in it).
		FHOLUnitView View;
		const bool bKnown = Units && Units->GetUnitView(Pair.Key, View);
		HOLSanity::FTrace Trace = Pair.Value;
		Trace.Radius = bKnown ? View.Radius : 60.f;
		const HOLSanity::FResult Result = HOLSanity::Analyse(Trace, StepSeconds);
		++Judged;
		if (SanityVerbose != 0) UE_LOG(LogHOLSimulation, Display, TEXT("Sanity: unit #%u path %.0f net %.0f turn %.1f rad reversals %d ordered %.2f"), Pair.Key, Result.Path, Result.Net, Result.Turn, Result.Reversals, Result.OrderedShare);
		if (SanityVerbose >= 2 && Result.Turn > 20.f)
		{
			FString Dump;
			for (int32 I = 100; I < FMath::Min(Trace.Places.Num(), 130); ++I)
			{
				const FVector2D M = Trace.Places[I] - Trace.Places[I - 1];
				Dump += FString::Printf(TEXT(" (%.1f,%.1f)"), M.X, M.Y);
			}
			for (int32 I = 100; I < FMath::Min(Trace.States.Num(), 160); I += 6)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Sanity: state #%u step %d: %s"), Pair.Key, I, *Trace.States[I]);
			}
			FHOLUnitView V2; if (Units) Units->GetUnitView(Pair.Key, V2);
			UE_LOG(LogHOLSimulation, Display, TEXT("Sanity: moves of #%u %s owner %d at step 110 (%.0f, %.0f):%s"), Pair.Key, *V2.TypeName.ToString(), V2.OwnerPlayer, Trace.Places.IsValidIndex(110) ? Trace.Places[110].X : 0., Trace.Places.IsValidIndex(110) ? Trace.Places[110].Y : 0., *Dump);
		}
		if (Result.bSpin || Result.bShake)
		{
			++Bad;
			UE_LOG(LogHOLSimulation, Display, TEXT("Sanity: FAIL %s unit #%u %s at (%.0f, %.0f): path %.0f cm, net %.0f cm, turn %.1f rad, %d reversals in %.1f s, ordered %.0f%%"),
				Result.bSpin ? TEXT("spin") : TEXT("shake"), Pair.Key, bKnown ? *View.TypeName.ToString() : TEXT("?"), Trace.Places.Last().X, Trace.Places.Last().Y,
				Result.Path, Result.Net, Result.Turn, Result.Reversals, (Trace.Places.Num() - 1) * StepSeconds, Result.OrderedShare * 100.f);
		}
	}
	UE_LOG(LogHOLSimulation, Display, TEXT("Sanity: %s motion (%d units judged, %d spinning or shaking)"), Bad == 0 ? TEXT("OK") : TEXT("FAIL"), Judged, Bad);
	return Bad;
}

namespace
{
	FAutoConsoleCommandWithWorldAndArgs CommandSanityMotion(TEXT("hol.Test.SanityMotion"),
		TEXT("hol.Test.SanityMotion [Seconds=8]: records every unit's place and orders for Seconds, then logs 'Sanity: FAIL spin|shake ...' per unit that spins or shakes and 'Sanity: OK|FAIL motion ...'; solo or server"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLSanitySubsystem* Sanity = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLSanitySubsystem>() : nullptr;
			if (!Sanity)
			{
				UE_LOG(LogHOLSimulation, Warning, TEXT("hol.Test.SanityMotion [Seconds=8], on the server or in solo"));
				return;
			}
			Sanity->StartMotion(Args.Num() >= 1 ? FCString::Atof(*Args[0]) : 8.f);
		}));
}






