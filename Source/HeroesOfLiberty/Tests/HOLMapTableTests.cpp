// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Map/HOLMapBounds.h"
#include "Map/HOLMapRules.h"
#include "Map/HOLMapSizeRules.h"
#include "Map/HOLStartSlot.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Resources/HOLSourceLayout.h"

#if WITH_DEV_AUTOMATION_TESTS

// C0.4 (Cartes): the common test of the table of maps (Tools/Data/Maps.csv), for every map at every size, read from the
// saved files without starting them (Map.PlainSizes stays the detailed test of Plain).
namespace UE::HeroesOfLiberty::Tests::MapTable
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** D493 (GDD 8.1, fairness between places): the sources nearest each place are the same distance away, to within 1 m. */
	constexpr double FairnessTolerance = 100.0;

	/** The sources of a type counted for the fairness of a place: its three nearest. */
	constexpr int32 FairnessSources = 3;

	/**
	 * M-Q4 = A, the budget per player (farms left out): no resource problem before at least 3/4 of a classic game (30 min,
	 * D02) unless the map says otherwise. ~45 active Peasants x 0.5 /s x 1350 s, a third of it by the (endless) farms, x 1.5.
	 * A player is the sources whose nearest slot is his (ties to the lowest index). Only the maps of Maps.csv with the
	 * Budget `standard` must hold it; the totals are reported for every map.
	 * M-Q4b: the budget depends on the size. Small to Large: 150 trees, 4 gold veins, 2 stone veins, 15 berry bushes.
	 * Huge only: 1.5 times that, 225 / 6 / 3 / 23 (22.5 rounded up). The recipe's table is BUDGET in Tools/Editor/maps/steppe.py.
	 */
	struct FBudgetLine
	{
		const TCHAR* Type;
		const TCHAR* Resource;
		int32 Needed;
		int32 NeededHuge;
	};
	constexpr FBudgetLine Budget[] = {
		{ TEXT("Tree"), TEXT("wood"), 150, 225 },
		{ TEXT("GoldVein"), TEXT("gold"), 4, 6 },
		{ TEXT("StoneVein"), TEXT("stone"), 2, 3 },
		{ TEXT("BerryBush"), TEXT("berries"), 15, 23 },
	};

	/** Whether Config/DefaultGame.ini carries this line (a commented line does not count). */
	bool IsCooked(const TArray<FString>& IniLines, const FString& Line)
	{
		return IniLines.ContainsByPredicate([&Line](const FString& Text) { return Text.TrimStartAndEnd() == Line; });
	}
}

