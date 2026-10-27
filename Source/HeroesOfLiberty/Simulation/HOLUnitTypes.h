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
 * Armour category of a unit type (J7-Q4, D21): what the bonus damage of the others is aimed at (the Spearman's bonus
 * against cavalry). Read by the combat rules (HOLCombat, J7b).
 */
UENUM()
enum class EHOLArmorCategory : uint8
{
	LightInfantry,
	HeavyInfantry,
	Ranged,
	Cavalry,
	Siege,
	Peasant,
	GunpowderInfantry,
	/** Cross-cutting tag of the units in heavy armour (D428, AoE4's Heavy), added to their other categories: what the
	 * Crossbowman's one bonus aims at. */
	Heavy,
	/**
	 * The King of Regicide (V5.1, D206 revised by the user on 2026-10-25): never healed. No healing exists yet; the ones to
	 * come (the Monk, the auras) skip a unit of this category (HOLCanBeHealed). Aimed at by no bonus for now.
	 */
	King,
};

/** Bit of a category in FHOLUnitTypeConstants::ArmorCategoryMask (D345: a unit has 1 to any number of categories). */
inline int32 HOLArmorCategoryBit(const EHOLArmorCategory Category)
{
	return 1 << static_cast<int32>(Category);
}

/** Whether a healing may aim at a unit of these categories (V5.1): never the King. Read by the healings to come. */
inline bool HOLCanBeHealed(const int32 ArmorCategoryMask)
{
	return (ArmorCategoryMask & HOLArmorCategoryBit(EHOLArmorCategory::King)) == 0;
}

/** Whether a unit of these categories is a King (V5.2: its death defeats its slot, AHOLGameState::HandleKingDeath). */
inline bool HOLIsKing(const int32 ArmorCategoryMask)
{
	return (ArmorCategoryMask & HOLArmorCategoryBit(EHOLArmorCategory::King)) != 0;
}

/**
 * Type of an attack (D345): which of the target's four armours it takes off (MeleeArmor, RangedArmor, SiegeArmor,
 * HeroArmor). Explicit in the table, no longer inferred from the reach.
 */
UENUM()
enum class EHOLAttackType : uint8
{
	Melee,
	Ranged,
	Siege,
	Hero,
};

