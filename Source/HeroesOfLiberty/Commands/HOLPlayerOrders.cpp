// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLPlayerOrders.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLPlayerOrdersBoard.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLMatchConfig.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLGarrisonSubsystem.h"
#include "Simulation/HOLUnitView.h"
#include "UObject/ObjectKey.h"

namespace HOLPlayerOrders
{
	namespace
	{
		/** The ids of an order as the whole numbers the script bridge takes (UHOLPlayerOrdersBoard; the queue's ids are uint32). */
		TArray<int32> ToBridgeIds(const TConstArrayView<uint32> UnitIds)
		{
			TArray<int32> Ids;
			Ids.Reserve(UnitIds.Num());
			for (const uint32 UnitId : UnitIds)
			{
				Ids.Add(static_cast<int32>(UnitId));
			}
			return Ids;
		}

		/** An order refused because the world has no script board: logged once per game (world) and per function, the AI asks in a loop. */
		void LogBoardAbsent(const UWorld* World, const TCHAR* Order)
		{
			static FCriticalSection Lock;
			static TMap<FName, FObjectKey> Logged;
			const FObjectKey Key(World);
			{
				FScopeLock ScopeLock(&Lock);
				const FObjectKey* Last = Logged.Find(FName(Order));
				if (Last && *Last == Key)
				{
					return;
				}
				Logged.Add(FName(Order), Key);
			}
			if (!IsInGameThread())
			{
				// Board::Find is null off the game thread whatever the boards: not the same cause as an absent board.
				UE_LOG(LogHOLMatch, Error, TEXT("PlayerOrders: %s refused, not on the game thread"), Order);
				return;
			}
			UE_LOG(LogHOLMatch, Error, TEXT("PlayerOrders: %s refused, script board absent"), Order);
		}

		/** The board's answer as the order's result: 1 accepted, 0 refused, -1 (no script board) refused and said once. The rule is the script's alone. */
		bool Verdict(const UWorld* World, const TCHAR* Order, const int32 Answer)
		{
			if (Answer < 0)
			{
				LogBoardAbsent(World, Order);
				return false;
			}
			return Answer != 0;
		}
	}

	bool IsAccepted(const int32 PlayerIndex, const int32 NumUnits)
	{
		// The script rule (no world argument here: the board of the game world); no board: refused.
		return Verdict(nullptr, TEXT("IsAccepted"), UHOLPlayerOrdersBoard::AskIsAcceptedAnywhere(PlayerIndex, NumUnits));
	}

