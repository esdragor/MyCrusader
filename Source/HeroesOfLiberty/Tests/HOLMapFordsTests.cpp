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

// C7 checks (Cartes): Les Gues at every size (Tools\make_maps.ps1 -Map Fords), read from the saved files without starting them. GDD 8.5, D496,
// D595: the water cannot be crossed except at the fords and the wide bridges. From 4 to 8 places a ring of water round the middle with a ford and
// a wide bridge in front of every base; with 2 places a straight river between the two halves with one wide bridge in the middle and two fords.
// The fairness and the budget per player are Map.Table's; here, what is the Gues' own: the numbers of fords and bridges, the gaps free of water,
// no source in the water, and the sealing (the water pieces of the file rasterised at 50 cm, a flood fill with the gaps plugged or not).
namespace UE::HeroesOfLiberty::Tests::MapFords
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The side of a cell of the raster the water is drawn on (50 cm). */
	constexpr double CellSize = 50.0;

	/** A bridge is free of water inside BridgeFree (a formation of 12 m passes: the gap is 16 m wide), a ford inside FordFree (the gap is 5 m wide). */
	constexpr double BridgeFree = 600.0;
	constexpr double FordFree = 200.0;

	/** A gap is plugged by a disc this big (the corridor through the water is 18 m long and at most 16 m wide: its corners are 12.1 m from its centre). */
	constexpr double PlugRadius = 1300.0;

	struct FWater
	{
		FVector2D At;
		FVector2D Half;  // half the length along the piece and half its width, in cm
		double Yaw;
	};

	struct FGap
	{
		bool bBridge;
		FVector2D At;
	};

	struct FData
	{
		FBox2D Area;
		TMap<int32, FVector2D> Starts;
		TArray<FHOLPlacedSource> Sources;
		TArray<FWater> Water;
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
			const FRotator Turn = Actor->GetRootComponent()->GetRelativeRotation();
			const FVector Scale = Actor->GetRootComponent()->GetRelativeScale3D();
#if WITH_EDITOR
			const FString Label = Actor->GetActorLabel();
#else
			const FString Label;
#endif
			if (const AStaticMeshActor* Mesh = Cast<AStaticMeshActor>(Actor))
			{
				const UStaticMesh* Shape = Mesh->GetStaticMeshComponent() ? Mesh->GetStaticMeshComponent()->GetStaticMesh() : nullptr;
				if (Shape && Shape->GetName() == TEXT("Cube") && Label.StartsWith(TEXT("Water")))
				{
					// The engine's cube is 1 m on a side.
					Data.Water.Add({ At, FVector2D(50.0 * Scale.X, 50.0 * Scale.Y), Turn.Yaw });
				}
			}
			else if (Actor->GetClass()->GetName() == TEXT("TargetPoint"))
			{
				if (Label.StartsWith(TEXT("FordGap")))
				{
					Data.Gaps.Add({ false, At });
				}
				else if (Label.StartsWith(TEXT("BridgeGap")))
				{
					Data.Gaps.Add({ true, At });
				}
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

	/** The raster of the water: 1 = water (or a plug). */
	struct FRaster
	{
		FVector2D Min;
		int32 N = 0;
		TArray<uint8> Blocked;

		FIntPoint CellOf(const FVector2D& At) const
		{
			return FIntPoint(FMath::Clamp(FMath::FloorToInt((At.X - Min.X) / CellSize), 0, N - 1), FMath::Clamp(FMath::FloorToInt((At.Y - Min.Y) / CellSize), 0, N - 1));
		}
		FVector2D CentreOf(const int32 I, const int32 J) const { return FVector2D(Min.X + (I + 0.5) * CellSize, Min.Y + (J + 0.5) * CellSize); }
		bool IsBlocked(const FVector2D& At) const { const FIntPoint Cell = CellOf(At); return Blocked[Cell.Y * N + Cell.X] != 0; }
	};

	FRaster Draw(const FData& Data)
	{
		FRaster Raster;
		Raster.Min = Data.Area.Min;
		Raster.N = FMath::CeilToInt((Data.Area.Max.X - Data.Area.Min.X) / CellSize);
		Raster.Blocked.SetNumZeroed(Raster.N * Raster.N);
		for (const FWater& Piece : Data.Water)
		{
			const double Reach = Piece.Half.Size();
			const double Cos = FMath::Cos(FMath::DegreesToRadians(Piece.Yaw)), Sin = FMath::Sin(FMath::DegreesToRadians(Piece.Yaw));
			const FIntPoint Low = Raster.CellOf(Piece.At - FVector2D(Reach)), High = Raster.CellOf(Piece.At + FVector2D(Reach));
			for (int32 J = Low.Y; J <= High.Y; ++J)
			{
				for (int32 I = Low.X; I <= High.X; ++I)
				{
					const FVector2D Offset = Raster.CentreOf(I, J) - Piece.At;
					const double Along = Offset.X * Cos + Offset.Y * Sin, Across = -Offset.X * Sin + Offset.Y * Cos;
					if (FMath::Abs(Along) <= Piece.Half.X && FMath::Abs(Across) <= Piece.Half.Y)
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

	/** The connected free regions (4 neighbours) of a raster: a label for each cell, -1 for water. */
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

	/** Whether the places (and the middle) are in one region of the raster; with bMiddle the middle must be in it too. */
	bool OneRegion(const FRaster& Raster, const FData& Data, const bool bMiddle)
	{
		const TArray<int32> Labels = Label(Raster);
		const int32 First = LabelAt(Raster, Labels, Data.Starts[0]);
		bool bSame = First > 0;
		for (const TPair<int32, FVector2D>& Start : Data.Starts)
		{
			bSame = bSame && LabelAt(Raster, Labels, Start.Value) == First;
		}
		return bSame && (!bMiddle || LabelAt(Raster, Labels, FVector2D::ZeroVector) == First);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapFordsTest, "HeroesOfLiberty.Map.Fords", UE::HeroesOfLiberty::Tests::MapFords::Flags)
bool FHOLMapFordsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapFords;

	for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
	{
		const FString File = FHOLMapSizeRules::GetMapFile(FHOLMapRules::GetAssetPath(TEXT("Fords")), Info.Size).ToString();
		const UWorld* Map = LoadObject<UWorld>(nullptr, *File);
		FData Data;
		if (!TestNotNull(FString::Printf(TEXT("%s (Tools\\make_maps.ps1 -Map Fords -Sizes %s)"), *File, Info.Key), Map) || !Read(Map, Data)
			|| !TestEqual(FString::Printf(TEXT("%s: one start per place"), Info.Key), Data.Starts.Num(), Info.NumPlaces))
		{
			continue;
		}
		const FString Name = FString::Printf(TEXT("Fords_%s"), Info.Key);
		const int32 Places = Info.NumPlaces;
		FRaster Raster = Draw(Data);

		// 1. The numbers: a ford and a wide bridge in front of every base (4 to 8 places), or one bridge and two fords on the river (2 places).
		int32 Bridges = 0, Fords = 0;
		for (const FGap& Gap : Data.Gaps)
		{
			++(Gap.bBridge ? Bridges : Fords);
		}
		TestTrue(FString::Printf(TEXT("%s: water drawn (%d pieces)"), *Name, Data.Water.Num()), Data.Water.Num() > 0);
		TestEqual(FString::Printf(TEXT("%s: wide bridges"), *Name), Bridges, Places == 2 ? 1 : Places);
		TestEqual(FString::Printf(TEXT("%s: fords"), *Name), Fords, Places == 2 ? 2 : Places);

		// 2. Every gap is free of water inside its width (a formation passes a bridge, a file a ford).
		for (const FGap& Gap : Data.Gaps)
		{
			const double Free = Gap.bBridge ? BridgeFree : FordFree;
			bool bFree = true;
			for (double Angle = 0.0; Angle < 2.0 * PI && bFree; Angle += PI / 12.0)
			{
				for (double Radius = 0.0; Radius <= Free && bFree; Radius += CellSize)
				{
					bFree = !Raster.IsBlocked(Gap.At + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
				}
			}
			TestTrue(FString::Printf(TEXT("%s: the %s at %s is free of water within %.0f cm"), *Name, Gap.bBridge ? TEXT("bridge") : TEXT("ford"), *Gap.At.ToString(), Free), bFree);
		}

		// 3. No source in the water.
		int32 Drowned = 0;
		for (const FHOLPlacedSource& Source : Data.Sources)
		{
			if (Raster.IsBlocked(FVector2D(Source.Location)))
			{
				++Drowned;
				AddInfo(FString::Printf(TEXT("%s: %s at %s is in the water"), *Name, *Source.Type.ToString(), *FVector2D(Source.Location).ToString()));
			}
		}
		TestEqual(FString::Printf(TEXT("%s: sources in the water"), *Name), Drowned, 0);

		// 4. The sealing. With the gaps open, every place reaches every other (and the middle); plugged, the water closes: the two halves of the river
		// are apart (2 places), or the middle is cut off from the places that stay together on the outside (4 to 8 places). Fords alone and bridges alone
		// each link everything too: the bridge and the fords are both ways across.
		TestTrue(FString::Printf(TEXT("%s: with the gaps open every place (and the middle) is in one region"), *Name), OneRegion(Raster, Data, true));
		for (const bool bPlugBridges : { true, false })
		{
			FRaster Part = Raster;
			for (const FGap& Gap : Data.Gaps)
			{
				if (Gap.bBridge == bPlugBridges)
				{
					PlugDisc(Part, Gap.At, PlugRadius);
				}
			}
			// With the only bridge of the river plugged the middle is a ford-less water gap: only the places are asked to meet.
			const bool bMiddle = !(Places == 2 && bPlugBridges);
			TestTrue(FString::Printf(TEXT("%s: with the %s alone open every place is in one region"), *Name, bPlugBridges ? TEXT("fords") : TEXT("bridges")), OneRegion(Part, Data, bMiddle));
		}
		for (const FGap& Gap : Data.Gaps)
		{
			PlugDisc(Raster, Gap.At, PlugRadius);
		}
		{
			const TArray<int32> Labels = Label(Raster);
			if (Places == 2)
			{
				TSet<int32> Regions;
				for (const TPair<int32, FVector2D>& Start : Data.Starts)
				{
					Regions.Add(LabelAt(Raster, Labels, Start.Value));
				}
				TestEqual(FString::Printf(TEXT("%s: regions of the two places with every gap plugged"), *Name), Regions.Num(), 2);
			}
			else
			{
				const int32 Outside = LabelAt(Raster, Labels, Data.Starts[0]);
				for (const TPair<int32, FVector2D>& Start : Data.Starts)
				{
					TestTrue(FString::Printf(TEXT("%s: place %d stays with place 0 outside the water"), *Name, Start.Key), Outside > 0 && LabelAt(Raster, Labels, Start.Value) == Outside);
				}
				TestTrue(FString::Printf(TEXT("%s: the middle is cut off by the water when every gap is plugged"), *Name), LabelAt(Raster, Labels, FVector2D::ZeroVector) != Outside);
			}
		}
	}
	return true;
}

#endif
