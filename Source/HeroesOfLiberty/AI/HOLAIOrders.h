// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLAIOrders.generated.h"

/**
 * What a unit or building type costs and does, read from the type tables (BuildingTypes.csv, UnitTypes.csv) for the AI's rules
 * (IA1.6): the numbers the script used to copy. Costs are the four resources; BuildTime in seconds; Population is the places a unit
 * takes or a building gives; the gathering fields are the unit's (0 for a building).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAITypeFigures
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostFood = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostWood = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostStone = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostGold = 0;

	/** Seconds one builder (a building) or the centre (a unit) needs. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float BuildTime = 0.f;

	/** Places a unit takes, or a building gives. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Population = 0;

	/** Walk speed (cm/s) of a unit. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float Speed = 0.f;

	/** What a gatherer carries before it goes back to the deposit. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CarryCapacity = 0;

	/** Resources gathered per second at the source (the unit's rates). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float GatherFood = 0.f;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float GatherWood = 0.f;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float GatherStone = 0.f;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float GatherGold = 0.f;

	// IA2.2, combat (units only; 0 / empty for a building): the table's columns the combat code reads (FHOLUnitTypeConstants),
	// for the script's counter rule.

	/** Health points at birth (MaxHealth). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Health = 0;

	/** Damage of one hit before bonus and armour (Attack). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Damage = 0;

	/** Seconds between two hits. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float AttackInterval = 0.f;

	/** Reach (cm) between the edges of the two bodies. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float AttackRange = 0.f;

	/** Taken off each melee hit received / each ranged hit received. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 MeleeArmor = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 RangedArmor = 0;

	/** Its attack is a ranged one (AttackType Ranged); false for melee, siege and hero attacks. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bRanged = false;

	/** Its armour categories by name (LightInfantry, HeavyInfantry, Ranged, Cavalry, Siege, Peasant, GunpowderInfantry, Heavy, King, Hero, Profane). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<FName> ArmorCategories;

	/**
	 * Its attack's bonuses, two parallel arrays: the FLAT damage added to each hit against a target of BonusCategories[i] (the table
	 * stores an added value, not a multiplier, and the combat code adds it as is: HOLCombat::GetDamage). Bonuses of the same category
	 * are summed; a bonus of 0 is left out.
	 */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<FName> BonusCategories;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	TArray<float> BonusDamage;
};

