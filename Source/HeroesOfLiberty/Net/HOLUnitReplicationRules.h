// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLSimulationFragments.h"
#include "HOLUnitReplicationRules.generated.h"

/**
 * Pure rules of the unit replication (M2.5): the reduced state of a unit (D269, D270), the send rule the server
 * applies to it (D269), and what the client does with it (D271: server clock, prediction, smoothed corrections).
 * The server and the client predict a unit with the same function from the same reduced values, so the server's
 * send rule measures exactly the error the client would show. No engine state: tested on their own.
 */

/**
 * State of one unit as sent (D269, D270), in the integers the element format writes with the exact bit widths below
 * (FHOLNetUnitStateNetSerializer). Horizontal only (D301): units walk on the ground, the client puts them on it itself
 * (UHOLUnitReceptionSubsystem::GetGroundHeight), so neither their height nor a vertical velocity is sent, and the send
 * rule measures the horizontal error. (Units standing on ramparts, D23, will need a bit "off the ground" and their
 * height.)
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLNetUnitState
{
	GENERATED_BODY()

	/** Position precision (cm): ~10 cm (D269). */
	static constexpr float PositionStep = 10.f;
	/** Velocity precision (cm/s): at most 2.5 cm/s off per axis, ~5 cm of drift over a 2 s refresh. */
	static constexpr float VelocityStep = 5.f;

	/** X and Y: +-1 638 m around the map's origin. */
	static constexpr int32 HorizontalBits = 15;
	/** Horizontal velocity, per axis: +-12.8 m/s (a walking unit goes at 3 m/s). */
	static constexpr int32 VelocityBits = 9;
	/**
	 * The activity, and while it is Gathering what it gathers (J5c.1: HOLUnitReplication::EncodeActivity, no bit more).
	 * 4 bits since J7b.4 (J7-Q5): values 8 to 10 are Attacking, Dying and "idle, facing its front"; 5 are left free.
	 */
	static constexpr int32 ActivityBits = 4;
	/** Low bits of the step the state was taken at: 256 steps, 12.8 s at 20 Hz, unwrapped by the client (UnwrapStep). */
	static constexpr int32 StepBits = 8;
	/**
	 * Load carried (J5, D298): one bit says whether the unit carries something, then only a carrier writes the resource
	 * (what J5c will show it carrying): a soldier pays one bit. The amount is not sent to everyone: only the owner's unit
	 * selected alone has it, at every change (D303, AHOLPlayerController's watched unit).
	 */
	static constexpr int32 LoadResourceBits = 2;
	/**
	 * Health (J7b.4, J7-Q5 revised): one bit "full health", then only a wounded unit writes its exact health points
	 * (HOLUnitReplication::GetWireHealth: 1 to MaxHealth - 1 alive, 0 dead). A change of health is a change of action
	 * (sent at once; the healing will come through the same field). A wounded unit has at most MaxHealth - 1, so 8 bits
	 * carry every maximum up to MaxWireHealth + 1 = 256 (the table's largest: 155, the Man-at-arms); the unit table's
	 * loading checks it (HOLUnitReplication::FitsWireHealth). Back to levels: HealthBits, GetWireHealth and
	 * HOLUnitView::GetReceivedHealth.
	 */
	static constexpr int32 HealthBits = 8;
	static constexpr uint16 MaxWireHealth = (1 << HealthBits) - 1;
	/**
	 * Target of an Attacking unit (J7b.4, J7-Q5), written only with that activity: its replication slot (the chunks'
	 * slots, whose identifier the client reads), cheaper than its identifier, which is never recycled. 4 095 slots
	 * (0 to 4 094, 65 chunks), beyond the population caps; InvalidTargetSlot: a target not replicated yet (sent again
	 * once it is), or in a slot past those (from the 66th chunk on): its attacker is then not turned to it (HasTargetSlot).
	 */
	static constexpr int32 TargetSlotBits = 12;
	static constexpr uint16 InvalidTargetSlot = (1 << TargetSlotBits) - 1;
	/**
	 * Facing of a unit at rest at its formation place (J7b.4, deferred item of J7a.4, choice B): written only with the
	 * activity "idle, facing its front", 64 directions (5.6 deg).
	 */
	static constexpr int32 FrontYawBits = 6;
	/** Bits of an unhurt unit that carries nothing and does not fight (J7-Q5: 62); the others write the extras below. */
	static constexpr int32 TotalBits = 2 * HorizontalBits + 2 * VelocityBits + ActivityBits + StepBits + 1 + 1;
	static constexpr int32 CarriedLoadBits = LoadResourceBits;
	static constexpr int32 WoundedBits = HealthBits;
	static constexpr int32 AttackingBits = TargetSlotBits;
	static constexpr int32 FacingFrontBits = FrontYawBits;

	UPROPERTY()
	int16 X = 0;
	UPROPERTY()
	int16 Y = 0;
	UPROPERTY()
	int16 VelocityX = 0;
	UPROPERTY()
	int16 VelocityY = 0;
	UPROPERTY()
	EHOLUnitActivity Activity = EHOLUnitActivity::Idle;
	/** What it gathers while Activity is Gathering (J5c.1), Food otherwise: written in the activity's bits. */
	UPROPERTY()
	EHOLResource GatherResource = EHOLResource::Food;
	/** StepBits low bits of the simulation step the state was taken at. */
	UPROPERTY()
	uint8 Step = 0;

	/** Whether it carries something, and what (J5, D298). Not a reason to send the unit (D269). */
	UPROPERTY()
	bool bCarrying = false;

	UPROPERTY()
	EHOLResource LoadResource = EHOLResource::Food;

	/** Unhurt (J7b.4): Health is not sent and stays 0. */
	UPROPERTY()
	bool bFullHealth = true;

	/** Its health points while wounded (HOLUnitReplication::GetWireHealth, 0 dead), 0 unhurt. */
	UPROPERTY()
	uint16 Health = 0;

	/** While Activity is Attacking: its target's slot (see TargetSlotBits); 0 otherwise. */
	UPROPERTY()
	uint16 TargetSlot = 0;

	/** Idle at its formation place, held turned to its front (J7a.4's bFacesFront): FrontYaw is sent. False otherwise. */
	UPROPERTY()
	bool bFacesFront = false;

	/** Its front, in 64ths of a turn (GetFrontYaw), while bFacesFront; 0 otherwise. */
	UPROPERTY()
	uint8 FrontYaw = 0;

	/** Reduces a state; values out of range are held at the range's edge. */
	static FHOLNetUnitState Make(const FVector& Position, const FVector& Velocity, EHOLUnitActivity Activity, int64 Step,
		EHOLResource LoadResource = EHOLResource::Food, int32 LoadAmount = 0, EHOLResource GatherResource = EHOLResource::Food);

	/**
	 * The fight and facing of the state (J7b.4), after Make: health (InHealth of MaxHealth), the target's slot while
	 * Attacking, the front (deg) while Idle with bInFacesFront. Values that do not apply are left at their defaults, so
	 * equal states compare equal.
	 */
	void SetCombat(int32 InHealth, int32 MaxHealth, uint16 InTargetSlot, bool bInFacesFront = false, float InFrontYawDegrees = 0.f);

	/** Attacking a known target: TargetSlot is a real slot, not InvalidTargetSlot (itself a slot from the 66th chunk on). */
	bool HasTargetSlot() const { return Activity == EHOLUnitActivity::Attacking && TargetSlot != InvalidTargetSlot; }

	/** Front (deg, -180 to 180) of a state that faces it. */
	float GetFrontYaw() const;

	/** Z = 0 for both, see above. */
	FVector GetPosition() const;
	FVector GetVelocity() const;

	bool operator==(const FHOLNetUnitState& Other) const = default;
};

