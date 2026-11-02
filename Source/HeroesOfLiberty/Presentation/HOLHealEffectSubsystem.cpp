// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLHealEffectSubsystem.h"

#include "DrawDebugHelpers.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Simulation/HOLHealProcessor.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLUnitView.h"

namespace UE::HeroesOfLiberty::Heal
{
	/** The heal's colour (a green, read on the grass and on the player colours) and the height of its line (cm, the unit's chest). */
	const FColor HealColour(70, 255, 100);
	constexpr float LineHeight = 110.f;
	constexpr float PlusHeight = 230.f;
	/** A client's guess: a unit whose health rose within this many seconds is being healed (the wire carries no heal). */
	constexpr double RiseSeconds = 0.6;

	FAutoConsoleCommandWithWorld ReportCommand(TEXT("hol.Heal.Report"),
		TEXT("hol.Heal.Report: logs the heals of the last step, the health points healed since the start and the heals drawn (solo or server)"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			const UHOLHealEffectSubsystem* Effect = World ? World->GetSubsystem<UHOLHealEffectSubsystem>() : nullptr;
			UE_LOG(LogHOLSimulation, Display, TEXT("Heal report: links %d, healed total %.2f, heal steps %lld, drawn %lld"), UHOLHealProcessor::GetLinks().Num(),
				UHOLHealProcessor::GetTotalHealed(), UHOLHealProcessor::GetHealSteps(), Effect ? Effect->GetDrawnCount() : 0LL);
		}));

	void DrawHeal(UWorld* World, const FVector& From, const FVector& To)
	{
#if ENABLE_DRAW_DEBUG
		DrawDebugLine(World, From + FVector(0., 0., LineHeight), To + FVector(0., 0., LineHeight), HealColour, false, -1.f, SDPG_Foreground, 6.f);
		DrawDebugString(World, To + FVector(0., 0., PlusHeight), TEXT("+"), nullptr, HealColour, 0.f, false, 3.f);
#endif
	}
}

bool UHOLHealEffectSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->GetNetMode() != NM_DedicatedServer && !IsRunningDedicatedServer();
}

bool UHOLHealEffectSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UHOLHealEffectSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHOLHealEffectSubsystem, STATGROUP_Tickables);
}

void UHOLHealEffectSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	using namespace UE::HeroesOfLiberty::Heal;
	if (World->GetNetMode() == NM_Client)
	{
		TickClient(*World);
		return;
	}
	const TArray<UHOLHealProcessor::FLink>& Links = UHOLHealProcessor::GetLinks();
	const UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	if (Links.IsEmpty() || !Units)
	{
		return;
	}
	for (const UHOLHealProcessor::FLink& Link : Links)
	{
		FHOLUnitView Healer;
		FHOLUnitView Target;
		if (!Units->GetUnitView(Link.HealerId, Healer) || !Units->GetUnitView(Link.TargetId, Target))
		{
			continue;
		}
		DrawHeal(World, Healer.Location, Target.Location);
		++Drawn;
	}
}

void UHOLHealEffectSubsystem::TickClient(UWorld& World)
{
	using namespace UE::HeroesOfLiberty::Heal;
	// A client does not run the heal: it sees a unit's health rise. A unit whose health rose a moment ago and a healer of its owner within
	// the healer's reach: the line from that healer (the nearest) to it.
	const UDataTable* Table = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (!Table)
	{
		return;
	}
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(World, Views);
	const double Now = World.GetTimeSeconds();
	TArray<TTuple<uint32, int32, bool>> Observed;
	Observed.Reserve(Views.Num());
	for (const FHOLUnitView& View : Views)
	{
		Observed.Add(MakeTuple(View.UnitId, View.Health, View.MaxHealth > 0));
	}
	Rises.Update(Observed, Now, 5.0);
	if (Rises.IsEmpty())
	{
		return;
	}
	for (const FHOLUnitView& Healer : Views)
	{
		const FHOLUnitTypeRow* Row = Table->FindRow<FHOLUnitTypeRow>(Healer.TypeName, TEXT("Heal effect"), false);
		if (!Row || Row->HealRate <= 0.f)
		{
			continue;
		}
		const FHOLUnitView* Best = nullptr;
		double BestDistance = 0.;
		for (const FHOLUnitView& Target : Views)
		{
			if (!Rises.IsRising(Target.UnitId, Now, RiseSeconds) || Target.UnitId == Healer.UnitId || Target.OwnerPlayer != Healer.OwnerPlayer)
			{
				continue;
			}
			const double Distance = FVector::Dist2D(Healer.Location, Target.Location);
			if (Distance <= Row->HealRange && (!Best || Distance < BestDistance))
			{
				Best = &Target;
				BestDistance = Distance;
			}
		}
		if (Best)
		{
			DrawHeal(&World, Healer.Location, Best->Location);
			++Drawn;
		}
	}
}
