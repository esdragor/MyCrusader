// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Map/HOLVisionRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLVisionSubsystem.generated.h"

struct FHOLMatchConfig;

/** D522: the fog of a match (the menu offers the three). */
UENUM()
enum class EHOLFogMode : uint8
{
	/** Everything in sight (« Sans brouillard »): the trials, the measures, the tests' default. */
	Disabled,
	/** D389: the map explored from the start (terrain and resources), the enemy's units and buildings hidden out of sight. */
	Explored,
	/** D381: the map black at the start. The game's default (F8). */
	Normal,
};

/**
 * F3 (FogInProgress.md, D381-D383, D521): the fog of every camp of the match, on the server and in a solo game (a client
 * computes its own from what it receives, F6e). Its observers are the living units (UHOLUnitSubsystem) and, later (F3.2),
 * the buildings; a camp is a team, or a slot alone (HOLVictoryRules::GetCamp). Each fixed step, only the observers that
 * changed cell are counted again. The forest is the source registry's trees (FHOLVisionMap::AddBlocker).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLVisionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	// UTickableWorldSubsystem: a client's fog is computed each frame from what it received (F6e); the server's and the solo
	// game's follow the fixed step (OnStepEnded).
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return bClient; }

	/**
	 * The match starts (OnWorldBeginPlay, or a test): its camps from Config, the largest grid (D344) centred on the origin,
	 * the trees already standing, then every source told (UHOLSourceSubsystem::OnSourceStanding). Again: starts over.
	 */
	void Start(const FHOLMatchConfig& Config, EHOLFogMode InMode);

	/**
	 * The match's configuration is set (a match, not the menu's world) and the fog not started yet: starts it, from
	 * hol.Fog.Mode (what the first step does). True when started. The unit replication calls it too (F6b): it must
	 * know the camps before it places a unit.
	 */
	bool TryStart();

	/** One step (after the simulation's, UHOLSimulationSubsystem::GetOnStepEnded): the observers that moved, came or went. */
	void Update();

	/**
	 * F6e (D525): a client's step, from what it received: its camp's displayed units (UHOLUnitReceptionSubsystem) and
	 * buildings are the observers, the trees of its source registry the forest. False while it cannot tell its camp yet.
	 */
	bool UpdateClient();

	/** F6e: this machine is a client (its fog is computed from what the server sends). */
	bool IsClient() const { return bClient; }
	/** F6e: the camp (an index of GetMap) whose fog this client computes; INDEX_NONE on the server, or before its slot is known. */
	int32 GetClientCamp() const { return ClientCamp; }

	EHOLFogMode GetMode() const { return Mode; }
	bool IsStarted() const { return bStarted; }

	/** The fog's camp of a slot (an index of GetMap), INDEX_NONE for a closed slot or none. */
	int32 GetCampIndex(int32 Slot) const;

	/** What Slot's camp knows of Location (Disabled: everything in sight; a slot with no camp: nothing). */
	EHOLCellVision GetVision(int32 Slot, const FVector& Location) const;
	bool IsVisible(int32 Slot, const FVector& Location) const { return GetVision(Slot, Location) == EHOLCellVision::Visible; }

	/** F5: some cell of an area (a building's footprint, Extent its half size) in sight of Slot's camp. */
	bool IsAreaVisible(int32 Slot, const FVector& Center, const FVector2D& Extent) const;

	/** F6c: the camps (a bit each, HOLVisionRules) with some cell of the area in sight; Disabled: every camp. */
	uint32 GetCampsSeeing(const FVector& Center, const FVector2D& Extent) const;

	const FHOLVisionMap& GetMap() const { return Map; }

	/**
	 * F4 (D382, D520): what Slot's camp remembers of the buildings and sources it saw (HOLVisionRules::UpdateGhosts), kept
	 * by the server for the reconnection (F6e); null for a slot with no camp, or no fog.
	 */
	const TMap<uint64, FHOLGhost>* GetGhosts(int32 Slot) const;

	/** F6d: a camp's memory (an index of GetMap) and how many times it changed, for what the server sends of it. */
	int32 GetNumCamps() const { return Ghosts.Num(); }
	const TMap<uint64, FHOLGhost>& GetCampGhosts(int32 Camp) const { return Ghosts[Camp]; }
	uint32 GetGhostVersion(int32 Camp) const { return GhostVersions.IsValidIndex(Camp) ? GhostVersions[Camp] : 0; }

	/** "Fog: mode Normal, camps 2, observers 12, camp 0 (slots 0) visible 812 explored 1650, ...; step 0.08 ms". */
	FString MakeReport() const;

	static const TCHAR* GetModeName(EHOLFogMode InMode);

private:
	/** One pass over the observers: BeginPass, the Observe calls, then EndPass (those not seen again are removed; the memory is refreshed). */
	void BeginPass();
	void ObserveUnit(uint32 UnitId, FName TypeName, uint8 Owner, const FVector& Location);
	/** Present in the memory; bObserve: its cells counted too (the buildings of the camp whose fog this is). */
	void ObserveBuilding(const class AHOLBuilding& Building, bool bObserve);
	/** bGhosts: the memory refreshed (OnlyCamp: a client's own camp's, INDEX_NONE: every camp's). */
	void EndPass(bool bGhosts, int32 OnlyCamp = INDEX_NONE);
	void OnSourceStanding(int32 SourceIndex, bool bStanding);
	void OnStepEnded(int64 StepIndex);
	double GetUnitSight(FName TypeName);

	struct FTracked
	{
		int32 Handle = INDEX_NONE;
		uint32 Stamp = 0;
		double Radius = 0.;
	};

	FHOLVisionMap Map;
	EHOLFogMode Mode = EHOLFogMode::Disabled;
	bool bStarted = false;
	/** F6e: this machine is a client; ClientCamp: the camp of its local player's slot (INDEX_NONE until known). */
	bool bClient = false;
	int32 ClientCamp = INDEX_NONE;
	float ClientSeconds = 0.f;
	/** Per slot: its camp's index in Map (INDEX_NONE: closed). */
	TArray<int32> CampOfSlot;
	/** The living units seen as observers, by identifier; Stamp: the last Update that walked them. */
	TMap<uint32, FTracked> UnitObservers;
	/** F3.2: the buildings, by actor (UObject's unique identifier); Radius: their sight at the last Update. */
	TMap<uint32, FTracked> BuildingObservers;
	TMap<FName, double> UnitSights;
	/** F4: per camp of Map, its memory. */
	TArray<TMap<uint64, FHOLGhost>> Ghosts;
	/** F6d: per camp, counts the changes of its memory. */
	TArray<uint32> GhostVersions;
	/** F4: the buildings and sources standing this step, as they are (gathered by Update). */
	TArray<FHOLGhost> Present;
	uint32 Stamp = 0;
	double LastUpdateMs = 0.;
	FDelegateHandle SourceHandle;
	FDelegateHandle StepHandle;
};
