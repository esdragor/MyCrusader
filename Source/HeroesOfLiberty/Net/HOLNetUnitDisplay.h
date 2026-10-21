// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "Net/HOLUnitReplicationRules.h"
#include "HOLNetUnitDisplay.generated.h"

/**
 * What a client knows of a unit it displays (D271): the last state received, the full step it was taken at, and the
 * correction being smoothed. Only on the display entities of a client; the solo game's units do not have it (D272).
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLNetUnitDisplayFragment : public FMassFragment
{
	GENERATED_BODY()

	/** A state older than this is no longer walked on: the unit waits for the next one (lost updates, D269's refresh). */
	static constexpr double MaxPredictionSeconds = 3.0;

	FHOLNetUnitState State;
	int64 StateStep = 0;
	FHOLDisplayCorrection Correction;
	/** Height (cm) of the ground under State's position, found by the client (D301: the height is not sent). */
	double GroundZ = 0.0;
	/** Horizontal velocity of State, for the walk animation (D234). */
	FVector Velocity = FVector::ZeroVector;
	/** Facing (deg), turned towards the velocity: the orientation is not sent. */
	float Yaw = 0.f;
	/**
	 * A shooter's arrows deduced by this client (J7c.2b2, Q4, 0 bit more): the server's cadence replayed on the clock from
	 * the step its Attacking state was taken at (HOLCombat::StartAttack, TickAttack). ShotStep: the last step replayed,
	 * -1 while not shooting; ShotCooldownMs: the replayed cool-down, kept while it does not shoot (J7c, retour d'essai 8:
	 * hit and run): it then runs down from ShotIdleStep, the step it was last replayed at (-1 before its first shot), as
	 * the server's (HOLCombat::TickIdle), so a shooter that walked off and stops again looses when the server's does.
	 * ShotSecondsToHit: while shooting, the time from this frame to its next replayed release (its gesture's phase).
	 */
	int64 ShotStep = -1;
	int32 ShotCooldownMs = 0;
	int64 ShotIdleStep = -1;
	float ShotSecondsToHit = 0.f;

	/**
	 * Starts replaying its shots from its Attacking state of AttackStep (that step already ticked on the server): the
	 * cool-down left from its last shots, run down over the steps between (ShotIdleStep + 1 to AttackStep - 1, idle on
	 * the server), 0 before its first shot; then HOLCombat::StartAttack (the wind-up at least) less that step's tick.
	 */
	void StartShots(int64 AttackStep, int32 StepMs, int32 IntervalMs, int32 WindUpPercent);

	/** Stops replaying its shots (no longer Attacking): the cool-down is kept, held at the last step replayed. */
	void StopShots();

	/** Where the unit is drawn at LocalSeconds, the server clock then being at ClockStep (D271). */
	FVector GetDisplayedPosition(const double ClockStep, const double LocalSeconds, const float StepSeconds) const
	{
		const double Elapsed = FMath::Clamp(ClockStep - static_cast<double>(StateStep), -MaxPredictionSeconds / StepSeconds, MaxPredictionSeconds / StepSeconds);
		return HOLUnitReplication::PredictPosition(State, Elapsed, StepSeconds) + FVector(0.0, 0.0, GroundZ) + Correction.GetOffset(LocalSeconds);
	}
};

/** Adds the network display fragment to the display template of a unit type (UHOLUnitSubsystem::FindTemplate). */
UCLASS(meta = (DisplayName = "HeroesOfLiberty Net Unit Display"))
class HEROESOFLIBERTY_API UHOLNetUnitDisplayTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

/**
 * Client, every frame (D271): puts each display entity where the server has it now (last state walked on to the
 * estimated server step, plus the fading correction) and turns it towards its walk. The drawing of M2 then reads the
 * transform as it reads a simulated unit's (D272: one display code).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLNetUnitDisplayProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLNetUnitDisplayProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
