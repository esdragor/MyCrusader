// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AHOLBuilding;
class UHOLCommandSubsystem;
class UWorld;

/**
 * A player's orders on its units (IA0.2): the limits every order of a slot meets before it enters the queue
 * (UHOLCommandSubsystem), whoever gives it: a human through its controller's RPCs (AHOLPlayerController::ServerXxx) or an
 * AI slot's brain on the server (UHOLAIOrderLibrary), which has no controller. Server and solo only (the queue's world).
 *
 * The rules: the order names a player's slot (0 to MaxSlots - 1: never INDEX_NONE, the server's own orders, which every
 * unit obeys), at most MaxOrderedUnits units, the chains drawn (FPlayerOrderScope, D328); an order to gather or to build
 * sends the slot's units that do not work (the soldiers) walking to its point instead (J7a.2). The queue and the order
 * processors check the rest at the next step: the units' owner (D288), a defeated slot (D358), the target.
 *
 * Each function returns whether the order entered the queue (false: refused here; the queue may still drop it, D358).
 */
namespace HOLPlayerOrders
{
	/** Units of one order at most: a player's whole population (D03), a client could send more. */
	inline constexpr int32 MaxOrderedUnits = 200;

	/** Whether PlayerIndex is a player's slot and NumUnits fits one order (the cap above). */
	HEROESOFLIBERTY_API bool IsAccepted(int32 PlayerIndex, int32 NumUnits);

	/**
	 * The units that neither gather nor build (CarryCapacity 0: the soldiers, J7a.2) taken out of an order to gather or to
	 * build: they walk to its point instead (the user's choice, J7a.2), as a move order of the player's (D288). Only
	 * PlayerIndex's own: another's stay in the list and are refused by the order's processor. bQueue (Shift): their walk
	 * goes at the end of their queue of orders.
	 */
	HEROESOFLIBERTY_API void SendNonWorkersTo(UWorld& World, UHOLCommandSubsystem& Commands, int32 PlayerIndex, TArray<uint32>& InOutUnitIds,
		const FVector& Point, bool bQueue = false);

	/** Go there (D286, D287); FrontLine and bFrontWidth: a drag's arrow (J7a.4). */
	HEROESOFLIBERTY_API bool Move(UWorld& World, int32 PlayerIndex, TConstArrayView<uint32> UnitIds, const FVector& Target, bool bQueue = false,
		const FVector2D& FrontLine = FVector2D::ZeroVector, bool bFrontWidth = false);
	/** Attack a unit (J7b.3). */
	HEROESOFLIBERTY_API bool Attack(UWorld& World, int32 PlayerIndex, TConstArrayView<uint32> UnitIds, uint32 TargetUnitId, bool bQueue = false);
	/** Attack a building (D345.2b). */
	HEROESOFLIBERTY_API bool AttackBuilding(UWorld& World, int32 PlayerIndex, TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, bool bQueue = false);
	/** Attack-move (J7b.3). */
	HEROESOFLIBERTY_API bool AttackMove(UWorld& World, int32 PlayerIndex, TConstArrayView<uint32> UnitIds, const FVector& Target, bool bQueue = false);
	/** Stop, or hold the position (J7b.3). */
	HEROESOFLIBERTY_API bool Stop(UWorld& World, int32 PlayerIndex, TConstArrayView<uint32> UnitIds, bool bHold);
	/** Gather a source (J5, D296), by its index in UHOLSourceSubsystem::GetSources; the soldiers walk to it. */
	HEROESOFLIBERTY_API bool Gather(UWorld& World, int32 PlayerIndex, TConstArrayView<uint32> UnitIds, int32 SourceIndex, bool bQueue = false);
	/** Build or repair a building (J6a.4, D312); the soldiers walk to it. */
	HEROESOFLIBERTY_API bool Build(UWorld& World, int32 PlayerIndex, TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, bool bQueue = false);
	/**
	 * A right click on one of the player's finished buildings that takes deliveries (D485, EtapeDefini L): the units carrying
	 * a resource it takes unload there, then stand idle; the others (nothing taken carried, soldiers) walk to Point. The
	 * sorting is the server's: a client does not know each unit's load. Not the player's, or no deposit: all walk to Point.
	 * With bQueue (Shift, L.3): every worker gets the deposit at the end of its queue (its load judged at its turn): a
	 * builder with a queue of work in that queue, the others in their queue of moves (L.5b); the soldiers a walk to Point
	 * at the end of theirs. All go through the queue of moves (EHOLOrderKind::DropOff), sorted there by the server.
	 */
	HEROESOFLIBERTY_API bool DropOff(UWorld& World, int32 PlayerIndex, TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const FVector& Point,
		bool bQueue = false);
	/**
	 * Place a foundation (J6a.3, D311): the slot pays and owns it; its builders are UnitIds, none above the cap (the
	 * foundation is placed all the same).
	 */
	HEROESOFLIBERTY_API bool Place(UWorld& World, int32 PlayerIndex, FName TypeName, const FVector& Point, TConstArrayView<uint32> UnitIds, bool bQueue = false);
}
