// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MassEntityTypes.h"
#include "HOLUnitTypes.generated.h"

class UAnimSequenceTransformProviderData;
class USkeletalMesh;
class UStaticMesh;

/**
 * Runtime cache of one unit type (D222): a single copy per type, shared by all its units, so the static values
 * are not duplicated 2 000 times and balancing them in the table reaches every unit of the type.
 *
 * Every field must be a UPROPERTY: Mass identifies a shared fragment by the CRC of its reflected fields, then
 * compares the values through that same reflection, so a field left out of the reflection is invisible to both
 * and two types would silently share one copy (the second type walking at the first one's speed). The factory
 * checks the fingerprint of each type for exactly that.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLUnitTypeConstants : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FName TypeName;

	UPROPERTY()
	float Radius = 40.f;

	UPROPERTY()
	float Speed = 300.f;

	UPROPERTY()
	int32 MaxHealth = 100;

	UPROPERTY()
	float AvoidanceDistance = 250.f;

	UPROPERTY()
	FSoftObjectPath Mesh;

	UPROPERTY()
	float MeshYawOffset = -90.f;

	/** Instanced skeletal mesh and its animation provider (D232); the static Mesh is the fallback. */
	UPROPERTY()
	FSoftObjectPath SkinnedMesh;

	UPROPERTY()
	FSoftObjectPath AnimationProvider;

	UPROPERTY()
	int32 IdleSequence = 0;

	UPROPERTY()
	int32 WalkSequence = 1;

	UPROPERTY()
	float WalkPlayRate = 1.f;

	/** Lengths (s) of the two sequences, read from the provider by the factory: random start offsets (D234). */
	UPROPERTY()
	float IdleLength = 0.f;

	UPROPERTY()
	float WalkLength = 0.f;
};

/**
 * One unit type (D221): a row of /Game/Units/UnitTypes, one row per type, values and soft references.
 * The values are provisional by nature and are balanced here (D227: the CSV Tools/Data/UnitTypes.csv is the
 * source of the asset, imported by Tools/Editor/make_unit_types.py).
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLUnitTypeRow : public FTableRowBase
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

	/**
	 * Instanced skeletal mesh (D232): when set, the unit is drawn animated and Mesh is not used. Its mesh must be
	 * built with "Optimize for Instancing" (Tools/Editor/make_skinned_mannequin.py).
	 */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	TSoftObjectPtr<USkeletalMesh> SkinnedMesh;

	/** GPU animation of SkinnedMesh: its sequences are the ones the presentation plays (D234). */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	TSoftObjectPtr<UAnimSequenceTransformProviderData> AnimationProvider;

	/** Index of the idle sequence in AnimationProvider. */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0"))
	int32 IdleSequence = 0;

	/** Index of the walk sequence in AnimationProvider. */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0"))
	int32 WalkSequence = 1;

	/** Play rate of the walk, so the stride matches Speed and the feet do not slide (D234). */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0.01"))
	float WalkPlayRate = 1.f;

	FHOLUnitTypeConstants ToConstants(FName TypeName) const;
};

inline FHOLUnitTypeConstants FHOLUnitTypeRow::ToConstants(const FName TypeName) const
{
	FHOLUnitTypeConstants Constants;
	Constants.TypeName = TypeName;
	Constants.Radius = Radius;
	Constants.Speed = Speed;
	Constants.MaxHealth = MaxHealth;
	Constants.AvoidanceDistance = AvoidanceDistance;
	Constants.Mesh = Mesh.ToSoftObjectPath();
	Constants.MeshYawOffset = MeshYawOffset;
	Constants.SkinnedMesh = SkinnedMesh.ToSoftObjectPath();
	Constants.AnimationProvider = AnimationProvider.ToSoftObjectPath();
	Constants.IdleSequence = IdleSequence;
	Constants.WalkSequence = WalkSequence;
	Constants.WalkPlayRate = WalkPlayRate;
	return Constants;
}
