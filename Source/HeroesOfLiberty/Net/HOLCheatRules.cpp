// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLCheatRules.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HeroesOfLibertyLog.h"
#include "Simulation/HOLSimulationSubsystem.h"

namespace
{
	/** A server's console only (the clients' ServerExec is refused by the same rule): it can only refuse more. */
	int32 RefuseCheats = 0;
	FAutoConsoleVariableRef CVarRefuseCheats(TEXT("hol.Cheat.Refuse"), RefuseCheats,
		TEXT("hol.Cheat.Refuse 1: this server refuses the clients' cheats even with -HOLCheats (the net test's refused cheat)"));

	/** The net test's refused pause: is this world paused, and has its simulation stepped since the last report. */
	FAutoConsoleCommandWithWorld PauseReportCommand(
		TEXT("hol.Match.PauseReport"),
		TEXT("hol.Match.PauseReport: logs whether this world is paused and its simulation step"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (!World)
			{
				return;
			}
			const UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			UE_LOG(LogHOLMatch, Display, TEXT("Pause report: paused %d, simulation step %lld"), World->IsPaused() ? 1 : 0,
				Simulation ? static_cast<int64>(Simulation->GetStepIndex()) : -1ll);
		}));
}

bool FHOLCheatRules::AreCheatsAllowed(const ENetMode NetMode, const bool bHOLCheatsOnCommandLine, const bool bRefusedForTest)
{
	if (NetMode == NM_Standalone)
	{
		return true;
	}
	return NetMode != NM_Client && bHOLCheatsOnCommandLine && !bRefusedForTest;
}

bool FHOLCheatRules::AreCheatsAllowed(const UWorld& World)
{
	return AreCheatsAllowed(World.GetNetMode(), FParse::Param(FCommandLine::Get(), TEXT("HOLCheats")), RefuseCheats != 0);
}

bool FHOLCheatRules::IsPauseAllowed(const ENetMode NetMode)
{
	return NetMode == NM_Standalone;
}
