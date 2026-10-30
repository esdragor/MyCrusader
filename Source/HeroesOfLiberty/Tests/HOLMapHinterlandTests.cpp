// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/StaticMeshComponent.h"
#include "Engine/Level.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Map/HOLMapBounds.h"
#include "Map/HOLMapRules.h"
#include "Map/HOLMapSizeRules.h"
#include "Map/HOLStartSlot.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceLayout.h"
#include "Resources/HOLSourceTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// C2 checks (Cartes): L'Arriere-pays at every size (Tools\make_maps.ps1 -Map Hinterland), read from the saved files without starting
// them. GDD 8.5: the riches are in a pocket behind each base, closed in by long walls of rock, reached by the paths along the edges of
// the map, never through the middle; the middle stays empty. The fairness between places and the budget per player are checked for every
// map by Map.Table; here, what is the Hinterland's own: the sealing (the rocks of the file rasterised at 50 cm, a flood fill with the
// openings and the passages plugged or not), the empty middle, the width of the gaps, the navigation volumes under the rocks.
namespace UE::HeroesOfLiberty::Tests::MapHinterland
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The side of a cell of the raster the rocks are drawn on (50 cm: a gap of 14 m is 28 cells). */
	constexpr double CellSize = 50.0;

	/** A source within this of its slot is the base (its wood of 25 trees and its 6 berry bushes); farther, it is the pocket's. */
	constexpr double BaseReach = 4000.0;

	/** The veins are in the pockets, never in the base: at least 50 m from every slot (the base's wood and berries are within 40 m). */
	constexpr double VeinFromSlot = 5000.0;

	/** Each pocket shuts in at least this many gold and stone veins (the big ones; the budget is 4 + 2, the recipe makes 5 + 3). */
	constexpr int32 MinPocketVeins = 6;

	/** A gap (an opening or a passage) is plugged by a disc of this radius, 10 m: the rocks stop 7 to 8.2 m from its centre. */
	constexpr double PlugRadius = 1000.0;

	/** The point just outside / inside a gap, along the line from the middle of the map: beyond the wall's thickness (10 m). */
	constexpr double BeyondGap = 1600.0;

	/** A gap's free width: a disc of 6 m radius round its marker is free of rock, so a formation of 12 m passes. */
	constexpr double FreeRadius = 600.0;

	/** No source closer than this to a rock's footprint (a Peasant works at a source's edge). */
	constexpr double SourceFromRock = 150.0;

	/** The passages of the belt: at most this many on one side of the map, away from the middle of it. */
	constexpr int32 MaxPassagesPerSide = 3;

	/** The border corridor: the rocks are at least this far from the edge of the bounds (about 20 m, 26 m less the wobble and a rock's half width). */
	constexpr double CorridorLeast = 1400.0;

	struct FRock
	{
		FVector2D At;
		FVector2D Scale;  // of the 1 m sphere: the footprint's semi axes are 50 cm times it
		double Yaw;
	};

	struct FVolume
	{
		FVector2D At;
		FVector2D Half;
		double Yaw;
	};

	struct FGap
	{
		FString Label;
		FVector2D At;
		int32 Slot;  // of a pocket's opening; -1 for a passage of the belt
	};

	/** What the Hinterland's file holds, as the tests read it. */
	struct FData
	{
		FBox2D Area;
		TMap<int32, FVector2D> Starts;
		TArray<FHOLPlacedSource> Sources;
		TArray<FRock> Rocks;
		TArray<FVolume> Volumes;
		TArray<FGap> Gaps;
	};

	bool Read(const UWorld* Map, FData& Data)
	{
		if (!Map || !Map->PersistentLevel)
		{
			return false;
		}
		const AHOLMapBounds* Bounds = nullptr;
		const AHOLSourceLayout* Layout = nullptr;
		for (const AActor* Actor : Map->PersistentLevel->Actors)
		{
			if (!Actor || !Actor->GetRootComponent())
			{
				continue;
			}
			const FVector2D At(Actor->GetRootComponent()->GetRelativeLocation());
			if (const AHOLStartSlot* Start = Cast<AHOLStartSlot>(Actor))
			{
				Data.Starts.Add(Start->GetSlotIndex(), At);
			}
			Bounds = Bounds ? Bounds : Cast<AHOLMapBounds>(Actor);
			Layout = Layout ? Layout : Cast<AHOLSourceLayout>(Actor);
			const FString ClassName = Actor->GetClass()->GetName();
			const FRotator Turn = Actor->GetRootComponent()->GetRelativeRotation();
			const FVector Scale = Actor->GetRootComponent()->GetRelativeScale3D();
			if (const AStaticMeshActor* Mesh = Cast<AStaticMeshActor>(Actor))
			{
				const UStaticMesh* Shape = Mesh->GetStaticMeshComponent() ? Mesh->GetStaticMeshComponent()->GetStaticMesh() : nullptr;
				if (Shape && Shape->GetName() == TEXT("Sphere"))
				{
					Data.Rocks.Add({ At, FVector2D(Scale.X, Scale.Y), Turn.Yaw });
				}
			}
			else if (ClassName == TEXT("NavModifierVolume"))
			{
				// The default brush is a cube of 2 m: its half sides are 1 m times the scale.
				Data.Volumes.Add({ At, FVector2D(100.0 * Scale.X, 100.0 * Scale.Y), Turn.Yaw });
			}
			else if (ClassName == TEXT("TargetPoint"))
			{
#if WITH_EDITOR
				const FString Label = Actor->GetActorLabel();
				int32 Slot = -1;
				if (Label.StartsWith(TEXT("PocketGap")))
				{
					FString Rest = Label.RightChop(9), Number, Unused;
					Rest.Split(TEXT("_"), &Number, &Unused);
					Slot = FCString::Atoi(*Number);
				}
				if (Label.StartsWith(TEXT("PocketGap")) || Label.StartsWith(TEXT("BeltGap")))
				{
					Data.Gaps.Add({ Label, At, Slot });
				}
#endif
			}
		}
		if (!Bounds || !Layout)
		{
			return false;
		}
		Data.Area = Bounds->GetArea();
		Data.Sources = Layout->GetSources();
		return true;
	}

	/** The raster of the rocks: 1 = rock. */
	struct FRaster
	{
		FVector2D Min;
		int32 N = 0;
		TArray<uint8> Blocked;

		int32 Index(const int32 I, const int32 J) const { return J * N + I; }
		FIntPoint CellOf(const FVector2D& At) const
		{
			return FIntPoint(FMath::Clamp(FMath::FloorToInt((At.X - Min.X) / CellSize), 0, N - 1), FMath::Clamp(FMath::FloorToInt((At.Y - Min.Y) / CellSize), 0, N - 1));
		}
		FVector2D CentreOf(const int32 I, const int32 J) const { return FVector2D(Min.X + (I + 0.5) * CellSize, Min.Y + (J + 0.5) * CellSize); }
	};

	FRaster Draw(const FData& Data)
	{
		FRaster Raster;
		Raster.Min = Data.Area.Min;
		Raster.N = FMath::CeilToInt((Data.Area.Max.X - Data.Area.Min.X) / CellSize);
		Raster.Blocked.SetNumZeroed(Raster.N * Raster.N);
		for (const FRock& Rock : Data.Rocks)
		{
			const double A = 50.0 * Rock.Scale.X, B = 50.0 * Rock.Scale.Y;
			const double Reach = FMath::Max(A, B);
			const double Cos = FMath::Cos(FMath::DegreesToRadians(Rock.Yaw)), Sin = FMath::Sin(FMath::DegreesToRadians(Rock.Yaw));
			const FIntPoint Low = Raster.CellOf(Rock.At - FVector2D(Reach)), High = Raster.CellOf(Rock.At + FVector2D(Reach));
			for (int32 J = Low.Y; J <= High.Y; ++J)
			{
				for (int32 I = Low.X; I <= High.X; ++I)
				{
					const FVector2D Offset = Raster.CentreOf(I, J) - Rock.At;
					const double Along = Offset.X * Cos + Offset.Y * Sin, Across = -Offset.X * Sin + Offset.Y * Cos;
					if (FMath::Square(Along / A) + FMath::Square(Across / B) <= 1.0)
					{
						Raster.Blocked[Raster.Index(I, J)] = 1;
					}
				}
			}
		}
		return Raster;
	}

	void PlugDisc(FRaster& Raster, const FVector2D& At, const double Radius)
	{
		const FIntPoint Low = Raster.CellOf(At - FVector2D(Radius)), High = Raster.CellOf(At + FVector2D(Radius));
		for (int32 J = Low.Y; J <= High.Y; ++J)
		{
			for (int32 I = Low.X; I <= High.X; ++I)
			{
				if (FVector2D::DistSquared(Raster.CentreOf(I, J), At) <= Radius * Radius)
				{
					Raster.Blocked[Raster.Index(I, J)] = 1;
				}
			}
		}
	}

	/** The connected free regions (4 neighbours) of a raster: a label for each cell, -1 for a rock. */
	TArray<int32> Label(const FRaster& Raster)
	{
		TArray<int32> Labels;
		Labels.Init(0, Raster.N * Raster.N);
		for (int32 Cell = 0; Cell < Labels.Num(); ++Cell)
		{
			Labels[Cell] = Raster.Blocked[Cell] ? -1 : 0;
		}
		int32 Next = 0;
		TArray<int32> Stack;
		for (int32 First = 0; First < Labels.Num(); ++First)
		{
			if (Labels[First] != 0)
			{
				continue;
			}
			++Next;
			Labels[First] = Next;
			Stack.Add(First);
			while (!Stack.IsEmpty())
			{
				const int32 Cell = Stack.Pop(EAllowShrinking::No);
				const int32 I = Cell % Raster.N, J = Cell / Raster.N;
				const int32 Around[4] = { I > 0 ? Cell - 1 : -1, I + 1 < Raster.N ? Cell + 1 : -1, J > 0 ? Cell - Raster.N : -1, J + 1 < Raster.N ? Cell + Raster.N : -1 };
				for (const int32 Other : Around)
				{
					if (Other >= 0 && Labels[Other] == 0)
					{
						Labels[Other] = Next;
						Stack.Add(Other);
					}
				}
			}
		}
		return Labels;
	}

	int32 NearestSlot(const FData& Data, const FVector2D& At)
	{
		int32 Best = 0;
		for (const TPair<int32, FVector2D>& Start : Data.Starts)
		{
			Best = FVector2D::DistSquared(At, Start.Value) < FVector2D::DistSquared(At, Data.Starts[Best]) ? Start.Key : Best;
		}
		return Best;
	}

	FVector2D Beyond(const FVector2D& At, const double Distance)
	{
		return At + At.GetSafeNormal() * Distance;
	}

	/**
	 * The sealing of a Hinterland: the number of failures, each reported through Fail. Four floods, by which gaps are plugged:
	 *  all plugged        no pocket source is in the middle's region, nor is the border corridor;
	 *  pockets open       a pocket's sources are in the corridor's region, still not in the middle's (nothing leads in from the middle);
	 *  passages open      the middle reaches the corridor (the passages), and still no pocket;
	 *  all open           the middle reaches every pocket source (by the passages, the corridor and the openings).
	 * A map without gaps (no markers) fails at once. The middle's region is the one of slot 0 (every slot is in it).
	 */
	int32 CheckSealing(const FData& Data, const FString& Name, const TFunctionRef<void(const FString&)> Fail)
	{
		int32 Failures = 0;
		const auto Report = [&](const FString& Text)
		{
			++Failures;
			Fail(Name + TEXT(": ") + Text);
		};
		TArray<const FGap*> Openings, Passages;
		for (const FGap& Gap : Data.Gaps)
		{
			(Gap.Slot >= 0 ? Openings : Passages).Add(&Gap);
		}
		if (Openings.IsEmpty() || Passages.IsEmpty() || !Data.Starts.Contains(0))
		{
			Report(FString::Printf(TEXT("no pocket openings (%d) or belt passages (%d) in the file: nothing is closed in"), Openings.Num(), Passages.Num()));
			return Failures;
		}
		const FRaster Rock = Draw(Data);
		// The pockets' sources: those in the region of an opening's inside point when every gap is plugged (the middle is then the region of
		// slot 0 and the pockets are closed rooms). Whatever else the map holds (the bases, the front lanes, the middle's scatter) is not asked.
		TArray<int32> PocketSources;
		{
			FRaster Closed = Rock;
			for (const FGap& Gap : Data.Gaps)
			{
				PlugDisc(Closed, Gap.At, PlugRadius);
			}
			const TArray<int32> Labels = Label(Closed);
			const auto LabelOf = [&](const FVector2D& At) { const FIntPoint Cell = Closed.CellOf(At); return Labels[Closed.Index(Cell.X, Cell.Y)]; };
			TSet<int32> Rooms;
			for (const FGap* Gap : Openings)
			{
				Rooms.Add(LabelOf(Beyond(Gap->At, -BeyondGap)));
			}
			Rooms.Remove(LabelOf(Data.Starts[0]));
			for (int32 Index = 0; Index < Data.Sources.Num(); ++Index)
			{
				if (Rooms.Contains(LabelOf(FVector2D(Data.Sources[Index].Location))))
				{
					PocketSources.Add(Index);
				}
			}
			TMap<int32, int32> VeinsOf;
			for (const int32 Index : PocketSources)
			{
				const FHOLPlacedSource& Source = Data.Sources[Index];
				if (HOLSourceTypeNames::IsGoldVein(Source.Type) || HOLSourceTypeNames::IsStoneVein(Source.Type))
				{
					VeinsOf.FindOrAdd(NearestSlot(Data, FVector2D(Source.Location)))++;
				}
			}
			for (const TPair<int32, FVector2D>& Start : Data.Starts)
			{
				if (VeinsOf.FindRef(Start.Key) < MinPocketVeins)
				{
					Report(FString::Printf(TEXT("slot %d has %d gold and stone veins shut in its pocket, under %d"), Start.Key, VeinsOf.FindRef(Start.Key), MinPocketVeins));
				}
			}
		}
		for (const FHOLPlacedSource& Source : Data.Sources)
		{
			const FIntPoint Cell = Rock.CellOf(FVector2D(Source.Location));
			if (Rock.Blocked[Rock.Index(Cell.X, Cell.Y)])
			{
				Report(FString::Printf(TEXT("%s at %s is inside a rock"), *Source.Type.ToString(), *FVector2D(Source.Location).ToString()));
			}
		}
		for (int32 Flood = 0; Flood < 4; ++Flood)
		{
			const bool bPockets = (Flood & 1) != 0, bPassages = (Flood & 2) != 0;
			const TCHAR* Which = Flood == 0 ? TEXT("all plugged") : (Flood == 1 ? TEXT("pockets open") : (Flood == 2 ? TEXT("passages open") : TEXT("all open")));
			FRaster Plugged = Rock;
			for (const FGap& Gap : Data.Gaps)
			{
				if ((Gap.Slot >= 0) ? !bPockets : !bPassages)
				{
					PlugDisc(Plugged, Gap.At, PlugRadius);
				}
			}
			const TArray<int32> Labels = Label(Plugged);
			const auto LabelAt = [&](const FVector2D& At)
			{
				const FIntPoint Cell = Plugged.CellOf(At);
				return Labels[Plugged.Index(Cell.X, Cell.Y)];
			};
			const int32 Middle = LabelAt(Data.Starts[0]);
			for (const TPair<int32, FVector2D>& Start : Data.Starts)
			{
				if (LabelAt(Start.Value) != Middle)
				{
					Report(FString::Printf(TEXT("[%s] slot %d is cut off from slot 0 (the middle is not one region)"), Which, Start.Key));
				}
			}
			for (const FGap* Gap : Passages)
			{
				const int32 Outside = LabelAt(Beyond(Gap->At, BeyondGap)), Inside = LabelAt(Beyond(Gap->At, -BeyondGap));
				if (bPassages && (Outside != Middle || Inside != Middle))
				{
					Report(FString::Printf(TEXT("[%s] the passage %s does not join the middle to the corridor (%d / %d, middle %d)"), Which, *Gap->Label, Inside, Outside, Middle));
				}
				if (!bPassages && Outside == Middle)
				{
					Report(FString::Printf(TEXT("[%s] the corridor beyond %s is in the middle's region: the belt leaks"), Which, *Gap->Label));
				}
			}
			for (const FGap* Gap : Openings)
			{
				const int32 Inside = LabelAt(Beyond(Gap->At, -BeyondGap)), Outside = LabelAt(Beyond(Gap->At, BeyondGap));
				if ((Inside == Middle) != (bPockets && bPassages))
				{
					Report(FString::Printf(TEXT("[%s] the inside of %s is %s the middle's region"), Which, *Gap->Label, Inside == Middle ? TEXT("in") : TEXT("not in")));
				}
				if ((Inside == Outside) != bPockets)
				{
					Report(FString::Printf(TEXT("[%s] the opening %s %s the pocket to the corridor"), Which, *Gap->Label, Inside == Outside ? TEXT("joins") : TEXT("does not join")));
				}
			}
			for (const int32 Index : PocketSources)
			{
				const FVector2D At(Data.Sources[Index].Location);
				const int32 Slot = NearestSlot(Data, At);
				const int32 Region = LabelAt(At);
				bool bOpeningReached = false;
				for (const FGap* Gap : Openings)
				{
					bOpeningReached |= Gap->Slot == Slot && LabelAt(Beyond(Gap->At, BeyondGap)) == Region;
				}
				const bool bShouldReachMiddle = bPockets && bPassages;
				if (bShouldReachMiddle != (Region == Middle))
				{
					Report(FString::Printf(TEXT("[%s] %s of slot %d at %s is %s the middle's region"), Which, *Data.Sources[Index].Type.ToString(), Slot, *At.ToString(),
						Region == Middle ? TEXT("in") : TEXT("not in")));
				}
				if (bPockets && !bOpeningReached)
				{
					Report(FString::Printf(TEXT("[%s] %s of slot %d at %s does not reach the corridor by an opening of its pocket"), Which, *Data.Sources[Index].Type.ToString(), Slot,
						*At.ToString()));
				}
				if (!bPockets && bOpeningReached)
				{
					Report(FString::Printf(TEXT("[%s] %s of slot %d at %s reaches the corridor with its openings plugged: the pocket leaks"), Which, *Data.Sources[Index].Type.ToString(), Slot,
						*At.ToString()));
				}
			}
		}
		return Failures;
	}

	/** Whether a point is inside the rectangle of a navigation volume. */
	bool InVolume(const FVolume& Volume, const FVector2D& At)
	{
		const double Cos = FMath::Cos(FMath::DegreesToRadians(Volume.Yaw)), Sin = FMath::Sin(FMath::DegreesToRadians(Volume.Yaw));
		const FVector2D Offset = At - Volume.At;
		return FMath::Abs(Offset.X * Cos + Offset.Y * Sin) <= Volume.Half.X && FMath::Abs(-Offset.X * Sin + Offset.Y * Cos) <= Volume.Half.Y;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapHinterlandTest, "HeroesOfLiberty.Map.Hinterland", UE::HeroesOfLiberty::Tests::MapHinterland::Flags)
