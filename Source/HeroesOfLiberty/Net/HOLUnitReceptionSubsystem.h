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
	/** The units displayed, counted by type and owner ("2 displayed: Peasant of slot 0 x1, ..."), for hol.Units.Report (J3). */
	FString DescribeDisplayed() const;
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

	void ReadClock(double Now);
	void ReadSlots(double Now);
	/** Display entities for the slots whose unit just arrived, by type. */
	void CreateEntities(TConstArrayView<int32> SlotIndices, double Now);
	/** A new state for a displayed unit: the gap to where it is drawn becomes a fading correction (D271). */
	void ApplyState(FSlot& Slot, double Now);
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
