// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "Match/HOLResources.h"
#include "HOLBuildingTypes.generated.h"

class UStaticMesh;

/**
 * Where a building stands in its construction (J6, D311, D312), replicated with it. The main centres (and the camps
 * the stress tests place by code) are born finished; a building placed by a player is born a foundation.
 */
UENUM()
enum class EHOLConstructionState : uint8
{
	/** Placed and paid (D311): a flat outline on the ground, no work yet; never blocks the navmesh (D312). */
	Foundation,
	/** Its first work done (J6a.4): blocks the navmesh, rises from the ground; still gives nothing. */
	UnderConstruction,
	/** Serves its owner: population (D83), deposit (D296), production. */
	Finished,
};

/**
 * What a building is for (D30, the user's trial of J6a.5): F1 selects the player's military buildings, F2 its economic
 * ones, F3 its technological ones. The centre, houses and camps are economic (§ 17), the Barracks military (J7-Q2).
 */
UENUM()
enum class EHOLBuildingCategory : uint8
{
	Economic,
	Military,
	Technological,
};

/**
 * One building type (D260): a row of /Game/Buildings/BuildingTypes, imported from Tools/Data/BuildingTypes.csv by
 * Tools/Editor/make_building_types.py, like the unit types (D221, D227). The values are provisional by nature and
 * are balanced in the CSV.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLBuildingTypeRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Shown to the player (J3 panel) and in the logs. */
	UPROPERTY(EditAnywhere, Category = "Display")
	FText DisplayName;

	/** Ground footprint (cm), X by Y: whole tiles (UHOLBuildingSettings::TileSize, D311). */
	UPROPERTY(EditAnywhere, Category = "Building", meta = (ForceUnits = "cm"))
	FVector2D Footprint = FVector2D(1200.0, 1200.0);

	/** Height of the provisional shape (cm). */
	UPROPERTY(EditAnywhere, Category = "Building", meta = (ClampMin = "1", ForceUnits = "cm"))
	float Height = 1000.f;

	/** Health points (M5 will spend them, with GAS). */
	UPROPERTY(EditAnywhere, Category = "Building", meta = (ClampMin = "1"))
	int32 MaxHealth = 2000;

	/** Final model; empty = the provisional shape of D261 (a block and a tower, the top at the owner's colour). */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** Population places the building gives its owner (D83: the centres give a base, the houses add to it). */
	UPROPERTY(EditAnywhere, Category = "Production", meta = (ClampMin = "0"))
	int32 PopulationProvided = 0;

	/** Unit types the building produces (rows of the unit type table), in the order of its command grid (D30). */
	UPROPERTY(EditAnywhere, Category = "Production")
	TArray<FName> Produces;

	/** Longest production queue (D283). */
	UPROPERTY(EditAnywhere, Category = "Production", meta = (ClampMin = "1"))
	int32 MaxQueue = 5;

	/**
	 * Resources its owner's gatherers bring to it (J5, D296): the main centre takes the four, the lumber camp wood,
	 * the mining camp stone and gold (D310). Empty: not a deposit.
	 */
	UPROPERTY(EditAnywhere, Category = "Gathering")
	TArray<EHOLResource> Accepts;

	bool AcceptsResource(const EHOLResource Resource) const { return Accepts.Contains(Resource); }

	/** What placing its foundation costs (D311: taken at placement, given back in full or by half if cancelled). */
	UPROPERTY(EditAnywhere, Category = "Construction")
	FHOLResources Cost;

	/** Construction time (s) for a single builder (D312); more builders shorten it, less and less (HOLConstruction). */
	UPROPERTY(EditAnywhere, Category = "Construction", meta = (ClampMin = "0", ForceUnits = "s"))
	float BuildTime = 0.f;

	/**
	 * Its slot in the construction grid of the builders (D313), the grid of D30 read row by row: 0 = A, 1 = Z,
	 * 2 = E, 3 = R (the Barracks, J7a.2), 4 = Q... -1: nobody builds it (the main centre comes with the map). One page
	 * for now: the pages come later, by age (AoE4).
	 */
	UPROPERTY(EditAnywhere, Category = "Construction", meta = (ClampMin = "-1", ClampMax = "11"))
	int32 BuildGridSlot = INDEX_NONE;

	bool IsBuildable() const { return BuildGridSlot != INDEX_NONE; }

	/** Its category (D30): the F key that selects it with the others of its kind (F1 military, F2 economic, F3 technological). */
	UPROPERTY(EditAnywhere, Category = "Display")
	EHOLBuildingCategory Category = EHOLBuildingCategory::Economic;
};

