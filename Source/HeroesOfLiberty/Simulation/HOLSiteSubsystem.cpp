// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSiteSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLPlayerState.h"
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

	int32 SiteGoldPerMinute = HOLSite::DefaultGoldPerMinute;
	FAutoConsoleVariableRef CVarSiteGoldPerMinute(TEXT("hol.Site.GoldPerMinute"), SiteGoldPerMinute,
		TEXT("Gold by the minute each held sacred site pays its owner alone (provisional 30, Retours/Q-sites-or-par-minute.md)"));

	float SiteVictorySeconds = HOLSite::DefaultVictorySeconds;
	FAutoConsoleVariableRef CVarSiteVictorySeconds(TEXT("hol.Site.VictorySeconds"), SiteVictorySeconds,
		TEXT("Seconds a camp must hold EVERY sacred site to win (600, chosen by the user, D747: SacredSitesInProgress.md S10)"));

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
			Line += FString::Printf(TEXT(" | victory leader %d held %d winner %d"), Sites->GetVictory().Leader, Sites->GetVictory().Held, Sites->GetVictory().Winner);
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
	Memory.Reset();
	Victory = FHOLSiteVictoryState();
	bVictoryApplied = false;
	PublishViews();
}

void UHOLSiteSubsystem::PublishViews()
{
	UWorld* World = GetWorld();
	const AHOLGameState* GameState = World && World->GetNetMode() != NM_Client ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState)
	{
		return;
	}
	const int32 Slots = FHOLMatchConfig::MaxSlots;
	const UHOLVisionSubsystem* Vision = World->GetSubsystem<UHOLVisionSubsystem>();
	const bool bFog = Vision && Vision->IsStarted();
	const int32 HoldSteps = FMath::Max(1, FMath::RoundToInt(SiteHoldSeconds * GetDefault<UHOLSimulationSettings>()->SimulationRateHz));
	Memory.SetNum(Slots);
	for (TArray<FKnown>& PerSlot : Memory)
	{
		PerSlot.SetNum(Sites.Num());
	}
	TArray<FHOLSiteView> Live;
	TArray<FHOLSiteView> Neutral;
	for (int32 Index = 0; Index < Sites.Num(); ++Index)
	{
		Live.Add(HOLSite::MakeView(Index, Sites[Index].Location, Sites[Index].Radius, Sites[Index].State, HoldSteps));
		Neutral.Add(HOLSite::MakeView(Index, Sites[Index].Location, Sites[Index].Radius, FHOLSiteState(), HoldSteps));
	}
	for (APlayerState* Each : GameState->PlayerArray)
	{
		AHOLPlayerState* Player = Cast<AHOLPlayerState>(Each);
		const int32 Slot = Player ? Player->GetPlayerIndex() : INDEX_NONE;
		if (Slot < 0 || Slot >= Slots)
		{
			continue;
		}
		const int32 Camp = bFog ? Vision->GetCampIndex(Slot) : INDEX_NONE;
		// Its camp: itself and its team (the fog's camps, D371); without a fog, itself.
		const auto IsOfMyCamp = [&](const int32 Other)
		{
			return Other == Slot || (Camp != INDEX_NONE && Other >= 0 && Other < Slots && Vision->GetCampIndex(Other) == Camp);
		};
		TArray<FHOLSiteView> Views;
		for (int32 Index = 0; Index < Sites.Num(); ++Index)
		{
			FKnown& Known = Memory[Slot][Index];
			const FHOLSiteState& State = Sites[Index].State;
			const FVector Centre(Sites[Index].Location.X, Sites[Index].Location.Y, 0.);
			const EHOLCellVision Sight = bFog ? Vision->GetVision(Slot, Centre) : EHOLCellVision::Visible;
			// D701: a site is seen by the camp that holds it or is taking it, and by the camp that has it in sight.
			if (Sight == EHOLCellVision::Visible || IsOfMyCamp(State.Owner) || IsOfMyCamp(State.Taker))
			{
				Known.bKnown = true;
				Known.View = Live[Index];
			}
			else if (Sight == EHOLCellVision::Explored && !Known.bKnown)
			{
				// Explored without ever being seen (an explored map): known, shown neutral until it is seen.
				Known.bKnown = true;
				Known.View = Neutral[Index];
			}
			if (Known.bKnown)
			{
				Views.Add(Known.View);
			}
		}
		Player->SetSiteViews(Views);
	}
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
	if (AHOLGameState* Mutable = World->GetGameState<AHOLGameState>())
	{
		StepVictory(*Mutable, TeamBySlot, Disturbers);
	}
	PublishViews();
	PayGold(*GameState);
}

