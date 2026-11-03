// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLVolcanoRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "Simulation/HOLLavaObstacle.h"
#include "HOLVolcanoSubsystem.generated.h"

/**
 * The volcano sites of a match (GDD 12, V5): server side only (D288). Holds the sites (the map's recipe gives them, V4b; a test or a
 * scenario sets them) and, at the head of each simulation step, reads each site's state from the match's configuration and the clock
 * (HOLVolcano::SiteStateOfMatch) and destroys what its lava runs over while it advances: units (heroes too, but a hero in a duel),
 * buildings (a main forum falls into ruin as from any blow, D350). Deposits are spared (D379).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLVolcanoSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's volcano book, or null. */
	static UHOLVolcanoSubsystem* Find(const UWorld* World) { return World ? World->GetSubsystem<UHOLVolcanoSubsystem>() : nullptr; }

	void SetSites(const TArray<HOLVolcano::FSite>& InSites);

	/** V4b (D607): the sites the map lays (AHOLVolcanoLayout, data of the map) replace the list; no layout in the world = the list is left as it is. Server or solo only. Done as the match begins. */
	void LoadLayout();
	const TArray<HOLVolcano::FSite>& GetSites() const { return Sites; }
	FHOLVolcanoSettings& GetMutableSettings() { return Settings; }
	const FHOLVolcanoSettings& GetSettings() const { return Settings; }

	/** Listens to the simulation's steps (done at the world's begin play; a test world calls it by hand). Server and solo only. */
	void BindSteps();

	/** The match clock (s) of a step: the step index over the step rate. */
	float GetClockOf(int64 StepIndex) const;

	/**
	 * Burns what Site's lava covers in State (nothing unless it is erupting); returns the units and buildings destroyed, added to the
	 * totals. The step calls it for every site; a test may call it with a state of its own.
	 */
	int32 Burn(const HOLVolcano::FSite& Site, const HOLVolcano::FSiteState& State);

	/**
	 * What a site's state asks of the world: the lava burns while it advances (Burn), the cooled flow forbids its ground to the navmesh
	 * (V6: an AHOLLavaObstacle for the cycle in Cooling, removed when the basalt is passable).
	 */
	void Apply(int32 SiteIndex, const HOLVolcano::FSite& Site, const HOLVolcano::FSiteState& State);

	/** The obstacle standing now for a site, null when its ground is free. */
	AHOLLavaObstacle* FindObstacle(const int32 SiteIndex) const { const TObjectPtr<AHOLLavaObstacle>* Found = Obstacles.Find(SiteIndex); return Found ? Found->Get() : nullptr; }

	int32 GetNumUnitsBurnt() const { return NumUnitsBurnt; }
	int32 GetNumBuildingsBurnt() const { return NumBuildingsBurnt; }

	/**
	 * V8 (D380, D348's rule): the state of a site NOW, wakes included (an altar held began that cycle's alert). A Dormant state without
	 * Catastrophe, without the unit table or for an index out of the sites. The HUD (V10) and the report read it.
	 */
	HOLVolcano::FSiteState GetSiteState(int32 SiteIndex) const;

	/** V8: the altar's count of a site (s held so far), what its zone is doing now, and how many times it has been held (cycles woken). */
	float GetAltarHeldSeconds(const int32 SiteIndex) const { return AltarHeld.IsValidIndex(SiteIndex) ? AltarHeld[SiteIndex] : 0.f; }
	HOLVolcano::EAltarHold GetAltarHold(const int32 SiteIndex) const { return AltarHolds.IsValidIndex(SiteIndex) ? AltarHolds[SiteIndex] : HOLVolcano::EAltarHold::Idle; }
	int32 GetNumWakes(const int32 SiteIndex) const { return Wakes.IsValidIndex(SiteIndex) ? Wakes[SiteIndex].Num() : 0; }

	/**
	 * V8: one step of the altar of a site at the clock Now (s): counts the camps with a living unit in its zone while the altar is lit, steps the
	 * count (HOLVolcano::StepAltar) and, when it reaches the hold time, records the wake: the cycle's alert starts at Now. The step calls it for every
	 * site; a test may call it by hand. Returns true on the step the altar is held.
	 */
	bool StepAltarOf(int32 SiteIndex, float Now, float DeltaSeconds);

	/** V11 (D639, D640): what the world says of one altar for one slot (the AI's), measured on the units of the zone and the slot's buildings. */
	struct FAltarMeasure
	{
		int32 SiteIndex = INDEX_NONE;
		FVector Location = FVector::ZeroVector;
		float Radius = 0.f;
		/** The slot's camp has a living unit in the zone, and no other camp (a neutral is a camp of its own, an adversary): the camp alone in the zone. */
		bool bOwnCampAlone = false;
		/** At least one living unit of another camp stands in the zone. */
		bool bAdversary = false;
		/** The adversary's strength in the zone: 1 per soldier (a Peasant counts AiAltarWorkerWeight). */
		float AdversaryStrength = 0.f;
		/** The distance (cm) from the slot's buildings (the nearest) to the nearest possible flow's line of the site; a huge number without building or flow. */
		float TraceFromBuildings = 0.f;
		/** The distance (cm) from the altar's middle to the nearest possible flow's line of the site. */
		float TraceFromAltar = 0.f;
	};

	/**
	 * V11: measures the altar of SiteIndex for Slot at this step. Returns false (nothing measured) when the site has no altar, the altar is not lit
	 * now (held and woken, or the site not Armed) or the world lacks what it reads. Server and solo only.
	 */
	bool MeasureAltar(int32 SiteIndex, int32 Slot, FAltarMeasure& Out) const;

	/** V11: one unit that must leave a flow's corridor, and where it goes. */
	struct FEvacuation
	{
		uint32 UnitId = 0;
		FVector Target = FVector::ZeroVector;
	};

	/**
	 * V11 (D427): the units of Slot caught in the corridor of a site in its alert or its eruption now (the whole corridor from the alert on, not
	 * only what the front has run over: the AI leaves before the lava moves) and the point each goes to. A body is caught when it is within
	 * LavaHalfWidth (the settings') plus its own radius of the flow's line (HOLVolcano::EvacuationReach). A hero in a duel stays (it cannot
	 * flee). Nothing without Catastrophe. Returns how many.
	 */
	int32 GetEvacuations(int32 Slot, TArray<FEvacuation>& Out) const;

	/** V11: GetEvacuations, then a move order for each unit (applied at the next step, the order a player's click sends). Returns how many were ordered. */
	int32 EvacuateOwner(int32 Slot);

	/** V7 (D379): the indices (in UHOLSourceSubsystem) of the fresh deposits a site has left so far, one per cooled cycle. */
	const TArray<int32>& GetFreshSourceIndices(const int32 SiteIndex) const { static const TArray<int32> None; return FreshSources.IsValidIndex(SiteIndex) ? FreshSources[SiteIndex] : None; }

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void OnStepStarted(int64 StepIndex);

	/** The state of a site at Now with its wakes, from the match's configuration (Dormant when the unit table or the game state is missing). */
	HOLVolcano::FSiteState StateOf(int32 SiteIndex, float Now) const;

	void SyncObstacle(int32 SiteIndex, const HOLVolcano::FSite& Site, const HOLVolcano::FSiteState& State);

	/** V7 (D379): the fresh deposit of the cycle whose basalt is passable, placed in the source registry (server or solo), once per cycle. */
	void SyncFreshDeposit(int32 SiteIndex, const HOLVolcano::FSite& Site, const HOLVolcano::FSiteState& State);

	/** The obstacle of each site, by index (V6). */
	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<AHOLLavaObstacle>> Obstacles;
	TArray<HOLVolcano::FSite> Sites;
	/** V8, by site: the altars held (cycle -> instant), the count of each altar, what its zone does, the cycle the count belongs to. */
	TArray<HOLVolcano::FWakes> Wakes;
	TArray<float> AltarHeld;
	TArray<HOLVolcano::EAltarHold> AltarHolds;
	TArray<int32> AltarCycle;
	/** The camp each altar's count belongs to (INDEX_NONE: nobody yet): a new holder restarts at 0. */
	TArray<int32> AltarHolder;
	/** V7, by site: the last cycle whose fresh deposit was placed (INDEX_NONE: none), and the source indices placed. */
	TArray<int32> FreshPlacedCycle;
	TArray<TArray<int32>> FreshSources;
	FHOLVolcanoSettings Settings;
	/** Height of the obstacle boxes (cm): well above the ground, as a building's. */
	float ObstacleHeight = 600.f;
	FDelegateHandle StepStartedHandle;
	int32 NumUnitsBurnt = 0;
	int32 NumBuildingsBurnt = 0;
};