	void SendNonWorkersTo(UWorld& World, UHOLCommandSubsystem& Commands, const int32 PlayerIndex, TArray<uint32>& InOutUnitIds, const FVector& Point,
		const bool bQueue)
	{
		TArray<uint32> Walkers;
		for (int32 Index = InOutUnitIds.Num() - 1; Index >= 0; --Index)
		{
			FHOLUnitView View;
			if (HOLUnitView::Find(World, InOutUnitIds[Index], View) && View.OwnerPlayer == PlayerIndex && View.CarryCapacity <= 0)
			{
				Walkers.Add(InOutUnitIds[Index]);
				InOutUnitIds.RemoveAt(Index);
			}
		}
		if (!Walkers.IsEmpty())
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Order of slot %d: %d units that do not work walk to (%.0f, %.0f)%s"), PlayerIndex, Walkers.Num(), Point.X, Point.Y,
				bQueue ? TEXT(", queued") : TEXT(""));
			Commands.IssueMoveOrder(Walkers, Point, PlayerIndex, /*bFromTask*/ false, FVector2D::ZeroVector, false, bQueue);
		}
	}

	bool Move(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, const FVector& Target, const bool bQueue,
		const FVector2D& FrontLine, const bool bFrontWidth)
	{
		return Verdict(&World, TEXT("Move"), UHOLPlayerOrdersBoard::AskMove(&World, PlayerIndex, ToBridgeIds(UnitIds), Target, bQueue, FrontLine, bFrontWidth));
	}

	bool Attack(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, const uint32 TargetUnitId, const bool bQueue)
	{
		return Verdict(&World, TEXT("Attack"), UHOLPlayerOrdersBoard::AskAttack(&World, PlayerIndex, ToBridgeIds(UnitIds), static_cast<int32>(TargetUnitId), bQueue));
	}

	bool AttackBuilding(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const bool bQueue)
	{
		return Verdict(&World, TEXT("AttackBuilding"), UHOLPlayerOrdersBoard::AskAttackBuilding(&World, PlayerIndex, ToBridgeIds(UnitIds), Building, bQueue));
	}

	bool AttackMove(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, const FVector& Target, const bool bQueue)
	{
		return Verdict(&World, TEXT("AttackMove"), UHOLPlayerOrdersBoard::AskAttackMove(&World, PlayerIndex, ToBridgeIds(UnitIds), Target, bQueue));
	}

	bool Stop(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, const bool bHold)
	{
		return Verdict(&World, TEXT("Stop"), UHOLPlayerOrdersBoard::AskStop(&World, PlayerIndex, ToBridgeIds(UnitIds), bHold));
	}

	bool ToggleDeploy(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds)
	{
		return Verdict(&World, TEXT("ToggleDeploy"), UHOLPlayerOrdersBoard::AskToggleDeploy(&World, PlayerIndex, ToBridgeIds(UnitIds)));
	}

	bool Gather(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, const int32 SourceIndex, const bool bQueue)
	{
		// The slot and the count are the script's rule, asked BEFORE the soldiers walk (this body does the effects the script cannot: HOLUnitView, UHOLSourceSubsystem).
		if (!Verdict(&World, TEXT("Gather"), UHOLPlayerOrdersBoard::AskIsAccepted(&World, PlayerIndex, UnitIds.Num())))
		{
			return false;
		}
		UHOLCommandSubsystem* Commands = World.GetSubsystem<UHOLCommandSubsystem>();
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		TArray<uint32> Workers(UnitIds);
		const UHOLSourceSubsystem* Sources = World.GetSubsystem<UHOLSourceSubsystem>();
		const bool bSource = Sources && Sources->GetSources().IsValidIndex(SourceIndex);
		const FVector SourceLocation = bSource ? FVector(Sources->GetSources()[SourceIndex].Location) : FVector::ZeroVector;
		if (bSource)
		{
			// With Shift, the soldiers' walk at the end of their queue (L.6, as a deposit's, L.3). Stays C++ (HOLUnitView is not exposed to the script).
			SendNonWorkersTo(World, *Commands, PlayerIndex, Workers, SourceLocation, bQueue);
		}
		// The script rule (HOLPlayerOrders.as Gather) orders the workers left.
		return Verdict(&World, TEXT("Gather"), UHOLPlayerOrdersBoard::AskGather(&World, PlayerIndex, ToBridgeIds(Workers), SourceIndex, bQueue, bSource, SourceLocation));
	}

	bool Build(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const bool bQueue)
	{
		if (!Verdict(&World, TEXT("Build"), UHOLPlayerOrdersBoard::AskIsAccepted(&World, PlayerIndex, UnitIds.Num())))
		{
			return false;
		}
		UHOLCommandSubsystem* Commands = World.GetSubsystem<UHOLCommandSubsystem>();
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		TArray<uint32> Workers(UnitIds);
		if (Building)
		{
			// A null site is the script's refusal (AskBuild below); the walk needs its location.
			SendNonWorkersTo(World, *Commands, PlayerIndex, Workers, Building->GetActorLocation(), bQueue);
		}
		return Verdict(&World, TEXT("Build"), UHOLPlayerOrdersBoard::AskBuild(&World, PlayerIndex, ToBridgeIds(Workers), Building, bQueue));
	}

	bool DropOff(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const FVector& Point,
		const bool bQueue)
	{
		if (!Verdict(&World, TEXT("DropOff"), UHOLPlayerOrdersBoard::AskIsAccepted(&World, PlayerIndex, UnitIds.Num())))
		{
			return false;
		}
		UHOLCommandSubsystem* Commands = World.GetSubsystem<UHOLCommandSubsystem>();
		if (!Commands)
		{
			return false;
		}
		const UHOLCommandSubsystem::FPlayerOrderScope PlayerOrder(*Commands);
		// bDeposit reads the building type row (FHOLBuildingTypeRow::Accepts, not exposed): game data computed here, the choice it makes is the script's.
		const FHOLBuildingTypeRow* Type = Building ? Building->GetType() : nullptr;
		const bool bDeposit = Type && !Type->Accepts.IsEmpty() && Building->TakesDeliveries() && Building->GetOwnerPlayer() == PlayerIndex;
		// D485 (EtapeDefini L.5b): With Shift, the soldiers walk to the point after their own queue (L.3); the rest is the script's (carriers unload, the others walk).
		TArray<uint32> Workers(UnitIds);
		if (bDeposit && bQueue)
		{
			SendNonWorkersTo(World, *Commands, PlayerIndex, Workers, Point, /*bQueue*/ true);
		}
		if (bDeposit && !Workers.IsEmpty())
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Drop-off order of slot %d: %d units to %s (%.0f, %.0f)%s"), PlayerIndex, Workers.Num(),
				*Building->GetTypeName().ToString(), Building->GetActorLocation().X, Building->GetActorLocation().Y, bQueue ? TEXT(", queued") : TEXT(""));
		}
		const TConstArrayView<uint32> Ordered = bDeposit ? TConstArrayView<uint32>(Workers) : UnitIds;
		return Verdict(&World, TEXT("DropOff"), UHOLPlayerOrdersBoard::AskDropOff(&World, PlayerIndex, ToBridgeIds(Ordered), Building, Point, bQueue, bDeposit));
	}

	bool Place(UWorld& World, const int32 PlayerIndex, const FName TypeName, const FVector& Point, const TConstArrayView<uint32> UnitIds, const bool bQueue)
	{
		return Verdict(&World, TEXT("Place"), UHOLPlayerOrdersBoard::AskPlace(&World, PlayerIndex, TypeName, Point, ToBridgeIds(UnitIds), bQueue));
	}

	bool QueueProduction(UWorld& World, const int32 PlayerIndex, AHOLBuilding* Building, const FName TypeName)
	{
		return Verdict(&World, TEXT("QueueProduction"), UHOLPlayerOrdersBoard::AskQueueProduction(&World, PlayerIndex, Building, TypeName));
	}

	bool MarketTrade(UWorld& World, const int32 PlayerIndex, AHOLBuilding* Building, const int32 Resource, const int32 Lots, const bool bBuy)
	{
		return Verdict(&World, TEXT("MarketTrade"), UHOLPlayerOrdersBoard::AskMarketTrade(&World, PlayerIndex, Building, Resource, Lots, bBuy));
	}

	bool SetRally(UWorld& World, const int32 PlayerIndex, AHOLBuilding* Building, const FVector& Point, const int32 SourceIndex, AHOLBuilding* Site,
		const bool bQueue)
	{
		// A source that is not standing (any more) is the ground (as the controller's RPC): the source subsystem is C++ data, the choice is the script's.
		const UHOLSourceSubsystem* Sources = World.GetSubsystem<UHOLSourceSubsystem>();
		const bool bSourceStanding = Sources && Sources->GetSources().IsValidIndex(SourceIndex) && Sources->GetSources()[SourceIndex].IsStanding();
		return Verdict(&World, TEXT("SetRally"), UHOLPlayerOrdersBoard::AskSetRally(&World, PlayerIndex, Building, Point, SourceIndex, bSourceStanding, Site, bQueue));
	}
}

