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
}
