// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SceneComponent.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Map/HOLMapRules.h"
#include "Map/HOLMapSizeRules.h"
#include "Map/HOLStartSlot.h"
#include "Map/HOLTerrainLayout.h"
#include "Map/HOLTerrainRules.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceLayout.h"

#if WITH_DEV_AUTOMATION_TESTS

// C5 checks (Cartes): La Foret profonde at every size (Tools\make_maps.ps1 -Map Forest), read from the saved files without starting them.
// CQ6a = A: the roads are round zones of 8 m (radius 400 cm) laid side by side in a corridor, nothing in them. The fairness and the budget per
// player are Map.Table's; here, what is the Forest's own: the road zones exist and are only roads of the same width (the forest itself has no
// zone, it has no effect), each place has as many as every other (the same roads turned onto every place), the discs of a road overlap
// (a corridor, not dots), and no source (tree, bush, vein) stands in a road.
namespace UE::HeroesOfLiberty::Tests::MapForest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A road's disc radius (forest.py ROAD_RADIUS: a corridor of 8 m) and the clearance of a source's centre beyond it. */
	constexpr float RoadRadius = 400.f;
	constexpr double SourceClearance = 100.0;

	struct FData
	{
		TMap<int32, FVector2D> Starts;
		TArray<FHOLPlacedSource> Sources;
		TArray<FHOLPlacedTerrainZone> Zones;
	};

	bool Read(const UWorld* Map, FData& Data)
	{
		if (!Map || !Map->PersistentLevel)
		{
			return false;
		}
		const AHOLSourceLayout* Layout = nullptr;
		for (const AActor* Actor : Map->PersistentLevel->Actors)
		{
			if (!Actor || !Actor->GetRootComponent())
			{
				continue;
			}
			if (const AHOLStartSlot* Start = Cast<AHOLStartSlot>(Actor))
			{
				Data.Starts.Add(Start->GetSlotIndex(), FVector2D(Actor->GetRootComponent()->GetRelativeLocation()));
			}
			Layout = Layout ? Layout : Cast<AHOLSourceLayout>(Actor);
			if (const AHOLTerrainLayout* Terrain = Cast<AHOLTerrainLayout>(Actor))
			{
				Data.Zones.Append(Terrain->GetZones());
			}
		}
		if (!Layout)
		{
			return false;
		}
		Data.Sources = Layout->GetSources();
		return true;
	}

	int32 NearestStart(const FData& Data, const FVector2D& At)
	{
		int32 Best = INDEX_NONE;
		double BestDistance = TNumericLimits<double>::Max();
		for (const TPair<int32, FVector2D>& Start : Data.Starts)
		{
			const double Distance = FVector2D::Distance(Start.Value, At);
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				Best = Start.Key;
			}
		}
		return Best;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapForestTest, "HeroesOfLiberty.Map.Forest", UE::HeroesOfLiberty::Tests::MapForest::Flags)
bool FHOLMapForestTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapForest;

	for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
	{
		const FString File = FHOLMapSizeRules::GetMapFile(FHOLMapRules::GetAssetPath(TEXT("Forest")), Info.Size).ToString();
		const UWorld* Map = LoadObject<UWorld>(nullptr, *File);
		FData Data;
		if (!TestNotNull(FString::Printf(TEXT("%s (Tools\\make_maps.ps1 -Map Forest -Sizes %s)"), *File, Info.Key), Map) || !Read(Map, Data)
			|| !TestEqual(FString::Printf(TEXT("%s: one start per place"), Info.Key), Data.Starts.Num(), Info.NumPlaces))
		{
			continue;
		}
		const FString Name = FString::Printf(TEXT("Forest_%s"), Info.Key);

		// 1. Road zones only (the forest has no zone), all of the same width, and some.
		TestTrue(FString::Printf(TEXT("%s: the map has road zones"), *Name), Data.Zones.Num() > 0);
		for (const FHOLPlacedTerrainZone& Zone : Data.Zones)
		{
			TestEqual(FString::Printf(TEXT("%s: the zone at %s is a road"), *Name, *Zone.Location.ToString()), Zone.Kind, HOLTerrainKind::Road);
			TestEqual(FString::Printf(TEXT("%s: the road at %s is 8 m wide"), *Name, *Zone.Location.ToString()), Zone.Radius, RoadRadius);
		}

		// 2. The same roads turned onto every place: the places sit in a ring at equal angles, so the whole road system turned by one place's
		// angle lands on itself (the same road for everybody, no one has a shorter way to the middle or to a neighbour).
		TestEqual(FString::Printf(TEXT("%s: a whole number of road discs per place"), *Name), Data.Zones.Num() % Info.NumPlaces, 0);
		const double Sector = 2.0 * PI / Info.NumPlaces;
		int32 Unmatched = 0;
		for (const FHOLPlacedTerrainZone& Zone : Data.Zones)
		{
			const FVector2D At(Zone.Location);
			const FVector2D Turned(At.X * FMath::Cos(Sector) - At.Y * FMath::Sin(Sector), At.X * FMath::Sin(Sector) + At.Y * FMath::Cos(Sector));
			bool bFound = false;
			for (const FHOLPlacedTerrainZone& Other : Data.Zones)
			{
				bFound = bFound || FVector2D::Distance(Turned, FVector2D(Other.Location)) < 50.0;
			}
			Unmatched += bFound ? 0 : 1;
		}
		TestEqual(FString::Printf(TEXT("%s: road discs whose turned copy is not a road disc"), *Name), Unmatched, 0);

		// 3. A corridor, not dots: every disc overlaps another one (their centres are at most one radius apart).
		for (const FHOLPlacedTerrainZone& Zone : Data.Zones)
		{
			double Nearest = TNumericLimits<double>::Max();
			for (const FHOLPlacedTerrainZone& Other : Data.Zones)
			{
				if (&Other != &Zone)
				{
					Nearest = FMath::Min(Nearest, FVector2D::Distance(FVector2D(Zone.Location), FVector2D(Other.Location)));
				}
			}
			TestTrue(FString::Printf(TEXT("%s: the road disc at %s touches another one (%.0f cm)"), *Name, *Zone.Location.ToString(), Nearest), Nearest <= Zone.Radius);
		}

		// 4. Nothing in a road: no source's centre within a road's radius plus the clearance.
		int32 OnRoad = 0;
		for (const FHOLPlacedSource& Source : Data.Sources)
		{
			for (const FHOLPlacedTerrainZone& Zone : Data.Zones)
			{
				if (FVector2D::Distance(FVector2D(Source.Location), FVector2D(Zone.Location)) < Zone.Radius + SourceClearance)
				{
					++OnRoad;
					AddInfo(FString::Printf(TEXT("%s: %s at %s is on a road"), *Name, *Source.Type.ToString(), *FVector2D(Source.Location).ToString()));
					break;
				}
			}
		}
		TestEqual(FString::Printf(TEXT("%s: sources on a road"), *Name), OnRoad, 0);
	}
	return true;
}

#endif
