// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Map/HOLFogMode.h"
#include "Map/HOLVisionRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLVisionSubsystem.generated.h"

struct FHOLMatchConfig;

/** F6e.3: a building a camp remembers, as the server sends it to a player who comes back (the key is made again by the client). */
USTRUCT()
struct FHOLFogGhostNet
{
	GENERATED_BODY()

	UPROPERTY()
	FName TypeName;
	UPROPERTY()
	uint8 Owner = 0;
	UPROPERTY()
	FVector2D Position = FVector2D::ZeroVector;
	/** EHOLBuildingSight. */
	UPROPERTY()
	uint8 State = 0;
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

	/**
	 * D357, D376: the camp whose sight the machine of Slot's player shows (HOLVisionRules::GetViewerCamp): its own, the team's when
	 * it is defeated while a teammate stands (D646), INDEX_NONE (no fog, everything in sight) for no camp, defeated alone or defeated
	 * with its whole team fallen. bDefeated: told by the caller, else read from the game state (IsSlotDefeated).
	 */
	int32 GetViewerCamp(int32 Slot) const;
	int32 GetViewerCamp(int32 Slot, bool bDefeated) const;

	/** D646: a teammate of Slot is not defeated (HOLVisionRules::HasStandingTeammate, the game state's defeats); false alone. */
	bool HasStandingTeammate(int32 Slot) const;

	/** What Slot's camp knows of Location (Disabled: everything in sight; a slot with no camp: nothing). */
	EHOLCellVision GetVision(int32 Slot, const FVector& Location) const;
	bool IsVisible(int32 Slot, const FVector& Location) const { return GetVision(Slot, Location) == EHOLCellVision::Visible; }

	/**
	 * F7 (D381): a unit of OwnerSlot at Location is drawn, picked and hovered by ViewerSlot's player: his camp's always, the others'
	 * in sight of his camp only. Everything without a fog, before it started, or for a viewer with no camp (a spectator, D376).
	 */
	bool IsUnitShown(int32 ViewerSlot, int32 OwnerSlot, const FVector& Location) const;

	/**
	 * F7.3 (the user's trial): where the line of an attack order ends for ViewerSlot's player. The order is his own and stays shown, but
	 * the line must not follow an enemy out of his sight: the target's position (TargetLocation) when that cell is in sight (or without
	 * a fog, or for a viewer with no camp), else the position the step recorded (StepLocation, where he saw it when he gave the order).
	 */
	FVector GetOrderLineEnd(int32 ViewerSlot, const FVector& StepLocation, const FVector& TargetLocation) const;

	/** F5: some cell of an area (a building's footprint, Extent its half size) in sight of Slot's camp. */
	bool IsAreaVisible(int32 Slot, const FVector& Center, const FVector2D& Extent) const;

	/** F6c: the camps (a bit each, HOLVisionRules) with some cell of the area in sight; Disabled: every camp. */
	uint32 GetCampsSeeing(const FVector& Center, const FVector2D& Extent) const;

	const FHOLVisionMap& GetMap() const { return Map; }

	/**
	 * F7.3 (b): tells each building how ViewerSlot's machine draws it (AHOLBuilding::SetFogDraw): live in sight or when its
	 * camp's; out of sight, as ViewerSlot's camp last saw it, or not at all if never seen. Without a fog, everything live.
	 */
	void RefreshBuildingMemory(int32 ViewerSlot);

	/**
	 * F7.3 (c): ViewerSlot's machine draws a tree cut out of its camp's sight as it was last seen (UHOLSourceSubsystem::SetKeepDrawnRule):
	 * sets the viewer, and lets go of the cut trees whose place is back in sight (the camp learned the cut) or forgotten.
	 */
	void RefreshSourceMemory(int32 ViewerSlot);

	/**
	 * F7.3 (b), a client: a building its camp remembers (ViewerSlot's) and whose place is out of sight has no actor here (the server stops
	 * sending it): one is made from the memory, drawn grey (AHOLBuilding::MarkFogGhostActor), and removed when the place is seen again
	 * or the real building is back. Returns how many such actors exist after the pass.
	 */
	int32 SyncGhostActors(int32 ViewerSlot);

	/**
	 * F4 (D382, D520): what Slot's camp remembers of the buildings and sources it saw (HOLVisionRules::UpdateGhosts), kept
	 * by the server for the reconnection (F6e); null for a slot with no camp, or no fog.
	 */
	const TMap<uint64, FHOLGhost>* GetGhosts(int32 Slot) const;

	/** F6d: a camp's memory (an index of GetMap) and how many times it changed, for what the server sends of it. */
	int32 GetNumCamps() const { return Ghosts.Num(); }
	const TMap<uint64, FHOLGhost>& GetCampGhosts(int32 Camp) const { return Ghosts[Camp]; }
	uint32 GetGhostVersion(int32 Camp) const { return GhostVersions.IsValidIndex(Camp) ? GhostVersions[Camp] : 0; }

