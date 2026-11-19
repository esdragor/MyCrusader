// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLBoardLog.h"

#include "Buildings/HOLConstructionBoard.h"
#include "Buildings/HOLProductionBoard.h"
#include "Commands/HOLPlayerOrdersBoard.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLChatSubsystem.h"
#include "Match/HOLMarketSubsystem.h"
#include "Match/HOLMatchFlowSubsystem.h"
#include "Match/HOLSignalSubsystem.h"
#include "Match/HOLSpecializationSubsystem.h"
#include "Player/HOLCommandGridBoard.h"
#include "Simulation/HOLCampBoard.h"
#include "Simulation/HOLGarrisonBoard.h"
#include "Simulation/HOLRelayBoard.h"
#include "Simulation/HOLSiteBoardSubsystem.h"
#include "Simulation/HOLUnitTierSubsystem.h"

namespace HOLBoardLog
{
	namespace
	{
		/** The worlds already reported (weak: a world that goes away leaves the set by itself). */
		TSet<TWeakObjectPtr<UWorld>>& ReportedWorlds()
		{
			static TSet<TWeakObjectPtr<UWorld>> Worlds;
			return Worlds;
		}

		/** The line of one bridge: the script class when a board born from the bridge exists in the world, else the C++ fallback warning. */
		template <typename TBridge>
		void Report(const UWorld& World, const TCHAR* BridgeName)
		{
			const UClass* ScriptClass = nullptr;
			const TArray<TBridge*> Boards = World.GetSubsystemArrayCopy<TBridge>();
			for (const TBridge* Board : Boards)
			{
				if (Board && !Board->IsUnreachable() && Board->GetClass() != TBridge::StaticClass()
					&& !Board->GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
				{
					ScriptClass = Board->GetClass();
					break;
				}
			}
			if (ScriptClass)
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Board: %s script %s"), BridgeName, *ScriptClass->GetName());
			}
			else
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Board: %s ABSENT (C++ fallback)"), BridgeName);
			}
		}
	}

	void LogWorldBoards(UWorld& World)
	{
		// Only a world of the game (a hand-made test world has no game instance: no line, no noise).
		if (!World.IsGameWorld() || World.GetGameInstance() == nullptr)
		{
			return;
		}
		TSet<TWeakObjectPtr<UWorld>>& Reported = ReportedWorlds();
		if (Reported.Contains(&World))
		{
			return;
		}
		Reported.Add(&World);

		Report<UHOLMatchFlowSubsystem>(World, TEXT("MatchFlow"));
		Report<UHOLMarketSubsystem>(World, TEXT("Market"));
		Report<UHOLConstructionBoard>(World, TEXT("Construction"));
		Report<UHOLPlayerOrdersBoard>(World, TEXT("PlayerOrders"));
		Report<UHOLProductionBoard>(World, TEXT("Production"));
		Report<UHOLCommandGridBoard>(World, TEXT("CommandGrid"));
		Report<UHOLUnitTierSubsystem>(World, TEXT("UnitTier"));
		Report<UHOLCampBoard>(World, TEXT("Camp"));
		Report<UHOLGarrisonBoard>(World, TEXT("Garrison"));
		Report<UHOLRelayBoard>(World, TEXT("Relay"));
		Report<UHOLSiteBoardSubsystem>(World, TEXT("Site"));
		Report<UHOLChatSubsystem>(World, TEXT("Chat"));
		Report<UHOLSignalSubsystem>(World, TEXT("Signal"));
		Report<UHOLSpecializationSubsystem>(World, TEXT("Specialization"));
	}
}
