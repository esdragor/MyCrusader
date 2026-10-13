// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
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

	/** The row of a type, or null (logged) if the table or the row is missing. */
	static const FHOLBuildingTypeRow* FindType(FName TypeName);
};

/** Row names of the building type table used by the code. */
namespace HOLBuildingTypeNames
{
	inline const FName MainCentre(TEXT("MainCentre"));
}