/**
 * An AI slot's orders for Blueprint and Angelscript (IA0.2, IA0.4's brain): the same queue and the same limits as a
 * human's (HOLPlayerOrders), without a controller. Server and solo only (the authority): refused on a client. Unit
 * identifiers as the summary gives them (FHOLAISummary::IdlePeasants: int32, script has no uint32).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLAIOrderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * PlayerIndex's units gather the source SourceIndex (FHOLAISourceNear::SourceIndex), applied at the next step; its
	 * soldiers among them walk to it. Whether the order entered the queue.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIGatherOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds, int32 SourceIndex, bool bQueue = false);

	/** PlayerIndex's units go to Target, applied at the next step. Whether the order entered the queue. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIMoveOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds, FVector Target, bool bQueue = false);

	/**
	 * T6: PlayerIndex's hero takes the talent of Family in the next row of its tree, applied at the end of the next step (the order a
	 * player's button sends, HOLHeroTalent::CanPick refuses what is not allowed there). Whether the order entered the queue.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAITalentOrder(const UObject* WorldContextObject, int32 PlayerIndex, int32 Family);

	/**
	 * D4/D9: PlayerIndex's hero challenges the hero of TargetSlot to a duel, applied at the end of the next step (the order a player's
	 * command sends: the duel book checks the conditions). Whether the order entered the queue.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIDuelChallenge(const UObject* WorldContextObject, int32 PlayerIndex, int32 TargetSlot);

	/** D4/D9: PlayerIndex's hero answers the challenge waiting for it (accepted or refused), applied at the end of the next step. Whether the order entered the queue. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIDuelAnswer(const UObject* WorldContextObject, int32 PlayerIndex, bool bAccept);

	/**
	 * V11 (volcano, D427): PlayerIndex's units caught in the corridor of a flow in its alert or its eruption (LavaHalfWidth plus their radius
	 * from its line, UHOLVolcanoSubsystem::GetEvacuations) walk out of it, applied at the next step. How many were ordered; 0 without the Catastrophe option.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static int32 IssueAIVolcanoEvacuation(const UObject* WorldContextObject, int32 PlayerIndex);

	/** PlayerIndex's units attack-move to Target (they take on the enemies met on the way), applied at the next step. Whether the order entered the queue. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIAttackMoveOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds, FVector Target, bool bQueue = false);

	/**
	 * PlayerIndex's units attack the unit TargetUnitId (IA4, D572: a Peasant finishing a weakened soldier), applied at the next step.
	 * Whether the order entered the queue; the order's processor refuses a unit of their own camp.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIAttackUnitOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds, int32 TargetUnitId, bool bQueue = false);

	/**
	 * PlayerIndex's units attack the building nearest BuildingLocation (within 100 cm, of ANY owner: an enemy's; the order's processor
	 * refuses a building of their own camp). False when no building stands there.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIAttackBuildingOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds, FVector BuildingLocation, bool bQueue = false);

	// IA1.4: building, production. A building is named by its ground point (FHOLAIBuilding::Location): the summary gives no
	// identifier, the nearest building of the slot within 100 cm is meant.

	/** Why a foundation of TypeName cannot go at Point for PlayerIndex (EHOLPlacementRefusal as an integer: 0 = it can), -1 for an unknown type. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static int32 CheckAIPlacement(const UObject* WorldContextObject, int32 PlayerIndex, FName TypeName, FVector Point);

	/**
	 * The free spot of TypeName nearest Centre, searched in rings of tiles around it (ring 0 touching, up to MaxRings), by the
	 * rules of a player's placement (CheckPlacement: map, buildings, sources, enemy centres, navmesh).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool FindAIPlacement(const UObject* WorldContextObject, int32 PlayerIndex, FName TypeName, FVector Centre, int32 MaxRings, FVector& OutPoint);

	/** A Mill with the 8 Farms around it (IA1.2): the first block, nearest Centre, where all nine are free. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool FindAIFarmBlock(const UObject* WorldContextObject, int32 PlayerIndex, FVector Centre, int32 MaxRings, FVector& OutMill, TArray<FVector>& OutFarms);

	/** PlayerIndex places a foundation of TypeName at Point (cost paid at the next step), UnitIds go and build it. False when the placement is refused. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIPlaceOrder(const UObject* WorldContextObject, int32 PlayerIndex, FName TypeName, FVector Point, const TArray<int32>& UnitIds, bool bQueue = false);

	/** PlayerIndex's units build (or repair) its building at BuildingLocation. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIBuildOrder(const UObject* WorldContextObject, int32 PlayerIndex, FVector BuildingLocation, const TArray<int32>& UnitIds, bool bQueue = false);

	/** One unit of TypeName queued at PlayerIndex's building at BuildingLocation. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIProduceOrder(const UObject* WorldContextObject, int32 PlayerIndex, FVector BuildingLocation, FName TypeName);

	/** The rally point of PlayerIndex's building at BuildingLocation: Point, or the source SourceIndex (INDEX_NONE: the ground). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAISetRallyOrder(const UObject* WorldContextObject, int32 PlayerIndex, FVector BuildingLocation, FVector Point, int32 SourceIndex = -1);

	// IA1.6: the type tables, read as the human's panels read them.

	/**
	 * M6 (Market, MK-Q3): a trade of Lots lots of Resource (0 food, 1 wood, 2 stone) at the finished Market of PlayerIndex at BuildingLocation;
	 * true when the order is queued (the production checks the stock and the price at its step).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIMarketOrder(const UObject* WorldContextObject, int32 PlayerIndex, FVector BuildingLocation, int32 Resource, int32 Lots, bool bBuy);

	/** The common book of prices (food, wood, stone, gold) as the game state holds it; false before a match has set it. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool ReadAIMarketBook(const UObject* WorldContextObject, TArray<int32>& OutPrices);

	/** The figures of a unit type (UnitTypes.csv); false for an unknown type. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool ReadAIUnitFigures(const UObject* WorldContextObject, FName TypeName, FHOLAITypeFigures& OutFigures);

	/** The figures of a building type (BuildingTypes.csv); false for an unknown type. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool ReadAIBuildingFigures(FName TypeName, FHOLAITypeFigures& OutFigures);
};
