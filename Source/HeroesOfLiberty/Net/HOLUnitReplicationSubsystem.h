// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "Net/HOLUnitReplicationRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLUnitReplicationSubsystem.generated.h"

class AHOLUnitReplicator;
class UNetConnection;

/** What the server's unit replication did since the last reset (S5, D273). */
struct FHOLUnitReplicationStats
{
	int32 NumSteps = 0;
	/** Units looked at, summed over the steps. */
	int64 NumUnitSteps = 0;
	/** Per EHOLUnitSendReason (None counts the births). */
	int64 Sends[4] = {};
	/**
	 * Of the Activity sends, those whose only change is the health (J7c.4, S5 in combat): with the health last sent, the
	 * unit would not have been sent at this step.
	 */
	int64 HealthSends = 0;
	int32 NumDepartures = 0;
	double TotalMs = 0.0;
	double MaxMs = 0.0;
	double StartSeconds = 0.0;
	/** Unit states written to the wire at the reset, all connections together. */
	int64 SerializedStates = 0;
	/** Bytes and packets each client connection had sent at the reset, for averages over the report's span. */
	TMap<TWeakObjectPtr<const UNetConnection>, TPair<int32, int32>> OutTotals;
};

/**
 * Server side of the unit replication (M2.5, D269, D270): at the end of every fixed step, every unit goes through the
 * send rule; a unit that must be sent gets its reduced state written into its slot, which marks it for Iris. A new
 * unit takes a free slot (its identity is written once), a unit gone frees its slot. Only in a networked match: the
 * solo game does not replicate (D272).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitReplicationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	AHOLUnitReplicator* GetReplicator() const { return Replicator; }
	const FHOLUnitReplicationStats& GetStats() const { return Stats; }
	void ResetStats();
	/** One report line (server: sends and cost; client: what it received). */
	FString MakeReport() const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	/** What the server knows of a unit it replicates. */
	struct FUnitRecord
	{
		int32 Slot = INDEX_NONE;
		FHOLUnitSentRecord Sent;
		uint8 OwnerPlayer = 0;
		int64 SeenStep = 0;
	};

	void OnStepEnded(int64 Step);
	int32 TakeSlot();
	void FreeSlot(int32 Slot);

	UPROPERTY(Transient)
	TObjectPtr<AHOLUnitReplicator> Replicator;

	TSharedPtr<struct FMassEntityManager> EntityManager;
	FMassEntityQuery UnitQuery;
	FDelegateHandle StepEndedHandle;
	float StepSeconds = 0.05f;

	TMap<uint32, FUnitRecord> Records;
	/** Free slots, the lowest last so it is taken first. */
	TArray<int32> FreeSlots;
	TMap<FName, uint8> TypeIndices;

	FHOLUnitReplicationStats Stats;
	double LastReportSeconds = 0.0;
};