/**
 * One bonus of an attack (D345): Damage added to each hit on a unit of Category. A unit's bonuses add up for each of
 * the target's categories they aim at. Written "((Category=LightInfantry,Damage=5),(Category=GunpowderInfantry,Damage=5))"
 * in the table, "()" for none.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLAttackBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Combat")
	EHOLArmorCategory Category = EHOLArmorCategory::LightInfantry;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
	int32 Damage = 0;
};

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
 * A prop of a unit's skinned mesh for each of the four base resources (J5c.2): the tool it holds gathering each one, or
 * the load it carries. A prop is part of the mesh, hidden unless the instance's 4th custom float names it (the Peasant's
 * material, Tools/Editor/make_peasant_mesh.py): 1 axe, 2 pickaxe, 3 basket of berries, 4 logs, 5 hotte of stone, 6 hotte
 * of gold, 7 hammer (the builder's, BuildProp), 8 knife (the Peasant's strike, AttackProp). 0: none. At most MaxId: a vertex of the mesh names the props it belongs to by the bits of one byte. Written
 * "(Food=3,Wood=4,Stone=5,Gold=6)" in the table, like GatherRates.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLResourceProps
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "0", ClampMax = "8"))
	int32 Food = 0;

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "0", ClampMax = "8"))
	int32 Wood = 0;

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "0", ClampMax = "8"))
	int32 Stone = 0;

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "0", ClampMax = "8"))
	int32 Gold = 0;

	/** Highest prop id: the material reads a vertex's props as the 8 bits of its colour's alpha. */
	static constexpr int32 MaxId = 8;

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

	int32 operator[](const EHOLResource Resource) const { return const_cast<FHOLResourceProps&>(*this)[Resource]; }

	bool operator==(const FHOLResourceProps& Other) const = default;
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

	/** Standing with a load (J5c.2), by resource; INDEX_NONE: the plain idle. */
	UPROPERTY()
	FHOLResourceSequences CarryIdleSequences;

	/** Prop shown gathering each resource and carrying a load of it (J5c.2); 0: none. */
	UPROPERTY()
	FHOLResourceProps GatherProps;

	UPROPERTY()
	FHOLResourceProps CarryProps;

	/** Working at a site (J6a.4): its gesture (INDEX_NONE: none, it stands idle), at its play rate, and its prop (0: none). */
	UPROPERTY()
	int32 BuildSequence = INDEX_NONE;

	UPROPERTY()
	float BuildPlayRate = 1.f;

	UPROPERTY()
	int32 BuildProp = 0;

	/**
	 * Fighting (J7b.5): its strike, looped at its attack cadence, and its fall while dying, played once (INDEX_NONE: none, it
	 * plays its idle).
	 */
	UPROPERTY()
	int32 AttackSequence = INDEX_NONE;

	/** Prop shown while striking (J7b: the Peasant's knife, 8). 0: none. */
	UPROPERTY()
	int32 AttackProp = 0;

	UPROPERTY()
	int32 FallSequence = INDEX_NONE;

	/** Lengths (s) of the provider's sequences by index, read from it by the factory: random start offsets (D234). */
	UPROPERTY()
	TArray<float> SequenceLengths;

	/** Gathering (J5, D296): rate per resource (per second, 0 = cannot gather it) and load carried to a deposit. */
	UPROPERTY()
	FHOLResourceRates GatherRates;

	UPROPERTY()
	int32 CarryCapacity = 0;

	/**
	 * Combat (J7-Q4, J7b.1, D345): the table's combat columns, read by HOLCombat and the combat processor (J7b.2). Its
	 * armour categories as a mask of HOLArmorCategoryBit (a UPROPERTY like every field: the shared fragment is told apart
	 * by its properties).
	 */
	UPROPERTY()
	int32 ArmorCategoryMask = 0;

	UPROPERTY()
	EHOLAttackType AttackType = EHOLAttackType::Melee;

	UPROPERTY()
	int32 Attack = 0;

	/** Time (s) between two hits. */
	UPROPERTY()
	float AttackInterval = 1.5f;

	/** Share (%) of AttackInterval its first hit waits once it starts attacking (HOLCombat::GetWindUpMilliseconds). */
	UPROPERTY()
	int32 AttackWindUpPercent = 50;

	/** Reach (cm) between the two bodies' edges. */
	UPROPERTY()
	float AttackRange = 50.f;

	UPROPERTY()
	int32 MeleeArmor = 0;

	UPROPERTY()
	int32 RangedArmor = 0;

	UPROPERTY()
	int32 SiegeArmor = 0;

	UPROPERTY()
	int32 HeroArmor = 0;

	/** Its attack's bonuses (D345), added up for each of the target's categories they aim at. */
	UPROPERTY()
	TArray<FHOLAttackBonus> Bonuses;

	bool HasArmorCategory(const EHOLArmorCategory Category) const
	{
		return (ArmorCategoryMask & HOLArmorCategoryBit(Category)) != 0;
	}

	/** Radius (cm, edge to edge) of the riposte; 0: only when ordered. */
	UPROPERTY()
	float AggressionRadius = 0.f;

	/** With no AggressionRadius, it still takes on the enemy unit hitting it (V5.1, the King), never one by itself. */
	UPROPERTY()
	bool bRipostes = false;

	/** Its attack against a building (D345.2b): type, damage (0: it cannot hit buildings), interval (s), reach (cm, its edge to the body's box). */
	UPROPERTY()
	EHOLAttackType BuildingAttackType = EHOLAttackType::Melee;
	UPROPERTY()
	int32 BuildingAttack = 0;
	UPROPERTY()
	float BuildingAttackInterval = 2.1f;
	UPROPERTY()
	float BuildingAttackRange = 400.f;
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

	/**
	 * Idle played standing with a load of each resource (J5c.2: the load stays held, at the play rate of the idle), in
	 * place of IdleSequence. -1: the plain idle (and no load shown).
	 */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	FHOLResourceSequences CarryIdleSequences;

	/**
	 * Prop of SkinnedMesh shown while gathering each resource (J5c.2: axe for wood, pickaxe for stone and gold, bare hands
	 * for berries). 0: none.
	 */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	FHOLResourceProps GatherProps;

	/** Prop shown carrying a load of each resource, walking or standing (J5c.2: basket, logs, hottes). 0: none. */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	FHOLResourceProps CarryProps;

	/**
	 * Sequence played working at a site (J6a.4: the Peasant's hammering), whatever the speed, as a gathering gesture. -1:
	 * none, it stands idle.
	 */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "-1"))
	int32 BuildSequence = INDEX_NONE;

	/** Play rate of BuildSequence (a gesture: the mesh scale does not change it). */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0.01"))
	float BuildPlayRate = 1.f;

	/** Prop of SkinnedMesh shown while building (J6a.4: the hammer, 7). 0: none. */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0", ClampMax = "8"))
	int32 BuildProp = 0;

	/**
	 * Sequence played striking (J7b.5: the activity Attacking), looped at the attack cadence: its play rate is its length over
	 * AttackInterval, and its loop starts so that its hit (HOLUnitAnimation::GetStrikeHitShare of the loop) falls on the
	 * server's. -1: none, it plays its idle.
	 */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "-1"))
	int32 AttackSequence = INDEX_NONE;

	/** Prop of SkinnedMesh shown while striking (J7b: the Peasant's knife, 8), the others hidden. 0: none. */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "0", ClampMax = "8"))
	int32 AttackProp = 0;

	/** Sequence played once while dying (J7b.5: the activity Dying, ~1 s), then held on its last frame. -1: none. */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "-1"))
	int32 FallSequence = INDEX_NONE;

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

	// Combat (J7-Q4, J7a.1, D345): filled in the table, copied into the shared fragment (J7b.1), read by HOLCombat. Damage
	// of a hit = Attack + the bonuses aimed at any of the target's categories - the target's armour of the attack's type
	// (melee, ranged, siege or hero), 1 at least.

	/** Armour categories (D21, D345: 1 to any number), what the others' bonuses aim at. Written "(Ranged,GunpowderInfantry)". */
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<EHOLArmorCategory> ArmorCategories;

	/** Type of its attack (D345): which armour of the target it takes off. */
	UPROPERTY(EditAnywhere, Category = "Combat")
	EHOLAttackType AttackType = EHOLAttackType::Melee;

	/** Damage of one hit, before bonus and armour. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
	int32 Attack = 0;

	/** Time (s) between two hits. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float AttackInterval = 1.5f;

	/**
	 * Wind-up (J7c, retour d'essai 8: hit and run): the share (%) of AttackInterval its first hit waits once it starts
	 * attacking (HOLCombat::StartAttack), and where the hit falls in its strike's loop (HOLUnitAnimation::GetStrikeHitShare).
	 * 50 for the melee (its gestures, J7b); 12 for the Archer (~0.2 s), which looses soon after it stops, so it can shoot
	 * and walk off.
	 */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "1", ClampMax = "100"))
	int32 AttackWindUpPercent = 50;

	/** Reach (cm) between the edges of the two bodies: a short one for a melee unit, a long one for a ranged unit. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0", ForceUnits = "cm"))
	float AttackRange = 50.f;

	/** Taken off each melee hit received. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
	int32 MeleeArmor = 0;

	/** Taken off each ranged hit received. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
	int32 RangedArmor = 0;

	/** Taken off each siege hit received. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
	int32 SiegeArmor = 0;

	/** Taken off each hero hit received. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
	int32 HeroArmor = 0;

	/** Its attack's bonuses (D345: the Spearman's +17 against cavalry, the Archer's +5 against light and gunpowder infantry). */
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FHOLAttackBonus> Bonuses;

	/** True if the row lists this armour category. */
	bool HasArmorCategory(const EHOLArmorCategory Category) const
	{
		return ArmorCategories.Contains(Category);
	}

	/** Radius (cm) within which the unit takes on an enemy it sees by itself (J7-Q4 riposte); 0: only when ordered. */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0", ForceUnits = "cm"))
	float AggressionRadius = 0.f;

	/**
	 * It strikes back at the enemy unit hitting it (column Ripostes). Only read with no AggressionRadius (a unit with one
	 * ripostes anyway): the King (V5.1) ripostes but never takes on an enemy by itself; the Peasant does neither.
	 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool Ripostes = false;

	/**
	 * Its attack against a building (D345.2b, always this one against a building): its type (the building's armour of that
	 * type taken off, HOLConstruction::GetBuildingDamage), damage (0: it never attacks a building), interval (s) and reach
	 * (cm, from its body's edge to the building's body box, D324).
	 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	EHOLAttackType BuildingAttackType = EHOLAttackType::Melee;
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
	int32 BuildingAttack = 0;
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float BuildingAttackInterval = 2.1f;
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0", ForceUnits = "cm"))
	float BuildingAttackRange = 400.f;

	/**
	 * The projectile drawn in flight for its shots (AR9, AR-Q5): SM_Arrow for the Archer, SM_Bolt for the Crossbowman, the
	 * same flight for both. Read by the arrow display only (UHOLArrowSubsystem, by the shooter's type), never by the
	 * simulation; empty (the melee types): SM_Arrow.
	 */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	TSoftObjectPtr<UStaticMesh> Projectile;

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
	Constants.CarryIdleSequences = CarryIdleSequences;
	Constants.GatherProps = GatherProps;
	Constants.CarryProps = CarryProps;
	Constants.BuildSequence = BuildSequence;
	Constants.BuildPlayRate = BuildPlayRate;
	Constants.BuildProp = BuildProp;
	Constants.AttackSequence = AttackSequence;
	Constants.AttackProp = AttackProp;
	Constants.FallSequence = FallSequence;
	Constants.GatherRates = GatherRates;
	Constants.CarryCapacity = CarryCapacity;
	for (const EHOLArmorCategory Category : ArmorCategories)
	{
		Constants.ArmorCategoryMask |= HOLArmorCategoryBit(Category);
	}
	Constants.AttackType = AttackType;
	Constants.Attack = Attack;
	Constants.AttackInterval = AttackInterval;
	Constants.AttackWindUpPercent = FMath::Clamp(AttackWindUpPercent, 1, 100);
	Constants.AttackRange = AttackRange;
	Constants.MeleeArmor = MeleeArmor;
	Constants.RangedArmor = RangedArmor;
	Constants.SiegeArmor = SiegeArmor;
	Constants.HeroArmor = HeroArmor;
	Constants.Bonuses = Bonuses;
	Constants.AggressionRadius = AggressionRadius;
	Constants.bRipostes = Ripostes;
	Constants.BuildingAttackType = BuildingAttackType;
	Constants.BuildingAttack = BuildingAttack;
	Constants.BuildingAttackInterval = BuildingAttackInterval;
	Constants.BuildingAttackRange = BuildingAttackRange;
	return Constants;
}
