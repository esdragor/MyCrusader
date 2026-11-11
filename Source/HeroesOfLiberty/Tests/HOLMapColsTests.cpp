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

#if WITH_DEV_AUTOMATION_TESTS

// C4 checks (Cartes): Les Cols at every size (Tools\make_maps.ps1 -Map Cols), read from the saved files without starting them. GDD 8.5:
// mountains cut the map into corridors between the bases. The fairness and the budget per player are Map.Table's; here, what is the
// Cols' own: a range of rock between every two neighbouring places that closes their two territories except through its cols (the
// rocks of the file rasterised at 50 cm, a flood fill with the cols and the central plaza plugged or not), the cols free of rock, no
// source inside a rock.
namespace UE::HeroesOfLiberty::Tests::MapCols
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The side of a cell of the raster the rocks are drawn on (50 cm). */
	constexpr double TestCellSize = 50.0;

	/** The plaza of the recipe (cols.py PLAZA_FRACTION) as a fraction of the half side, and the margin of its plug beyond the ranges' ends. */
	constexpr double PlazaFraction = 0.2;
	constexpr double PlazaPlugMargin = 1.05;

	/** A col is plugged by a disc of this radius (a massif is two chains 14 m apart, each cut at the col: the disc covers both cuts), and free of rock inside FreeRadius (a formation of 12 m). */
	constexpr double PlugRadius = 1800.0;
	constexpr double FreeRadius = 600.0;

	/** A range has this many cols at least and at most (cols.py COL_FRACTIONS). */
	constexpr int32 MinColsPerRange = 1;
	constexpr int32 MaxColsPerRange = 3;

	struct FRock
	{
		FVector2D At;
		FVector2D Scale;  // of the 1 m sphere: the footprint's semi axes are 50 cm times it
		double Yaw;
	};

	struct FCol
	{
		int32 Range;
		FVector2D At;
	};

	struct FData
	{
		FBox2D Area;
		TMap<int32, FVector2D> Starts;
		TArray<FHOLPlacedSource> Sources;
		TArray<FRock> Rocks;
		TArray<FCol> Cols;
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
			else if (Actor->GetClass()->GetName() == TEXT("TargetPoint"))
			{
#if WITH_EDITOR
				const FString Label = Actor->GetActorLabel();
				if (Label.StartsWith(TEXT("ColGap")))
				{
					FString Number, Unused;
					Label.RightChop(6).Split(TEXT("_"), &Number, &Unused);
					Data.Cols.Add({ FCString::Atoi(*Number), At });
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

		FIntPoint CellOf(const FVector2D& At) const
		{
			return FIntPoint(FMath::Clamp(FMath::FloorToInt((At.X - Min.X) / TestCellSize), 0, N - 1), FMath::Clamp(FMath::FloorToInt((At.Y - Min.Y) / TestCellSize), 0, N - 1));
		}
		FVector2D CentreOf(const int32 I, const int32 J) const { return FVector2D(Min.X + (I + 0.5) * TestCellSize, Min.Y + (J + 0.5) * TestCellSize); }
		bool IsBlocked(const FVector2D& At) const { const FIntPoint Cell = CellOf(At); return Blocked[Cell.Y * N + Cell.X] != 0; }
	};

	FRaster Draw(const FData& Data)
	{
		FRaster Raster;
		Raster.Min = Data.Area.Min;
		Raster.N = FMath::CeilToInt((Data.Area.Max.X - Data.Area.Min.X) / TestCellSize);
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
						Raster.Blocked[J * Raster.N + I] = 1;
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
					Raster.Blocked[J * Raster.N + I] = 1;
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

	int32 LabelAt(const FRaster& Raster, const TArray<int32>& Labels, const FVector2D& At)
	{
		const FIntPoint Cell = Raster.CellOf(At);
		return Labels[Cell.Y * Raster.N + Cell.X];
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapColsTest, "HeroesOfLiberty.Map.Cols", UE::HeroesOfLiberty::Tests::MapCols::Flags)
bool FHOLMapColsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapCols;

	for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
	{
		const FString File = FHOLMapSizeRules::GetMapFile(FHOLMapRules::GetAssetPath(TEXT("Cols")), Info.Size).ToString();
		const UWorld* Map = LoadObject<UWorld>(nullptr, *File);
		FData Data;
		if (!TestNotNull(FString::Printf(TEXT("%s (Tools\\make_maps.ps1 -Map Cols -Sizes %s)"), *File, Info.Key), Map) || !Read(Map, Data)
			|| !TestEqual(FString::Printf(TEXT("%s: one start per place"), Info.Key), Data.Starts.Num(), Info.NumPlaces))
		{
			continue;
		}
		const FString Name = FString::Printf(TEXT("Cols_%s"), Info.Key);
		const int32 Places = Info.NumPlaces;
		const double Half = Data.Area.Max.X;
		FRaster Raster = Draw(Data);

		// 1. One range between every two neighbouring places, each with 1 to 3 cols, the same number on each.
		TMap<int32, int32> ColsOfRange;
		for (const FCol& Col : Data.Cols)
		{
			++ColsOfRange.FindOrAdd(Col.Range);
		}
		TestEqual(FString::Printf(TEXT("%s: a range between every two neighbouring places"), *Name), ColsOfRange.Num(), Places);
		int32 PerRange = -1;
		for (const TPair<int32, int32>& Range : ColsOfRange)
		{
			PerRange = PerRange < 0 ? Range.Value : PerRange;
			TestEqual(FString::Printf(TEXT("%s: range %d has as many cols as the others"), *Name, Range.Key), Range.Value, PerRange);
			TestTrue(FString::Printf(TEXT("%s: range %d has %d cols (%d to %d)"), *Name, Range.Key, Range.Value, MinColsPerRange, MaxColsPerRange), Range.Value >= MinColsPerRange && Range.Value <= MaxColsPerRange);
		}

		// 2. Every col is free of rock inside FreeRadius (a formation passes).
		for (const FCol& Col : Data.Cols)
		{
			bool bFree = true;
			for (double Angle = 0.0; Angle < 2.0 * PI && bFree; Angle += PI / 12.0)
			{
				for (double Radius = 0.0; Radius <= FreeRadius && bFree; Radius += TestCellSize)
				{
					bFree = !Raster.IsBlocked(Col.At + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
				}
			}
			TestTrue(FString::Printf(TEXT("%s: the col of range %d at %s is free of rock within %.0f cm"), *Name, Col.Range, *Col.At.ToString(), FreeRadius), bFree);
		}

		// 3. No source inside a rock.
		int32 Buried = 0;
		for (const FHOLPlacedSource& Source : Data.Sources)
		{
			if (Raster.IsBlocked(FVector2D(Source.Location)))
			{
				++Buried;
				AddInfo(FString::Printf(TEXT("%s: %s at %s is inside a rock"), *Name, *Source.Type.ToString(), *FVector2D(Source.Location).ToString()));
			}
		}
		TestEqual(FString::Printf(TEXT("%s: sources inside a rock"), *Name), Buried, 0);

		// 4. The sealing. The central plaza plugged (the ranges start at its edge): with the cols open every place reaches every other (the cols are
		// the corridors); with the cols plugged too, no two places are in the same region (the ranges close the territories).
		PlugDisc(Raster, FVector2D::ZeroVector, PlazaFraction * Half * PlazaPlugMargin);
		{
			const TArray<int32> Labels = Label(Raster);
			const int32 First = LabelAt(Raster, Labels, Data.Starts[0]);
			for (const TPair<int32, FVector2D>& Start : Data.Starts)
			{
				TestTrue(FString::Printf(TEXT("%s: place %d reaches place 0 through the cols"), *Name, Start.Key), First > 0 && LabelAt(Raster, Labels, Start.Value) == First);
			}
		}
		for (const FCol& Col : Data.Cols)
		{
			PlugDisc(Raster, Col.At, PlugRadius);
		}
		{
			const TArray<int32> Labels = Label(Raster);
			TSet<int32> Regions;
			for (const TPair<int32, FVector2D>& Start : Data.Starts)
			{
				Regions.Add(LabelAt(Raster, Labels, Start.Value));
			}
			TestEqual(FString::Printf(TEXT("%s: regions of the places with every col and the plaza plugged"), *Name), Regions.Num(), Places);
		}
	}
	return true;
}

#endif
