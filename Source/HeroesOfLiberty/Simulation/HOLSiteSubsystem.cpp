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
	FAutoConsoleVariableRef CVarSiteEnabled(TEXT("hol.Site.Enabled"), bSitesEnabled,
		TEXT("1 to let the sacred sites work (the match option 'Sacred sites' replaces it at S7); 0 by default: the game does not change"));

	float SiteHoldSeconds = HOLSite::DefaultHoldSeconds;
	FAutoConsoleVariableRef CVarSiteHoldSeconds(TEXT("hol.Site.HoldSeconds"), SiteHoldSeconds,
		TEXT("Seconds an adverse side must hold a sacred site's zone alone to take it (provisional 60, like the relay's)"));

	FAutoConsoleCommandWithWorldAndArgs SiteCommandAdd(TEXT("hol.Site.Add"),
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

	FAutoConsoleCommandWithWorldAndArgs SiteCommandClear(TEXT("hol.Site.Clear"), TEXT("hol.Site.Clear: no sacred site left (development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UHOLSiteSubsystem* Sites = World ? World->GetSubsystem<UHOLSiteSubsystem>() : nullptr)
			{
				Sites->ClearSites();
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs SiteCommandReport(TEXT("hol.Site.Report"), TEXT("hol.Site.Report: each sacred site's owner, taker and count in the log"),
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

void UHOLSiteSubsystem::PublishViews()
{
	// S5 stub: nothing is published yet (the test of the views is red).
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
	UWorld* World = GetWorld();
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!IsEnabled() || Sites.IsEmpty() || !Units || !GameState)
	{
		return;
	}
	const FHOLMatchConfig& Config = GameState->GetLaunchConfig();
	const int32 Slots = FHOLMatchConfig::MaxSlots;
	TArray<int32> TeamBySlot;
	TArray<FName> FactionBySlot;
	TArray<FName> HeroTypeBySlot;
	TArray<FName> WorkerTypeBySlot;
	for (int32 Slot = 0; Slot < Slots; ++Slot)
	{
		const int32 Camp = HOLVictoryRules::GetCamp(Config, Slot);
		TeamBySlot.Add(Camp != INDEX_NONE ? Camp : 1000 + Slot);
		FactionBySlot.Add(HOLFactionRules::GetSlotFactionKey(Config, Slot));
		HeroTypeBySlot.Add(HOLFactionRules::GetRoleTypeName(Config, Slot, HOLFactionRules::Roles::Hero));
		WorkerTypeBySlot.Add(HOLFactionRules::GetRoleTypeName(Config, Slot, HOLFactionRules::Roles::Worker));
	}

	// One walk of the units for all the sites; a neutral owner's units (out of the slots) count for nothing, as at the relays.
	TArray<TArray<int32>> Takers;
	TArray<TArray<int32>> Disturbers;
	Takers.SetNum(Sites.Num());
	Disturbers.SetNum(Sites.Num());
	for (int32 Index = 0; Index < Sites.Num(); ++Index)
	{
		HOLSite::ResetCounts(Takers[Index], Disturbers[Index], Slots);
	}
	Units->ForEachLivingUnitWithType([this, Slots, &Takers, &Disturbers, &FactionBySlot, &HeroTypeBySlot, &WorkerTypeBySlot](const uint32 UnitId, const FName TypeName, const uint8 Owner, const FVector& Location)
	{
		if (Owner >= Slots)
		{
			return;
		}
		const FVector2D Point(Location);
		const bool bCanTake = TypeName == HeroTypeBySlot[Owner] || IsHolyType(FactionBySlot[Owner], TypeName);
		const bool bIsPeasant = TypeName == WorkerTypeBySlot[Owner];
		for (int32 Index = 0; Index < Sites.Num(); ++Index)
		{
			if (FVector2D::DistSquared(Point, Sites[Index].Location) <= FMath::Square(Sites[Index].Radius))
			{
				HOLSite::AddUnit(Takers[Index], Disturbers[Index], Owner, bCanTake, bIsPeasant);
			}
		}
	});

	const FHOLCampTable Relations = FHOLCampTable::FromWorld(World);
	const int32 HoldSteps = FMath::Max(1, FMath::RoundToInt(SiteHoldSeconds * GetDefault<UHOLSimulationSettings>()->SimulationRateHz));
	for (int32 Index = 0; Index < Sites.Num(); ++Index)
	{
		FHOLSiteState& State = Sites[Index].State;
		const int32 OldOwner = State.Owner;
		// Diplomacy (D369, D401, D703): the owner's camp is the owner and its allies (both said Ally); anyone else, a Neutral included, is an
		// adversary of its own (the book is read each step, so a break or an alliance counts at once).
		TArray<int32> OwnerRelativeTeams;
		if (Relations.IsDiplomacy() && State.Owner >= 0 && State.Owner < Slots)
		{
			for (int32 Slot = 0; Slot < Slots; ++Slot)
			{
				OwnerRelativeTeams.Add(Slot == State.Owner || !Relations.AreHostile(State.Owner, Slot) ? 500 : 1000 + Slot);
			}
		}
		State = HOLSite::Step(State, Takers[Index], Disturbers[Index], OwnerRelativeTeams.IsEmpty() ? TeamBySlot : OwnerRelativeTeams, HoldSteps, HOLSite::DefaultDecaySteps);
		if (State.Owner != OldOwner)
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Site %d: slot %d takes it from slot %d at step %d"), Index, State.Owner, OldOwner, static_cast<int32>(StepIndex));
		}
	}
	PublishViews();
}
