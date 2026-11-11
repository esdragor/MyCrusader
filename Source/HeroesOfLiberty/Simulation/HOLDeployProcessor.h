// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "HOLDeployProcessor.generated.h"

/**
 * The deployment state of one deployable siege engine (Palier3InProgress.md G6, GDD D712): small and fixed. Phase and ElapsedMs are
 * HOLDeploy::FState (HOLDeployRules.h) stored as numbers; an order is left here by the command side (PendingOrder, PendingDistanceCm)
 * and read once by UHOLDeployProcessor, which writes what it decided (LastAction, LastReason). Only a unit that carries this fragment
 * deploys: which unit types carry it is not decided yet (Retours/Q-palier3-g4-roles.md, Q-trebuchet-deploy.md).
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLDeployFragment : public FMassFragment
{
	GENERATED_BODY()

	/** HOLDeploy::EPhase. */
	uint8 Phase = 0;
	/** How long a change of state has lasted (ms). */
	int32 ElapsedMs = 0;
	/** The order waiting to be read: 0 none, 1 attack, 2 move (HOLDeploy::EOrder + 1), 3 the Mount / Dismount button (HOLDeploy::Toggle). */
	uint8 PendingOrder = 0;
	/** The distance (cm) to the target of a pending attack. */
	int32 PendingDistanceCm = 0;
	/** What the last order was decided to be (HOLDeploy::EAction) and why it was refused (HOLDeploy::TooClose). */
	uint8 LastAction = 0;
	int32 LastReason = 0;
	/** Counterweight (D715): the percentage by which the two delays are shortened. */
	int32 CounterweightPercent = 0;
};

/**
 * Whether the Mangonel stands in for the deployable engine (cvar hol.Siege.Deployable, default 0): the unit subsystem gives every Mangonel
 * born while it is 1 a FHOLDeployFragment (Retours/Q-palier3-deploy-wiring.md, Q1 = A, the default coded).
 */
HEROESOFLIBERTY_API bool HOLDeployStandInEnabled();

/**
 * Carries the deployment of every unit with a FHOLDeployFragment through the fixed step: the order left by the command side is decided
 * by HOLDeploy::Decide, applied by HOLDeploy::Apply, then the step's time passes through HOLDeploy::Advance. Server and solo only.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLDeployProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLDeployProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