/** Why the server sends a unit at this step (D269). */
enum class EHOLUnitSendReason : uint8
{
	None,
	/** Its action changed (starts walking, stops...): sent at once, whatever the error. */
	Activity,
	/** The position the client predicts is past the threshold. */
	Error,
	/** Slow refresh: nothing sent for a while, catches what was lost. */
	Refresh,
};

/** Values of the send rule (D269), tuned in S5. */
struct FHOLUnitSendRules
{
	/** Prediction error (cm) past which a unit is sent again (D274: 35 cm). */
	float ErrorThreshold = 35.f;
	/** Cycle of the slow refresh: 40 steps = 2 s at 20 Hz (see GetSendReason). */
	int32 RefreshSteps = 40;
};

/** What the server keeps of the last state it sent of one unit. */
struct FHOLUnitSentRecord
{
	FHOLNetUnitState State;
	/** Full step of State (State.Step only has its low bits). */
	int64 Step = 0;
};

namespace HOLUnitReplication
{
	/**
	 * First value of "gathering this resource" on the wire (J5c.1, option A of the user): the activity's 3 bits carry
	 * Idle, Moving and GatherTrip as themselves and Gathering as GatheringWireBase + its resource, 4 to 7, the four values
	 * the activities left free. The client knows what is gathered as soon as the gathering starts, at no bit more.
	 */
	constexpr uint8 GatheringWireBase = 4;

