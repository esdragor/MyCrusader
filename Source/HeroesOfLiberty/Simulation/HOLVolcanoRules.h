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
	/**
	 * V8 (D380): how much sooner the altar held brings the alert (s), never before the instant it was held. The user chose option B (Q13, 2026-11-02):
	 * the alert starts AT ONCE, so the default is far more than any wait (1e6 s); a smaller value (option A was 120 s) is a tuning.
	 */
	float AltarAdvance = 1.0e6f;
	/** V8 (D380, D348): seconds of the altar held without an adversary in its zone that wake the site (~60 s, like the ruin). */
	float AltarHoldSeconds = 60.f;
	/** V8 (D348's rule, as the ruin's ForumHoldDecay): seconds of the count lost each second nobody stands in the zone. */
	float AltarDecayRate = 2.f;
	/**
	 * V11 (D427, answer of the user 2026-11-02, Q-ia-autel-escouade: "a minimum of strategy"). All PROVISIONAL, to confirm in trial: the AI sends
	 * ceil(AiAltarMargin x the adversary's strength on the altar) soldiers, at least AiAltarMinSend, among the free ones but never the AiAltarReserveShare
	 * share of them it keeps at home; an adversary too big for that is a sign its own camp is bare (Hard: no massive send, a diversion instead).
	 */
	float AiAltarMargin = 1.5f;
	float AiAltarReserveShare = 0.4f;
	int32 AiAltarMinSend = 2;
	/** The weight, in the adversary's strength on the altar, of a unit that is not a soldier (a Peasant). */
	float AiAltarWorkerWeight = 0.25f;
	/**
	 * V11 (D640, answer of the user 2026-11-02, Q-ia-autel-tenir = C): the AI holds a FREE altar itself with this share of its free soldiers (one in 4; one in 8
	 * in Easy), when no adversary stands in the zone and the nearest possible flow's line is farther than AiAltarSafeDistance (cm, 40 m) from its positions.
	 * PROVISIONAL, to confirm in trial.
	 */
	float AiAltarHoldShare = 0.25f;
	float AiAltarHoldShareEasy = 0.125f;
	float AiAltarSafeDistance = 4000.f;
	/**
	 * V7 (D379, PROVISIONAL, no ruling of the user yet on the numbers): the fresh deposit that appears on the basalt when a flow has cooled. It stands on the
	 * flow's line FreshDepositAlong of the way from the crater (0 crater, 1 the flow's end; 0.5 = the middle, away from both camps), is gold with the chance
	 * FreshDepositGoldShare and stone otherwise (drawn from the match seed, no memory), and is of the source type named here (a row of the source type table).
	 */
	float FreshDepositAlong = 0.5f;
	float FreshDepositGoldShare = 0.5f;
	FName FreshDepositGoldType = TEXT("GoldVein");
	FName FreshDepositStoneType = TEXT("StoneVein");

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

	/**
	 * V11 (D427, V5-Q1 = B): how far from the line of a flow (cm) a body must stand to be out of the lava's way: the flow's half width plus the
	 * body's own radius, the same reach IsInLava counts (never a typed constant: HalfWidth is FHOLVolcanoSettings::LavaHalfWidth).
	 */
	inline float EvacuationReach(const float HalfWidth, const float Radius) { return FMath::Max(HalfWidth, 0.f) + FMath::Max(Radius, 0.f); }

	/**
	 * V11: whether a body at Point with Radius lies in the whole corridor of the flow, from the crater to its end (the lava's reach once the
	 * front has gone all the way): the AI leaves it from the alert on, before the front moves.
	 */
	HEROESOFLIBERTY_API bool IsInCorridor(const FFlow& Flow, float HalfWidth, const FVector& Point, float Radius = 0.f);

	/**
	 * V11: where a body at Point goes to leave the corridor: the nearest point of the flow's line, then straight away from it until
	 * EvacuationReach + Beyond is cleared (on the other side if a bend of the flow still covers that point). Point itself when it is already out.
	 */
	HEROESOFLIBERTY_API FVector EvacuationPoint(const FFlow& Flow, float HalfWidth, const FVector& Point, float Radius = 0.f, float Beyond = 200.f);

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

	/** V8 (D380): the ancient altar of a site, near the crater: where the map puts it (a point and a zone), held to wake the site. */
	struct FAltar
	{
		bool bPresent = false;
		FVector Location = FVector::ZeroVector;
		/** The zone's reach from the altar's middle (cm), level design; the ruin's zone is 1200 cm from the forum's edge (D348). */
		float Radius = 1200.f;
	};

	/** A site as the map gives it (V4): its flows, the crater being the first point of each, and its altar (V8). Several sites may share a match. */
	struct FSite
	{
		TArray<FFlow> Flows;
		FAltar Altar;
	};

	/** V8: the altars held so far, by cycle: the instant (s since the match start) the altar was held, which began that cycle's alert at once. */
	using FWakes = TMap<int32, float>;

	/** V8: what the altar's zone is doing this step (D348's rule: an adversary in the zone freezes the count). */
	enum class EAltarHold : uint8
	{
		/** Nobody in the zone: the count goes back down. */
		Idle,
		/** Units of one camp only: the count goes up. */
		Holding,
		/** Units of two camps or more: the count is frozen. */
		Contested,
	};

	/**
	 * V8: one step of the altar's count (HeldSeconds, in seconds held). CampsInZone = how many different camps (teams) have a living unit in the
	 * zone: 0 lowers the count by AltarDecayRate x DeltaSeconds (not below 0), 1 raises it by DeltaSeconds, 2 or more freeze it.
	 */
	HEROESOFLIBERTY_API EAltarHold StepAltar(float& HeldSeconds, int32 CampsInZone, float DeltaSeconds, const FHOLVolcanoSettings& Settings);

	/**
	 * StepAltar with the count's OWNER: Holder = the camp the count belongs to (INDEX_NONE: nobody yet), SoleCamp = the camp alone in the zone (read only
	 * when CampsInZone == 1). When a different camp is alone in the zone, the count restarts at 0 for it (the loser's percentage is never inherited);
	 * the same camp keeps its (possibly decayed) count. The world's altar uses this one.
	 */
	HEROESOFLIBERTY_API EAltarHold StepAltarOwned(float& HeldSeconds, int32& Holder, int32 CampsInZone, int32 SoleCamp, float DeltaSeconds, const FHOLVolcanoSettings& Settings);

	/** The levels of the AI as the altar rule reads them (Easy and Normal alike here: only Hard dares the diversion of a big adversary army). */
	enum class EAltarLevel : uint8 { Easy, Normal, Hard };

	/** V11: what the AI knows at this reflection: the adversary's strength on the altar, its free soldiers, the soldiers already sent there. */
	struct FAltarSendInput
	{
		float AdversaryStrength = 0.f;
		int32 FreeSoldiers = 0;
		int32 AlreadySent = 0;
		EAltarLevel Level = EAltarLevel::Normal;
	};

	/** V11: the size of the squad at the altar now (the soldiers already sent included, never recalled: no to-and-fro) and the diversion flag. */
	struct FAltarSend
	{
		int32 Squad = 0;
		/** Too big an adversary army on the altar for the AI to beat: its camp is bare, the Hard level strikes elsewhere instead (no massive send). */
		bool bDiversion = false;
	};

	/**
	 * V11 (D427, user answer 2026-11-02): how many soldiers the AI has at the altar an adversary holds. Need = max(AiAltarMinSend, ceil(AiAltarMargin x
	 * strength)); Available = free soldiers minus the reserve kept at home (ceil(AiAltarReserveShare x free)). Need beyond Available is a big army:
	 * Hard sends nothing new (diversion), the others send all they can spare. Otherwise the squad is Need. Never fewer than AlreadySent.
	 */
	HEROESOFLIBERTY_API FAltarSend AltarSend(const FAltarSendInput& Input, const FHOLVolcanoSettings& Settings);

	/** V11 (D640): what the AI knows to hold a free altar itself. */
	struct FAltarHoldInput
	{
		int32 FreeSoldiers = 0;
		int32 AlreadySent = 0;
		/** An adversary (another camp, a neutral too) stands in the altar's zone: not a free altar, the contest rule (AltarSend) applies. */
		bool bAdversaryInZone = false;
		/** The distance (cm) from the AI's positions to the nearest possible flow's line of the site. */
		float NearestTraceDistance = 0.f;
		EAltarLevel Level = EAltarLevel::Normal;
	};

	/**
	 * V11 (D640): how many soldiers hold a free altar: ceil(share x free soldiers), the share AiAltarHoldShare (AiAltarHoldShareEasy in Easy), when no
	 * adversary is in the zone and the nearest trace is farther than AiAltarSafeDistance; otherwise no new send. Never fewer than AlreadySent (no recall).
	 */
	HEROESOFLIBERTY_API int32 AltarHoldSquad(const FAltarHoldInput& Input, const FHOLVolcanoSettings& Settings);

	/** V7 (D379): what a flow leaves when it has cooled. */
	enum class EFreshKind : uint8 { Gold, Stone };

	/** V7: the fresh deposit of one cooled flow: bPresent false for a flow without length. */
	struct FFreshDeposit
	{
		bool bPresent = false;
		EFreshKind Kind = EFreshKind::Gold;
		/** On the flow's line (PointAt). */
		FVector Location = FVector::ZeroVector;
		/** The row of the source type table. */
		FName TypeName;
	};

	/**
	 * V7 (D379): the new deposit (gold or stone) the cycle CycleIndex of a site leaves on its cooled Flow: a function of the site's seed (SiteSeed), the cycle
	 * and the flow only, same on every machine; the kind is drawn on a stream of its own, with no memory of the previous cycles.
	 */
	HEROESOFLIBERTY_API FFreshDeposit FreshDeposit(int32 SiteSeedValue, int32 CycleIndex, const FFlow& Flow, const FHOLVolcanoSettings& Settings);

	/** The distance (cm, ground plane) from Point to the nearest point of the flow's line; a huge number for a flow without point. */
	HEROESOFLIBERTY_API float DistanceToFlow(const FFlow& Flow, const FVector& Point);

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
		/** V8: the altar can be held now (the site is Armed, its cycle not yet woken); it lights again at the re-arming (D400). */
		bool bAltarLit = false;
		/** V8: this cycle's alert was begun by the altar held (Wakes). */
		bool bWoken = false;
	};

	/** V7: whether the deposit of the state's cycle stands now: the basalt is passable (Rest, D379, D400) and the cycle has a flow. */
	HEROESOFLIBERTY_API bool FreshDepositDue(const FSiteState& State);

	/**
	 * V8 (D380, Q-volcan-autel option A by default): the altar held for ~60 s with no enemy (the ruin's rule, D348) wakes the site earlier. The alert
	 * (and everything after it in the cycle) comes FHOLVolcanoSettings::AltarAdvance seconds sooner, never before HeldAt (the instant the altar was
	 * taken), and unchanged if the alert has already begun (HeldAt at or after its date). The alert's own length stays (60 to 90 s, D380).
	 */
	HEROESOFLIBERTY_API FCycle AdvanceForAltar(const FCycle& Cycle, float HeldAt, const FHOLVolcanoSettings& Settings);

	/** The seed of one site: the match's seed and the site's index mixed, so two sites never erupt alike. */
	HEROESOFLIBERTY_API int32 SiteSeed(int32 MatchSeed, int32 SiteIndex);

	/**
	 * V4: the whole state of a site at Now (seconds since the match start), a function of the options, the match seed, the site and
	 * the time only, so every machine reads the same without anything replicated but the clock. The cycles follow one another: each
	 * draws its flow (DrawFlow) and lasts as long as its lava needs (EruptionDuration at LavaSpeed of ManAtArmsSpeed, the unit table's).
	 * Dormant without Catastrophe (D398).
	 */
	HEROESOFLIBERTY_API FSiteState SiteStateAt(bool bCatastrophe, int32 MatchSeed, int32 SiteIndex, const FSite& Site, float Now, const FHOLVolcanoSettings& Settings, float ManAtArmsSpeed, const FWakes* Wakes = nullptr);

	/**
	 * V4c: the state of one site of a match, from the match's configuration (its Seed and its Catastrophe option, both identical on every machine:
	 * the configuration is replicated with the match, AHOLGameState::LaunchConfig) and the simulation's clock in seconds. Same configuration and
	 * same clock, same eruption, whichever machine asks and whenever the match was launched (D608).
	 */
	HEROESOFLIBERTY_API FSiteState SiteStateOfMatch(const FHOLMatchConfig& Config, int32 SiteIndex, const FSite& Site, float Now, const FHOLVolcanoSettings& Settings, float ManAtArmsSpeed, const FWakes* Wakes = nullptr);

	/** Phase at Now (seconds): always Dormant without the Catastrophe option (D398). EruptionLength as in MakeCycle. */
	HEROESOFLIBERTY_API EHOLVolcanoPhase GetPhase(bool bCatastrophe, int32 Seed, float Now, const FHOLVolcanoSettings& Settings, float EruptionLength = 0.f);
}
