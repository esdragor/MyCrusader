// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "HOLAvoidanceHoldProcessors.generated.h"

/**
 * The avoidance solver is the whole cost of a step at 2 000 units (D220), so it only runs one step out of
 * several - and an avoidance impulse applied one step in six and forgotten the next is not avoidance at all:
 * two units crossing at right angles walk through each other (0.9 to 5.2 cm apart at one run in two, three or
 * six, against 65.9 cm with the solver running every step; scenario C). Mass does not keep the force either:
 * UMassApplyForceProcessor resets the force fragment to zero after applying it. These two processors keep the
 * last avoidance impulse alive between the solver's runs: continuity without paying for the solver (D230).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLAvoidanceSnapshotProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLAvoidanceSnapshotProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

/**
 * Remembers the impulse on the step the solver runs, then re-applies it on exactly AvoidanceHoldSteps of the
 * following steps, the total capped at the type's MaxAcceleration as the solver caps its own result.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLAvoidanceHoldProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLAvoidanceHoldProcessor();

	/** Re-applications after each solver run, console override included (D230); 0 = off. */
	static int32 GetHoldSteps();
	/** Factor applied to the held impulse after each re-application, console override included (D230). */
	static float GetHoldDecay();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	/** Appends the traced units' rows of this step to Saved/AvoidanceTrace.csv (hol.Units.AvoidanceTrace). */
	void WriteTraceRows(TArray<FString>& Rows);

	FMassEntityQuery EntityQuery;
	bool bTraceFileStarted = false;
};
