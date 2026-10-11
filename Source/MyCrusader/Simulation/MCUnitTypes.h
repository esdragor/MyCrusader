// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MassEntityTypes.h"
#include "MCUnitTypes.generated.h"

class UStaticMesh;

/**
 * Runtime cache of one unit type (D222): a single copy per type, shared by all its units, so the static values
 * are not duplicated 2 000 times and balancing them in the table reaches every unit of the type.
 */
USTRUCT()
struct MYCRUSADER_API FMCUnitTypeConstants : public FMassConstSharedFragment
{
	GENERATED_BODY()

	FName TypeName;
	float Radius = 40.f;
	float Speed = 300.f;
	int32 MaxHealth = 100;
	float AvoidanceDistance = 250.f;
	FSoftObjectPath Mesh;
	float MeshYawOffset = -90.f;
};

/**
 * One unit type (D221): a row of /Game/Units/UnitTypes, one row per type, values and soft references.
 * The values are provisional by nature and are balanced here (D227: the CSV Tools/Data/UnitTypes.csv is the
 * source of the asset, imported by Tools/Editor/make_unit_types.py).
 */
USTRUCT()
struct MYCRUSADER_API FMCUnitTypeRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Shown in the logs and reports. */
	UPROPERTY(EditAnywhere, Category = "Display")
	FString DisplayName;

	/** Body radius (cm): the avoidance solver and the navigation use it (FAgentRadiusFragment). */
	UPROPERTY(EditAnywhere, Category = "Unit", meta = (ClampMin = "1", ForceUnits = "cm"))
	float Radius = 40.f;

	/** Walk speed (cm/s): the movement parameters of the type. */
	UPROPERTY(EditAnywhere, Category = "Unit", meta = (ClampMin = "1", ForceUnits = "cm/s"))
	float Speed = 300.f;

	/** Health points at birth (M5 will spend them). */
	UPROPERTY(EditAnywhere, Category = "Unit", meta = (ClampMin = "1"))
	int32 MaxHealth = 100;

	/**
	 * Distance (cm) within which the unit looks for other units to avoid (D220). The value lived in the
	 * avoidance trait during the Mass trial; it belongs to the unit type now that the types are described.
	 */
	UPROPERTY(EditAnywhere, Category = "Avoidance", meta = (ClampMin = "0", ForceUnits = "cm"))
	float AvoidanceDistance = 250.f;

	/** Mesh drawn as an instance (M2 draws the instances per type and per player, D226). */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** Yaw (deg) added to the drawn instance: the mesh does not necessarily face Mass' forward (+X). */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	float MeshYawOffset = -90.f;

	FMCUnitTypeConstants ToConstants(FName TypeName) const;
};

inline FMCUnitTypeConstants FMCUnitTypeRow::ToConstants(const FName TypeName) const
{
	FMCUnitTypeConstants Constants;
	Constants.TypeName = TypeName;
	Constants.Radius = Radius;
	Constants.Speed = Speed;
	Constants.MaxHealth = MaxHealth;
	Constants.AvoidanceDistance = AvoidanceDistance;
	Constants.Mesh = Mesh.ToSoftObjectPath();
	Constants.MeshYawOffset = MeshYawOffset;
	return Constants;
}
