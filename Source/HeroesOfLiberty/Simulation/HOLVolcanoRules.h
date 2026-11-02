// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMatchConfig.h"

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
	/**
	 * The lava advances this long when no flow is given (placeholder of V1); with a flow, its length over the lava's speed
	 * (HOLVolcano::EruptionDuration) is used.
	 */
	float EruptionLength = 90.f;
	/** The lava walks at a Man-at-arms' pace less 30 % (V-Q4 = A, D575): ~488 cm/s with the table's 697. */
	float LavaSpeedFactor = 0.7f;
	/** Cooling, impassable (D379), and rest after it (D400). */
	float CoolingMin = 120.f;
	float CoolingMax = 180.f;
	float RestMin = 360.f;
	float RestMax = 480.f;
	/**
	 * Half the width of a flow (cm): the lava destroys what is within this of the line from the crater to its front (and a unit's or a
	 * building's own radius more). V5-Q1 = B, the user's answer (2026-11-02): 800 cm, a flow 16 m wide.
	 */
	float LavaHalfWidth = 800.f;

	/** The lava's speed (cm/s) for the Man-at-arms' speed read from the unit type table (never typed here). */
	float LavaSpeed(const float ManAtArmsSpeed) const { return ManAtArmsSpeed * LavaSpeedFactor; }
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

	/** One flow of lava (D377): a polyline on the ground, from the crater to the plain (cm). The front advances along it. */
	struct FFlow
	{
		TArray<FVector> Points;

		/** The length of the polyline (cm), the ground plane only (Z ignored); 0 with fewer than two points. */
		HEROESOFLIBERTY_API float Length() const;
	};

	/** How long the lava takes to reach the end of a flow (s); 0 for a flow without length or a speed that is not positive. */
	HEROESOFLIBERTY_API float EruptionDuration(const FFlow& Flow, float Speed);

	/**
	 * How far the front has gone along the flow (cm) at Now: 0 before EruptAt, Speed x (Now - EruptAt) after, never beyond the
	 * flow's end and never backward.
	 */
	HEROESOFLIBERTY_API float FrontDistance(const FFlow& Flow, float Now, float EruptAt, float Speed);

	/**
	 * V5 (D377): whether something at Point (ground plane, Z ignored) with a body of Radius is under the lava of a flow whose front has
	 * gone FrontDistance (cm) from the crater: its distance to the part of the polyline already run (the head of the front rounded) is
	 * within HalfWidth + Radius. False while the front is still at the crater (before the eruption), for a flow without length.
	 */
	HEROESOFLIBERTY_API bool IsInLava(const FFlow& Flow, float FrontDistance, float HalfWidth, const FVector& Point, float Radius = 0.f);

	/** One oriented rectangle on the ground (cm): the navmesh takes boxes. */
	struct FObstacleBox
	{
		FVector2D Centre = FVector2D::ZeroVector;
		/** Half the length along the segment, and half the width across it. */
		float HalfLength = 0.f;
		float HalfWidth = 0.f;
		/** Turn of the segment about the vertical (degrees), 0 = along +X. */
		float YawDegrees = 0.f;
	};

	/**
	 * V6 (D379): the ground a cooling flow forbids (impassable ~2-3 min): one box per segment of the polyline, HalfWidth either side, and
	 * HalfWidth beyond each end so the corners and the flow's end are covered (the lava's own reach, IsInLava, is inside the union).
	 */
	HEROESOFLIBERTY_API TArray<FObstacleBox> GetObstacleBoxes(const FFlow& Flow, float HalfWidth);

	/** V5 (GDD 12.2, 9.8, 10.2): the lava destroys everything it touches but a hero in a duel (the duel's arena is not burnt). */
	inline bool LavaDestroys(const bool bIsHero, const bool bHeroInDuel) { return !(bIsHero && bHeroInDuel); }

	/** The point at Distance along the flow (clamped to its two ends); the origin for a flow without point. */
	HEROESOFLIBERTY_API FVector PointAt(const FFlow& Flow, float Distance);

	/**
	 * Cycle Index (0 first) of the site drawn with Seed; every cycle starts where the previous one re-armed. The lava advances
	 * EruptionLength seconds when it is positive (the duration of the drawn flow: EruptionDuration), else the settings' placeholder.
	 */
	HEROESOFLIBERTY_API FCycle MakeCycle(int32 Seed, int32 Index, float PreviousReArmAt, const FHOLVolcanoSettings& Settings, float EruptionLength = 0.f);

	/**
	 * Which of the site's FlowCount flows the eruption of cycle Index takes (V3, D379, V-Q2 = B of the user): drawn freely, every flow with
	 * the same chance, no memory of the previous eruption (fairness comes from the symmetrical flows of the map's recipe, not from the draw).
	 * A function of the seed and the cycle only, same on every machine and independent of the cycle's dates; INDEX_NONE without flow.
	 */
	HEROESOFLIBERTY_API int32 DrawFlow(int32 Seed, int32 Index, int32 FlowCount);

	/** A site as the map gives it (V4): its flows, the crater being the first point of each. Several sites may share a match. */
	struct FSite
	{
		TArray<FFlow> Flows;
	};

	/** What a site is doing at one instant (V4): everything the alert, the lava, the damage and the obstacle need. */
	struct FSiteState
	{
		EHOLVolcanoPhase Phase = EHOLVolcanoPhase::Dormant;
		/** The cycle in progress (0 first); the dates of the one whose phase this is. */
		int32 CycleIndex = 0;
		/** The flow this cycle's eruption takes (V3's draw); INDEX_NONE for a dormant site or a site without flow. */
		int32 FlowIndex = INDEX_NONE;
		float WarningAt = 0.f;
		float EruptAt = 0.f;
		/** Seconds left before the lava starts, during the alert only; negative otherwise. */
		float SecondsToErupt = -1.f;
		/** The front along the flow (cm) and its place; 0 and the crater before the eruption, the end of the flow once it has arrived. */
		float FrontDistance = 0.f;
		FVector FrontPoint = FVector::ZeroVector;
	};

	/** The seed of one site: the match's seed and the site's index mixed, so two sites never erupt alike. */
	HEROESOFLIBERTY_API int32 SiteSeed(int32 MatchSeed, int32 SiteIndex);

	/**
	 * V4: the whole state of a site at Now (seconds since the match start), a function of the options, the match seed, the site and
	 * the time only, so every machine reads the same without anything replicated but the clock. The cycles follow one another: each
	 * draws its flow (DrawFlow) and lasts as long as its lava needs (EruptionDuration at LavaSpeed of ManAtArmsSpeed, the unit table's).
	 * Dormant without Catastrophe (D398).
	 */
	HEROESOFLIBERTY_API FSiteState SiteStateAt(bool bCatastrophe, int32 MatchSeed, int32 SiteIndex, const FSite& Site, float Now, const FHOLVolcanoSettings& Settings, float ManAtArmsSpeed);

	/**
	 * V4c: the state of one site of a match, from the match's configuration (its Seed and its Catastrophe option, both identical on every machine:
	 * the configuration is replicated with the match, AHOLGameState::LaunchConfig) and the simulation's clock in seconds. Same configuration and
	 * same clock, same eruption, whichever machine asks and whenever the match was launched (D608).
	 */
	HEROESOFLIBERTY_API FSiteState SiteStateOfMatch(const FHOLMatchConfig& Config, int32 SiteIndex, const FSite& Site, float Now, const FHOLVolcanoSettings& Settings, float ManAtArmsSpeed);

	/** Phase at Now (seconds): always Dormant without the Catastrophe option (D398). EruptionLength as in MakeCycle. */
	HEROESOFLIBERTY_API EHOLVolcanoPhase GetPhase(bool bCatastrophe, int32 Seed, float Now, const FHOLVolcanoSettings& Settings, float EruptionLength = 0.f);
}