	/**
	 * Building (J6a.4) on the wire: value 2, Gathering's own value, never written since J5c.1. It was the last free value of
	 * the activity's 3 bits: one more activity needs a bit more.
	 */
	constexpr uint8 BuildingWireValue = static_cast<uint8>(EHOLUnitActivity::Gathering);

	/** Attacking (J7b.4) on the wire. */
	constexpr uint8 AttackingWireValue = 8;
	/** Dying (J7b.4) on the wire. */
	constexpr uint8 DyingWireValue = 9;
	/** Idle and facing its front at its formation place (J7b.4): Idle, with FrontYaw written. */
	constexpr uint8 IdleFacingFrontWireValue = 10;

	/**
	 * Health of a wounded unit as sent (J7-Q5 revised): its health points, 0 only at 0 health or below, held at
	 * MaxWireHealth (never reached by a table that FitsWireHealth).
	 */
	HEROESOFLIBERTY_API uint16 GetWireHealth(int32 Health, int32 MaxHealth);

	/** Every wounded health of a type of this maximum fits the wire (MaxHealth - 1 <= MaxWireHealth). */
	constexpr bool FitsWireHealth(const int32 MaxHealth) { return MaxHealth - 1 <= FHOLNetUnitState::MaxWireHealth; }

	/** The activity is one the wire can carry (EncodeActivity). */
	HEROESOFLIBERTY_API bool IsValidActivity(EHOLUnitActivity Activity);

	/** The activity as written on the wire, with what it gathers while Gathering, and whether it faces its front while Idle. */
	HEROESOFLIBERTY_API uint8 EncodeActivity(EHOLUnitActivity Activity, EHOLResource GatherResource, bool bFacesFront = false);

	/** Back from the wire; GatherResource is Food unless the activity is Gathering. */
	HEROESOFLIBERTY_API void DecodeActivity(uint8 WireActivity, EHOLUnitActivity& OutActivity, EHOLResource& OutGatherResource);
	HEROESOFLIBERTY_API void DecodeActivity(uint8 WireActivity, EHOLUnitActivity& OutActivity, EHOLResource& OutGatherResource, bool& bOutFacesFront);

	/**
	 * The action of two states is the same (D269, J7b.4): activity on the wire (with what is gathered, the facing of the
	 * front), health, target and front. Another action is sent at once, whatever the position.
	 */
	HEROESOFLIBERTY_API bool IsSameAction(const FHOLNetUnitState& A, const FHOLNetUnitState& B);

	/**
	 * A unit doing Activity (as on the wire: Attacking, Dying over its simulation activity) is sent with its velocity (J7c.4,
	 * S5 in combat) only while it walks (IsWalkingActivity: Moving, GatherTrip). The others stand (an attacker stops its
	 * walk once in reach, UHOLCombatProcessor::StopWalk; a gatherer, a builder at their spot; a dying unit stopped dead):
	 * their step's move is only the separation's pushes (D292), jitter a client would extrapolate and the server resend
	 * at the threshold. Sent still, a stander drifting by pushes is sent again once it really drifted past the threshold.
	 */
	HEROESOFLIBERTY_API bool SendsVelocity(EHOLUnitActivity Activity);

	/** Full step whose low bits are StepLowBits, the nearest to NearStep (the client's estimate of the server clock). */
	HEROESOFLIBERTY_API int64 UnwrapStep(uint8 StepLowBits, int64 NearStep);