/** Where the building type table lives (DefaultGame.ini). */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "HeroesOfLiberty Buildings"))
class HEROESOFLIBERTY_API UHOLBuildingSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Building type table (D260): one row per type. Nothing builds without it. */
	UPROPERTY(config, EditAnywhere, Category = "Buildings")
	TSoftObjectPtr<UDataTable> BuildingTypesTable;

	/**
	 * Gap (cm) every unit's body keeps from a building's footprint (D310, J6a.1), the same for every building: the
	 * navmesh's forbidden area is the footprint widened by it (the navmesh widens it again by the agent's radius, the
	 * largest body). A body pushed into a footprint anyway is put back against its wall (UHOLFootprintKeepOutProcessor).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Buildings", meta = (ClampMin = "0", ForceUnits = "cm"))
	float BodyClearance = 25.f;

	static float GetBodyClearance() { return FMath::Max(0.f, GetDefault<UHOLBuildingSettings>()->BodyClearance); }

	/**
	 * How far (cm) a building's body stands inside its tiles, on every side, whatever its size (D324, the user's rule of
	 * 2026-10-17). The tiles (the table's Footprint) are what it occupies: grid, placement, overlaps, map, anti-rush, the
	 * placement ghost's validity. The body (HOLConstruction::GetBodySize) is what blocks (its visible shape is wider,
	 * VisualInset): the navmesh's forbidden area (widened by BodyClearance, D310), the footprints' guard, the builders' and the
	 * deposits' spots, the ways a new building cuts (J6b.2). Two neighbours whose tiles touch leave twice this between their
	 * bodies: room for one Peasant (1.2 m wide), not for two abreast.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Buildings", meta = (ClampMin = "0", ForceUnits = "cm"))
	float BodyInset = 105.f;

	static float GetBodyInset() { return FMath::Max(0.f, GetDefault<UHOLBuildingSettings>()->BodyInset); }

	/** The body of a building whose tiles measure Footprint (D324): inset by BodyInset on every side. */
	static FVector2D GetBodySize(const FVector2D& Footprint);

	/**
	 * How far (cm) a building's visible shape stands inside its tiles, on every side, whatever its size (D324 refined, the
	 * user's rule of 2026-10-17: what is seen fills at least 90 % of the tiles). Its provisional shape (D261), its slab and
	 * rising site, its planned ghost, the placement ghost's volume edges, its selection frame and the click (its pieces) use
	 * it; everything that blocks or places units keeps the body (BodyInset), narrower: Peasants walk a little into the
	 * shape, still one at a time between two neighbours.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Buildings", meta = (ClampMin = "0", ForceUnits = "cm"))
	float VisualInset = 30.f;

	static float GetVisualInset() { return FMath::Max(0.f, GetDefault<UHOLBuildingSettings>()->VisualInset); }

	/** The visible shape of a building whose tiles measure Footprint (D324 refined): inset by VisualInset on every side. */
	static FVector2D GetVisualSize(const FVector2D& Footprint);

	/**
	 * Side of a tile (cm, D311, user's choice of 2026-10-16): the placement grid. Footprints are whole tiles (main
	 * centre 4 x 4, houses and camps 2 x 2), their corners on the grid lines, the main centres included (placed on it
	 * at their start, AHOLStartSlot::GetMainCentreLocation): every building lines up and touches its neighbours.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Construction", meta = (ClampMin = "1", ForceUnits = "cm"))
	float TileSize = 300.f;

	static double GetTileSize() { return FMath::Max(1.0, static_cast<double>(GetDefault<UHOLBuildingSettings>()->TileSize)); }

	/**
	 * Anti-rush radius (cm, D22): no footprint may come nearer than this to the middle of an enemy main centre.
	 * Indicative, to tune in test: start slots are 115 m apart at 8 players, the contested veins ~60 m from each.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Construction", meta = (ClampMin = "0", ForceUnits = "cm"))
	float AntiRushRadius = 4000.f;

	/**
	 * Diminishing returns of several builders (D312): N builders work N^BuilderExponent times as fast as one.
	 * 0.63 (ln 2 / ln 3): 3 builders twice as fast, 2 x1.55, 4 x2.4, 6 x3.1, 10 x4.3. 1 = each as useful as the first.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Construction", meta = (ClampMin = "0", ClampMax = "1"))
	float BuilderExponent = 0.63f;

	/** The row of a type, or null (logged) if the table or the row is missing. */
	static const FHOLBuildingTypeRow* FindType(FName TypeName);

	/** The types builders build, each with its cell in their grid (BuildGridSlot, D313), in the table's order. */
	static void GetBuildableTypes(TArray<TPair<FName, int32>>& OutTypes);
};

/** Row names of the building type table used by the code. */
namespace HOLBuildingTypeNames
{
	inline const FName MainCentre(TEXT("MainCentre"));
	/** +10 population (D83, D310). */
	inline const FName House(TEXT("House"));
	/** Wood deposit (D310); S7 places some by code. */
	inline const FName LumberCamp(TEXT("LumberCamp"));
	/** Stone and gold deposit (D310). */
	inline const FName MiningCamp(TEXT("MiningCamp"));
	/** Military (J7-Q2): produces the Spearman and the Man-at-arms; on R of the Peasant's grid (J7a.2). */
	inline const FName Barracks(TEXT("Barracks"));
}
