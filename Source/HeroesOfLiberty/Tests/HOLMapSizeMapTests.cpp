// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Map/HOLMapBounds.h"
#include "Map/HOLMapSizeRules.h"
#include "Map/HOLStartSlot.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceLayout.h"

#if WITH_DEV_AUTOMATION_TESTS

// TC2 checks: Plaine at every size (Tools/Editor/create_match_map.py <size>), read from the saved files without starting
// them, as Map.StartSlots does for /Game/Maps/Plain.
namespace UE::HeroesOfLiberty::Tests::MapSizeMaps
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const TCHAR* PlainPath = TEXT("/Game/Maps/Plain.Plain");

	/** Past this distance from every slot, a source is neutral (TC-Q7 = B: more of them on the larger sizes). */
	constexpr double BaseReach = 7000.0;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapPlainSizesTest, "HeroesOfLiberty.Map.PlainSizes", UE::HeroesOfLiberty::Tests::MapSizeMaps::Flags)
bool FHOLMapPlainSizesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapSizeMaps;
	// Unregistered components: places from the roots' relative transforms (see Map.StartSlots).
	const auto Place = [](const AActor& Actor) { return FVector2D(Actor.GetRootComponent()->GetRelativeLocation()); };
	TMap<EHOLMapSize, int32> NeutralPerPlayer;

	for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
	{
		const FString File = FHOLMapSizeRules::GetMapFile(FSoftObjectPath(PlainPath), Info.Size).ToString();
		const UWorld* Map = LoadObject<UWorld>(nullptr, *File);
		if (!TestNotNull(FString::Printf(TEXT("%s (Tools/Editor/create_match_map.py %s)"), *File, Info.Key), Map) || !Map->PersistentLevel)
		{
			continue;
		}
		TMap<int32, FVector2D> Starts;
		TMap<int32, double> Yaws;
		const AHOLMapBounds* Bounds = nullptr;
		const AHOLSourceLayout* Layout = nullptr;
		for (const AActor* Actor : Map->PersistentLevel->Actors)
		{
			if (const AHOLStartSlot* Start = Cast<AHOLStartSlot>(Actor))
			{
				TestFalse(FString::Printf(TEXT("%s: slot %d placed once"), Info.Key, Start->GetSlotIndex()), Starts.Contains(Start->GetSlotIndex()));
				Starts.Add(Start->GetSlotIndex(), Place(*Start));
				Yaws.Add(Start->GetSlotIndex(), Start->GetRootComponent()->GetRelativeRotation().Yaw);
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
		bool bIndices = true;
		for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
		{
			bIndices &= Starts.Contains(Slot);
		}
		if (!TestTrue(FString::Printf(TEXT("%s: slots 0 to %d"), Info.Key, Info.NumPlaces - 1), bIndices))
		{
			continue;
		}

		// Bounds: the size's side, centred on the origin, within the largest map (D344).
		const FBox2D Area = Bounds->GetArea();
		const double Half = 0.5 * Info.SideTiles * UHOLBuildingSettings::GetTileSize();
		TestTrue(FString::Printf(TEXT("%s: bounds of %d tiles a side, centred (%s)"), Info.Key, Info.SideTiles, *Area.ToString()),
			Area.Min.Equals(FVector2D(-Half), 1.0) && Area.Max.Equals(FVector2D(Half), 1.0));
		TestTrue(FString::Printf(TEXT("%s: within the largest map (D344)"), Info.Key), AHOLMapBounds::FitsMaxSize(Area));

		// Slots on a circle, numbered in order around it (DrawPlaces: place k and k + 1 neighbours), facing the centre.
		const double Radius = Starts[0].Size();
		const double Step = 360.0 / Info.NumPlaces;
		const auto Angle = [&Starts](const int32 Slot) { return FMath::RadiansToDegrees(FMath::Atan2(Starts[Slot].Y, Starts[Slot].X)); };
		for (int32 Slot = 0; Slot < Info.NumPlaces; ++Slot)
		{
			TestTrue(FString::Printf(TEXT("%s: slot %d inside the bounds"), Info.Key, Slot), Area.IsInside(Starts[Slot]));
			TestTrue(FString::Printf(TEXT("%s: slot %d as far from the centre as slot 0"), Info.Key, Slot),
				FMath::IsNearlyEqual(Starts[Slot].Size(), Radius, 1.0));
			const int32 Next = (Slot + 1) % Info.NumPlaces;
			TestTrue(FString::Printf(TEXT("%s: slot %d then %d, %.0f degrees on around the circle"), Info.Key, Slot, Next, Step),
				FMath::IsNearlyEqual(FMath::FindDeltaAngleDegrees(Angle(Slot), Angle(Next)), Step, 0.5)
				|| (Info.NumPlaces == 2 && FMath::IsNearlyEqual(FMath::Abs(FMath::FindDeltaAngleDegrees(Angle(Slot), Angle(Next))), 180.0, 0.5)));
			TestTrue(FString::Printf(TEXT("%s: slot %d faces the centre"), Info.Key, Slot),
				FMath::IsNearlyEqual(FMath::Abs(FMath::FindDeltaAngleDegrees(Yaws[Slot], Angle(Slot))), 180.0, 1.0));
		}

		// The content of a base and of a territory is the standard budget (D564, D576): Map.Table checks it for Plain as for every other map.
		const TArray<FHOLPlacedSource>& Sources = Layout->GetSources();

		// Symmetric (TC-Q7, § 8.1): turned by one place, every source lands on a source of its type.
		int32 Unmatched = 0;
		int32 Neutral = 0;
		const double Turn = FMath::DegreesToRadians(Step);
		for (const FHOLPlacedSource& Source : Sources)
		{
			const FVector2D At(Source.Location);
			const FVector2D Turned(At.X * FMath::Cos(Turn) - At.Y * FMath::Sin(Turn), At.X * FMath::Sin(Turn) + At.Y * FMath::Cos(Turn));
			const bool bMatched = Sources.ContainsByPredicate([&Source, &Turned](const FHOLPlacedSource& Other)
				{ return Other.Type == Source.Type && FVector2D::DistSquared(FVector2D(Other.Location), Turned) < 4.0; });
			Unmatched += !bMatched;
			bool bNearSlot = false;
			for (const TPair<int32, FVector2D>& Start : Starts)
			{
				bNearSlot |= FVector2D::Distance(At, Start.Value) < BaseReach;
			}
			Neutral += !bNearSlot;
			TestTrue(FString::Printf(TEXT("%s: source at %s inside the bounds"), Info.Key, *At.ToString()), Area.IsInside(At) || !Area.bIsValid);
		}
		TestEqual(FString::Printf(TEXT("%s: sources symmetric by a turn of one place"), Info.Key), Unmatched, 0);
		TestTrue(FString::Printf(TEXT("%s: neutral sources between the bases (%d)"), Info.Key, Neutral), Neutral > 0);
		NeutralPerPlayer.Add(Info.Size, Neutral / Info.NumPlaces);
	}

	if (NeutralPerPlayer.Contains(EHOLMapSize::Large) && NeutralPerPlayer.Contains(EHOLMapSize::Huge))
	{
		TestTrue(FString::Printf(TEXT("more neutral sources per player on Huge (%d) than on Large (%d)"), NeutralPerPlayer[EHOLMapSize::Huge],
			NeutralPerPlayer[EHOLMapSize::Large]), NeutralPerPlayer[EHOLMapSize::Huge] > NeutralPerPlayer[EHOLMapSize::Large]);
	}
	return true;
}

#endif