// F7 lots 4a / 4b (FortificationsInProgress.md, D693, D374): the garrison orders of a player. Retours/Q-garrison-ui.md Q2 (default A: the
// deposits before the garrison in AHOLPlayerController::OrderAt); 0: a right click on one's own tower is a plain move.
static TAutoConsoleVariable<int32> CVarHOLGarrisonRightClickEnters(TEXT("hol.Garrison.RightClickEnters"), 1,
	TEXT("F7 4b (Retours/Q-garrison-ui.md): 1 = a right click of units on one's own finished building with places is a garrison order; 0 = a move."));

namespace HOLPlayerOrders
{
	bool TakesGarrisonOrder(const AHOLBuilding& Building, const int32 PlayerIndex)
	{
		if (CVarHOLGarrisonRightClickEnters.GetValueOnAnyThread() == 0 || PlayerIndex < 0 || Building.GetOwnerPlayer() != PlayerIndex
			|| Building.NeedsBuilders())
		{
			return false;
		}
		return UHOLGarrisonSubsystem::GetSlots(Building) > 0;
	}

	bool Garrison(UWorld& World, const int32 PlayerIndex, const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building)
	{
		return Verdict(&World, TEXT("Garrison"), UHOLPlayerOrdersBoard::AskGarrison(&World, PlayerIndex, ToBridgeIds(UnitIds), Building));
	}

	bool UngarrisonAll(UWorld& World, const int32 PlayerIndex, AHOLBuilding* Building)
	{
		return Verdict(&World, TEXT("UngarrisonAll"), UHOLPlayerOrdersBoard::AskUngarrisonAll(&World, PlayerIndex, Building));
	}

	bool UngarrisonUnit(UWorld& World, const int32 PlayerIndex, const uint32 UnitId)
	{
		return false; // STUB (red test first)
	}
}