// For every map of the table and every size: the file loads and is in the cooking list, its places are numbered 0 to N - 1
// once each inside the bounds, and the sources are fair between places (the 3 nearest of each type the same distance away).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapTableTest, "HeroesOfLiberty.Map.Table", UE::HeroesOfLiberty::Tests::MapTable::Flags)
bool FHOLMapTableTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapTable;
	// Unregistered components: places from the roots' relative transforms (see Map.StartSlots).
	const auto Place = [](const AActor& Actor) { return FVector2D(Actor.GetRootComponent()->GetRelativeLocation()); };

	TArray<FString> IniLines;
	const FString Ini = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("DefaultGame.ini"));
	FString IniText;
	TestTrue(FString::Printf(TEXT("%s read"), *Ini), FFileHelper::LoadFileToString(IniText, *Ini));
	IniText.ParseIntoArrayLines(IniLines);

	for (const FHOLMapInfo& MapInfo : FHOLMapRules::GetMaps())
	{
		for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
		{
			const FString Name = FString::Printf(TEXT("%s_%s"), MapInfo.Key, Info.Key);

			// 2. In the cooking list.
			const FString CookLine = FString::Printf(TEXT("+MapsToCook=(FilePath=\"/Game/Maps/%s\")"), *Name);
			TestTrue(FString::Printf(TEXT("%s: not in the cooking list (add %s to Config/DefaultGame.ini)"), *Name, *CookLine), IsCooked(IniLines, CookLine));

			// 1. The file loads.
			const FString File = FHOLMapSizeRules::GetMapFile(FHOLMapRules::GetAssetPath(MapInfo.Key), Info.Size).ToString();
			const UWorld* Map = LoadObject<UWorld>(nullptr, *File);
			if (!TestNotNull(FString::Printf(TEXT("%s: file missing (Tools\\make_maps.ps1 -Map %s -Sizes %s)"), *File, MapInfo.Key, Info.Key), Map) || !Map->PersistentLevel)
			{
				continue;
			}

			// 3. The places, the bounds, the layout.
			TMap<int32, FVector2D> Starts;
			int32 NumBounds = 0, NumLayouts = 0, NumStarts = 0;
			const AHOLMapBounds* Bounds = nullptr;
			const AHOLSourceLayout* Layout = nullptr;
			for (const AActor* Actor : Map->PersistentLevel->Actors)
			{
				if (const AHOLStartSlot* Start = Cast<AHOLStartSlot>(Actor))
				{
					++NumStarts;
					TestFalse(FString::Printf(TEXT("%s: slot %d placed once"), *Name, Start->GetSlotIndex()), Starts.Contains(Start->GetSlotIndex()));
					Starts.Add(Start->GetSlotIndex(), Place(*Start));
				}
				if (const AHOLMapBounds* Found = Cast<AHOLMapBounds>(Actor))
				{
					Bounds = Found;
					++NumBounds;
				}
				if (const AHOLSourceLayout* Found = Cast<AHOLSourceLayout>(Actor))
				{
					Layout = Found;
					++NumLayouts;
				}
			}
			TestEqual(FString::Printf(TEXT("%s: one map bounds"), *Name), NumBounds, 1);
			TestEqual(FString::Printf(TEXT("%s: one source layout"), *Name), NumLayouts, 1);
			TestEqual(FString::Printf(TEXT("%s: one start per place"), *Name), NumStarts, Info.NumPlaces);
			bool bIndices = Starts.Num() == Info.NumPlaces;
			for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
			{
				bIndices &= Starts.Contains(Slot);
			}
			if (!TestTrue(FString::Printf(TEXT("%s: slots 0 to %d once each"), *Name, Info.NumPlaces - 1), bIndices) || !Bounds || !Layout)
			{
				continue;
			}
			const FBox2D Area = Bounds->GetArea();
			const double Half = 0.5 * Info.SideTiles * UHOLBuildingSettings::GetTileSize();
			TestTrue(FString::Printf(TEXT("%s: bounds of %d tiles a side, centred (%s)"), *Name, Info.SideTiles, *Area.ToString()),
				Area.Min.Equals(FVector2D(-Half), 1.0) && Area.Max.Equals(FVector2D(Half), 1.0));
			for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
			{
				TestTrue(FString::Printf(TEXT("%s: slot %d inside the bounds"), *Name, Slot), Area.IsInside(Starts[Slot]));
			}

			// 4. Fair: for each type of source, the sorted distances to the 3 nearest sources are the same at every place. The
			// places are compared with each other, never with a constant: a recipe may leave a type out of the bases (the same
			// for every place) or put it in them.
			const TArray<FHOLPlacedSource>& Sources = Layout->GetSources();
			TSet<FName> Types;
			for (const FHOLPlacedSource& Source : Sources)
			{
				Types.Add(Source.Type);
			}
			for (const FName Type : Types)
			{
				TArray<double> First;
				double Spread = 0.0;
				for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
				{
					TArray<double> Distances;
					for (const FHOLPlacedSource& Source : Sources)
					{
						if (Source.Type == Type)
						{
							Distances.Add(FVector2D::Distance(FVector2D(Source.Location), Starts[Slot]));
						}
					}
					Distances.Sort();
					Distances.SetNum(FMath::Min(Distances.Num(), FairnessSources));
					if (Slot == 0)
					{
						First = Distances;
						continue;
					}
					for (int32 Rank = 0; Rank < Distances.Num(); ++Rank)
					{
						Spread = FMath::Max(Spread, FMath::Abs(Distances[Rank] - First[Rank]));
					}
				}
				AddInfo(FString::Printf(TEXT("%s: %s, nearest %d from slot 0: %.0f / %.0f / %.0f cm, spread between places %.1f cm"), *Name, *Type.ToString(), First.Num(),
					First.IsValidIndex(0) ? First[0] : 0.0, First.IsValidIndex(1) ? First[1] : 0.0, First.IsValidIndex(2) ? First[2] : 0.0, Spread));
				TestTrue(FString::Printf(TEXT("%s: %s unfair between places (the 3 nearest differ by %.1f cm, over %.0f cm)"), *Name, *Type.ToString(), Spread, FairnessTolerance),
					Spread <= FairnessTolerance);
			}

			// 6. Scenery never gets in the way (M-Q5, any map): every instanced mesh in a map file has no collision and no navigation.
			for (const AActor* Actor : Map->PersistentLevel->Actors)
			{
				TArray<UInstancedStaticMeshComponent*> Meshes;
				if (Actor)
				{
					Actor->GetComponents(Meshes);
				}
				for (const UInstancedStaticMeshComponent* Mesh : Meshes)
				{
					TestEqual(FString::Printf(TEXT("%s: instanced mesh %s (%d instances) has collision"), *Name, *Mesh->GetName(), Mesh->GetInstanceCount()), Mesh->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
					TestFalse(FString::Printf(TEXT("%s: instanced mesh %s (%d instances) affects the navigation"), *Name, *Mesh->GetName(), Mesh->GetInstanceCount()), Mesh->CanEverAffectNavigation());
				}
			}

			// 5. The budget per player (M-Q4 = A): the sources of each territory (nearest slot, ties to the lowest index).
			TArray<TMap<FName, int32>> Territories;
			Territories.SetNum(Info.NumPlaces);
			for (const FHOLPlacedSource& Source : Sources)
			{
				int32 Owner = 0;
				for (int32 Slot = 1; Slot < Info.NumPlaces; ++Slot)
				{
					Owner = FVector2D::DistSquared(FVector2D(Source.Location), Starts[Slot]) < FVector2D::DistSquared(FVector2D(Source.Location), Starts[Owner]) ? Slot : Owner;
				}
				Territories[Owner].FindOrAdd(Source.Type)++;
			}
			for (const FBudgetLine& Line : Budget)
			{
				const int32 Needed = Info.Size == EHOLMapSize::Huge ? Line.NeededHuge : Line.Needed;
				int32 Least = MAX_int32, Most = 0;
				for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
				{
					const int32 Held = Territories[Slot].FindRef(Line.Type);
					Least = FMath::Min(Least, Held);
					Most = FMath::Max(Most, Held);
					if (MapInfo.bBudget)
					{
						TestTrue(FString::Printf(TEXT("%s: slot %d's territory holds %d %s (%s), under the budget of %d at size %s (Small to Large %d, Huge %d): a player would run out of %s before 3/4 of a 30 min game (put more deposits in the recipe, then Tools\\make_maps.ps1 -Map %s -Sizes %s)"),
							*Name, Slot, Held, Line.Type, Line.Resource, Needed, Info.Key, Line.Needed, Line.NeededHuge, Line.Resource, MapInfo.Key, Info.Key), Held >= Needed);
					}
				}
				AddInfo(FString::Printf(TEXT("%s: budget %s per territory %d to %d (needed %d%s)"), *Name, Line.Type, Least, Most, Needed, MapInfo.bBudget ? TEXT("") : TEXT(", map without Budget")));
			}
		}
	}
	return true;
}

#endif