bool FHOLMapHinterlandTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapHinterland;

	for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
	{
		const FString File = FHOLMapSizeRules::GetMapFile(FHOLMapRules::GetAssetPath(TEXT("Hinterland")), Info.Size).ToString();
		const UWorld* Map = LoadObject<UWorld>(nullptr, *File);
		FData Data;
		if (!TestNotNull(FString::Printf(TEXT("%s (Tools\\make_maps.ps1 -Map Hinterland -Sizes %s)"), *File, Info.Key), Map) || !Read(Map, Data)
			|| !TestEqual(FString::Printf(TEXT("%s: one start per place"), Info.Key), Data.Starts.Num(), Info.NumPlaces))
		{
			continue;
		}
		const FString Name = FString::Printf(TEXT("Hinterland_%s"), Info.Key);
		const double Half = Data.Area.Max.X;

		// 1. The middle holds something now (M-Q7 replaces A-Q2's empty middle; the rule of the largest empty circle is Map.Table's): wood groves and
		// outcrops are in the file, and some rock is within 100 m of the centre or some source.
		double ClosestSource = TNumericLimits<double>::Max(), ClosestRock = TNumericLimits<double>::Max();
		for (const FHOLPlacedSource& Source : Data.Sources)
		{
			ClosestSource = FMath::Min(ClosestSource, FVector2D(Source.Location).Size());
		}
		for (const FRock& Rock : Data.Rocks)
		{
			ClosestRock = FMath::Min(ClosestRock, Rock.At.Size());
		}

		// 2. The veins are in the pockets: 50 m from every slot at least; the base itself (within 40 m of its slot) has none.
		int32 Veins = 0, Pocket = 0;
		for (const FHOLPlacedSource& Source : Data.Sources)
		{
			const FVector2D At(Source.Location);
			const double FromSlot = FVector2D::Distance(At, Data.Starts[NearestSlot(Data, At)]);
			Pocket += FromSlot >= BaseReach;
			if (HOLSourceTypeNames::IsGoldVein(Source.Type) || HOLSourceTypeNames::IsStoneVein(Source.Type))
			{
				++Veins;
				TestTrue(FString::Printf(TEXT("%s: a %s at %s is %.0f cm from its slot, in the base (under %.0f)"), *Name, *Source.Type.ToString(), *At.ToString(), FromSlot, VeinFromSlot),
					FromSlot >= VeinFromSlot);
			}
		}
		TestTrue(FString::Printf(TEXT("%s: %d veins"), *Name, Veins), Veins >= 6 * Info.NumPlaces);

		// 3. The rocks: inside the bounds with a corridor of about 20 m beyond them, each inside a navigation volume (a unit never stands in
		// a rock), a volume for every 8 rocks at most (a piece of wall of 20 m is about 6 rocks).
		double LeastCorridor = TNumericLimits<double>::Max();
		int32 Unguarded = 0;
		for (const FRock& Rock : Data.Rocks)
		{
			const double Reach = 50.0 * FMath::Max(Rock.Scale.X, Rock.Scale.Y);
			LeastCorridor = FMath::Min(LeastCorridor, Half - FMath::Max(FMath::Abs(Rock.At.X), FMath::Abs(Rock.At.Y)) - Reach);
			bool bCovered = false;
			for (const FVolume& Volume : Data.Volumes)
			{
				if (InVolume(Volume, Rock.At))
				{
					bCovered = true;
					break;
				}
			}
			Unguarded += !bCovered;
		}
		TestTrue(FString::Printf(TEXT("%s: the corridor beyond the rocks is %.0f cm wide, under %.0f"), *Name, LeastCorridor, CorridorLeast), LeastCorridor >= CorridorLeast);
		TestEqual(FString::Printf(TEXT("%s: rocks outside every NavArea_Null volume"), *Name), Unguarded, 0);
		TestTrue(FString::Printf(TEXT("%s: %d rocks and %d navigation volumes"), *Name, Data.Rocks.Num(), Data.Volumes.Num()), Data.Volumes.Num() * 8 >= Data.Rocks.Num() && Data.Rocks.Num() >= 400);

		// 4. The gaps: 1 or 2 openings in every pocket, a free disc of 6 m round each marker, at most 3 passages of the belt on a side.
		TMap<int32, int32> OpeningsOf;
		TMap<int32, int32> PassagesOn;
		double LeastFree = TNumericLimits<double>::Max();
		const FRaster Rock = Draw(Data);
		for (const FGap& Gap : Data.Gaps)
		{
			if (Gap.Slot >= 0)
			{
				OpeningsOf.FindOrAdd(Gap.Slot)++;
			}
			else
			{
				const int32 Side = FMath::Abs(Gap.At.X) > FMath::Abs(Gap.At.Y) ? (Gap.At.X > 0 ? 0 : 1) : (Gap.At.Y > 0 ? 2 : 3);
				PassagesOn.FindOrAdd(Side)++;
			}
			const FIntPoint Low = Rock.CellOf(Gap.At - FVector2D(1.5 * FreeRadius)), High = Rock.CellOf(Gap.At + FVector2D(1.5 * FreeRadius));
			double Nearest = 1.5 * FreeRadius;
			for (int32 J = Low.Y; J <= High.Y; ++J)
			{
				for (int32 I = Low.X; I <= High.X; ++I)
				{
					if (Rock.Blocked[Rock.Index(I, J)])
					{
						Nearest = FMath::Min(Nearest, FVector2D::Distance(Rock.CentreOf(I, J), Gap.At));
					}
				}
			}
			const double Free = Nearest;
			LeastFree = FMath::Min(LeastFree, Free);
			TestTrue(FString::Printf(TEXT("%s: the gap %s has a rock %.0f cm from its middle, under %.0f (too narrow)"), *Name, *Gap.Label, Free, FreeRadius), Free >= FreeRadius);
		}
		for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
		{
			TestTrue(FString::Printf(TEXT("%s: slot %d's pocket has %d openings, not 1 or 2"), *Name, Slot, OpeningsOf.FindRef(Slot)), OpeningsOf.FindRef(Slot) >= 1 && OpeningsOf.FindRef(Slot) <= 2);
		}
		int32 MostOnASide = 0;
		for (const TPair<int32, int32>& Side : PassagesOn)
		{
			MostOnASide = FMath::Max(MostOnASide, Side.Value);
		}
		TestTrue(FString::Printf(TEXT("%s: %d passages on one side, over %d"), *Name, MostOnASide, MaxPassagesPerSide), MostOnASide <= MaxPassagesPerSide);
		TestTrue(FString::Printf(TEXT("%s: no passage in the belt"), *Name), PassagesOn.Num() > 0);

		// 5. The sealing, the real chain: the file's rocks rasterised, the gaps plugged or not.
		const int32 Failures = CheckSealing(Data, Name, [this](const FString& Text) { AddError(Text); });
		int32 NumPassages = 0;
		for (const TPair<int32, int32>& Side : PassagesOn)
		{
			NumPassages += Side.Value;
		}
		AddInfo(FString::Printf(TEXT("%s: %d places, %d rocks, %d volumes, %d openings, %d passages (most on a side %d), free radius at the gaps %.0f cm, corridor %.0f cm, %d pocket sources, nearest source %.0f cm and rock %.0f cm from the centre, sealing failures %d"),
			*Name, Info.NumPlaces, Data.Rocks.Num(), Data.Volumes.Num(), Data.Gaps.Num() - NumPassages, NumPassages, MostOnASide, LeastFree, LeastCorridor, Pocket, ClosestSource, ClosestRock, Failures));
	}
	return true;
}

