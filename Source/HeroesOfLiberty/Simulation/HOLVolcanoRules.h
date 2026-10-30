// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Where an event site stands (GDD 12.1, 12.2 bis; D28, D377-D380, D398-D400). */
enum class EHOLVolcanoPhase : uint8
{
	/** Catastrophe is off (D398): the site stays harmless, its rich deposits are just a zone to fight for (D399). */
	Dormant,
	/** A trigger window is open or about to: nothing visible yet. */
	Armed,
	/** The alert: 60 to 90 s, flows drawn on the ground, visible to all (D28, D377). */
	Warning,
	/** The lava advances along its flow (D377). */
	Erupting,
	/** The flow has stopped and is cooling: impassable (D379). */
	Cooling,
	/** Basalt, passable; the site rests before its next window (D400). */
	Rest,
};

/** Times of the volcano, in seconds. Indicative, tuned in test (V-Q1 of VolcanoInProgress.md is not yet answered: option A). */
struct FHOLVolcanoSettings
{
	/** The trigger window of a cycle opens this long after the match start (first cycle) or the previous re-arming. */
	float WindowOpensAfter = 360.f;
	float WindowLength = 180.f;
	/** Alert length drawn between these (D28). */
	float WarningMin = 60.f;
	float WarningMax = 90.f;
	/** The lava advances this long (placeholder until the flow's length and speed are known: V2). */
	float EruptionLength = 90.f;
	/** Cooling, impassable (D379), and rest after it (D400). */
	float CoolingMin = 120.f;
	float CoolingMax = 180.f;
	float RestMin = 360.f;
	float RestMax = 480.f;
};

/** Pure rules of one volcano site: the phase is a function of the match time and a seed, same on every machine. */
namespace HOLVolcano
{
	/** The dates of one cycle, in seconds since the match start. */
	struct FCycle
	{
		float WarningAt = 0.f;
		float EruptAt = 0.f;
		float EruptEndAt = 0.f;
		float CooledAt = 0.f;
		/** The window of the next cycle opens here (D400). */
		float ReArmAt = 0.f;
	};

	/** Cycle Index (0 first) of the site drawn with Seed; every cycle starts where the previous one re-armed. */
	HEROESOFLIBERTY_API FCycle MakeCycle(int32 Seed, int32 Index, float PreviousReArmAt, const FHOLVolcanoSettings& Settings);

	/** Phase at Now (seconds): always Dormant without the Catastrophe option (D398). */
	HEROESOFLIBERTY_API EHOLVolcanoPhase GetPhase(bool bCatastrophe, int32 Seed, float Now, const FHOLVolcanoSettings& Settings);
}