	/**
	 * F6e.3 (D520, R7), server: what Slot's camp explored and remembers of the buildings, for a player who comes back
	 * (his client begins with a black map). False when there is nothing to send (no fog, not started, nothing explored).
	 */
	bool MakeMemoryFor(int32 Slot, TArray<uint32>& OutRuns, TArray<FHOLFogGhostNet>& OutGhosts) const;

	/** F6e.3, client: the memory the server sent; taken into the client's own camp as soon as its fog runs (UpdateClient). */
	void ReceiveMemory(const TArray<uint32>& Runs, const TArray<FHOLFogGhostNet>& BuildingGhosts);

	/**
	 * F6f, the leak test, client: what this machine holds beyond its camp's reach, by the camp's own fog (D525): "Fog leaks:
	 * camp 0, sources 240 held 0 on unexplored cells, units of others 3 held 0 out of sight, buildings of others 1 held 0 out of sight".
	 * Empty on the server, or before the client's camp is known.
	 */
	FString MakeLeakReport() const;

	/**
	 * F7.3 (the HUD report's "Fog" line): what ViewerSlot's player has of the fog, in one line: "mode Normal, camp 0, in sight 812,
	 * explored 1650 of 48400 cells, building ghosts 3, units of others 5 shown 2 hidden 3". Without a fog, or before it started:
	 * "mode Disabled, everything in sight"; a slot with no camp (a spectator): "mode Normal, no camp, everything in sight".
	 */
	FString MakeViewerLine(int32 ViewerSlot) const;

	/** "Fog: mode Normal, camps 2, observers 12, camp 0 (slots 0) visible 812 explored 1650, ...; step 0.08 ms". */
	FString MakeReport() const;

	static const TCHAR* GetModeName(EHOLFogMode InMode);

private:
	/** One pass over the observers: BeginPass, the Observe calls, then EndPass (those not seen again are removed; the memory is refreshed). */
	void BeginPass();
	/**
	 * bOwnCamp: counted for the owner's camp (false on a client, which computes the fog of its own camp only: an ally's unit counts there
	 * only through the vision the ally shares, D371). The camps the owner shares its vision with (ShareCamps) count it too.
	 */
	void ObserveUnit(uint32 UnitId, FName TypeName, uint8 Owner, const FVector& Location, bool bOwnCamp = true);
	/**
	 * D371 (D6 of DiplomacyInProgress.md): per owner slot, the other camps that see what it sees (the Diplomacy mode: an ally it gave its
	 * vision to, HOLDiplomacy::SharesVision; read from the game state's book, so the same on a client). OnlyCamp: a client keeps its own.
	 */
	void BuildShareCamps(int32 OnlyCamp = INDEX_NONE);
	/** Present in the memory; bObserve: its cells counted too (the buildings of the camp whose fog this is). */
	void ObserveBuilding(const class AHOLBuilding& Building, bool bObserve);
	/** bGhosts: the memory refreshed (OnlyCamp: a client's own camp's, INDEX_NONE: every camp's). */
	void EndPass(bool bGhosts, int32 OnlyCamp = INDEX_NONE);
	void OnSourceStanding(int32 SourceIndex, bool bStanding);
	/** F7.3 (c): the source just used up is kept drawn for the viewer's camp. */
	bool KeepSourceDrawn(int32 SourceIndex) const;
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
	/** F6e.3: the memory received and not taken in yet. */
	bool bMemoryPending = false;
	TArray<uint32> PendingRuns;
	TArray<FHOLFogGhostNet> PendingGhosts;
	/** Per slot: its camp's index in Map (INDEX_NONE: closed). */
	TArray<int32> CampOfSlot;
	/** The living units seen as observers, by identifier; Stamp: the last Update that walked them. */
	TMap<uint32, FTracked> UnitObservers;
	/** F3.2: the buildings, by actor (UObject's unique identifier); Radius: their sight at the last Update. */
	TMap<uint32, FTracked> BuildingObservers;
	/** D371: the same observers counted in an ally's camp, by (camp << 32 | identifier). */
	TMap<uint64, FTracked> SharedUnitObservers;
	TMap<uint64, FTracked> SharedBuildingObservers;
	/** D371: per owner slot, the camps it shares its vision with (BuildShareCamps). */
	TArray<TArray<int32>> ShareCamps;
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
	/** F7.3 (c): the camp of the machine's own player, whose memory keeps the cut trees drawn (INDEX_NONE: none). */
	int32 SourceViewerCamp = INDEX_NONE;
	/** F7.3 (b), a client: the actors made for the remembered buildings, by ghost key. */
	TMap<uint64, TWeakObjectPtr<class AHOLBuilding>> GhostActors;
	FDelegateHandle StepHandle;
};
