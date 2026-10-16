// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MassEntityTypes.h"
#include "Match/HOLResources.h"
#include "HOLUnitTypes.generated.h"

class UAnimSequenceTransformProviderData;
class USkeletalMesh;
class UStaticMesh;

/**
 * A sequence of a unit's animation provider for each of the four base resources (J5c.1): what it plays gathering each
 * one, or walking with a load of it. INDEX_NONE (-1): none, the unit plays its idle or walk instead. Written
 * "(Food=4,Wood=2,Stone=3,Gold=3)" in the table, like GatherRates.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLResourceSequences
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "-1"))
	int32 Food = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "-1"))
	int32 Wood = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "-1"))
	int32 Stone = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "-1"))
	int32 Gold = INDEX_NONE;

	int32& operator[](const EHOLResource Resource)
	{
		switch (Resource)
		{
		case EHOLResource::Wood: return Wood;
		case EHOLResource::Stone: return Stone;
		case EHOLResource::Gold: return Gold;
		default: return Food;
		}
	}

	int32 operator[](const EHOLResource Resource) const { return const_cast<FHOLResourceSequences&>(*this)[Resource]; }

	/** One of the four is Sequence. */
	bool Contains(const int32 Sequence) const
	{
		return Sequence != INDEX_NONE && (Food == Sequence || Wood == Sequence || Stone == Sequence || Gold == Sequence);
	}

	bool operator==(const FHOLResourceSequences& Other) const = default;
};

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

	/** Drawing scale of the mesh (J3: units larger than life, as in RTS games, to read them next to buildings). */
	UPROPERTY()
	float MeshScale = 1.f;

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

	/** Gathering at a source and walking with a load (J5c.1), by resource; INDEX_NONE: none. */
	UPROPERTY()
	FHOLResourceSequences GatherSequences;

	UPROPERTY()
	float GatherPlayRate = 1.f;

	UPROPERTY()
	FHOLResourceSequences CarryWalkSequences;

	UPROPERTY()
	float CarryWalkPlayRate = 1.f;

	/** Lengths (s) of the provider's sequences by index, read from it by the factory: random start offsets (D234). */
	UPROPERTY()
	TArray<float> SequenceLengths;

	/** Gathering (J5, D296): rate per resource (per second, 0 = cannot gather it) and load carried to a deposit. */
	UPROPERTY()
	FHOLResourceRates GatherRates;

	UPROPERTY()
	int32 CarryCapacity = 0;
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
	 * Drawing scale of the mesh (J3, user's choice: x1.5). RTS units are drawn larger than life so they read next to the
	 * buildings; Radius is set to match in the table, and the walk is slowed by the same factor so the feet do not slide.
	 */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0.1"))
	float MeshScale = 1.f;

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

	/**
	 * Sequence played gathering each resource at its source (J5c.1: axe, pick, picking berries), whatever the speed: a
	 * gatherer pushed aside keeps working. -1: none, it stands idle.
	 */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	FHOLResourceSequences GatherSequences;

	/** Play rate of the gathering sequences (a gesture, not a stride: the mesh scale does not change it). */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0.01"))
	float GatherPlayRate = 1.f;

	/** Walk played carrying a load of each resource (J5c.1), in place of WalkSequence. -1: the plain walk. */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	FHOLResourceSequences CarryWalkSequences;

	/** Play rate of the carrying walks, set as WalkPlayRate is (Speed / the walk's planted-foot speed, D234). */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0.01"))
	float CarryWalkPlayRate = 1.f;

	/** What producing one unit costs (J3, D283), taken when it is queued. Read by the production, not by Mass. */
	UPROPERTY(EditAnywhere, Category = "Production")
	FHOLResources Cost;

	/** Production time (s) at its building (J3, D283). */
	UPROPERTY(EditAnywhere, Category = "Production", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float BuildTime = 20.f;

	/** Population places the unit takes (§ 5.3; an elite unit of the Heritiers takes 2). */
	UPROPERTY(EditAnywhere, Category = "Production", meta = (ClampMin = "0"))
	int32 Population = 1;

	/** How fast the unit gathers each resource (per second, J5, D296: ~0.7, indicative); 0 everywhere = not a gatherer. */
	UPROPERTY(EditAnywhere, Category = "Gathering")
	FHOLResourceRates GatherRates;

	/** Load the unit carries before it walks to a deposit (J5, D296: 10; the Wheelbarrow of § 11 will raise it). */
	UPROPERTY(EditAnywhere, Category = "Gathering", meta = (ClampMin = "0"))
	int32 CarryCapacity = 0;

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
	Constants.MeshScale = MeshScale;
	Constants.SkinnedMesh = SkinnedMesh.ToSoftObjectPath();
	Constants.AnimationProvider = AnimationProvider.ToSoftObjectPath();
	Constants.IdleSequence = IdleSequence;
	Constants.WalkSequence = WalkSequence;
	Constants.WalkPlayRate = WalkPlayRate;
	Constants.GatherSequences = GatherSequences;
	Constants.GatherPlayRate = GatherPlayRate;
	Constants.CarryWalkSequences = CarryWalkSequences;
	Constants.CarryWalkPlayRate = CarryWalkPlayRate;
	Constants.GatherRates = GatherRates;
	Constants.CarryCapacity = CarryCapacity;
	return Constants;
}
