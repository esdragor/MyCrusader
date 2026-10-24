// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLCheatRules.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
	/** A server's console only (the clients' ServerExec is refused by the same rule): it can only refuse more. */
	int32 RefuseCheats = 0;
	FAutoConsoleVariableRef CVarRefuseCheats(TEXT("hol.Cheat.Refuse"), RefuseCheats,
		TEXT("hol.Cheat.Refuse 1: this server refuses the clients' cheats even with -HOLCheats (the net test's refused cheat)"));
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
