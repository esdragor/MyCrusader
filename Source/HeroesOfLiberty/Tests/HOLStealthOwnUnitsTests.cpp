// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "Misc/AutomationTest.h"
#include "Presentation/HOLStealthOwnUnits.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLUnitSubsystem.h"

// ARWEN-1122c: the stealth binding reads the REAL simulation (UHOLUnitSubsystem, Mass fragments), as HOLVisionMatchTests do: units born for two players,
// moved to known places; the binding must list the asked player's units only, with the identifier the processors read (State.UnitId) and the position.
namespace UE::HeroesOfLiberty::Tests::StealthOwnUnits
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	struct FOwnUnitsWorld
	{
		UWorld* World = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UMassEntitySubsystem* Entities = nullptr;

		FOwnUnitsWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Entities = World->GetSubsystem<UMassEntitySubsystem>();
		}

		~FOwnUnitsWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Units && Entities; }

		/** A Peasant born for Owner, put at Location; its identifier (0 if none). */
		uint32 Place(const int32 Owner, const FVector& Location) const
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(TEXT("Peasant"));
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Owner);
			Request.Center = Location;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
			if (After.Num() != 1)
			{
				return 0;
			}
			if (FTransformFragment* Transform = Entities->GetMutableEntityManager().GetFragmentDataPtr<FTransformFragment>(Units->FindEntity(After[0])))
			{
				Transform->GetMutableTransform().SetLocation(Location);
			}
			return After[0];
		}

		/** The identifier the processors read: FHOLUnitStateFragment::UnitId of the unit's entity (0 if none). */
		uint32 StateId(const uint32 UnitId) const
		{
			const FHOLUnitStateFragment* State = Entities->GetEntityManager().GetFragmentDataPtr<FHOLUnitStateFragment>(Units->FindEntity(UnitId));
			return State ? State->UnitId : 0;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStealthOwnUnitsTest, "HeroesOfLiberty.Stealth.OwnUnits.OnlyThePlayersUnits", UE::HeroesOfLiberty::Tests::StealthOwnUnits::Flags)
bool FHOLStealthOwnUnitsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::StealthOwnUnits;
	FOwnUnitsWorld Match;
	if (!TestTrue(TEXT("the simulation's subsystems"), Match.IsReady()))
	{
		return false;
	}
	const uint32 A = Match.Place(0, FVector(1000., 2000., 0.));
	const uint32 B = Match.Place(0, FVector(-3000., 4500., 0.));
	const uint32 Enemy = Match.Place(2, FVector(7000., -7000., 0.));
	if (!TestTrue(TEXT("three units born"), A != 0 && B != 0 && Enemy != 0))
	{
		return false;
	}

	TArray<int32> Ids;
	TArray<FVector2D> Positions;
	UHOLStealthOwnUnitsLibrary::GetPlayerUnits(Match.World, 0, Ids, Positions);
	TestEqual(TEXT("player 0: its two units, not the enemy's"), Ids.Num(), 2);
	TestEqual(TEXT("one position per id"), Positions.Num(), Ids.Num());
	const int32 IndexA = Ids.Find(static_cast<int32>(Match.StateId(A)));
	const int32 IndexB = Ids.Find(static_cast<int32>(Match.StateId(B)));
	TestTrue(TEXT("the id is the State.UnitId of unit A"), IndexA != INDEX_NONE);
	TestTrue(TEXT("the id is the State.UnitId of unit B"), IndexB != INDEX_NONE);
	TestFalse(TEXT("the enemy's id is not listed"), Ids.Contains(static_cast<int32>(Match.StateId(Enemy))));
	if (Positions.IsValidIndex(IndexA))
	{
		TestEqual(TEXT("A's X"), Positions[IndexA].X, 1000., 1.);
		TestEqual(TEXT("A's Y"), Positions[IndexA].Y, 2000., 1.);
	}
	if (Positions.IsValidIndex(IndexB))
	{
		TestEqual(TEXT("B's X"), Positions[IndexB].X, -3000., 1.);
		TestEqual(TEXT("B's Y"), Positions[IndexB].Y, 4500., 1.);
	}

	UHOLStealthOwnUnitsLibrary::GetPlayerUnits(Match.World, 2, Ids, Positions);
	TestEqual(TEXT("player 2: its one unit"), Ids.Num(), 1);
	UHOLStealthOwnUnitsLibrary::GetPlayerUnits(Match.World, 5, Ids, Positions);
	TestEqual(TEXT("a player with no unit: nothing"), Ids.Num(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
