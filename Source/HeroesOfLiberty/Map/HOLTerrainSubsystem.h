// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Map/HOLTerrainRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLTerrainSubsystem.generated.h"

/**
 * C3 (MapsInProgress.md): the terrain zones of the match, on the server and in a solo game (the walkers' step asks the speed of the
 * ground under a unit). Read from the map's AHOLTerrainLayout when the world begins play; a test sets them with SetZones.
 * The marsh's strength is hol.Terrain.MarshSpeedPercent (60 until the user settles CQ2a).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLTerrainSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Replaces the zones (a test, or the map read). */
	void SetZones(TConstArrayView<FHOLTerrainZone> InZones);
	void AddZone(const FHOLTerrainZone& Zone);
	TConstArrayView<FHOLTerrainZone> GetZones() const { return Zones; }

	/** The percent of its normal speed a walker at Location walks at (100 outside every zone). */
	int32 GetSpeedPercentAt(const FVector& Location) const;
	int32 GetSpeedPercentAt(const FVector2D& Location) const;

	/** The marsh's strength now (percent), the cvar held to 1..100. */
	static int32 GetMarshSpeedPercent();

private:
	TArray<FHOLTerrainZone> Zones;
};
