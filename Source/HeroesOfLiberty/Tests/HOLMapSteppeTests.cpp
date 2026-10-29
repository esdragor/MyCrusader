// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Map/HOLMapBounds.h"
#include "Map/HOLMapRules.h"
#include "Map/HOLMapSizeRules.h"
#include "Map/HOLStartSlot.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceLayout.h"

#if WITH_DEV_AUTOMATION_TESTS

// C1 / C1b checks (Cartes): La Steppe at every size (Tools\make_maps.ps1 -Map Steppe), read from the saved files without
// starting them, as Map.PlainSizes does for Plain. GDD 8.5: an open plain, the resources in many small deposits over the
// whole square, you expand early and there is no single point to hold. The fairness between places and the budget per
// player (M-Q4) are checked for every map by Map.Table; here, what is the Steppe's own.
namespace UE::HeroesOfLiberty::Tests::MapSteppe
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/**
	 * S-Q1 = A, the reduced base (C1c: organic): a small irregular wood of 20 trees within 36 m of the slot (its trees 17 m to 35 m from
	 * it, out of the way of the Peasants born at the centre's foot), the 6 berry bushes of one patch within 29 m.
	 */
	constexpr double WoodReach = 3600.0;
	constexpr int32 WoodTrees = 20;
	constexpr double BerryReach = 2900.0;
	constexpr int32 BerryBushes = 6;

	/** Past this distance from every slot a source belongs to a deposit (the base's room, common.BASE_REACH, 60 m). */
	constexpr double BaseReach = 6000.0;

	/** No stone or gold in the base: none within 100 m of any slot (the recipe keeps its veins at least 110 m away). */
	constexpr double VeinFreeReach = 10000.0;

	/** Each player has a gold vein and a stone vein within this distance of its slot (170 m): what the start needs. */
	constexpr double VeinStartReach = 17000.0;

	/**
	 * The near layer (steppe.py NEAR_REACH): within 75 m of slot 0 every source is the image of one of every other slot's
	 * (the slots are on a circle: the pattern is made once and turned). The veins within 130 m are all near ones (the far
	 * veins are farther than the third nearest near vein of their type), so they turn onto each other too.
	 */
	constexpr double NearReach = 7500.0;
	constexpr double NearVeinReach = 13000.0;

	/**
	 * The scenery (C1b, M-Q5: decoration without collision, for every map): instanced meshes in one actor, scattered over the whole
	 * square, none within 8 m of a slot, a source or a rock, at most a few thousand, and cheap (no shadow, a cull distance).
	 */
	constexpr double DecorClear = 800.0;
	constexpr int32 DecorMax = 6000;
	constexpr int32 DecorMinPerHectare = 20;
	constexpr double DecorCullMax = 15000.0;  // 150 m: farther away the instances are not drawn
	constexpr double RidgeAhead = 3000.0;     // common.RIDGE_AHEAD
	constexpr double RidgeOffsets[] = { -650.0, 0.0, 650.0 };  // common.RIDGE_BOULDERS

	/**
	 * C1c, nothing on a grid. No two trees closer than 130 cm (their trunks of 60 cm would overlap), whatever the shape of a grove. A
	 * grove of 8 or more trees is not a lattice: the directions from each tree to its nearest neighbour, taken modulo 90 degrees, are
	 * spread (a grid of rows and columns sends them all the same way: their resultant length is near 1; random ones, 1 over the root
	 * of the number of trees); no more than MaxLatticeShare of the groves have a resultant over LatticeResultant, and the mean one
	 * is under MeanLatticeResultant. A berry patch is not a row: one bush at least BushOffRow off the line through the two farthest
	 * apart. The far layer is not the near one turned onto every slot: under MaxCopiedShare of the sources beyond 130 m of every slot
	 * have an image, turned by some number of places, within 50 cm.
	 */
	constexpr double TrunkOverlap = 130.0;
	constexpr double LatticeResultant = 0.85;
	constexpr double MaxLatticeShare = 0.15;
	constexpr double MeanLatticeResultant = 0.55;
	constexpr double BushOffRow = 100.0;
	constexpr double MaxCopiedShare = 0.2;

	/** No point of the square is farther than this (90 m) from a source or a slot: no blank in the corners. */
	constexpr double LargestEmptyCircle = 9000.0;
	constexpr double EmptyGridStep = 2000.0;

	/** Nothing in the middle of the map (S-Q2 = A: no point to hold): no source within 20 m of the centre. */
	constexpr double CentreClear = 2000.0;

	/** Sources closer than this are one deposit: a wood is 2.2 m between trees, berry bushes 3 m apart. */
	constexpr double SameDeposit = 1000.0;

	/** Two deposits are at least this far apart, from source to source (25 m: a Peasant can work one in peace). */
	constexpr double DepositGap = 2500.0;

	/** The scattered deposits keep 15 m from the edge of the bounds. */
	constexpr double EdgeMargin = 1500.0;

	/** The veins of one kind of one player are at least this far apart (60 m): they do not sit in one corner of its territory. */
	constexpr double VeinSpread = 6000.0;

	/**
	 * What a deposit is made of: many small ones. A grove of 8 to 16 trees (a few of 24), whatever its shape (an irregular blob, with lone
	 * trees and clumps of 2 or 3 just outside: they are within 10 m of it, so one deposit), a patch of 3 to 5 berry bushes, one vein.
	 */
	bool IsWholeDeposit(const FName Type, const int32 Size)
	{
		if (Type == TEXT("Tree"))
		{
			return Size == 8 || Size == 10 || Size == 12 || Size == 16 || Size == 24;
		}
		return Type == TEXT("BerryBush") ? Size >= 3 && Size <= 5 : Size == 1;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSteppeTest, "HeroesOfLiberty.Map.Steppe", UE::HeroesOfLiberty::Tests::MapSteppe::Flags)
