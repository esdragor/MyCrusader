// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLSelectionUnitReadLibrary.generated.h"

/**
 * H-9 lot B (the selection block's units and statistics, Script\UI\HOLSelectionUnit.as; HOLSelectionUnitRead:: in script): the selected units as
 * this machine knows them, in simple types. Activity kinds (ActivityKind, WorkerActivities): 0 Idle, 1 Moving, 2 Gathering (Gathering and
 * GatherTrip, D307), 3 Building, 4 Attacking. Resource numbers (LoadResource): 0 Food, 1 Wood, 2 Stone, 3 Gold.
 */
USTRUCT(BlueprintType)
struct FHOLUnitSelView
{
	GENERATED_BODY()

	/** Number of units selected and shown (0: none known to this machine). */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 UnitCount = 0;

	/** The first unit's owner slot, whether the viewer owns it, its colour and its name (Vous / Joueur N / the slot's name), as GetOwnerNameIn of the C++ screen. */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 OwnerSlot = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	bool bOwn = false;

	/** A2 (S2-c step 9): the morale effects on the first unit, one bit per effect row of the table (FHOLUnitView::EffectMask, received from the server); 0: none. */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 EffectMask = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	FLinearColor OwnerColour = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	FText OwnerName;

	/** The types selected, parallel arrays in gathering order (the script sorts them): type key, number, and whether the type carries (a worker, CarryCapacity > 0). */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	TArray<FName> TypeNames;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	TArray<int32> TypeCounts;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	TArray<bool> TypeWorkers;

	/** The activity kind of each worker selected (CarryCapacity > 0), for the group's count by state. */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	TArray<int32> WorkerActivities;

	/** The first unit's own figures (what a single unit shows): health, hero, activity, load. */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	bool bHero = false;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 Health = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 MaxHealth = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	bool bHealthExact = false;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 ActivityKind = 0;

	/** UEnum::GetValueAsString(Activity) and of LoadResource, as the C++ screen writes them into the report line ("EHOLUnitActivity::Idle"). */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	FString ActivityName;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	FString LoadResourceName;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 LoadResource = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 LoadAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	bool bLoadAmountKnown = false;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 CarryCapacity = 0;
};

/** A unit type's statistics row for one owner's faction, and the viewer's own technologies' bonus on its attack. */
USTRUCT(BlueprintType)
struct FHOLUnitStatsRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 Attack = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 BuildingAttack = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 MeleeArmor = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 RangedArmor = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 HeroArmor = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 SiegeArmor = 0;

	/** The row's speed in cm/s (the screen shows Speed / 100 m/s). */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	float Speed = 0.f;

	/** The technologies' bonus percent on the attack: the viewer's own units only (a client does not know the others' technologies), else 0. */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Selection")
	int32 BonusPercent = 0;
};

UCLASS()
class HEROESOFLIBERTYUI_API UHOLSelectionUnitReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The units the local controller has selected, as this machine knows them (RefreshUnitSelection of the C++ screen: HOLUnitView::Gather, or Find for one). ViewerSlot: the shown player's slot (UHOLScriptScreen::GetShownSlot), for bOwn. False when none is selected or known. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Selection", meta = (WorldContext = "WorldContextObject"))
	static bool GetUnitSelectionView(const UObject* WorldContextObject, int32 ViewerSlot, FHOLUnitSelView& Out);

	/** The statistics row of Type for OwnerSlot's faction (FindUnitTypeFor) and the viewer's bonus on its attack. False for an unknown type. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Selection", meta = (WorldContext = "WorldContextObject"))
	static bool GetUnitStatsRow(const UObject* WorldContextObject, FName Type, int32 OwnerSlot, int32 ViewerSlot, FHOLUnitStatsRow& Out);

	/** The console variable hol.HUD.UnitStats (0 hidden, 1 one line, 2 one line per statistic); the variable stays registered by the C++ screen, read here by name. 1 when not registered. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Selection")
	static int32 GetUnitStatsMode();

	/** The console variable hol.HUD.UnitStatsBuilding (0 never, 1 only without an ordinary attack, 2 also next to one when it differs). 1 when not registered. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Selection")
	static int32 GetUnitStatsBuildingMode();

	/** The name of a unit type as its owner's faction calls it (HOLGameNames::GetUnitName(Type, Row, FactionKey), UnitNameFor of the C++ screen); a negative owner: the base name. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Selection", meta = (WorldContext = "WorldContextObject"))
	static FText GetUnitName(const UObject* WorldContextObject, FName Type, int32 OwnerSlot);

	/** The number as the stats line writes it (FormatStat: FText::AsNumber, at most one decimal, none when whole), in the culture of the game. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Selection")
	static FText FormatStat(float Value);
};
