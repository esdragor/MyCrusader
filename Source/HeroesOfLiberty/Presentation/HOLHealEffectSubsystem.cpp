// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLHealEffectSubsystem.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Simulation/HOLHealProcessor.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

namespace UE::HeroesOfLiberty::Heal
{
	/** The heal's colour (a green, read on the grass and on the player colours) and the height of its line (cm, the unit's chest). */
	const FColor HealColour(70, 255, 100);
	constexpr float LineHeight = 110.f;
	constexpr float PlusHeight = 230.f;

	FAutoConsoleCommandWithWorld ReportCommand(TEXT("hol.Heal.Report"),
		TEXT("hol.Heal.Report: logs the heals of the last step, the health points healed since the start and the heals drawn (solo or server)"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			const UHOLHealEffectSubsystem* Effect = World ? World->GetSubsystem<UHOLHealEffectSubsystem>() : nullptr;
			UE_LOG(LogHOLSimulation, Display, TEXT("Heal report: links %d, healed total %.2f, heal steps %lld, drawn %lld"), UHOLHealProcessor::GetLinks().Num(),
				UHOLHealProcessor::GetTotalHealed(), UHOLHealProcessor::GetHealSteps(), Effect ? Effect->GetDrawnCount() : 0LL);
		}));
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
#if ENABLE_DRAW_DEBUG
	const TArray<UHOLHealProcessor::FLink>& Links = UHOLHealProcessor::GetLinks();
	UWorld* World = GetWorld();
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (Links.IsEmpty() || !Units)
	{
		return;
	}
	using namespace UE::HeroesOfLiberty::Heal;
	for (const UHOLHealProcessor::FLink& Link : Links)
	{
		FHOLUnitView Healer;
		FHOLUnitView Target;
		if (!Units->GetUnitView(Link.HealerId, Healer) || !Units->GetUnitView(Link.TargetId, Target))
		{
			continue;
		}
		DrawDebugLine(World, Healer.Location + FVector(0., 0., LineHeight), Target.Location + FVector(0., 0., LineHeight), HealColour, false, -1.f, SDPG_Foreground, 6.f);
		DrawDebugString(World, Target.Location + FVector(0., 0., PlusHeight), TEXT("+"), nullptr, HealColour, 0.f, false, 3.f);
		++Drawn;
	}
#endif
}
