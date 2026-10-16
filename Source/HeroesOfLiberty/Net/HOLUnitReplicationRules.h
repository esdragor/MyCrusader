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
	/** The activity, and while it is Gathering what it gathers (J5c.1: HOLUnitReplication::EncodeActivity, no bit more). */
	static constexpr int32 ActivityBits = 3;
	/** Low bits of the step the state was taken at: 256 steps, 12.8 s at 20 Hz, unwrapped by the client (UnwrapStep). */
	static constexpr int32 StepBits = 8;
	/**
	 * Load carried (J5, D298): one bit says whether the unit carries something, then only a carrier writes the resource
	 * (what J5c will show it carrying): a soldier pays one bit. The amount is not sent to everyone: only the owner's unit
	 * selected alone has it, at every change (D303, AHOLPlayerController's watched unit).
	 */
	static constexpr int32 LoadResourceBits = 2;
	/** Bits of a unit that carries nothing; a carrier writes CarriedLoadBits more. */
	static constexpr int32 TotalBits = 2 * HorizontalBits + 2 * VelocityBits + ActivityBits + StepBits + 1;
	static constexpr int32 CarriedLoadBits = LoadResourceBits;

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

	/** Reduces a state; values out of range are held at the range's edge. */
	static FHOLNetUnitState Make(const FVector& Position, const FVector& Velocity, EHOLUnitActivity Activity, int64 Step,
		EHOLResource LoadResource = EHOLResource::Food, int32 LoadAmount = 0, EHOLResource GatherResource = EHOLResource::Food);

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

	/** The activity as written on the wire, with what it gathers while Gathering. */
	HEROESOFLIBERTY_API uint8 EncodeActivity(EHOLUnitActivity Activity, EHOLResource GatherResource);

	/** Back from the wire; GatherResource is Food unless the activity is Gathering. */
	HEROESOFLIBERTY_API void DecodeActivity(uint8 WireActivity, EHOLUnitActivity& OutActivity, EHOLResource& OutGatherResource);

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
