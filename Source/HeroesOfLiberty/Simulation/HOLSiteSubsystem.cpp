// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSiteSubsystem.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLFactionRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace
{
	bool bSitesEnabled = false;
	FAutoConsoleVariableRef CVarEnabled(TEXT("hol.Site.Enabled"), bSitesEnabled,
		TEXT("1 to let the sacred sites work (the match option 'Sacred sites' replaces it at S7); 0 by default: the game does not change"));

	float HoldSeconds = HOLSite::DefaultHoldSeconds;
	FAutoConsoleVariableRef CVarHoldSeconds(TEXT("hol.Site.HoldSeconds"), HoldSeconds,
		TEXT("Seconds an adverse side must hold a sacred site's zone alone to take it (provisional 60, like the relay's)"));

	FAutoConsoleCommandWithWorldAndArgs CommandAdd(TEXT("hol.Site.Add"),
		TEXT("hol.Site.Add <X> <Y> [Radius=1200]: a neutral sacred site there, server or solo (development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLSiteSubsystem* Sites = World ? World->GetSubsystem<UHOLSiteSubsystem>() : nullptr;
			const int32 Index = Sites && Args.Num() >= 2
				? Sites->AddSite(FVector2D(FCString::Atod(*Args[0]), FCString::Atod(*Args[1])), Args.Num() >= 3 ? static_cast<float>(FCString::Atod(*Args[2])) : 1200.f)
				: INDEX_NONE;
			if (Index != INDEX_NONE)
			{
				UE_LOG(LogHOLMatch, Display, TEXT("hol.Site.Add: site %d"), Index);
			}
			else
			{
				UE_LOG(LogHOLMatch, Display, TEXT("hol.Site.Add: refused (<X> <Y>, server or solo)"));
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs CommandClear(TEXT("hol.Site.Clear"), TEXT("hol.Site.Clear: no sacred site left (development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UHOLSiteSubsystem* Sites = World ? World->GetSubsystem<UHOLSiteSubsystem>() : nullptr)
			{
				Sites->ClearSites();
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs CommandReport(TEXT("hol.Site.Report"), TEXT("hol.Site.Report: each sacred site's owner, taker and count in the log"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const UHOLSiteSubsystem* Sites = World ? World->GetSubsystem<UHOLSiteSubsystem>() : nullptr;
			if (!Sites)
			{
				return;
			}
			FString Line = FString::Printf(TEXT("Site report: %d site(s)"), Sites->GetSites().Num());
			for (int32 Index = 0; Index < Sites->GetSites().Num(); ++Index)
			{
				const FHOLSiteState& State = Sites->GetSites()[Index].State;
				Line += FString::Printf(TEXT(" | #%d owner %d taker %d held %d"), Index, State.Owner, State.Taker, State.Held);
			}
			UE_LOG(LogHOLMatch, Display, TEXT("%s"), *Line);
		}));
}

bool UHOLSiteSubsystem::IsEnabled()
{
	return bSitesEnabled;
}

bool UHOLSiteSubsystem::IsHolyType(const FName FactionKey, const FName UnitType)
{
	static const FName Monk(TEXT("Monk"));
	static const FName LuminousMonk(TEXT("LuminousMonk"));
	return FactionKey == HOLFactionRules::Dawn && (UnitType == Monk || UnitType == LuminousMonk);
}

bool UHOLSiteSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLSiteSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	BindSteps();
}

void UHOLSiteSubsystem::Deinitialize()
{
	UnbindSteps();
	Super::Deinitialize();
}

bool UHOLSiteSubsystem::BindSteps()
{
	UWorld* World = GetWorld();
	UHOLSimulationSubsystem* Simulation = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (!Simulation)
	{
		return false;
	}
	UnbindSteps();
	StepEndedHandle = Simulation->GetOnStepEnded().AddUObject(this, &UHOLSiteSubsystem::OnStepEnded);
	return true;
}

void UHOLSiteSubsystem::UnbindSteps()
{
	UHOLSimulationSubsystem* Simulation = GetWorld() ? GetWorld()->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	if (Simulation && StepEndedHandle.IsValid())
	{
		Simulation->GetOnStepEnded().Remove(StepEndedHandle);
	}
	StepEndedHandle.Reset();
}

int32 UHOLSiteSubsystem::AddSite(const FVector2D& Location, const float Radius)
{
	if (!GetWorld() || GetWorld()->GetNetMode() == NM_Client)
	{
		return INDEX_NONE;
	}
	FSite& Site = Sites.AddDefaulted_GetRef();
	Site.Location = Location;
	Site.Radius = FMath::Max(1.f, Radius);
	return Sites.Num() - 1;
}

void UHOLSiteSubsystem::ClearSites()
{
	Sites.Reset();
}

int32 UHOLSiteSubsystem::CountOwned(const int32 Slot) const
{
	int32 Count = 0;
	for (const FSite& Site : Sites)
	{
		Count += Site.State.Owner == Slot ? 1 : 0;
	}
	return Count;
}

void UHOLSiteSubsystem::OnStepEnded(const int64 StepIndex)
{
	// Red-first stub (S5): the rule is not wired yet.
}
