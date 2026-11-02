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

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
	/** The target of each healer, kept while it stays healable and in reach (healer id -> unit id). */
	TMap<uint32, uint32> Kept;

	static TArray<FLink> Links;
	static double TotalHealed;
	static int64 HealSteps;
};
