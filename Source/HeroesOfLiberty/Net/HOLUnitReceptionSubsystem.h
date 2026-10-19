// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Net/HOLUnitReplicationRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLUnitReceptionSubsystem.generated.h"

class AHOLUnitChunk;
class AHOLUnitReplicator;

/**
 * Client side of the unit replication (M2.5, D271, D272): reads the slots of the chunk actors, creates a display
 * entity for each unit that arrives (the drawing of M2, without simulation), removes the ones that leave, feeds the
 * server clock with the replicated step and starts a smoothed correction at each new state. UHOLNetUnitDisplayProcessor
 * places the entities every frame.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitReceptionSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	const FHOLServerClock& GetClock() const { return Clock; }
	float GetStepSeconds() const { return StepSeconds; }
	int32 GetNumDisplayed() const { return NumDisplayed; }
	/** The chunk actors received, by chunk index (null: not received yet). */
	const TArray<TWeakObjectPtr<AHOLUnitChunk>>& GetChunks() const { return Chunks; }
	/** Called by a chunk actor when it arrives (BeginPlay) and leaves (EndPlay). */
	void AddChunk(AHOLUnitChunk* Chunk);
	void RemoveChunk(const AHOLUnitChunk* Chunk);
	/** One report line: units displayed, states received, corrections (S5, D273). */
	FString MakeReport() const;
	/** Where the states sat in what was received (D300): the bits Iris wrote around them, over that many seconds. */
	static FString MakeBitProbeReport(double Seconds);
	/** The units displayed, counted by type and owner ("2 displayed: Peasant of slot 0 x1, ..."), for hol.Units.Report (J3). */
	FString DescribeDisplayed() const;

	/**
	 * J7a.6: how this client draws its displayed units, per type and slot: the representation, the skinned mesh and the
	 * colour index the presentation writes (the owner's player state, as FSlotColors), with their counts.
	 */
	FString DescribeLooks() const;
	/** The units displayed, where they are drawn (J4: what the player selects and orders). */
	void GetDisplayedUnits(TArray<struct FHOLUnitView>& OutUnits) const;
	/** The identifier and the display entity of the target of an Attacking state (J7b.4: its slot); 0 / unset: none known. */
	uint32 GetTargetUnitId(const FHOLNetUnitState& State) const;
	FMassEntityHandle GetTargetEntity(const FHOLNetUnitState& State) const;
	/** One displayed unit; false when it is not displayed. */
	bool GetDisplayedUnit(uint32 UnitId, struct FHOLUnitView& OutUnit) const;
	/**
	 * The amount the player's own unit selected alone carries (D303), as the server last sent it to this player
	 * (AHOLPlayerController): the states sent to everyone only say whether a unit carries. UnitId 0: none.
	 */
	void SetWatchedLoad(uint32 UnitId, EHOLResource Resource, int32 Amount);
	void ResetStats();

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return bActive; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	/** What the client last read in one slot of the shared actor. */
	struct FSlot
	{
		uint32 UnitId = 0;
		uint8 TypeIndex = 0;
		uint8 OwnerPlayer = 0;
		FHOLNetUnitState State;
		FMassEntityHandle Entity;
	};

	/** The view of one slot's unit; false when it has no display entity. */
	bool MakeView(const FSlot& Slot, struct FHOLUnitView& OutView) const;
	void ReadClock(double Now);
	void ReadSlots(double Now);
	/** Display entities for the slots whose unit just arrived, by type. */
	void CreateEntities(TConstArrayView<int32> SlotIndices, double Now);
	/** A new state for a displayed unit: the gap to where it is drawn becomes a fading correction (D271). */
	void ApplyState(FSlot& Slot, double Now);
	/** Height (cm) of the ground at Position's horizontal position (D301), Fallback where there is none. */
	double GetGroundHeight(const FVector& Position, double Fallback) const;
	/** The ground is looked for between these heights (cm), the range the 12 bits of height used to cover. */
	static constexpr double GroundTraceHalfHeight = 20000.0;
	/** S5 (D273): logs where the sampled units are displayed now, to compare with the server's true positions. */
	void LogSample(double Now) const;

	bool bActive = false;
	TWeakObjectPtr<AHOLUnitReplicator> Replicator;
	TArray<TWeakObjectPtr<AHOLUnitChunk>> Chunks;
	int32 LastServerStep = INDEX_NONE;
	FHOLServerClock Clock;
	float StepSeconds = 0.05f;
	TArray<FName> TypeNames;
	TArray<FSlot> Slots;
	int32 NumDisplayed = 0;
	uint32 WatchedUnitId = 0;
	EHOLResource WatchedResource = EHOLResource::Food;
	int32 WatchedAmount = 0;

	static constexpr int32 MaxFrames = 36000;

	struct FStats
	{
		double StartSeconds = 0.0;
		int64 NumStates = 0;
		int64 NumCorrections = 0;
		double TotalCorrectionCm = 0.0;
		double MaxCorrectionCm = 0.0;
		int32 NumSnaps = 0;
		int32 NumArrivals = 0;
		int32 NumDepartures = 0;
		/** Bytes the connection had received at the reset. */
		int32 InTotalBytes = 0;
		/** Unit states read off the wire at the reset (process total). */
		int64 DeserializedStates = 0;
		/** Frame times (s) since the reset, for the frame rate of S5; at most MaxFrames when nothing resets them. */
		TArray<float> FrameSeconds;
	};
	FStats Stats;
	double LastReportSeconds = 0.0;
	double LastSampleSeconds = 0.0;
};
