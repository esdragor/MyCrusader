// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "Match/HOLResources.h"
#include "HOLSourceTypes.generated.h"

class UStaticMesh;

/**
 * One type of resource source (J5, D295, D297): a tree, a berry bush, a stone or a gold vein. A row of
 * /Game/Resources/SourceTypes, imported from Tools/Data/SourceTypes.csv by Tools/Editor/make_source_types.py, like the
 * building types (D260). The values are provisional by nature and are balanced in the CSV.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLSourceTypeRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Shown to the player (selection block, J5b) and in the logs. */
	UPROPERTY(EditAnywhere, Category = "Display")
	FText DisplayName;

	/** What the source gives. */
	UPROPERTY(EditAnywhere, Category = "Source")
	EHOLResource Resource = EHOLResource::Wood;

	/** What it holds when the match starts; it is used up (D295) and leaves the map at 0. */
	UPROPERTY(EditAnywhere, Category = "Source", meta = (ClampMin = "1"))
	int32 Amount = 100;

	/**
	 * Body radius on the ground (cm): the gatherers stand at its edge, and the navmesh is forbidden in a box of this
	 * half-width (D297).
	 */
	UPROPERTY(EditAnywhere, Category = "Source", meta = (ClampMin = "1", ForceUnits = "cm"))
	float Radius = 60.f;

	/** Height of the provisional shape (cm). */
	UPROPERTY(EditAnywhere, Category = "Presentation", meta = (ClampMin = "1", ForceUnits = "cm"))
	float Height = 600.f;

	/** Final model; empty = the provisional shape of D297 (trunk and cone, ball of berries, grey or golden blocks). */
	UPROPERTY(EditAnywhere, Category = "Presentation")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/**
	 * How many gatherers work it at once (IA1.2): 0 = as many as the spots around its body (every 100 cm on the circle,
	 * D297); a farm field takes 1.
	 */
	UPROPERTY(EditAnywhere, Category = "Source", meta = (ClampMin = "0"))
	int32 Places = 0;

	/**
	 * Scale of the gatherer's own rate on this source (IA1.2): 1 = the unit type's GatherRates. A farm field gives the
	 * Peasant 0.55/s from his 0.7/s: written 0.7857143 in the CSV, the float product is exactly 0.55.
	 */
	UPROPERTY(EditAnywhere, Category = "Source", meta = (ClampMin = "0"))
	float RateScale = 1.f;

	/** Never used up (IA1.2, a farm field): Amount is not counted down. */
	UPROPERTY(EditAnywhere, Category = "Source")
	bool Endless = false;

	/**
	 * Born of a building (IA1.2, a farm field): never placed by a map, no shape of its own, no hole in the navmesh, not
	 * clicked (the click falls on the building).
	 */
	UPROPERTY(EditAnywhere, Category = "Source")
	bool Linked = false;

	/** The rate (per second) a gatherer whose own rate for this resource is UnitRate gets on this source. */
	float GatherRate(float UnitRate) const { return UnitRate * RateScale; }
};

/** Where the source type table lives (DefaultGame.ini). */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "HeroesOfLiberty Resource Sources"))
class HEROESOFLIBERTY_API UHOLSourceSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Source type table (D297): one row per type. No source is placed without it. */
	UPROPERTY(config, EditAnywhere, Category = "Sources")
	TSoftObjectPtr<UDataTable> SourceTypesTable;

	/**
	 * Network updates per second of the source chunks (D297): what remains of a source is for the selection block, not
	 * worth the units' rate; a used-up source is sent at the next update whatever this rate.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Network", meta = (ClampMin = "0.1"))
	float NetUpdateFrequency = 2.f;

	/** The row of a type, or null (logged) if the table or the row is missing. */
	static const FHOLSourceTypeRow* FindType(FName TypeName);
};

/** Row names of the source type table used by the code. */
namespace HOLSourceTypeNames
{
	inline const FName Tree(TEXT("Tree"));
	inline const FName BerryBush(TEXT("BerryBush"));
	inline const FName StoneVein(TEXT("StoneVein"));
	inline const FName GoldVein(TEXT("GoldVein"));
	/** The field of a farm (IA1.2): endless, one place, linked to its building. */
	inline const FName FarmField(TEXT("FarmField"));
}
