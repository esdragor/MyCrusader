// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLSiteRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLSiteSubsystem.generated.h"

/**
 * The sacred sites of a match (SacredSitesInProgress.md S5, GDD 14.3, D429 to D431, D697 to D703): where the simulation runs (solo, server),
 * each step it walks the living units once, counts in each site's zone the takers (a holy unit of the slot's faction, or its hero) and the
 * disturbers (every unit but the worker) of each slot, and applies HOLSite::Step (the rule S1/S2 wrote and tested). Beside UHOLRelaySubsystem,
 * on its model; the relays are untouched.
 *
 * OFF BY DEFAULT: the match option "Sacred sites" is S7 (not written yet), until then hol.Site.Enabled (0) is the switch; off, a step does
 * nothing, and with no site laid (S4: the maps lay none yet; hol.Site.Add for development) it does nothing either, so the game as it is does
 * not change. Not here yet (the next steps of the dossier): the replicated views and the visibility (S5/S9), the gold by the minute (S6), the
 * victory count (S10), the holy units from the faction table's column (S3 of the dossier: for now Holy below, provisional).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLSiteSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	struct FSite
	{
		FVector2D Location = FVector2D::ZeroVector;
		/** The zone's radius, cm. */
		float Radius = 1200.f;
		FHOLSiteState State;
	};

	/** The switch (hol.Site.Enabled, 0 by default; the match option replaces it at S7). */
	static bool IsEnabled();

	/**
	 * Whether the unit type is a holy unit of the faction (it takes a site). PROVISIONAL list (D702: the Dawn's Monk and Luminous Monk; the
	 * Legions' Necromancer does not exist yet, so they take by their hero alone) until the faction table's column replaces it.
	 */
	static bool IsHolyType(FName FactionKey, FName UnitType);

	/** A site at Location with a zone of Radius cm; its index. Server or solo only (INDEX_NONE on a client). */
	int32 AddSite(const FVector2D& Location, float Radius);
	void ClearSites();
	const TArray<FSite>& GetSites() const { return Sites; }
	/** Sites the slot owns. */
	int32 CountOwned(int32 Slot) const;
	/** The step the simulation is on is applied: counts, rule, then the views. Public for the tests. */
	void OnStepEnded(int64 StepIndex);
	bool BindSteps();

	/**
	 * S5 (D701): writes to each seat's player state the sites its camp may know: a site is known to a slot when its camp owns it or is taking
	 * it, or when the fog says the camp explored the site's centre (everything is known without a fog). The view is the site's state while the
	 * camp sees it (or owns / takes it), and stays as it last saw it while the fog hides it (a site known only by exploration, never seen,
	 * is shown neutral). A site never known is not in the view. Server or solo; public for the tests.
	 */
	void PublishViews();

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	void UnbindSteps();

	/** What one slot has learnt of one site: known yet, and the view as it last saw it. */
	struct FKnown
	{
		bool bKnown = false;
		FHOLSiteView View;
	};

	TArray<FSite> Sites;
	/** [slot][site]: the memory of each seat (S5), emptied with the sites. */
	TArray<TArray<FKnown>> Memory;
	FDelegateHandle StepEndedHandle;
};
