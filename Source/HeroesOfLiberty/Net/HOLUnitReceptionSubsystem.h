// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Net/HOLUnitReplicationRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLUnitReceptionSubsystem.generated.h"

class AHOLUnitReplicator;

/**
 * Client side of the unit replication (M2.5, D271, D272): reads the slots the shared actor brings, creates a display
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
	/** One report line: units displayed, states received, corrections (S5, D273). */
	FString MakeReport() const;
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

	bool bActive = false;
	TWeakObjectPtr<AHOLUnitReplicator> Replicator;
	int32 LastServerStep = INDEX_NONE;
	FHOLServerClock Clock;
	float StepSeconds = 0.05f;
	TArray<FName> TypeNames;
	TArray<FSlot> Slots;
	int32 NumDisplayed = 0;

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
	};
	FStats Stats;
};