bool FHOLMapSteppeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapSteppe;
	// Unregistered components: places from the roots' relative transforms (see Map.StartSlots).
	const auto Place = [](const AActor& Actor) { return FVector2D(Actor.GetRootComponent()->GetRelativeLocation()); };

	for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
	{
		const FString File = FHOLMapSizeRules::GetMapFile(FHOLMapRules::GetAssetPath(TEXT("Steppe")), Info.Size).ToString();
		const UWorld* Map = LoadObject<UWorld>(nullptr, *File);
		if (!TestNotNull(FString::Printf(TEXT("%s (Tools\\make_maps.ps1 -Map Steppe -Sizes %s)"), *File, Info.Key), Map) || !Map->PersistentLevel)
		{
			continue;
		}
		TMap<int32, FVector2D> Starts;
		const AHOLMapBounds* Bounds = nullptr;
		const AHOLSourceLayout* Layout = nullptr;
		for (const AActor* Actor : Map->PersistentLevel->Actors)
		{
			if (const AHOLStartSlot* Start = Cast<AHOLStartSlot>(Actor))
			{
				Starts.Add(Start->GetSlotIndex(), Place(*Start));
			}
			Bounds = Bounds ? Bounds : Cast<AHOLMapBounds>(Actor);
			Layout = Layout ? Layout : Cast<AHOLSourceLayout>(Actor);
		}
		if (!TestNotNull(FString::Printf(TEXT("%s: map bounds"), Info.Key), Bounds)
			|| !TestNotNull(FString::Printf(TEXT("%s: source layout"), Info.Key), Layout)
			|| !TestEqual(FString::Printf(TEXT("%s: one start per place"), Info.Key), Starts.Num(), Info.NumPlaces))
		{
			continue;
		}
		const FBox2D Area = Bounds->GetArea();
		const TArray<FHOLPlacedSource>& Sources = Layout->GetSources();

		// The nearest slot of a point: the player whose room it is in.
		const auto NearestSlot = [&Starts](const FVector2D& At)
		{
			int32 Best = 0;
			for (const TPair<int32, FVector2D>& Start : Starts)
			{
				Best = FVector2D::DistSquared(At, Start.Value) < FVector2D::DistSquared(At, Starts[Best]) ? Start.Key : Best;
			}
			return Best;
		};

		// 1. The reduced base (S-Q1 = A): a wood of 20 trees within 30 m and 6 bushes within 25 m of every slot, and nothing
		// else within the 60 m of the base's room (no deposit of the scatter). 2. Stone and gold are not in the base: you
		// have to leave home for them.
		for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
		{
			int32 Trees = 0, Bushes = 0, InBase = 0;
			double NearestStone = TNumericLimits<double>::Max(), NearestGold = TNumericLimits<double>::Max();
			for (const FHOLPlacedSource& Source : Sources)
			{
				const double Distance = FVector2D::Distance(FVector2D(Source.Location), Starts[Slot]);
				Trees += Source.Type == TEXT("Tree") && Distance < WoodReach;
				Bushes += Source.Type == TEXT("BerryBush") && Distance < BerryReach;
				InBase += Distance < BaseReach;
				NearestStone = Source.Type == TEXT("StoneVein") ? FMath::Min(NearestStone, Distance) : NearestStone;
				NearestGold = Source.Type == TEXT("GoldVein") ? FMath::Min(NearestGold, Distance) : NearestGold;
			}
			TestEqual(FString::Printf(TEXT("%s: slot %d's wood of 20 trees"), Info.Key, Slot), Trees, WoodTrees);
			TestEqual(FString::Printf(TEXT("%s: slot %d's berries"), Info.Key, Slot), Bushes, BerryBushes);
			TestEqual(FString::Printf(TEXT("%s: slot %d's base holds only its wood and its berries"), Info.Key, Slot), InBase, WoodTrees + BerryBushes);
			TestTrue(FString::Printf(TEXT("%s: slot %d has stone %.0f cm away and gold %.0f cm away, in its base (under %.0f)"), Info.Key, Slot, NearestStone, NearestGold,
				VeinFreeReach), NearestStone >= VeinFreeReach && NearestGold >= VeinFreeReach);
			if (Slot == 0)
			{
				AddInfo(FString::Printf(TEXT("%s: nearest stone %.0f cm, nearest gold %.0f cm from slot 0"), Info.Key, NearestStone, NearestGold));
			}
		}

		// The deposits: the sources past the base's room, grouped (a source within SameDeposit of another is in its deposit).
		TArray<int32> Neutral;
		for (int32 Index = 0; Index < Sources.Num(); ++Index)
		{
			bool bBase = false;
			for (const TPair<int32, FVector2D>& Start : Starts)
			{
				bBase |= FVector2D::Distance(FVector2D(Sources[Index].Location), Start.Value) < BaseReach;
			}
			if (!bBase)
			{
				Neutral.Add(Index);
			}
		}
		TMap<int32, int32> Parent;
		for (const int32 Index : Neutral)
		{
			Parent.Add(Index, Index);
		}
		const auto Root = [&Parent](int32 Index)
		{
			while (Parent[Index] != Index)
			{
				Index = Parent[Index];
			}
			return Index;
		};
		for (int32 First = 0; First < Neutral.Num(); ++First)
		{
			for (int32 Second = First + 1; Second < Neutral.Num(); ++Second)
			{
				if (FVector2D::Distance(FVector2D(Sources[Neutral[First]].Location), FVector2D(Sources[Neutral[Second]].Location)) < SameDeposit)
				{
					Parent[Root(Neutral[First])] = Root(Neutral[Second]);
				}
			}
		}
		TMap<int32, TArray<int32>> Deposits;
		for (const int32 Index : Neutral)
		{
			Deposits.FindOrAdd(Root(Index)).Add(Index);
		}

		// 3. Each deposit is one kind of source and the size of its kind (so none is two merged or cut); 4. at least DepositGap
		// from every other deposit, source to source; 5. inside the bounds with a margin; 6. nothing at the centre.
		TMap<int32, int32> PerPlayer, Stone, Gold, Sizes;
		TMap<int32, TArray<FVector2D>> Veins;
		for (const TPair<int32, TArray<int32>>& Entry : Deposits)
		{
			const FName Type = Sources[Entry.Value[0]].Type;
			const FVector2D First(Sources[Entry.Value[0]].Location);
			bool bOneKind = true;
			for (const int32 Index : Entry.Value)
			{
				bOneKind &= Sources[Index].Type == Type;
			}
			TestTrue(FString::Printf(TEXT("%s: the deposit at %s is of one kind (%s)"), Info.Key, *First.ToString(), *Type.ToString()), bOneKind);
			TestTrue(FString::Printf(TEXT("%s: the %s deposit at %s holds %d sources, not a whole small deposit (no two merged, none cut)"), Info.Key, *Type.ToString(), *First.ToString(),
				Entry.Value.Num()), IsWholeDeposit(Type, Entry.Value.Num()));
			Sizes.FindOrAdd(Type == TEXT("Tree") ? Entry.Value.Num() : (Type == TEXT("BerryBush") ? 100 + Entry.Value.Num() : 1000))++;
			PerPlayer.FindOrAdd(NearestSlot(First))++;
			if (Type == TEXT("StoneVein") || Type == TEXT("GoldVein"))
			{
				(Type == TEXT("StoneVein") ? Stone : Gold).FindOrAdd(NearestSlot(First))++;
				Veins.FindOrAdd(NearestSlot(First) * 2 + (Type == TEXT("StoneVein") ? 0 : 1)).Add(First);
			}
			for (const int32 Index : Entry.Value)
			{
				const FVector2D At(Sources[Index].Location);
				TestTrue(FString::Printf(TEXT("%s: source at %s is %.0f cm from the centre, in the middle of the map"), Info.Key, *At.ToString(), At.Size()), At.Size() >= CentreClear);
				TestTrue(FString::Printf(TEXT("%s: source at %s too near the edge of the bounds"), Info.Key, *At.ToString()),
					FMath::Abs(At.X) <= Area.Max.X - EdgeMargin && FMath::Abs(At.Y) <= Area.Max.Y - EdgeMargin);
				for (const TPair<int32, TArray<int32>>& Other : Deposits)
				{
					if (Other.Key == Entry.Key)
					{
						continue;
					}
					for (const int32 OtherIndex : Other.Value)
					{
						const double Gap = FVector2D::Distance(At, FVector2D(Sources[OtherIndex].Location));
						if (Gap < DepositGap)
						{
							AddError(FString::Printf(TEXT("%s: deposits at %s and %s are %.0f cm apart, under %.0f"), Info.Key, *At.ToString(),
								*FVector2D(Sources[OtherIndex].Location).ToString(), Gap, DepositGap));
							break;
						}
					}
				}
			}
		}
		double Closest = TNumericLimits<double>::Max();
		for (const FHOLPlacedSource& Source : Sources)
		{
			Closest = FMath::Min(Closest, FVector2D(Source.Location).Size());
		}

		// 7. A gold vein and a stone vein within 170 m of every slot: the start's need, "not at home" (nothing within 100 m, 2.) but
		// not a long walk. (How much of everything a player holds is the budget, Map.Table.)
		for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
		{
			double NearestStone = TNumericLimits<double>::Max(), NearestGold = TNumericLimits<double>::Max();
			for (const FHOLPlacedSource& Source : Sources)
			{
				const double Distance = FVector2D::Distance(FVector2D(Source.Location), Starts[Slot]);
				NearestStone = Source.Type == TEXT("StoneVein") ? FMath::Min(NearestStone, Distance) : NearestStone;
				NearestGold = Source.Type == TEXT("GoldVein") ? FMath::Min(NearestGold, Distance) : NearestGold;
			}
			TestTrue(FString::Printf(TEXT("%s: slot %d's nearest stone %.0f cm and gold %.0f cm, over %.0f"), Info.Key, Slot, NearestStone, NearestGold, VeinStartReach),
				NearestStone <= VeinStartReach && NearestGold <= VeinStartReach);
			// 8. Veins of one kind spread out, not in one corner of the player's territory.
			for (int32 Kind = 0; Kind < 2; ++Kind)
			{
				const TArray<FVector2D>* Own = Veins.Find(Slot * 2 + Kind);
				for (int32 First = 0; Own && First < Own->Num(); ++First)
				{
					for (int32 Second = First + 1; Second < Own->Num(); ++Second)
					{
						const double Apart = FVector2D::Distance((*Own)[First], (*Own)[Second]);
						TestTrue(FString::Printf(TEXT("%s: slot %d's %s veins %.0f cm apart, under %.0f (bunched)"), Info.Key, Slot, Kind ? TEXT("gold") : TEXT("stone"),
							Apart, VeinSpread), Apart >= VeinSpread);
					}
				}
			}
		}

		// 9. The near layer is symmetric (GDD 8.1): the sources within NearReach of slot 0 (and its veins within NearVeinReach), turned
		// by k places around the centre, land on a source of their type, for every other slot k. The far layer is not.
		int32 Unmatched = 0, NearSources = 0;
		for (int32 Slot = 1; Slot < Info.NumPlaces; ++Slot)
		{
			const double Turn = FMath::DegreesToRadians(360.0 * Slot / Info.NumPlaces);
			for (const FHOLPlacedSource& Source : Sources)
			{
				const FVector2D At(Source.Location);
				const bool bVein = Source.Type == TEXT("StoneVein") || Source.Type == TEXT("GoldVein");
				if (FVector2D::Distance(At, Starts[0]) >= (bVein ? NearVeinReach : NearReach))
				{
					continue;
				}
				NearSources += Slot == 1;
				const FVector2D Turned(At.X * FMath::Cos(Turn) - At.Y * FMath::Sin(Turn), At.X * FMath::Sin(Turn) + At.Y * FMath::Cos(Turn));
				Unmatched += !Sources.ContainsByPredicate([&Source, &Turned](const FHOLPlacedSource& Other)
					{ return Other.Type == Source.Type && FVector2D::DistSquared(FVector2D(Other.Location), Turned) < 4.0; });
			}
		}
		TestEqual(FString::Printf(TEXT("%s: the near layer of slot 0 (%d sources) turns onto every other slot's"), Info.Key, NearSources), Unmatched, 0);

		// 12. Nothing on a grid (C1c): no two trunks overlapping; the groves are not lattices (the directions to the nearest neighbour,
		// modulo 90 degrees, are spread); the berry patches are not rows; and the far layer is not the near one turned onto every slot.
		{
			double ClosestTrees = TNumericLimits<double>::Max();
			TArray<FVector2D> Trees;
			for (const FHOLPlacedSource& Source : Sources)
			{
				if (Source.Type == TEXT("Tree"))
				{
					Trees.Add(FVector2D(Source.Location));
				}
			}
			for (int32 First = 0; First < Trees.Num(); ++First)
			{
				for (int32 Second = First + 1; Second < Trees.Num(); ++Second)
				{
					ClosestTrees = FMath::Min(ClosestTrees, FVector2D::Distance(Trees[First], Trees[Second]));
				}
			}
			TestTrue(FString::Printf(TEXT("%s: two trees %.0f cm apart, under %.0f (overlapping trunks)"), Info.Key, ClosestTrees, TrunkOverlap), ClosestTrees >= TrunkOverlap);

			// The groups: a base's wood and berries (within BaseReach of a slot), the deposits (the groups found above).
			TArray<TArray<FVector2D>> Woods, Patches;
			for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
			{
				TArray<FVector2D>& Wood = Woods.AddDefaulted_GetRef();
				TArray<FVector2D>& Patch = Patches.AddDefaulted_GetRef();
				for (const FHOLPlacedSource& Source : Sources)
				{
					if (FVector2D::Distance(FVector2D(Source.Location), Starts[Slot]) < BaseReach)
					{
						(Source.Type == TEXT("Tree") ? Wood : Patch).Add(FVector2D(Source.Location));
					}
				}
			}
			for (const TPair<int32, TArray<int32>>& Entry : Deposits)
			{
				const FName Type = Sources[Entry.Value[0]].Type;
				if (Type == TEXT("Tree") || Type == TEXT("BerryBush"))
				{
					TArray<FVector2D>& Group = (Type == TEXT("Tree") ? Woods : Patches).AddDefaulted_GetRef();
					for (const int32 Index : Entry.Value)
					{
						Group.Add(FVector2D(Sources[Index].Location));
					}
				}
			}
			int32 Groves = 0, Lattices = 0;
			double SumResultant = 0.0;
			for (const TArray<FVector2D>& Wood : Woods)
			{
				if (Wood.Num() < 8)
				{
					continue;
				}
				double Cos = 0.0, Sin = 0.0;
				for (int32 Tree = 0; Tree < Wood.Num(); ++Tree)
				{
					int32 Neighbour = Tree == 0 ? 1 : 0;
					for (int32 Other = 0; Other < Wood.Num(); ++Other)
					{
						Neighbour = Other != Tree && FVector2D::DistSquared(Wood[Tree], Wood[Other]) < FVector2D::DistSquared(Wood[Tree], Wood[Neighbour]) ? Other : Neighbour;
					}
					const double Angle = FMath::Atan2(Wood[Neighbour].Y - Wood[Tree].Y, Wood[Neighbour].X - Wood[Tree].X);
					Cos += FMath::Cos(4.0 * Angle);
					Sin += FMath::Sin(4.0 * Angle);
				}
				const double Resultant = FMath::Sqrt(Cos * Cos + Sin * Sin) / Wood.Num();
				++Groves;
				Lattices += Resultant > LatticeResultant;
				SumResultant += Resultant;
			}
			TestTrue(FString::Printf(TEXT("%s: %d of %d groves look like a lattice (rows and columns), over %.0f%%"), Info.Key, Lattices, Groves, MaxLatticeShare * 100.0), Lattices <= MaxLatticeShare * Groves);
			TestTrue(FString::Printf(TEXT("%s: the groves' mean lattice resultant %.2f, over %.2f (rows and columns)"), Info.Key, Groves ? SumResultant / Groves : 0.0, MeanLatticeResultant),
				Groves == 0 || SumResultant / Groves <= MeanLatticeResultant);
			int32 Rows = 0, PatchesChecked = 0;
			for (const TArray<FVector2D>& Patch : Patches)
			{
				if (Patch.Num() < 3)
				{
					continue;
				}
				FVector2D First = Patch[0], Second = Patch[1];
				for (const FVector2D& A : Patch)
				{
					for (const FVector2D& B : Patch)
					{
						if (FVector2D::DistSquared(A, B) > FVector2D::DistSquared(First, Second))
						{
							First = A;
							Second = B;
						}
					}
				}
				const FVector2D Line = (Second - First).GetSafeNormal();
				double Off = 0.0;
				for (const FVector2D& Bush : Patch)
				{
					Off = FMath::Max(Off, FMath::Abs(FVector2D::CrossProduct(Bush - First, Line)));
				}
				++PatchesChecked;
				Rows += Off < BushOffRow;
			}
			TestEqual(FString::Printf(TEXT("%s: %d of %d berry patches are a straight row (no bush %.0f cm off the line)"), Info.Key, Rows, PatchesChecked, BushOffRow), Rows, 0);

			int32 Far = 0, Copied = 0;
			for (const FHOLPlacedSource& Source : Sources)
			{
				const FVector2D At(Source.Location);
				bool bFar = true;
				for (const TPair<int32, FVector2D>& Start : Starts)
				{
					bFar &= FVector2D::Distance(At, Start.Value) >= NearVeinReach;
				}
				if (!bFar)
				{
					continue;
				}
				++Far;
				bool bImage = false;
				for (int32 Turned = 1; Turned < Info.NumPlaces && !bImage; ++Turned)
				{
					const double Turn = FMath::DegreesToRadians(360.0 * Turned / Info.NumPlaces);
					const FVector2D Image(At.X * FMath::Cos(Turn) - At.Y * FMath::Sin(Turn), At.X * FMath::Sin(Turn) + At.Y * FMath::Cos(Turn));
					bImage = Sources.ContainsByPredicate([&Source, &Image](const FHOLPlacedSource& Other)
						{ return Other.Type == Source.Type && FVector2D::DistSquared(FVector2D(Other.Location), Image) < 2500.0; });
				}
				Copied += bImage;
			}
			TestTrue(FString::Printf(TEXT("%s: %d of the %d sources beyond the near layer are the image of another one turned by some places (the far layer is a copy), over %.0f%%"), Info.Key, Copied, Far,
				MaxCopiedShare * 100.0), Copied <= MaxCopiedShare * Far);
			AddInfo(FString::Printf(TEXT("%s: %d groves of 8 or more trees, %d look like a lattice, mean resultant %.2f; %d berry patches, %d rows; nearest two trees %.0f cm; %d of %d far sources are copies"), Info.Key, Groves,
				Lattices, Groves ? SumResultant / Groves : 0.0, PatchesChecked, Rows, ClosestTrees, Copied, Far));
		}

		// 11. The scenery: instanced meshes without collision nor navigation nor shadow, a cull distance, all inside the ground,
		// none within 8 m of a slot, a source or a rock, a few per hectare over the whole square (the corners too).
		int32 DecorInstances = 0, DecorComponents = 0, DecorTooNear = 0, DecorOutside = 0;
		int32 DecorInCorners = 0;
		TArray<FVector2D> Rocks;
		for (const TPair<int32, FVector2D>& Start : Starts)
		{
			const FVector2D Outward = Start.Value.GetSafeNormal();
			const FVector2D Left(-Outward.Y, Outward.X);
			for (const double Offset : RidgeOffsets)
			{
				Rocks.Add(Outward * (Start.Value.Size() - RidgeAhead) + Left * Offset);
			}
		}
		for (const AActor* Actor : Map->PersistentLevel->Actors)
		{
			TArray<UInstancedStaticMeshComponent*> Meshes;
			if (Actor)
			{
				Actor->GetComponents(Meshes);
			}
			for (const UInstancedStaticMeshComponent* Mesh : Meshes)
			{
				++DecorComponents;
				TestEqual(FString::Printf(TEXT("%s: scenery %s has no collision"), Info.Key, *Mesh->GetName()), Mesh->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
				TestFalse(FString::Printf(TEXT("%s: scenery %s affects the navigation"), Info.Key, *Mesh->GetName()), Mesh->CanEverAffectNavigation());
				TestFalse(FString::Printf(TEXT("%s: scenery %s casts shadows (D258)"), Info.Key, *Mesh->GetName()), Mesh->CastShadow);
				TestTrue(FString::Printf(TEXT("%s: scenery %s has no cull distance up to %.0f cm (%d)"), Info.Key, *Mesh->GetName(), DecorCullMax, Mesh->InstanceEndCullDistance),
					Mesh->InstanceEndCullDistance > 0 && Mesh->InstanceEndCullDistance <= DecorCullMax);
				for (int32 Instance = 0; Instance < Mesh->GetInstanceCount(); ++Instance)
				{
					FTransform Transform;
					Mesh->GetInstanceTransform(Instance, Transform, true);
					const FVector2D At(Transform.GetLocation());
					++DecorInstances;
					DecorOutside += !(FMath::Abs(At.X) <= Area.Max.X + 1.0 && FMath::Abs(At.Y) <= Area.Max.Y + 1.0);
					DecorInCorners += FMath::Abs(At.X) > 0.75 * Area.Max.X && FMath::Abs(At.Y) > 0.75 * Area.Max.Y;
					bool bTooNear = false;
					for (const TPair<int32, FVector2D>& Start : Starts)
					{
						bTooNear |= FVector2D::Distance(At, Start.Value) < DecorClear;
					}
					for (const FVector2D& Rock : Rocks)
					{
						bTooNear |= FVector2D::Distance(At, Rock) < DecorClear;
					}
					for (const FHOLPlacedSource& Source : Sources)
					{
						bTooNear |= FVector2D::DistSquared(At, FVector2D(Source.Location)) < DecorClear * DecorClear;
					}
					DecorTooNear += bTooNear;
				}
			}
		}
		const double Hectares = FMath::Square(2.0 * Area.Max.X) / 1.0e8;
		TestTrue(FString::Printf(TEXT("%s: no scenery (instanced meshes) in the map"), Info.Key), DecorInstances > 0);
		TestEqual(FString::Printf(TEXT("%s: scenery within 8 m of a slot, a source or a rock"), Info.Key), DecorTooNear, 0);
		TestEqual(FString::Printf(TEXT("%s: scenery outside the ground"), Info.Key), DecorOutside, 0);
		TestTrue(FString::Printf(TEXT("%s: %d scenery instances, over %d (a few thousand at most)"), Info.Key, DecorInstances, DecorMax), DecorInstances <= DecorMax);
		TestTrue(FString::Printf(TEXT("%s: %d scenery instances on %.1f ha, under %d per hectare"), Info.Key, DecorInstances, Hectares, DecorMinPerHectare),
			DecorInstances >= DecorMinPerHectare * Hectares);
		TestTrue(FString::Printf(TEXT("%s: %d scenery instances in the corners (outer quarter of both sides)"), Info.Key, DecorInCorners), DecorInCorners > 0);
		AddInfo(FString::Printf(TEXT("%s: scenery %d instances in %d components (%.0f per hectare, %d in the corners)"), Info.Key, DecorInstances, DecorComponents, DecorInstances / Hectares, DecorInCorners));

		// 10. No blank: the largest empty circle of the square, on a grid of 20 m, from the nearest source or slot, at most 90 m.
		double Largest = 0.0;
		FVector2D LargestAt = FVector2D::ZeroVector;
		for (double X = Area.Min.X; X <= Area.Max.X + 1.0; X += EmptyGridStep)
		{
			for (double Y = Area.Min.Y; Y <= Area.Max.Y + 1.0; Y += EmptyGridStep)
			{
				double Nearest = TNumericLimits<double>::Max();
				for (const FHOLPlacedSource& Source : Sources)
				{
					Nearest = FMath::Min(Nearest, FVector2D::DistSquared(FVector2D(X, Y), FVector2D(Source.Location)));
				}
				for (const TPair<int32, FVector2D>& Start : Starts)
				{
					Nearest = FMath::Min(Nearest, FVector2D::DistSquared(FVector2D(X, Y), Start.Value));
				}
				if (Nearest > Largest * Largest)
				{
					Largest = FMath::Sqrt(Nearest);
					LargestAt = FVector2D(X, Y);
				}
			}
		}
		TestTrue(FString::Printf(TEXT("%s: a blank of %.0f m radius at %s (over %.0f m): corners or sides without a deposit"), Info.Key, Largest / 100.0, *LargestAt.ToString(),
			LargestEmptyCircle / 100.0), Largest <= LargestEmptyCircle);

		AddInfo(FString::Printf(TEXT("%s: %d places, %d deposits per player (slot 0: %d stone, %d gold), nearest source %.0f cm from the centre, largest empty circle %.1f m, groves of 8/10/12/16/24: %d/%d/%d/%d/%d, berry patches of 3/4/5: %d/%d/%d"),
			Info.Key, Info.NumPlaces, PerPlayer.FindRef(0), Stone.FindRef(0), Gold.FindRef(0), Closest, Largest / 100.0, Sizes.FindRef(8), Sizes.FindRef(10), Sizes.FindRef(12), Sizes.FindRef(16),
			Sizes.FindRef(24), Sizes.FindRef(103), Sizes.FindRef(104), Sizes.FindRef(105)));
	}
	return true;
}

#endif
