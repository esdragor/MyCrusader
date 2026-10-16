// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Simulation/HOLUnitTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLUnitSubsystem.generated.h"

class AHOLBuilding;
class UDataTable;
class UMassEntityTraitBase;
struct FMassEntityTemplate;

/** What a unit template is built for (D272): the server's simulated unit, or a client's display entity (D271). */
enum class EHOLUnitTemplateKind : uint8
{
	/** Movement, avoidance, navigation and drawing: the authoritative unit (server, solo game). */
	Simulation,
	/** Drawing only, placed from the replicated states (UHOLNetUnitDisplayProcessor). */
	Display,
};

/** Birth request (D224): "Count units of TypeName, on the navmesh around Center within Radius". */
struct FHOLUnitSpawnRequest
{
	FName TypeName;
	int32 Count = 0;
	FVector Center = FVector::ZeroVector;
	float Radius = 0.f;
	/** Player the units belong to (D233). */
	uint8 OwnerPlayer = 0;
	/** Once born, the units go there (a building's rally point, J4, D288). */
	bool bHasMoveTarget = false;
	FVector MoveTarget = FVector::ZeroVector;
	/** ... and gather this source there (a rally point on a source, J5, D298), if it still stands. */
	int32 GatherSource = INDEX_NONE;
	/** ... or build this site there (a rally point on a site, the user's trial of J6a.4), if it is not finished. */
	TWeakObjectPtr<AHOLBuilding> BuildSite;
};

/**
 * Authority over the units (D223, D224, D225), server side only (D29): the unit type table, the Mass entity
 * configurations built from it (one archetype per type), the identifier table and the birth and destruction
 * queues. Nothing else creates or destroys a unit.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Row of a type of the table; nullptr when the table is missing or has no such row. */
	const FHOLUnitTypeRow* FindTypeRow(FName TypeName) const;

	/**
	 * Types of the table in lexical order: the same list on the server and on the clients, so the unit replication
	 * sends a type as its index in it (D270).
	 */
	void GetTypeNames(TArray<FName>& OutTypeNames) const;

	/** Puts a birth request in the queue (D224); UHOLUnitBirthProcessor serves it at the next steps. */
	void RequestUnits(const FHOLUnitSpawnRequest& Request);

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
	/**
	 * Population places taken by each player's units (J3, D282), births waiting in the queue included: OutPopulation
	 * gets one entry per OwnerPlayer (D233) up to the highest one with a unit.
	 */
	void GetPopulationByOwner(TArray<int32>& OutPopulation) const;
	/** "3 units: #1 Peasant slot 0 at (x, y, z) LOD 0 visible, drawn as instances; ...", the first MaxUnits (hol.Units.Report). */
	FString DescribeUnits(int32 MaxUnits) const;
	/** The units simulated here (solo, server), where they are (J4: what the player selects and orders in solo). */
	void GetUnitViews(TArray<struct FHOLUnitView>& OutUnits) const;
	/** One unit simulated here; false when it is gone. */
	bool GetUnitView(uint32 UnitId, struct FHOLUnitView& OutUnit) const;

	/** Restarts the draw of birth positions: two stress tests in one process start from the same crowd (D230). */
	void ResetSpawnSeed() { SpawnRandom.Initialize(SpawnSeed); }

	/** Mass entity template of a type, built once from its row and cached (D225); nullptr for an unknown type. */
	const FMassEntityTemplate* FindTemplate(FName TypeName, EHOLUnitTemplateKind Kind = EHOLUnitTemplateKind::Simulation);

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Deinitialize() override;

private:
	/** A unit of the identifier table (D223). */
	struct FHOLUnitRecord
	{
		FMassEntityHandle Entity;
		FName TypeName;
		uint8 OwnerPlayer = 0;
	};

	const UDataTable* GetTypeTable() const;
	/** Births created in one step: the setting, or the whole queue when it is 0 (stress tests measure the peak). */
	int32 GetBirthBudget() const;
	void CreateUnitEntities(const FHOLUnitSpawnRequest& Request, const FMassEntityTemplate& Template, int32 Count);

	/** Identifiers and their entities (D223), plus the reverse lookup. */
	TMap<uint32, FHOLUnitRecord> Units;
	TMap<FMassEntityHandle, uint32> EntityToId;
	uint32 NextUnitId = 1;

	/** Births waiting for their share of the per-step budget, and units to destroy at the end of the step (D224). */
	TArray<FHOLUnitSpawnRequest> BirthQueue;
	TArray<uint32> PendingDestruction;

	/** One Mass entity configuration, so one archetype, per type, built from the table (D225). */
	TMap<FName, TSharedPtr<struct FMassEntityConfig>> TemplateCache;
	/** The same, drawing only, for a client's display entities (D271). */
	TMap<FName, TSharedPtr<struct FMassEntityConfig>> DisplayTemplateCache;
	/** Fingerprint of each type's constants (D222), to catch two types sharing one copy. */
	TMap<uint32, FName> ConstantsCrcToType;
	/** Keeps the traits alive: FMassEntityConfig is held outside of a UPROPERTY. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMassEntityTraitBase>> Traits;

	/** A UPROPERTY, so that the garbage collector keeps the table (J3: the HUD and the production read it all along). */
	UPROPERTY(Transient)
	mutable TObjectPtr<const UDataTable> CachedTypeTable = nullptr;
	mutable bool bTypeTableLookedUp = false;

	/** Birth positions come from a fixed seed, so two runs of a stress test are comparable (D230). */
	static constexpr int32 SpawnSeed = 1066;
	FRandomStream SpawnRandom{ SpawnSeed };
};
