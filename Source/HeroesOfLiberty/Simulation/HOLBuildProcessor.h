// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Buildings/HOLConstructionRules.h"
#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "Simulation/HOLOrderChainRules.h"
#include "HOLBuildProcessor.generated.h"

class AHOLBuilding;
class ANavigationData;
class UHOLCommandSubsystem;
class UHOLSourceSubsystem;
class UHOLUnitSubsystem;
class UNavigationSystemV1;
struct FHOLBuildFragment;
struct FHOLGatherFragment;
struct FHOLMoveOrderFragment;
struct FHOLUnitStateFragment;
struct FHOLUnitTypeConstants;

/**
 * The building (J6a.4, D312), on the server (and in solo), after the orders and the gathering of the step: a builder
 * walks to a free spot around the footprint of the site at the head of its queue (the side facing it first), and works
 * there at contact. Each site's builders at work add their work together, with diminishing returns
 * (HOLConstruction::AddWork, 3 builders ~2x one). The first work makes a bare foundation a site: seen by everyone, its
 * footprint blocking the navmesh, rising from the ground (at once finished with hol.Build.Instant, development). Finished,
 * it serves (population, deposit), and its builders go on to the next item of their queue (Shift, D311), else to their
 * player's nearest site within hol.Build.NextSiteRange of it (the user's trial of J6a.4), else stand idle; a camp finished,
 * those with nothing left in their queue gather the nearest source it accepts within hol.Build.CampGatherRange of it first
 * (D312, J6b.1). A source in the queue (Shift + right click on a resource) is handed to the gathering when it comes, until
 * its massif is used up. Its walks are move orders it gives itself through the order queue (as the gathering's); a
 * player's move or gather order ends it.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLBuildProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLBuildProcessor();

	/**
	 * The standing source the builders of Camp go on to gather once it is finished (D312, J6b.1): the nearest one it accepts
	 * within hol.Build.CampGatherRange of its footprint (HOLConstruction::FindCampSource), its gap (cm) in OutGap; INDEX_NONE
	 * if Camp is no camp or none is near enough. A unit born with its rally point on a camp finished meanwhile goes there too.
	 */
	static int32 FindCampSource(const UHOLSourceSubsystem& Sources, const AHOLBuilding& Camp, double* OutGap = nullptr);
	/** hol.Build.CampGatherRange (cm). */
	static double GetCampGatherRange();

	/** A builder's chain (D328, J7b.5b-1a): its entity, its steps, head first (a site: its actor's unique id). */
	struct FChainOfBuilder
	{
		FMassEntityHandle Entity;
		TArray<HOLOrderChain::FStep> Steps;
	};
	/**
	 * The builders whose queue was given with Shift (more than one item at some point since their last order without
	 * Shift), their steps: sites and sources, head first. Only the builders' queues are looked at (no cost without one).
	 */
	void GetChains(TArray<FChainOfBuilder>& OutChains) const;
	/** Some builder has a queue given with Shift (D328): the chains are worth collecting. */
	bool HasChains() const;
	/** The world it last ran for (hol.Orders.Chains finds it so). */
	const UWorld* GetLastWorld() const { return LastWorld.Get(); }

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	/** What one step works with. */
	struct FStepContext
	{
		FMassEntityManager* EntityManager = nullptr;
		UHOLCommandSubsystem* Commands = nullptr;
		UHOLSourceSubsystem* Sources = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UNavigationSystemV1* NavSystem = nullptr;
		const ANavigationData* NavData = nullptr;
		int64 StepIndex = 0;
	};

	using FWorkItem = HOLConstruction::TWorkItem<TWeakObjectPtr<AHOLBuilding>>;

	/**
	 * A builder's queue (Shift, D311): sites, and sources to gather (the user's trial of J6a.4), the head first; Current,
	 * the head being carried out (the site its spot was found for, the source handed to the gathering).
	 */
	struct FBuilderQueue
	{
		TArray<FWorkItem> Items;
		FWorkItem Current;
		/** Chains (D328): a queue given with Shift; false again with an order without Shift. */
		bool bChain = false;
		/** Chains (D328): the site of a rally point the unit was born for, drawn white while it heads its queue, until the unit starts building. */
		TOptional<FWorkItem> RallyItem;
		bool IsRallyHead() const { return RallyItem.IsSet() && !Items.IsEmpty() && Items[0] == *RallyItem; }
	};

	/** A spot held by a builder: walked to, or worked at. */
	struct FSpotClaim
	{
		FMassEntityHandle Owner;
		FVector2D Spot = FVector2D::ZeroVector;
	};

	/** A site's construction so far, for the log of its end (the scripted checks of J6a.4). */
	struct FSiteStats
	{
		int64 FirstStep = 0;
		int32 WorkSteps = 0;
		/** Steps worked by 1, 2, 3, ... builders (the last one: that many or more). */
		int32 StepsByBuilders[6] = {};
		/** Builders at work at the last step (a change is logged). */
		int32 LastBuilders = 0;
	};

	/** A camp whose builders went on at this step (J6b.1): the source they were sent to, how many went there, how many could not. */
	struct FCampSend
	{
		int32 SourceIndex = INDEX_NONE;
		double Gap = 0.;
		int32 Sent = 0;
		int32 NotSent = 0;
	};

	/** The builder met by the step, and what it needs. */
	struct FBuilder
	{
		FMassEntityHandle Entity;
		FHOLBuildFragment& Build;
		const FHOLMoveOrderFragment& Move;
		FHOLUnitStateFragment& State;
		FHOLGatherFragment& Gather;
		FVector Location;
		const FHOLUnitTypeConstants& Constants;
	};

	void ApplyBuildOrders(const FStepContext& Step);
	void Advance(const FStepContext& Step, FBuilder& Builder);
	/** Walks to a free spot around its site, or works at once if it already stands at contact on a free place. */
	void WalkToSite(const FStepContext& Step, FBuilder& Builder, const AHOLBuilding& Site, const FBox2D& Footprint);
	/** A free spot around Footprint for Self, claimed (HOLConstruction::GetBuildSpots, on the navmesh, clear of the other buildings). */
	bool FindSpot(const FStepContext& Step, const AHOLBuilding& Site, const FBox2D& Footprint, const FVector& From, float BodyRadius,
		FMassEntityHandle Self, FVector& OutSpot);
	/** Another builder's spot within MinSpacing of Spot. */
	bool IsSpotTaken(const FVector2D& Spot, double MinSpacing, FMassEntityHandle Self) const;
	/** At its spot: it works there, the site's footprint kept for the display (its hammering turned towards it). */
	void StartBuilding(FBuilder& Builder, const FBox2D& Footprint);
	/** The head of its queue is a source: handed to the gathering (as a gather order would), until its massif is used up. */
	void StartGathering(const FStepContext& Step, FBuilder& Builder, int32 SourceIndex);
	/**
	 * Its player's nearest site not finished (bare foundations too) within hol.Build.NextSiteRange of Finished, footprint to
	 * footprint (HOLConstruction::FindNextSite), or null.
	 */
	AHOLBuilding* FindNextSite(const FBuilder& Builder, const AHOLBuilding& Finished) const;
	/**
	 * Camp just finished, nothing left in its builder's queue (D312, J6b.1): the builder gathers the nearest source Camp
	 * accepts, as a gather order would send it (the gathering's spots, massifs and groves, D309), its queue forgotten. False if
	 * Camp is no camp or no such source is near enough: it goes on as after a house.
	 */
	bool GatherForCamp(const FStepContext& Step, FBuilder& Builder, AHOLBuilding& Camp);
	/** One line per camp whose builders went on at this step (J6b.1, scripted checks): where they were sent, or why not. */
	void LogCampSends(const FStepContext& Step);
	/** Nothing left in its queue (or its site out of reach): it stands idle where it is, its queue forgotten. */
	void Stop(FBuilder& Builder, const TCHAR* Why);
	void IssueWalk(const FStepContext& Step, FBuilder& Builder, const FVector& Spot);
	/** Its activity (D307): working at its site, else walking there (never idle while it builds). */
	static void SyncActivity(FBuilder& Builder);
	/** Each site worked this step: its builders' work added, its first work, its end (D312). */
	void ApplyWork(const FStepContext& Step);

	/** Each builder's queue, by entity: a fragment holds no array. Swept of the units that no longer build. */
	TMap<FMassEntityHandle, FBuilderQueue> Queues;
	TWeakObjectPtr<const UWorld> LastWorld;
	/** The spots held this step. */
	TArray<FSpotClaim> SpotClaims;
	/** The bodies (D324) of every building and foundation of the match this step: a spot is never on one (D310). */
	TArray<FBox2D> Footprints;
	TArray<TWeakObjectPtr<AHOLBuilding>> FootprintBuildings;
	/** This step's builders at work, by site. */
	TMap<TWeakObjectPtr<AHOLBuilding>, int32> WorkersBySite;
	TMap<TWeakObjectPtr<AHOLBuilding>, FSiteStats> SiteStats;
	/** This step's camps whose builders went on (J6b.1). */
	TMap<TWeakObjectPtr<AHOLBuilding>, FCampSend> CampSends;
	int32 StepsSinceSweep = 0;

	FMassEntityQuery EntityQuery;
};
