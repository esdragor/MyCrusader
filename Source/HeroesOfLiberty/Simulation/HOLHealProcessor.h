// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "HOLHealProcessor.generated.h"

/**
 * The Monk's heal (Q-chapelle-moine = A), on the server and in solo: after the combat of the step, each unit whose type has a HealRate
 * (FHOLUnitTypeConstants::HealRate) heals one ally at a time within its HealRange (HOLHeal::ChooseTarget: never itself, never a King or a
 * siege engine, never a dying unit), HealRate health points per second, held at the target's maximum. Units are taken in identifier order,
 * so the outcome does not depend on the chunks. The target a healer keeps from step to step is remembered here (not replicated: a client
 * sees the health only).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLHealProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLHealProcessor();

	/** A heal of the last step: who heals whom (the presentation's line, the report). */
	struct FLink
	{
		uint32 HealerId = 0;
		uint32 TargetId = 0;
	};
	/** The heals of the last step, in healer order (solo and host only: the simulation runs here). */
	static const TArray<FLink>& GetLinks() { return Links; }
	/** Health points healed since the program started. */
	static double GetTotalHealed() { return TotalHealed; }
	/** Steps with at least one heal since the program started: a Monk healing its allies one after the other counts each step once per Monk. */
	static int64 GetHealSteps() { return HealSteps; }

	/**
	 * The player's order (a right click on an ally with Monks selected, ServerHealUnit): Healer heals TargetId first, whatever is nearer,
	 * while it is healable and within its HealRange, for OrderSteps steps at most or until it is healed; one order per healer, a new one
	 * replaces it. Server and solo only (the simulation runs there).
	 */
	static void OrderHeal(uint32 HealerId, uint32 TargetId);
	static void ClearOrders();
	/**
	 * Forgets the heal orders of these units: another order of their player (a move, an attack, a stop...) replaces the heal order, as
	 * the walk it gave is replaced. Not while this processor gives its own walks (IsIssuing).
	 */
	static void CancelOrders(TConstArrayView<uint32> UnitIds);
	/** True while the processor gives a healer its walk or its stop (the command subsystem then keeps the heal order). */
	static bool IsIssuing() { return bIssuing; }
	/** The order lasts this many steps (30 s at 20 Hz). */
	static constexpr int64 OrderSteps = 600;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
	/** The target of each healer, kept while it stays healable and in reach (healer id -> unit id). */
	TMap<uint32, uint32> Kept;

	struct FOrder
	{
		uint32 Target = 0;
		/** The step it was first seen at (-1: not yet). */
		int64 Step = -1;
		/** The order has been looked at once (the first look ends any earlier walk of the healer). */
		bool bStarted = false;
		/** A walk this order gave is on, towards Goal (the target's place when it was given). */
		bool bWalking = false;
		FVector2D Goal = FVector2D::ZeroVector;
	};
	static bool bIssuing;
	static TMap<uint32, FOrder> Orders;

	static TArray<FLink> Links;
	static double TotalHealed;
	static int64 HealSteps;
};
