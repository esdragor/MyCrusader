// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLCheatRules.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HeroesOfLibertyLog.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Match/HOLGameState.h"
#include "Match/HOLSpecializationSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"

namespace
{
	/** A server's console only (the clients' ServerExec is refused by the same rule): it can only refuse more. */
	int32 RefuseCheats = 0;
	FAutoConsoleVariableRef CVarRefuseCheats(TEXT("hol.Cheat.Refuse"), RefuseCheats,
		TEXT("hol.Cheat.Refuse 1: this server refuses the clients' cheats even with -HOLCheats (the net test's refused cheat)"));

	/** D799, development (the net test): the server makes a seat Nomad or standard (the game state's replicated bits). */
	FAutoConsoleCommandWithWorldAndArgs NomadSetSeatCommand(
		TEXT("hol.Nomad.SetSeat"),
		TEXT("hol.Nomad.SetSeat <Slot> <0|1>: the server marks that seat Nomad (1) or standard (0), replicated to every client (development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
			if (Args.Num() < 2 || !GameState || !GameState->HasAuthority())
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Nomad.SetSeat <Slot> <0|1> (server only)"));
				return;
			}
			GameState->SetSeatNomad(FCString::Atoi(*Args[0]), FCString::Atoi(*Args[1]) != 0);
		}));

	/** D799 (the net test): what this machine computes for a seat's first forum (its grid's cell, its price). */
	FAutoConsoleCommandWithWorldAndArgs NomadReportCommand(
		TEXT("hol.Nomad.Report"),
		TEXT("hol.Nomad.Report <Slot>: logs whether this machine knows the seat Nomad, its forum cell open and the forum's price it shows"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
			const int32 Slot = Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0]);
			if (!GameState)
			{
				return;
			}
			const FHOLResources Cost = UHOLBuildingSettings::GetBuildCost(World, HOLBuildingTypeNames::MainCentre, Slot);
			UE_LOG(LogHOLMatch, Display, TEXT("Nomad report: slot %d nomad %d, forum complete %d, forum cell open %d, forum price wood %d stone %d"),
				Slot, GameState->IsSeatNomad(Slot) ? 1 : 0, UHOLSpecializationSubsystem::IsForumComplete(World, Slot) ? 1 : 0,
				UHOLBuildingSettings::IsFirstForumOpen(World, Slot) ? 1 : 0, Cost.Wood, Cost.Stone);
		}));

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
			// P5 (D513): the match's own pause after the engine's (refused in a networked match).
			const AHOLGameState* GameState = World->GetGameState<AHOLGameState>();
			UE_LOG(LogHOLMatch, Display, TEXT("Pause report: paused %d, simulation step %lld, match paused %d by slot %d, %d left"),
				World->IsPaused() ? 1 : 0, Simulation ? static_cast<int64>(Simulation->GetStepIndex()) : -1ll,
				GameState && GameState->IsMatchPaused() ? 1 : 0, GameState ? GameState->GetPausedBySlot() : INDEX_NONE,
				GameState ? GameState->GetPausesLeft() : -1);
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