// The control of the sealing check: the same check on a map with no pockets (the Steppe) must fail. A check that cannot be red proves nothing.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapHinterlandControlTest, "HeroesOfLiberty.Map.HinterlandControl", UE::HeroesOfLiberty::Tests::MapHinterland::Flags)
bool FHOLMapHinterlandControlTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapHinterland;

	const UWorld* Map = LoadObject<UWorld>(nullptr, TEXT("/Game/Maps/Steppe_Small"));
	FData Data;
	if (!TestNotNull(TEXT("Steppe_Small"), Map) || !Read(Map, Data))
	{
		return false;
	}
	TArray<FString> Messages;
	const int32 Failures = CheckSealing(Data, TEXT("Steppe_Small"), [&Messages](const FString& Text) { Messages.Add(Text); });
	TestTrue(FString::Printf(TEXT("the sealing check finds the Steppe open (%d failures: %s)"), Failures, Messages.IsEmpty() ? TEXT("none") : *Messages[0]), Failures > 0);

	// The same check on the Steppe with markers dropped on its sources and walls made of nothing: still red (the gaps alone prove nothing).
	FData Gapped = Data;
	Gapped.Gaps.Add({ TEXT("PocketGap0_0"), FVector2D(-16500.0, 3000.0), 0 });
	Gapped.Gaps.Add({ TEXT("BeltGap0_0"), FVector2D(0.0, 20000.0), -1 });
	Messages.Reset();
	const int32 GappedFailures = CheckSealing(Gapped, TEXT("Steppe_Small with gaps"), [&Messages](const FString& Text) { Messages.Add(Text); });
	TestTrue(FString::Printf(TEXT("the sealing check finds the Steppe with gaps open too (%d failures)"), GappedFailures), GappedFailures > 0);
	return true;
}

#endif
