// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLTerrainSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLTerrainLayout.h"

namespace UE::HeroesOfLiberty::Terrain
{
	int32 MarshSpeedPercent = FHOLTerrainRules::DefaultMarshSpeedPercent;
	FAutoConsoleVariableRef CVarMarshSpeedPercent(TEXT("hol.Terrain.MarshSpeedPercent"), MarshSpeedPercent,
		TEXT("A walker's speed in a marsh, in percent of its normal speed, on this server (C3, El Dorado; CQ2a is not settled: 60 is the ")
		TEXT("recommendation, 75 the other preset). Read at each step. 100: the marsh does not slow"));

	int32 RoadSpeedPercent = FHOLTerrainRules::DefaultRoadSpeedPercent;
	FAutoConsoleVariableRef CVarRoadSpeedPercent(TEXT("hol.Terrain.RoadSpeedPercent"), RoadSpeedPercent,
		TEXT("A walker's speed on a road, in percent of its normal speed, on this server (C5, La Foret profonde; CQ6a = A: 130). Read at each ")
		TEXT("step. 100: the road does not speed up"));

	FAutoConsoleCommand CommandReport(TEXT("hol.Terrain.Report"),
		TEXT("The terrain zones of the match (C3): how many, the marsh's strength"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* World = Context.World();
				if (const UHOLTerrainSubsystem* Terrain = World && World->IsGameWorld() ? World->GetSubsystem<UHOLTerrainSubsystem>() : nullptr)
				{
					int32 Roads = 0;
					for (const FHOLTerrainZone& Zone : Terrain->GetZones())
					{
						Roads += Zone.Kind == HOLTerrainKind::Road ? 1 : 0;
					}
					UE_LOG(LogHOLMatch, Display, TEXT("Terrain: %d zones (%d road), marsh speed %d %%, road speed %d %%"), Terrain->GetZones().Num(), Roads,
						UHOLTerrainSubsystem::GetMarshSpeedPercent(), UHOLTerrainSubsystem::GetRoadSpeedPercent());
				}
			}
		}));
}

bool UHOLTerrainSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLTerrainSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}
	Zones.Reset();
	for (TActorIterator<AHOLTerrainLayout> It(&InWorld); It; ++It)
	{
		for (const FHOLPlacedTerrainZone& Placed : It->GetZones())
		{
			FHOLTerrainZone& Zone = Zones.AddDefaulted_GetRef();
			Zone.Kind = Placed.Kind;
			Zone.X = FMath::RoundToInt(Placed.Location.X);
			Zone.Y = FMath::RoundToInt(Placed.Location.Y);
			Zone.Radius = FMath::RoundToInt(Placed.Radius);
		}
	}
	if (!Zones.IsEmpty())
	{
		UE_LOG(LogHOLMatch, Log, TEXT("%d terrain zones placed by the map, marsh speed %d %%"), Zones.Num(), GetMarshSpeedPercent());
	}
}

void UHOLTerrainSubsystem::SetZones(const TConstArrayView<FHOLTerrainZone> InZones)
{
	Zones = InZones;
}

void UHOLTerrainSubsystem::AddZone(const FHOLTerrainZone& Zone)
{
	Zones.Add(Zone);
}

int32 UHOLTerrainSubsystem::GetSpeedPercentAt(const FVector2D& Location) const
{
	if (Zones.IsEmpty())
	{
		return 100;
	}
	return FHOLTerrainRules::GetSpeedPercentAt(GetMarshSpeedPercent(), Zones, FMath::RoundToInt(Location.X), FMath::RoundToInt(Location.Y), GetRoadSpeedPercent());
}

int32 UHOLTerrainSubsystem::GetSpeedPercentAt(const FVector& Location) const
{
	return GetSpeedPercentAt(FVector2D(Location.X, Location.Y));
}

int32 UHOLTerrainSubsystem::GetRoadSpeedPercent()
{
	return FMath::Clamp(UE::HeroesOfLiberty::Terrain::RoadSpeedPercent, 100, 300);
}

int32 UHOLTerrainSubsystem::GetMarshSpeedPercent()
{
	return FMath::Clamp(UE::HeroesOfLiberty::Terrain::MarshSpeedPercent, 1, 100);
}
