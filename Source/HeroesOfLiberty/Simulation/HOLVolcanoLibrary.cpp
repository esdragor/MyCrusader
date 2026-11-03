// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLVolcanoLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Buildings/HOLBuilding.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLGameState.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLVolcanoSubsystem.h"

namespace UE::HeroesOfLiberty::VolcanoTools
{
	UWorld* FindMatchWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client && World->GetGameState<AHOLGameState>())
			{
				return World;
			}
		}
		return nullptr;
	}

	/**
	 * hol.Volcano.Site <Dx> <Dy> [AltarDx AltarDy] [Length]: poses a site (a trial and scenario tool: V4b will read the sites in the map's recipe). The crater
	 * lies at the first player's main forum plus (Dx, Dy) cm; four straight flows of Length cm (6000 by default) leave it east, north, west and south;
	 * the altar lies at the crater plus (AltarDx, AltarDy) (1800, 1800 by default: between two flows, out of the lava's reach, 1200 cm zone).
	 */
	void RunSite(const TArray<FString>& Args)
	{
		UWorld* World = FindMatchWorld();
		UHOLVolcanoSubsystem* Volcano = World ? UHOLVolcanoSubsystem::Find(World) : nullptr;
		AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
		if (!Volcano || !GameState || Args.Num() < 2)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("hol.Volcano.Site <Dx> <Dy> [AltarDx AltarDy] [Length]: needs a running match (server or solo)"));
			return;
		}
		const AHOLBuilding* Forum = AHOLGameState::FindPrimaryForum(*World, 0);
		const FVector Origin = Forum ? Forum->GetActorLocation() : FVector::ZeroVector;
		// In the main forum's frame, as hol.Camera.FocusNear (the scenarios place everything the same way).
		const FRotator Frame = Forum ? Forum->GetActorRotation() : FRotator::ZeroRotator;
		FVector Crater = Origin + Frame.RotateVector(FVector(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.));
		Crater.Z = 0.;
		const FVector AltarOffset = Frame.RotateVector(Args.Num() >= 4 ? FVector(FCString::Atod(*Args[2]), FCString::Atod(*Args[3]), 0.) : FVector(1800., 1800., 0.));
		const double Length = Args.Num() >= 5 ? FCString::Atod(*Args[4]) : 6000.;
		HOLVolcano::FSite Site;
		for (const FVector& Direction : { FVector(1., 0., 0.), FVector(0., 1., 0.), FVector(-1., 0., 0.), FVector(0., -1., 0.) })
		{
			HOLVolcano::FFlow Flow;
			Flow.Points = { Crater, Crater + Frame.RotateVector(Direction) * Length };
			Site.Flows.Add(Flow);
		}
		Site.Altar.bPresent = true;
		Site.Altar.Location = Crater + AltarOffset;
		Volcano->SetSites({ Site });
		const AHOLBuilding* Enemy = AHOLGameState::FindPrimaryForum(*World, 1);
		UE_LOG(LogHOLMatch, Display, TEXT("Volcano site: first forum at (%.0f, %.0f), second at (%.0f, %.0f)"), Origin.X, Origin.Y, Enemy ? Enemy->GetActorLocation().X : 0., Enemy ? Enemy->GetActorLocation().Y : 0.);
		UE_LOG(LogHOLMatch, Display, TEXT("Volcano site posed: crater (%.0f, %.0f), altar (%.0f, %.0f) zone %.0f cm, 4 flows of %.0f cm, Catastrophe %d, seed %d"),
			Crater.X, Crater.Y, Site.Altar.Location.X, Site.Altar.Location.Y, Site.Altar.Radius, Length, GameState->GetLaunchConfig().bCatastrophe ? 1 : 0, GameState->GetLaunchConfig().Seed);
	}

	/** V9a: the world of a client of a network match (it has the game state, replicated), or null. */
	UWorld* FindClientWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (World && World->IsGameWorld() && World->GetNetMode() == NM_Client && World->GetGameState<AHOLGameState>())
			{
				return World;
			}
		}
		return nullptr;
	}

	/**
	 * hol.Volcano.Window <Seconds> [Length]: a trial and test tool (server or solo): the first trigger window of every site opens Seconds after the
	 * present clock instant (Length s long, 1 by default) so that the alert comes within seconds instead of after 6 minutes. Dormant without Catastrophe.
	 */
	void RunWindow(const TArray<FString>& Args)
	{
		UWorld* World = FindMatchWorld();
		UHOLVolcanoSubsystem* Volcano = World ? UHOLVolcanoSubsystem::Find(World) : nullptr;
		const UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
		if (!Volcano || !Simulation || Args.IsEmpty())
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("hol.Volcano.Window <Seconds> [Length]: needs a running match (server or solo)"));
			return;
		}
		const float Now = Volcano->GetClockOf(Simulation->GetStepIndex());
		Volcano->GetMutableSettings().WindowOpensAfter = Now + static_cast<float>(FCString::Atod(*Args[0]));
		Volcano->GetMutableSettings().WindowLength = Args.Num() >= 2 ? static_cast<float>(FCString::Atod(*Args[1])) : 1.f;
		UE_LOG(LogHOLMatch, Display, TEXT("Volcano window: the trigger window opens at %.1f s (clock now %.1f s) and lasts %.1f s"), Volcano->GetSettings().WindowOpensAfter, Now, Volcano->GetSettings().WindowLength);
	}

	/** hol.Volcano.Report: one line per site (phase, countdown, altar), for the scenarios and the journal. */
	void RunReport(const TArray<FString>& Args)
	{
		UWorld* World = FindMatchWorld();
		const UHOLVolcanoSubsystem* Volcano = World ? UHOLVolcanoSubsystem::Find(World) : nullptr;
		if (!Volcano)
		{
			// V9a: a client has no site, only the views the server replicates through the game state.
			UWorld* ClientWorld = FindClientWorld();
			const AHOLGameState* ClientState = ClientWorld ? ClientWorld->GetGameState<AHOLGameState>() : nullptr;
			if (!ClientState)
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Volcano report: no match"));
				return;
			}
			UE_LOG(LogHOLMatch, Display, TEXT("Volcano views (client): catastrophe %d, %d site(s)"), ClientState->GetLaunchConfig().bCatastrophe ? 1 : 0, ClientState->GetVolcanoViews().Num());
			for (int32 Index = 0; Index < ClientState->GetVolcanoViews().Num(); ++Index)
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Volcano view: site %d %s"), Index, *ClientState->GetVolcanoViews()[Index].Describe());
			}
			return;
		}
		if (Volcano->GetSites().IsEmpty())
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Volcano report: no site"));
			return;
		}
		for (int32 Index = 0; Index < Volcano->GetSites().Num(); ++Index)
		{
			const FHOLVolcanoSiteView View = UHOLVolcanoLibrary::GetSiteView(World, Index);
			UE_LOG(LogHOLMatch, Display, TEXT("Volcano report: site %d phase %d to-erupt %d s altar lit %d held %d%% (%d s left) zone %d woken %d wakes %d, %d flows, crater (%.0f, %.0f), altar (%.0f, %.0f), fresh deposits %d"),
				Index, View.Phase, View.SecondsToErupt, View.bAltarLit ? 1 : 0, View.AltarPercent, View.AltarSecondsLeft, View.AltarHold, View.bWoken ? 1 : 0, Volcano->GetNumWakes(Index),
				Volcano->GetSites()[Index].Flows.Num(), Volcano->GetSites()[Index].Flows.IsEmpty() || Volcano->GetSites()[Index].Flows[0].Points.IsEmpty() ? 0. : Volcano->GetSites()[Index].Flows[0].Points[0].X,
				Volcano->GetSites()[Index].Flows.IsEmpty() || Volcano->GetSites()[Index].Flows[0].Points.IsEmpty() ? 0. : Volcano->GetSites()[Index].Flows[0].Points[0].Y,
				Volcano->GetSites()[Index].Altar.Location.X, Volcano->GetSites()[Index].Altar.Location.Y, Volcano->GetFreshSourceIndices(Index).Num());
			const UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			// V9a: the same line a client prints from the views it receives (the server's, from the truth).
			UE_LOG(LogHOLMatch, Display, TEXT("Volcano view: site %d %s"), Index, *Volcano->BuildNetView(Index, Simulation ? Volcano->GetClockOf(Simulation->GetStepIndex()) : 0.f).Describe());
		}
	}

	FAutoConsoleCommand CmdSite(TEXT("hol.Volcano.Site"),
		TEXT("hol.Volcano.Site <Dx> <Dy> [AltarDx AltarDy] [Length]: poses a volcano site relative to the first player's main forum (trial tool, V12); needs the Catastrophe option"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&RunSite));
	FAutoConsoleCommand CmdWindow(TEXT("hol.Volcano.Window"), TEXT("hol.Volcano.Window <Seconds> [Length]: the sites' first trigger window opens Seconds from now (trial and test tool)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&RunWindow));
	FAutoConsoleCommand CmdReport(TEXT("hol.Volcano.Report"), TEXT("hol.Volcano.Report: one log line per volcano site (phase, countdown, altar)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&RunReport));
}

int32 UHOLVolcanoLibrary::GetSiteCount(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UHOLVolcanoSubsystem* Volcano = UHOLVolcanoSubsystem::Find(World);
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (World && World->GetNetMode() == NM_Client)
	{
		// V9a: a client has no site, it counts the views the server replicates.
		return GameState && GameState->GetLaunchConfig().bCatastrophe ? GameState->GetVolcanoViews().Num() : 0;
	}
	return Volcano && GameState && GameState->GetLaunchConfig().bCatastrophe ? Volcano->GetSites().Num() : 0;
}

FHOLVolcanoSiteView UHOLVolcanoLibrary::GetSiteView(const UObject* WorldContextObject, const int32 SiteIndex)
{
	FHOLVolcanoSiteView View;
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UHOLVolcanoSubsystem* Volcano = UHOLVolcanoSubsystem::Find(World);
	if (World && World->GetNetMode() == NM_Client)
	{
		// V9a: read from the views the server replicates (the same numbers it reads from its sites).
		const AHOLGameState* ClientState = World->GetGameState<AHOLGameState>();
		if (!ClientState || !ClientState->GetLaunchConfig().bCatastrophe || !ClientState->GetVolcanoViews().IsValidIndex(SiteIndex))
		{
			return View;
		}
		const FHOLVolcanoSiteNet& Net = ClientState->GetVolcanoViews()[SiteIndex];
		View.Phase = Net.Phase;
		View.SecondsToErupt = Net.SecondsToErupt;
		View.bAltarPresent = Net.bAltarPresent;
		View.bAltarLit = Net.bAltarLit;
		View.bWoken = Net.bWoken;
		View.AltarPercent = Net.AltarPercent;
		View.AltarSecondsLeft = Net.AltarSecondsLeft;
		View.AltarHold = Net.AltarHold;
		return View;
	}
	if (!Volcano || !Volcano->GetSites().IsValidIndex(SiteIndex))
	{
		return View;
	}
	const HOLVolcano::FSiteState State = Volcano->GetSiteState(SiteIndex);
	View.Phase = static_cast<int32>(State.Phase);
	View.SecondsToErupt = State.SecondsToErupt >= 0.f ? FMath::CeilToInt(State.SecondsToErupt) : -1;
	View.bAltarPresent = Volcano->GetSites()[SiteIndex].Altar.bPresent;
	View.bAltarLit = State.bAltarLit;
	View.bWoken = State.bWoken;
	const float HoldSeconds = FMath::Max(Volcano->GetSettings().AltarHoldSeconds, UE_KINDA_SMALL_NUMBER);
	const float Held = Volcano->GetAltarHeldSeconds(SiteIndex);
	View.AltarPercent = FMath::Clamp(FMath::FloorToInt(100.f * Held / HoldSeconds), 0, 100);
	View.AltarSecondsLeft = FMath::Max(0, FMath::CeilToInt(HoldSeconds - Held));
	View.AltarHold = static_cast<int32>(Volcano->GetAltarHold(SiteIndex));
	return View;
}

namespace
{
	int32 AIAltarTraceFrom = 0;
	FAutoConsoleVariableRef CVarAIAltarTraceFrom(TEXT("hol.AI.AltarTraceFrom"), AIAltarTraceFrom,
		TEXT("Q-ia-autel-distance, provisional: from which positions the AI measures the 40 m to the nearest possible flow before it holds a free altar: 0 its buildings (default), 1 the altar itself (where its squad stands)"));
}

namespace
{
	int32 AIAltarStrikeBack = 1;
	FAutoConsoleVariableRef CVarAIAltarStrikeBack(TEXT("hol.AI.AltarStrikeBack"), AIAltarStrikeBack,
		TEXT("Q-ia-autel-diversion = B (user answer 2026-11-02): 1 (default) Hard, facing a big adversary army on the altar (its camp is bare), launches its attack wave at once toward the enemy forum, without waiting for the usual ratio; 0 it only does not send the squad"));
}

bool UHOLVolcanoLibrary::GetAIAltarStrikeBack()
{
	return AIAltarStrikeBack != 0;
}

void UHOLVolcanoLibrary::ReadAIAltars(const UObject* WorldContextObject, const int32 PlayerIndex, TArray<FHOLAIAltar>& OutAltars)
{
	OutAltars.Reset();
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UHOLVolcanoSubsystem* Volcano = UHOLVolcanoSubsystem::Find(World);
	if (!Volcano)
	{
		return;
	}
	for (int32 Index = 0; Index < Volcano->GetSites().Num(); ++Index)
	{
		UHOLVolcanoSubsystem::FAltarMeasure Measure;
		if (!Volcano->MeasureAltar(Index, PlayerIndex, Measure))
		{
			continue;
		}
		FHOLAIAltar Altar;
		Altar.SiteIndex = Index;
		Altar.Location = Measure.Location;
		Altar.Radius = Measure.Radius;
		Altar.bAdversary = Measure.bAdversary;
		Altar.AdversaryStrength = Measure.AdversaryStrength;
		Altar.TraceDistance = AIAltarTraceFrom == 1 ? Measure.TraceFromAltar : Measure.TraceFromBuildings;
		OutAltars.Add(Altar);
	}
}

void UHOLVolcanoLibrary::DecideAltarSquad(const UObject* WorldContextObject, const FHOLAIAltar& Altar, const int32 FreeSoldiers, const int32 AlreadySent, const int32 Level, int32& OutSquad, bool& bOutDiversion)
{
	OutSquad = 0;
	bOutDiversion = false;
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UHOLVolcanoSubsystem* Volcano = UHOLVolcanoSubsystem::Find(World);
	const FHOLVolcanoSettings Defaults;
	const FHOLVolcanoSettings& Settings = Volcano ? Volcano->GetSettings() : Defaults;
	const HOLVolcano::EAltarLevel AltarLevel = Level <= 0 ? HOLVolcano::EAltarLevel::Easy : (Level >= 2 ? HOLVolcano::EAltarLevel::Hard : HOLVolcano::EAltarLevel::Normal);
	if (Altar.bAdversary)
	{
		HOLVolcano::FAltarSendInput Input;
		Input.AdversaryStrength = Altar.AdversaryStrength;
		Input.FreeSoldiers = FreeSoldiers;
		Input.AlreadySent = AlreadySent;
		Input.Level = AltarLevel;
		const HOLVolcano::FAltarSend Send = HOLVolcano::AltarSend(Input, Settings);
		OutSquad = Send.Squad;
		bOutDiversion = Send.bDiversion;
		return;
	}
	HOLVolcano::FAltarHoldInput Input;
	Input.FreeSoldiers = FreeSoldiers;
	Input.AlreadySent = AlreadySent;
	Input.NearestTraceDistance = Altar.TraceDistance;
	Input.Level = AltarLevel;
	OutSquad = HOLVolcano::AltarHoldSquad(Input, Settings);
}
