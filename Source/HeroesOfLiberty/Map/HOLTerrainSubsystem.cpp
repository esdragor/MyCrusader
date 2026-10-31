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

	FAutoConsoleCommand CommandReport(TEXT("hol.Terrain.Report"),
		TEXT("The terrain zones of the match (C3): how many, the marsh's strength"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* World = Context.World();
				if (const UHOLTerrainSubsystem* Terrain = World && World->IsGameWorld() ? World->GetSubsystem<UHOLTerrainSubsystem>() : nullptr)
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Terrain: %d zones, marsh speed %d %%"), Terrain->GetZones().Num(), UHOLTerrainSubsystem::GetMarshSpeedPercent());
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
	return FHOLTerrainRules::GetSpeedPercentAt(GetMarshSpeedPercent(), Zones, FMath::RoundToInt(Location.X), FMath::RoundToInt(Location.Y));
}

int32 UHOLTerrainSubsystem::GetSpeedPercentAt(const FVector& Location) const
{
	return GetSpeedPercentAt(FVector2D(Location.X, Location.Y));
}

int32 UHOLTerrainSubsystem::GetMarshSpeedPercent()
{
	return FMath::Clamp(UE::HeroesOfLiberty::Terrain::MarshSpeedPercent, 1, 100);
}