void UHOLSiteSubsystem::StepVictory(AHOLGameState& GameState, const TArray<int32>& TeamBySlot, const TArray<TArray<int32>>& Disturbers)
{
	const int32 Slots = FHOLMatchConfig::MaxSlots;
	const FHOLCampTable Relations = FHOLCampTable::FromWorld(GetWorld());
	// The camp key of each slot: its team; in the Diplomacy mode the lowest slot that is not its adversary (itself included), so an alliance that
	// breaks changes the key of the camp that held every site and resets the count (D703).
	TArray<int32> VictoryTeam;
	for (int32 Slot = 0; Slot < Slots; ++Slot)
	{
		int32 Key = TeamBySlot[Slot];
		if (Relations.IsDiplomacy())
		{
			Key = Slot;
			for (int32 Other = 0; Other < Slot; ++Other)
			{
				if (!Relations.AreHostile(Slot, Other))
				{
					Key = Other;
					break;
				}
			}
		}
		VictoryTeam.Add(Key);
	}
	TArray<int32> OwnerBySite;
	TArray<int32> ContestedBySite;
	for (int32 Index = 0; Index < Sites.Num(); ++Index)
	{
		const int32 Owner = Sites[Index].State.Owner;
		OwnerBySite.Add(Owner);
		int32 Contested = 0;
		for (int32 Slot = 0; Owner >= 0 && Owner < Slots && Slot < Slots; ++Slot)
		{
			if (Disturbers[Index][Slot] > 0 && Relations.AreHostile(Owner, Slot))
			{
				Contested = 1;
			}
		}
		ContestedBySite.Add(Contested);
	}
	const int32 VictorySteps = FMath::Max(1, FMath::RoundToInt(SiteVictorySeconds * GetDefault<UHOLSimulationSettings>()->SimulationRateHz));
	Victory = HOLSite::VictoryStep(Victory, OwnerBySite, ContestedBySite, VictoryTeam, VictorySteps);
	if (Victory.Winner == INDEX_NONE || bVictoryApplied || OwnerBySite.IsEmpty())
	{
		return;
	}
	// The count is full: every adversary of the winning camp is defeated, through the collapse of the forum's defeat (D358).
	bVictoryApplied = true;
	const int32 Reference = OwnerBySite[0];
	UE_LOG(LogHOLMatch, Display, TEXT("Site victory: the camp of slot %d held every site, its adversaries are defeated"), Reference);
	for (int32 Slot = 0; Slot < Slots; ++Slot)
	{
		if (Slot != Reference && Relations.AreHostile(Reference, Slot))
		{
			GameState.DefeatSlot(Slot, EHOLDefeatReason::SacredSites);
		}
	}
}

void UHOLSiteSubsystem::PayGold(const AHOLGameState& GameState)
{
	// S6 (D699): the OWNER alone is paid (its allies get nothing, D703 counts camps for the take, not for the pay), a defeated slot no more.
	const int32 Slots = FHOLMatchConfig::MaxSlots;
	const int32 Rate = GetDefault<UHOLSimulationSettings>()->SimulationRateHz;
	GoldCarry.SetNumZeroed(Slots);
	for (APlayerState* Each : GameState.PlayerArray)
	{
		AHOLPlayerState* Player = Cast<AHOLPlayerState>(Each);
		const int32 Slot = Player ? Player->GetPlayerIndex() : INDEX_NONE;
		if (Slot < 0 || Slot >= Slots)
		{
			continue;
		}
		if (GameState.IsSlotDefeated(Slot))
		{
			GoldCarry[Slot] = 0;
			continue;
		}
		const int32 Owned = CountOwned(Slot);
		if (Owned == 0 && GoldCarry[Slot] == 0)
		{
			continue;
		}
		const int32 Gold = HOLSite::GoldStep(GoldCarry[Slot], Owned, SiteGoldPerMinute, Rate);
		if (Gold > 0)
		{
			FHOLResources Stock = Player->GetResources();
			Stock.Gold += Gold;
			Player->SetResources(Stock);
		}
	}
}
