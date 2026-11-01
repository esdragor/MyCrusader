// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Map/HOLMapBounds.h"
#include "Map/HOLMapRules.h"
#include "Map/HOLMapSizeRules.h"
#include "Map/HOLStartSlot.h"
#include "Map/HOLTerrainLayout.h"
#include "Map/HOLTerrainRules.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Resources/HOLSourceLayout.h"
#include "Resources/HOLSourceTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// C0.4 (Cartes): the common test of the table of maps (Tools/Data/Maps.csv), for every map at every size, read from the
// saved files without starting them (Map.PlainSizes stays the detailed test of Plain).
namespace UE::HeroesOfLiberty::Tests::MapTable
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** C3 (El Dorado): no gold vein within 100 m of a slot (the Steppe's nearest is 110 m), all of it within 100 m of the middle of the map (the cluster is 23 to 51 m across). */
	constexpr double GoldFromSlot = 10000.0;
	constexpr double GoldClusterReach = 10000.0;

	/** C3c: the marsh is a ring of at least this many round zones, and its edge no nearer than 50 m to a slot (the bases are 165 m or more from the middle). */
	constexpr int32 MinMarshZones = 8;
	constexpr double MarshFromSlot = 5000.0;

	/** D493 (GDD 8.1, fairness between places): the sources nearest each place are the same distance away, to within 1 m. */
	constexpr double FairnessTolerance = 100.0;

	/** The sources of a type counted for the fairness of a place: its three nearest. */
	constexpr int32 FairnessSources = 3;

	/**
	 * M-Q4 = A, the budget per player (farms left out): no resource problem before at least 3/4 of a classic game (30 min,
	 * D02) unless the map says otherwise. ~45 active Peasants x 0.5 /s x 1350 s, a third of it by the (endless) farms, x 1.5.
	 * A player is the sources whose nearest slot is his (ties to the lowest index). Only the maps of Maps.csv with the
	 * Budget `standard` must hold it; the totals are reported for every map.
	 * M-Q4b: the budget depends on the size. Small to Large: 4 gold veins, 2 stone veins, 15 berry bushes; Huge only: 1.5 times that, 6 / 3 / 23
	 * (22.5 rounded up). M-Q10b (the user, 2026-10-30: "tighten the trees and double their number"): the trees are 300 (it was 150) and, on Huge, 360
	 * (the most the Steppe's far layer holds at that spacing, see steppe.py BUDGET). The recipe's table is BUDGET in Tools/Editor/maps/steppe.py.
	 */
	struct FBudgetLine
	{
		const TCHAR* Type;
		const TCHAR* Resource;
		int32 Needed;
		int32 NeededHuge;
	};
	constexpr FBudgetLine Budget[] = {
		{ TEXT("Tree"), TEXT("wood"), 300, 360 },
		{ TEXT("BerryBush"), TEXT("berries"), 15, 23 },
	};

	/**
	 * M-Q9 (the user, 2026-10-30: "I did not ask for cosmetics", "small AND big"): the gold and the stone are counted in VALUE, not in veins. The
	 * total per territory is the M-Q4 one (4 gold veins of 2 500 = 10 000 gold, 2 stone veins of 2 500 = 5 000 stone; Huge x1.5 as the other lines) but
	 * it comes as a MIX of sizes (the source types of Tools/Data/SourceTypes.csv: Small 500, Medium 1 000, Standard 2 500, Big 5 000): the sum of
	 * the Amounts of a territory's sources of a resource is at least the target and at most ValueMargin times it (the recipes are exact, the margin is
	 * for a recipe that aims a little above, like the trees), the territory holds at least MinMineralSources mineral sources and each resource is
	 * made of at least MinMineralSizes different sizes. Territories are read as the other budget lines (nearest slot).
	 */
	struct FValueLine
	{
		EHOLResource Resource;
		const TCHAR* Name;
		int32 Needed;
		int32 NeededHuge;
	};
	constexpr FValueLine MineralValue[] = {
		{ EHOLResource::Gold, TEXT("gold"), 10000, 15000 },
		{ EHOLResource::Stone, TEXT("stone"), 5000, 7500 },
	};
	constexpr double ValueMargin = 1.10;
	constexpr int32 MinMineralSizes = 3;
	constexpr int32 MinMineralSources = 10;

	/**
	 * M-Q7 (2026-10-30) asked that no point of the square be farther than 28 m from a landmark (start slot, source, rock, big scenery piece);
	 * M-Q10b = A (D578) withdrew it: large empty spaces are accepted. The largest empty circle is only MEASURED and reported, on a grid
	 * of EmptyGrid from the corner of the bounds, for every map; nothing is required.
	 */
	constexpr double EmptyGrid = 2000.0;

	/**
	 * M-Q8 (the user, 2026-10-30, after the Hinterland trial): the forests are PACKS of 14 trees, many small clumps scattered over the whole
	 * map, not a few big forests ("the map looks desolate"). Trees closer than PackLink (6 m, trunks 60 cm: two neighbours of a grove are 2.2 to
	 * 2.4 m apart) are one pack; no pack has more than MaxPackTrees (14 plus a little: a base's wood may be two packs), and every territory
	 * (the trees whose nearest slot is the player's) holds at least ceil(budget / PackTrees) - PackSlack packs of at least MinPackTrees trees.
	 */
	constexpr double PackLink = 600.0;
	constexpr int32 PackTrees = 14;
	constexpr int32 MaxPackTrees = 20;
	constexpr int32 MinPackTrees = 10;
	constexpr int32 PackSlack = 2;
	/** C5 (CQ7 = B, 2026-11-01): the Forest's groves are 30 to 40 trees (steppe.py GROVE_LADDER overridden in forest.py): a cluster has at most 45, a pack counts from 25. */
	constexpr int32 DenseMaxPackTrees = 45;
	constexpr int32 DenseMinPackTrees = 25;
	constexpr int32 DenseBiggestPackTrees = 40;
	/** M-Q10b: the biggest pack a recipe makes (the Steppe's ladder is 12, 14, 16, 18) and the packs a territory needs are counted with it. */
	constexpr int32 BiggestPackTrees = 18;
	/**
	 * M-Q10b (the user, 2026-10-30, after the Hinterland trial: "tighten the trees"): the trees of a pack are closer than they were (2.2 to 2.4 m
	 * between trunks): the mean distance from a tree to its nearest tree is at most MeanTreeSpacing (1.6 to 1.74 m asked of the generator, 10 to 20 cm more
	 * for the trees at the edge of a pack).
	 * 2026-10-31 (the user, after the tree_spacing trial: "a forest is trees that touch, compact, a minimum of white"): the trunks (60 cm radius)
	 * nearly touch: 1.25 to 1.35 m asked of the generator; the mean is 1.34 to 1.37 m on Plain and Hinterland, 1.50 to 1.55 m on the Steppe (its lone trees and clumps), at most 1.6 m.
	 * 2026-11-01 (the user, after the tree_spacing_compare trial: REGLAGE 0.8 m): the trunks may overlap (60 cm radius, 80 cm between the centres):
	 * 0.8 to 0.9 m asked of the generator (GROVE_SPACING 80), the mean over the groves, lone trees and clumps is measured below, at most 1.2 m.
	 */
	constexpr double MeanTreeSpacing = 120.0;
	/** Plain's wood is a grid (wood_points, 134 cm between neighbours), not a grove: its limit is the old one. */
	constexpr double MeanTreeSpacingGrid = 160.0;

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
			TArray<FHOLPlacedTerrainZone> TerrainZones;
			int32 NumTerrainLayouts = 0;
			for (const AActor* Actor : Map->PersistentLevel->Actors)
			{
				if (const AHOLTerrainLayout* Terrain = Cast<AHOLTerrainLayout>(Actor))
				{
					TerrainZones.Append(Terrain->GetZones());
					++NumTerrainLayouts;
				}
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
			// The fairness is read per RESOURCE (M-Q9: gold comes in several types, one per size): the 3 nearest sources of the resource, each as a
			// (distance, amount) pair, are the same at every place.
			const auto Kind = [](const FName Type)
			{
				const FHOLSourceTypeRow* Row = UHOLSourceSettings::FindType(Type);
				return Row ? Row->Resource : EHOLResource::Wood;
			};
			const auto Amount = [](const FName Type)
			{
				const FHOLSourceTypeRow* Row = UHOLSourceSettings::FindType(Type);
				return Row ? Row->Amount : 0;
			};
			TSet<EHOLResource> Kinds;
			for (const FHOLPlacedSource& Source : Sources)
			{
				Kinds.Add(Kind(Source.Type));
			}
			for (const EHOLResource Resource : Kinds)
			{
				const TCHAR* ResourceName = Resource == EHOLResource::Gold ? TEXT("gold") : Resource == EHOLResource::Stone ? TEXT("stone") : Resource == EHOLResource::Wood ? TEXT("wood") : TEXT("food");
				TArray<TPair<double, int32>> First;
				double Spread = 0.0;
				bool bSameAmounts = true;
				for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
				{
					TArray<TPair<double, int32>> Nearest;
					for (const FHOLPlacedSource& Source : Sources)
					{
						if (Kind(Source.Type) == Resource)
						{
							Nearest.Add(TPair<double, int32>(FVector2D::Distance(FVector2D(Source.Location), Starts[Slot]), Amount(Source.Type)));
						}
					}
					Nearest.Sort([](const TPair<double, int32>& A, const TPair<double, int32>& B) { return A.Key < B.Key; });
					Nearest.SetNum(FMath::Min(Nearest.Num(), FairnessSources));
					if (Slot == 0)
					{
						First = Nearest;
						continue;
					}
					for (int32 Rank = 0; Rank < Nearest.Num(); ++Rank)
					{
						Spread = FMath::Max(Spread, FMath::Abs(Nearest[Rank].Key - First[Rank].Key));
						bSameAmounts &= Nearest[Rank].Value == First[Rank].Value;
					}
				}
				AddInfo(FString::Printf(TEXT("%s: %s, nearest %d from slot 0: %.0f (%d) / %.0f (%d) / %.0f (%d) cm, spread between places %.1f cm"), *Name, ResourceName, First.Num(),
					First.IsValidIndex(0) ? First[0].Key : 0.0, First.IsValidIndex(0) ? First[0].Value : 0, First.IsValidIndex(1) ? First[1].Key : 0.0, First.IsValidIndex(1) ? First[1].Value : 0,
					First.IsValidIndex(2) ? First[2].Key : 0.0, First.IsValidIndex(2) ? First[2].Value : 0, Spread));
				TestTrue(FString::Printf(TEXT("%s: %s unfair between places (the 3 nearest differ by %.1f cm, over %.0f cm)"), *Name, ResourceName, Spread, FairnessTolerance),
					Spread <= FairnessTolerance);
				TestTrue(FString::Printf(TEXT("%s: %s unfair between places (the 3 nearest do not hold the same amounts)"), *Name, ResourceName), bSameAmounts);
			}

			// 6. Scenery never gets in the way (M-Q5, any map): every instanced mesh in a map file has no collision and no navigation.
			// M-Q10 (the user, 2026-10-30): and a map of the Budget `standard` has none at all (no flowers, tufts, boulders, thickets: nothing
			// without a strategic, economic or military interest; a map with water may bring back sand or reeds as its identity).
			for (const AActor* Actor : Map->PersistentLevel->Actors)
			{
				TArray<UInstancedStaticMeshComponent*> Meshes;
				if (Actor)
				{
					Actor->GetComponents(Meshes);
				}
				if (MapInfo.bBudget)
				{
					TestEqual(FString::Printf(TEXT("%s: %d scenery instances in %s (M-Q10: none on a map without water)"), *Name, Meshes.Num(), *Actor->GetName()), Meshes.Num(), 0);
				}
				for (const UInstancedStaticMeshComponent* Mesh : Meshes)
				{
					TestEqual(FString::Printf(TEXT("%s: instanced mesh %s (%d instances) has collision"), *Name, *Mesh->GetName(), Mesh->GetInstanceCount()), Mesh->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
					TestFalse(FString::Printf(TEXT("%s: instanced mesh %s (%d instances) affects the navigation"), *Name, *Mesh->GetName(), Mesh->GetInstanceCount()), Mesh->CanEverAffectNavigation());
				}
			}

			// 7. No empty place (M-Q7): the largest empty circle of the square, from the nearest landmark.
			{
				TArray<FVector2D> Landmarks;
				for (const TPair<int32, FVector2D>& Start : Starts)
				{
					Landmarks.Add(Start.Value);
				}
				for (const FHOLPlacedSource& Source : Sources)
				{
					Landmarks.Add(FVector2D(Source.Location));
				}
				for (const AActor* Actor : Map->PersistentLevel->Actors)
				{
					if (!Actor || !Actor->GetRootComponent())
					{
						continue;
					}
					if (const AStaticMeshActor* Mesh = Cast<AStaticMeshActor>(Actor))
					{
						const UStaticMesh* Shape = Mesh->GetStaticMeshComponent() ? Mesh->GetStaticMeshComponent()->GetStaticMesh() : nullptr;
						if (Shape && Shape->GetName() == TEXT("Sphere"))
						{
							Landmarks.Add(FVector2D(Actor->GetRootComponent()->GetRelativeLocation()));
						}
					}
				}
				// The distance to the nearest landmark of each grid point, exact up to 4 cells (80 m), beyond that "far".
				const int32 Cells = FMath::FloorToInt((Area.Max.X - Area.Min.X) / EmptyGrid) + 1;
				TArray<float> Nearest;
				Nearest.Init(1.0e9f, Cells * Cells);
				for (const FVector2D& Landmark : Landmarks)
				{
					const int32 CX = FMath::RoundToInt((Landmark.X - Area.Min.X) / EmptyGrid), CY = FMath::RoundToInt((Landmark.Y - Area.Min.Y) / EmptyGrid);
					for (int32 Y = FMath::Max(0, CY - 4); Y <= FMath::Min(Cells - 1, CY + 4); ++Y)
					{
						for (int32 X = FMath::Max(0, CX - 4); X <= FMath::Min(Cells - 1, CX + 4); ++X)
						{
							const float Distance = FVector2D::Distance(Landmark, FVector2D(Area.Min.X + X * EmptyGrid, Area.Min.Y + Y * EmptyGrid));
							Nearest[Y * Cells + X] = FMath::Min(Nearest[Y * Cells + X], Distance);
						}
					}
				}
				double Largest = 0.0;
				FVector2D LargestAt = FVector2D::ZeroVector;
				for (int32 Y = 0; Y < Cells; ++Y)
				{
					for (int32 X = 0; X < Cells; ++X)
					{
						if (Nearest[Y * Cells + X] > Largest)
						{
							Largest = Nearest[Y * Cells + X];
							LargestAt = FVector2D(Area.Min.X + X * EmptyGrid, Area.Min.Y + Y * EmptyGrid);
						}
					}
				}
				// M-Q10b = A (the user, 2026-10-30, D578): large empty spaces are accepted, the empty-circle rule is withdrawn. The size is only
				// reported (to be discussed again once game animals and points of interest exist); nothing is asserted.
				AddInfo(FString::Printf(TEXT("%s: largest empty circle %s%.1f m at %s (%d landmarks, measure only)"), *Name, Largest >= 8000.0 ? TEXT("over ") : TEXT(""), FMath::Min(Largest, 8000.0) / 100.0, *LargestAt.ToString(),
					Landmarks.Num()));
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
			// 9. The value budget of the minerals (M-Q9): the sum of the amounts per resource and territory, the number of mineral sources, the sizes.
			{
				for (const FValueLine& Line : MineralValue)
				{
					const int32 Needed = Info.Size == EHOLMapSize::Huge ? Line.NeededHuge : Line.Needed;
					int32 LeastValue = MAX_int32, MostValue = 0, LeastSizes = MAX_int32;
					for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
					{
						int32 Value = 0;
						TSet<int32> Sizes;
						for (const TPair<FName, int32>& Held : Territories[Slot])
						{
							const FHOLSourceTypeRow* Row = UHOLSourceSettings::FindType(Held.Key);
							if (Row && Row->Resource == Line.Resource)
							{
								Value += Row->Amount * Held.Value;
								Sizes.Add(Row->Amount);
							}
						}
						LeastValue = FMath::Min(LeastValue, Value);
						MostValue = FMath::Max(MostValue, Value);
						LeastSizes = FMath::Min(LeastSizes, Sizes.Num());
						if (MapInfo.bBudget)
						{
							TestTrue(FString::Printf(TEXT("%s: slot %d's territory holds %d %s in value, not %d to %d (the budget of M-Q4 as a mix of sizes, M-Q9; Tools\\make_maps.ps1 -Map %s -Sizes %s)"),
								*Name, Slot, Value, Line.Name, Needed, FMath::FloorToInt(Needed * ValueMargin), MapInfo.Key, Info.Key), Value >= Needed && Value <= FMath::FloorToInt(Needed * ValueMargin));
							TestTrue(FString::Printf(TEXT("%s: slot %d's territory has %d different sizes of %s sources, under %d (small AND big piles, M-Q9)"), *Name, Slot, Sizes.Num(), Line.Name, MinMineralSizes),
								Sizes.Num() >= MinMineralSizes);
						}
					}
					AddInfo(FString::Printf(TEXT("%s: %s value per territory %d to %d (needed %d to %d), at least %d different sizes%s"), *Name, Line.Name, LeastValue, MostValue, Needed,
						FMath::FloorToInt(Needed * ValueMargin), LeastSizes == MAX_int32 ? 0 : LeastSizes, MapInfo.bBudget ? TEXT("") : TEXT(", map without Budget")));
				}
				int32 FewestMinerals = MAX_int32, MostMinerals = 0;
				for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
				{
					int32 Minerals = 0;
					for (const TPair<FName, int32>& Held : Territories[Slot])
					{
						const FHOLSourceTypeRow* Row = UHOLSourceSettings::FindType(Held.Key);
						Minerals += Row && (Row->Resource == EHOLResource::Gold || Row->Resource == EHOLResource::Stone) ? Held.Value : 0;
					}
					FewestMinerals = FMath::Min(FewestMinerals, Minerals);
					MostMinerals = FMath::Max(MostMinerals, Minerals);
					if (MapInfo.bBudget)
					{
						TestTrue(FString::Printf(TEXT("%s: slot %d's territory has %d gold and stone sources, under %d (many piles of different values, M-Q9)"), *Name, Slot, Minerals, MinMineralSources),
							Minerals >= MinMineralSources);
					}
				}
				AddInfo(FString::Printf(TEXT("%s: gold and stone sources per territory %d to %d (needed %d%s)"), *Name, FewestMinerals, MostMinerals, MinMineralSources, MapInfo.bBudget ? TEXT("") : TEXT(", map without Budget")));
			}
			// 10. The gold at the centre (C3, GDD 8.5, the Budget `centre`): none in the bases, all of it in one cluster round the middle of the map, and
			// the whole of the gold budget there (each place's share is what its territory holds, tested above).
			// C5 (La Foret profonde): its roads are zones too, checked by its own test (Map.Forest).
			if (!MapInfo.bGoldAtCentre && FCString::Strcmp(MapInfo.Key, TEXT("Forest")) != 0)
			{
				TestEqual(FString::Printf(TEXT("%s: %d terrain layouts on a map without marsh"), *Name, NumTerrainLayouts), NumTerrainLayouts, 0);
			}
			if (MapInfo.bGoldAtCentre)
			{
				int32 GoldNearSlot = 0, GoldFarFromCentre = 0, GoldTotal = 0, GoldVeins = 0;
				double FarthestGold = 0.0;
				for (const FHOLPlacedSource& Source : Sources)
				{
					const FHOLSourceTypeRow* Row = UHOLSourceSettings::FindType(Source.Type);
					if (!Row || Row->Resource != EHOLResource::Gold)
					{
						continue;
					}
					++GoldVeins;
					GoldTotal += Row->Amount;
					for (const TPair<int32, FVector2D>& Start : Starts)
					{
						GoldNearSlot += FVector2D::Distance(FVector2D(Source.Location), Start.Value) < GoldFromSlot ? 1 : 0;
					}
					const double FromCentre = FVector2D(Source.Location).Size();
					FarthestGold = FMath::Max(FarthestGold, FromCentre);
					GoldFarFromCentre += FromCentre > GoldClusterReach ? 1 : 0;
				}
				const int32 GoldBudget = (Info.Size == EHOLMapSize::Huge ? MineralValue[0].NeededHuge : MineralValue[0].Needed) * Info.NumPlaces;
				// The marsh (C3c): one terrain layout, a ring of round zones round the cluster, none on a base; the gold is in the clear in the middle.
				TestEqual(FString::Printf(TEXT("%s: %d terrain layouts (one: the marsh zones)"), *Name, NumTerrainLayouts), NumTerrainLayouts, 1);
				TestTrue(FString::Printf(TEXT("%s: %d marsh zones, under %d (a ring)"), *Name, TerrainZones.Num(), MinMarshZones), TerrainZones.Num() >= MinMarshZones);
				int32 ZonesNearSlot = 0, GoldInMarsh = 0, OthersInMarsh = 0;
				double NearestZoneEdge = 1.0e9;
				for (const FHOLPlacedTerrainZone& Zone : TerrainZones)
				{
					TestEqual(FString::Printf(TEXT("%s: a zone is a marsh"), *Name), Zone.Kind, HOLTerrainKind::Marsh);
					for (const TPair<int32, FVector2D>& Start : Starts)
					{
						const double Edge = FVector2D::Distance(FVector2D(Zone.Location), Start.Value) - Zone.Radius;
						NearestZoneEdge = FMath::Min(NearestZoneEdge, Edge);
						ZonesNearSlot += Edge < MarshFromSlot ? 1 : 0;
					}
				}
				for (const FHOLPlacedSource& Source : Sources)
				{
					const FHOLSourceTypeRow* Row = UHOLSourceSettings::FindType(Source.Type);
					bool bIn = false;
					for (const FHOLPlacedTerrainZone& Zone : TerrainZones)
					{
						bIn |= FVector2D::Distance(FVector2D(Source.Location), FVector2D(Zone.Location)) <= Zone.Radius;
					}
					(Row && Row->Resource == EHOLResource::Gold ? GoldInMarsh : OthersInMarsh) += bIn ? 1 : 0;
				}
				TestEqual(FString::Printf(TEXT("%s: %d marsh zones within %.0f m of a start slot (nearest edge %.0f m)"), *Name, ZonesNearSlot, MarshFromSlot / 100.0, NearestZoneEdge / 100.0), ZonesNearSlot, 0);
				TestEqual(FString::Printf(TEXT("%s: %d gold veins in the marsh (the cluster is in the clear)"), *Name, GoldInMarsh), GoldInMarsh, 0);
				TestEqual(FString::Printf(TEXT("%s: %d trees, berries or stone veins in the marsh (nothing to gather there: the Steppe's far layer keeps out of the ring)"), *Name, OthersInMarsh), OthersInMarsh, 0);
				AddInfo(FString::Printf(TEXT("%s: %d marsh zones, nearest edge to a slot %.0f m"), *Name, TerrainZones.Num(), NearestZoneEdge / 100.0));
				TestEqual(FString::Printf(TEXT("%s: %d gold veins within %.0f m of a start slot (GDD 8.5: no gold in the bases)"), *Name, GoldNearSlot, GoldFromSlot / 100.0), GoldNearSlot, 0);
				TestEqual(FString::Printf(TEXT("%s: %d gold veins farther than %.0f m from the middle (all the gold is in the cluster; the farthest %.0f m)"), *Name, GoldFarFromCentre, GoldClusterReach / 100.0, FarthestGold / 100.0), GoldFarFromCentre, 0);
				TestEqual(FString::Printf(TEXT("%s: the cluster holds %d gold in %d veins, not the whole budget of %d"), *Name, GoldTotal, GoldVeins, GoldBudget), GoldTotal, GoldBudget);
				AddInfo(FString::Printf(TEXT("%s: gold at the centre: %d veins, %d gold, the farthest %.0f m from the middle, none within %.0f m of a slot"), *Name, GoldVeins, GoldTotal, FarthestGold / 100.0, GoldFromSlot / 100.0));
			}
			for (const FBudgetLine& Line : Budget)
			{
				// C5 (CQ7 = B): the Budget `dense` holds twice the wood.
				const int32 WoodFactor = MapInfo.bDenseWood && FCString::Strcmp(Line.Type, TEXT("Tree")) == 0 ? 2 : 1;
				const int32 Needed = (Info.Size == EHOLMapSize::Huge ? Line.NeededHuge : Line.Needed) * WoodFactor;
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

				// 8. The forests are packs of PackTrees (M-Q8): the trees closer than PackLink make a pack.
				{
					TArray<FVector2D> Trees;
					for (const FHOLPlacedSource& Source : Sources)
					{
						if (Source.Type == TEXT("Tree"))
						{
							Trees.Add(FVector2D(Source.Location));
						}
					}
					TArray<int32> Pack;
					Pack.Init(INDEX_NONE, Trees.Num());
					TArray<int32> PackSizes;
					TArray<FVector2D> PackSums;
					for (int32 Seed = 0; Seed < Trees.Num(); ++Seed)
					{
						if (Pack[Seed] != INDEX_NONE)
						{
							continue;
						}
						const int32 Number = PackSizes.Add(0);
						PackSums.Add(FVector2D::ZeroVector);
						TArray<int32> Open = { Seed };
						Pack[Seed] = Number;
						while (!Open.IsEmpty())
						{
							const int32 Tree = Open.Pop();
							++PackSizes[Number];
							PackSums[Number] += Trees[Tree];
							for (int32 Other = 0; Other < Trees.Num(); ++Other)
							{
								if (Pack[Other] == INDEX_NONE && FVector2D::DistSquared(Trees[Tree], Trees[Other]) <= FMath::Square(PackLink))
								{
									Pack[Other] = Number;
									Open.Add(Other);
								}
							}
						}
					}
					double SpacingSum = 0.0;
					for (int32 Tree = 0; Tree < Trees.Num(); ++Tree)
					{
						double Nearest = 1.0e9;
						for (int32 Other = 0; Other < Trees.Num(); ++Other)
						{
							if (Other != Tree)
							{
								Nearest = FMath::Min(Nearest, static_cast<double>(FVector2D::Distance(Trees[Tree], Trees[Other])));
							}
						}
						SpacingSum += Nearest;
					}
					const double MeanSpacing = Trees.Num() ? SpacingSum / Trees.Num() : 0.0;
					AddInfo(FString::Printf(TEXT("%s: mean distance from a tree to its nearest tree %.0f cm (limit %.0f cm%s)"), *Name, MeanSpacing, MeanTreeSpacing, MapInfo.bBudget ? TEXT("") : TEXT(", map without Budget")));
					if (MapInfo.bBudget)
					{
						TestTrue(FString::Printf(TEXT("%s: the trees are %.0f cm from their nearest tree on average, over %.0f (M-Q10b: tighten the trees, GROVE_SPACING in Tools/Editor/maps/common.py; Tools\\make_maps.ps1 -Map %s -Sizes %s)"), *Name, MeanSpacing,
							MeanTreeSpacing, MapInfo.Key, Info.Key), MeanSpacing <= (FCString::Strcmp(MapInfo.Key, TEXT("Plain")) == 0 ? MeanTreeSpacingGrid : MeanTreeSpacing));
					}
					// A map of the Budget `dense` (C5, CQ7 = B) has twice the wood in groves of 30 to 40 trees.
					const int32 MapMinPack = MapInfo.bDenseWood ? DenseMinPackTrees : MinPackTrees;
					const int32 MapMaxPack = MapInfo.bDenseWood ? DenseMaxPackTrees : MaxPackTrees;
					TArray<int32> PacksOf;
					PacksOf.Init(0, Info.NumPlaces);
					int32 Biggest = 0, Packs = 0, Smallest = MAX_int32;
					for (int32 Number = 0; Number < PackSizes.Num(); ++Number)
					{
						Biggest = FMath::Max(Biggest, PackSizes[Number]);
						if (PackSizes[Number] < MapMinPack)
						{
							continue;
						}
						++Packs;
						Smallest = FMath::Min(Smallest, PackSizes[Number]);
						const FVector2D Centre = PackSums[Number] / PackSizes[Number];
						int32 Owner = 0;
						for (int32 Slot = 1; Slot < Info.NumPlaces; ++Slot)
						{
							Owner = FVector2D::DistSquared(Centre, Starts[Slot]) < FVector2D::DistSquared(Centre, Starts[Owner]) ? Slot : Owner;
						}
						++PacksOf[Owner];
					}
					// A map of the Budget `dense` (C5, CQ7 = B) has twice the wood in groves of 30 to 40 trees.
					const int32 Budgeted = (Info.Size == EHOLMapSize::Huge ? Budget[0].NeededHuge : Budget[0].Needed) * (MapInfo.bDenseWood ? 2 : 1);
					const int32 PacksNeeded = FMath::DivideAndRoundUp(Budgeted, MapInfo.bDenseWood ? DenseBiggestPackTrees : BiggestPackTrees) - PackSlack;
					int32 LeastPacks = MAX_int32;
					for (const int32 Held : PacksOf)
					{
						LeastPacks = FMath::Min(LeastPacks, Held);
					}
					AddInfo(FString::Printf(TEXT("%s: %d trees in %d packs of %d trees or more (smallest %d, biggest cluster %d trees), per territory %d at least (needed %d%s)"), *Name, Trees.Num(),
						Packs, MapMinPack, Packs ? Smallest : 0, Biggest, LeastPacks == MAX_int32 ? 0 : LeastPacks, PacksNeeded, MapInfo.bBudget ? TEXT("") : TEXT(", map without Budget")));
					if (MapInfo.bBudget)
					{
						TestTrue(FString::Printf(TEXT("%s: a cluster of %d trees, over %d: the forests are packs of %d trees (M-Q8; Tools\\make_maps.ps1 -Map %s -Sizes %s)"), *Name, Biggest, MapMaxPack, PackTrees, MapInfo.Key, Info.Key),
							Biggest <= MapMaxPack);
						for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
						{
							TestTrue(FString::Printf(TEXT("%s: slot %d's territory has %d packs of %d trees or more, under %d (the budget of %d trees in packs of %d, less %d: many small forests, M-Q8)"), *Name, Slot, PacksOf[Slot],
								MapMinPack, PacksNeeded, Budgeted, PackTrees, PackSlack), PacksOf[Slot] >= PacksNeeded);
						}
					}
				}
			}
		}
	}
	return true;
}

#endif
