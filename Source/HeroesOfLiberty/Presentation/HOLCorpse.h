// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Presentation/HOLPresentationFragments.h"
#include "Simulation/HOLSimulationFragments.h"

struct FHOLUnitTypeConstants;
struct FMassEntityManager;
class UHOLUnitSubsystem;
class UWorld;

/**
 * A body to lay (J7c, bodies kept 30 s): read from a unit about to be destroyed (HOLCorpse::CollectSeeds), laid once it is
 * gone (HOLCorpse::Lay).
 */
struct FHOLCorpseSeed
{
	/** Its unit's type: the body is made from that type's Corpse template. */
	FName TypeName;
	/** The body's fragment, complete. */
	FHOLCorpseFragment Corpse;
};

/**
 * Rules of the bodies a displaying machine keeps (J7c, bodies kept 30 s): the server still destroys a unit 1 s after
 * its death; a client (or solo) lays a body in its place, a separate Mass entity that ends the fall, holds its last pose
 * HoldSeconds, sinks SinkDepth in SinkSeconds and goes. The rules have no engine state (CollectSeeds and Lay read and make
 * the entities): tested on their own (HeroesOfLiberty.Animation.Corpse).
 */
namespace HOLCorpse
{
	/** Seconds a body holds its last pose, from its unit's destruction (~1 s after its death, when its fall ends). */
	constexpr float HoldSeconds = 30.f;
	/** Seconds it then takes to sink. */
	constexpr float SinkSeconds = 1.f;
	/** How deep it sinks (cm) before it goes. */
	constexpr float SinkDepth = 100.f;

	/**
	 * Position (s) of the fall the body resumes at: where its unit was (StartPosition) plus the time since (ElapsedSeconds)
	 * at the fall's play rate, held at its last frame (Length); never negative.
	 */
	HEROESOFLIBERTY_API float GetResumePosition(float StartPosition, double ElapsedSeconds, float PlayRate, float Length);

	/** How far down (cm) a body Age seconds old is drawn: 0 while it holds, then SinkDepth over SinkSeconds. */
	HEROESOFLIBERTY_API float GetSinkOffset(double Age);

	/** Whether a body Age seconds old has sunk and goes. */
	HEROESOFLIBERTY_API bool IsExpired(double Age);

	/** Whether a unit leaves a body: dying (or at 0 health) and its type has a fall to end. */
	HEROESOFLIBERTY_API bool LeavesCorpse(const FHOLUnitTypeConstants& Type, EHOLUnitActivity Activity, float CurrentHealth);

	/**
	 * Reads the body each of Entities leaves (LeavesCorpse: dying, or at 0 health for a simulated unit), before they are
	 * destroyed: where it was last drawn (its fall's transform, else its place at its mesh's scale), the fall's position
	 * (from its start, else as far as DyingSteps went), its prop and its owner. Now: World->GetTimeSeconds(). Entities no
	 * longer valid, or of no type, are skipped.
	 */
	HEROESOFLIBERTY_API void CollectSeeds(const FMassEntityManager& EntityManager, TConstArrayView<FMassEntityHandle> Entities, double Now,
		TArray<FHOLCorpseSeed>& Out);

	/** Lays Seeds: one body entity each, per type from its Corpse template (UHOLUnitSubsystem::FindTemplate), placed at its Base. */
	HEROESOFLIBERTY_API void Lay(FMassEntityManager& EntityManager, UHOLUnitSubsystem& Units, TConstArrayView<FHOLCorpseSeed> Seeds);

	/** The bodies of a world that went, since it began (counted when they go, read by hol.Units.CorpseReport). */
	struct FRemovedStats
	{
		int32 NumRemoved = 0;
		/** The oldest age (s) a body had when it went. */
		double LongestKept = 0.;
	};

	/** Counts a body of World that goes at Age seconds (UHOLInterpolatedInstancesProcessor, as it destroys it). */
	HEROESOFLIBERTY_API void NoteRemoved(const UWorld& World, double Age);

	/** The bodies of World that went (none for a world that has just begun: each world has its own counts). */
	HEROESOFLIBERTY_API FRemovedStats GetRemovedStats(const UWorld& World);
}