	/** Where the unit of State is StepsElapsed steps after it was taken, walking in a straight line (D269, D271). */
	HEROESOFLIBERTY_API FVector PredictPosition(const FHOLNetUnitState& State, double StepsElapsed, float StepSeconds);

	/**
	 * Send rule of the server (D269), at step Step for a unit at Position doing Activity, last sent as Sent. The slow
	 * refresh comes at the unit's own step of the cycle, set by its identifier, so units sent together (born together,
	 * stopped by the same order) are refreshed spread over the cycle and never all at one step; it is skipped when the
	 * unit was sent less than a quarter of a cycle before. A unit at rest is thus sent once a cycle, and no unit waits
	 * more than a cycle and a quarter (2.5 s). GatherResource, what it gathers while Activity is Gathering, is part of
	 * the activity: another resource is sent at once too (J5c.1).
	 */
	HEROESOFLIBERTY_API EHOLUnitSendReason GetSendReason(const FHOLUnitSentRecord& Sent, const FVector& Position,
		EHOLUnitActivity Activity, uint32 UnitId, int64 Step, float StepSeconds, const FHOLUnitSendRules& Rules,
		EHOLResource GatherResource = EHOLResource::Food);

	/** The same rule, the unit's present action being that of Current (IsSameAction: its health, target, front, J7b.4). */
	HEROESOFLIBERTY_API EHOLUnitSendReason GetSendReason(const FHOLUnitSentRecord& Sent, const FVector& Position,
		const FHOLNetUnitState& Current, uint32 UnitId, int64 Step, float StepSeconds, const FHOLUnitSendRules& Rules);
}

/**
 * The client's estimate of the server's simulation clock (D271), in steps. Each replicated server step, received with
 * the one-way delay of the connection (half the round trip), gives a sample; the estimate follows the samples slowly
 * so the units do not jerk. A far earlier sample (first sample, a burst of samples lost) is taken as is. A far later
 * one is the first step read after a hitch (S5: a client frame of half a second pulled the clock back 11 steps): it
 * is ignored, unless the shift lasts (the server lost time). The samples are late by the time between the step's end
 * and the send, but the round trip, which counts the frames' waits, overestimates the delay by about as much: S5
 * measured the plain average a tenth of a step off, and following the earliest samples half a step ahead.
 */
struct HEROESOFLIBERTY_API FHOLServerClock
{
	/** Share of the gap to a sample caught up per sample. */
	static constexpr double Blend = 0.1;
	/** A sample further off than this (s) is far: taken as is if earlier, ignored if later unless the shift lasts. */
	static constexpr double SnapSeconds = 0.25;
	/** Far later samples in a row after which the shift is taken (~1 s at 10 updates per second). */
	static constexpr int32 LateSamplesToFollow = 10;

	void Reset(float InStepSeconds);
	void AddSample(int64 ServerStep, double LocalSeconds, double OneWaySeconds);
	bool IsValid() const { return bValid; }
	/** Server step (with its fraction) estimated at LocalSeconds. */
	double GetStep(double LocalSeconds) const;

private:
	float StepSeconds = 0.05f;
	/** Local time minus server time (s). */
	double Offset = 0.0;
	int32 NumFarLateSamples = 0;
	bool bValid = false;
};

/**
 * Smoothed correction of a displayed unit (D271): when a new state arrives, the gap between where the unit is drawn and
 * where the new state puts it is kept as an offset that fades out over SmoothingSeconds, instead of a jump.
 */
struct HEROESOFLIBERTY_API FHOLDisplayCorrection
{
	/** ~0.2 s (D271). */
	static constexpr float SmoothingSeconds = 0.2f;
	/** A gap wider than this (cm) is not smoothed: the unit is put at its place at once. */
	static constexpr float SnapDistance = 500.f;

	/** Starts a correction at LocalSeconds from where the unit is drawn to where the new state predicts it. */
	void Start(const FVector& Displayed, const FVector& Predicted, double LocalSeconds);
	/** Offset to add to the predicted position at LocalSeconds: fades linearly to zero. */
	FVector GetOffset(double LocalSeconds) const;

private:
	FVector Offset = FVector::ZeroVector;
	double StartSeconds = 0.0;
};
