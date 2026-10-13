// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Simulation/MCUnitTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "MCUnitSubsystem.generated.h"

class UDataTable;
class UMassEntityTraitBase;
struct FMassEntityTemplate;

/** Birth request (D224): "Count units of TypeName, on the navmesh around Center within Radius". */
struct FMCUnitSpawnRequest
{
	FName TypeName;
	int32 Count = 0;
	FVector Center = FVector::ZeroVector;
	float Radius = 0.f;
	/** Player the units belong to (D233). */
	uint8 OwnerPlayer = 0;
};

/**
 * Authority over the units (D223, D224, D225), server side only (D29): the unit type table, the Mass entity
 * configurations built from it (one archetype per type), the identifier table and the birth and destruction
 * queues. Nothing else creates or destroys a unit.
 */
UCLASS()
class MYCRUSADER_API UMCUnitSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Row of a type of the table; nullptr when the table is missing or has no such row. */
	const FMCUnitTypeRow* FindTypeRow(FName TypeName) const;

	/** Puts a birth request in the queue (D224); UMCUnitBirthProcessor serves it at the next steps. */
	void RequestUnits(const FMCUnitSpawnRequest& Request);

	/** Creates entities for the queued requests, up to the per-step budget (D224). Returns how many were created. */
	int32 ServeBirthRequests();

	/** Queues a destruction, applied by FlushDestructions() at the end of the step (D224). */
	void RequestDestroyUnits(TConstArrayView<uint32> UnitIds);
	void RequestDestroyAllUnits();
	/** Destroys the queued units; returns how many were destroyed. Called at the end of the fixed step. */
	int32 FlushDestructions();

	/** Entity of a unit identifier (D223); an invalid handle when the unit is gone. */
	FMassEntityHandle FindEntity(uint32 UnitId) const;
	/** Identifier of an entity, 0 when it is not a unit (the reverse table of D223). */
	uint32 FindUnitId(FMassEntityHandle Entity) const;

	/** Identifiers of the units of a type, in creation order. */
	void GetUnitIdsOfType(FName TypeName, TArray<uint32>& OutUnitIds) const;
	/** Identifiers of every living unit, in creation order. */
	void GetAllUnitIds(TArray<uint32>& OutUnitIds) const;

	int32 GetNumUnits() const { return Units.Num(); }
	int32 GetNumPendingBirths() const { return BirthQueue.Num(); }
	void GetUnitCountsByType(TMap<FName, int32>& OutCounts) const;

	/** Restarts the draw of birth positions: two stress tests in one process start from the same crowd (D230). */
	void ResetSpawnSeed() { SpawnRandom.Initialize(SpawnSeed); }

	/** Mass entity template of a type, built once from its row and cached (D225); nullptr for an unknown type. */
	const FMassEntityTemplate* FindTemplate(FName TypeName);

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Deinitialize() override;

private:
	/** A unit of the identifier table (D223). */
	struct FMCUnitRecord
	{
		FMassEntityHandle Entity;
		FName TypeName;
	};

	const UDataTable* GetTypeTable() const;
	/** Births created in one step: the setting, or the whole queue when it is 0 (stress tests measure the peak). */
	int32 GetBirthBudget() const;
	void CreateUnitEntities(const FMCUnitSpawnRequest& Request, const FMassEntityTemplate& Template, int32 Count);

	/** Identifiers and their entities (D223), plus the reverse lookup. */
	TMap<uint32, FMCUnitRecord> Units;
	TMap<FMassEntityHandle, uint32> EntityToId;
	uint32 NextUnitId = 1;

	/** Births waiting for their share of the per-step budget, and units to destroy at the end of the step (D224). */
	TArray<FMCUnitSpawnRequest> BirthQueue;
	TArray<uint32> PendingDestruction;

	/** One Mass entity configuration, so one archetype, per type, built from the table (D225). */
	TMap<FName, TSharedPtr<struct FMassEntityConfig>> TemplateCache;
	/** Fingerprint of each type's constants (D222), to catch two types sharing one copy. */
	TMap<uint32, FName> ConstantsCrcToType;
	/** Keeps the traits alive: FMassEntityConfig is held outside of a UPROPERTY. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMassEntityTraitBase>> Traits;

	mutable TObjectPtr<const UDataTable> CachedTypeTable = nullptr;
	mutable bool bTypeTableLookedUp = false;

	/** Birth positions come from a fixed seed, so two runs of a stress test are comparable (D230). */
	static constexpr int32 SpawnSeed = 1066;
	FRandomStream SpawnRandom{ SpawnSeed };
};
