// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "Match/HOLResources.h"
#include "HOLBuildingTypes.generated.h"

class UStaticMesh;

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

	/** Ground footprint (cm), X by Y. */
	UPROPERTY(EditAnywhere, Category = "Building", meta = (ForceUnits = "cm"))
	FVector2D Footprint = FVector2D(1500.0, 1500.0);

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
	 * Resources its owner's gatherers bring to it (J5, D296): the main centre takes the four, the camps of the
	 * construction step one each. Empty: not a deposit.
	 */
	UPROPERTY(EditAnywhere, Category = "Gathering")
	TArray<EHOLResource> Accepts;

	bool AcceptsResource(const EHOLResource Resource) const { return Accepts.Contains(Resource); }
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

	/** The row of a type, or null (logged) if the table or the row is missing. */
	static const FHOLBuildingTypeRow* FindType(FName TypeName);
};

/** Row names of the building type table used by the code. */
namespace HOLBuildingTypeNames
{
	inline const FName MainCentre(TEXT("MainCentre"));
	/** Provisional (J5a, stress test S7): a wood deposit; the construction step will give the camps their place (D83). */
	inline const FName LumberCamp(TEXT("LumberCamp"));
}
