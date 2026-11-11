// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * The deployable siege engine's pure rule (Palier3InProgress.md G5, GDD D712, D715): the C++ twin of Script/Siege/HOLDeployRules.as
 * (the simulation reads C++, the interface and the AI read the script, D483). The Trebuchet is PACKED (it walks, it does not fire),
 * DEPLOYED (it fires, it does not walk), or changing from one to the other (DEPLOYING, PACKING: no action, no defence); the order does
 * the work (an attack on a target in range deploys then fires, a walk order packs then walks) and a manual button toggles; a target
 * nearer than the MINIMUM RANGE cannot be aimed. Whole numbers only (milliseconds, centimetres, percent: D468 determinism), no world,
 * no unit, no position. The values are PROPOSED, not decided (Retours/Q-trebuchet-deploy.md), one line each.
 */
namespace HOLDeploy
{
	/** The four states of an engine that deploys. */
	enum class EPhase : uint8
	{
		Packed,
		Deploying,
		Deployed,
		Packing,
	};

	/** The orders that ask something of a deployable engine. */
	enum class EOrder : uint8
	{
		Attack,
		Move,
	};

	/** What the engine does now. */
	enum class EAction : uint8
	{
		/** A change of state is under way: the order waits for its end. */
		Wait,
		March,
		Deploy,
		Pack,
		Fire,
		/** Stop deploying: back to packed at once, the progress lost. */
		CancelDeploy,
		/** The order cannot be carried out; Reason says why. */
		Refuse,
	};

	/** The two delays (ms) and the range the engine can aim at (cm, both ends inside it). */
	struct FProfile
	{
		int32 DeployMs = 0;
		int32 PackMs = 0;
		int32 MinRangeCm = 0;
		int32 MaxRangeCm = 0;
	};

	/** Where an engine is: its phase and, during a change of state, how long it has lasted (ms). Small and fixed. */
	struct FState
	{
		EPhase Phase = EPhase::Packed;
		int32 ElapsedMs = 0;
	};

	/** What to do with an order: the action, why when it is refused, the phase right after the decision is applied. */
	struct FDecision
	{
		EAction Action = EAction::Wait;
		int32 Reason = 0;
		EPhase NextPhase = EPhase::Packed;
	};

	/** The reason of a refusal: the target is nearer than the minimum range. */
	constexpr int32 TooClose = 1;

	/** PROPOSED values (Q-trebuchet-deploy.md): deploying 8 s, packing 5 s, the target no nearer than 12 m nor farther than 50 m. */
	constexpr int32 ProposedDeployMs = 8000;
	constexpr int32 ProposedPackMs = 5000;
	constexpr int32 ProposedMinRangeCm = 1200;
	constexpr int32 ProposedMaxRangeCm = 5000;

	/** The phase at an index (0 to 3, in the order of the enum). */
	HEROESOFLIBERTY_API EPhase PhaseAt(int32 Index);

	/** The Trebuchet's profile (until the unit table carries the columns). */
	HEROESOFLIBERTY_API FProfile DefaultProfile();

	/** D712: only a deployed engine fires. */
	HEROESOFLIBERTY_API bool CanFire(const FState& State);

	/** D712: only a packed engine walks. */
	HEROESOFLIBERTY_API bool CanMove(const FState& State);

	/** D712: no action and no defence during a change of state. */
	HEROESOFLIBERTY_API bool CanAct(const FState& State);

	/** A target at this distance can be aimed: neither nearer than the minimum range nor farther than the maximum. */
	HEROESOFLIBERTY_API bool CanAim(const FProfile& Profile, int32 DistanceCm);

	/** A delay shortened by Counterweight (D715) by a percentage, 0 to 100 (outside it, the nearest end), never below nothing. */
	HEROESOFLIBERTY_API int32 Shortened(int32 BaseMs, int32 CounterweightPercent);

	HEROESOFLIBERTY_API int32 DeployDelayMs(const FProfile& Profile, int32 CounterweightPercent);
	HEROESOFLIBERTY_API int32 PackDelayMs(const FProfile& Profile, int32 CounterweightPercent);

	/** Time passes: a change of state ends when its delay has passed (the time left over is not carried); a settled state stays. */
	HEROESOFLIBERTY_API FState Advance(const FState& State, int32 DtMs, const FProfile& Profile, int32 CounterweightPercent);

	/** What an order asks of the engine now (D712). DistanceCm means nothing for a walk. */
	HEROESOFLIBERTY_API FDecision Decide(const FState& State, EOrder Order, int32 DistanceCm, const FProfile& Profile);

	/** The manual button, Mount / Dismount: deploys a packed engine, packs a deployed one, cancels a deploying one, waits on a packing one. */
	HEROESOFLIBERTY_API FDecision Toggle(const FState& State);

	/** The state once a decision is applied: the next phase, the change of state starting at zero. */
	HEROESOFLIBERTY_API FState Apply(const FState& State, const FDecision& Decision);
}
