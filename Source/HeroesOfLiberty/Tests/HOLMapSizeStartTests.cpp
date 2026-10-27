// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Map/HOLMapSizeRules.h"
#include "Map/HOLStartSlot.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// TC3: the start of each slot at the launch, through the real chain (AHOLGameState::InitVictory, as the game mode calls it,
// then AHOLStartSlot::FindForSlot, which the game mode's centres, Peasants, hero and King and the player's camera use):
// fixed places unchanged (scenarios, quick_check), drawn places applied, a configuration that does not fit refused.
namespace UE::HeroesOfLiberty::Tests::MapSizeStart
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The configuration's occupied slots: Occupied[i] for slot i. */
	FHOLMatchConfig MakeConfig(const TArray<bool>& Occupied, const EHOLMapSize Size, const bool bDraw, const int32 Seed)
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Occupied.IsValidIndex(Index) && Occupied[Index] ? (Index ? EHOLSlotState::AI : EHOLSlotState::Human)
				: EHOLSlotState::Closed;
		}
		Config.MapSize = Size;
		Config.bDrawPlaces = bDraw;
		Config.Seed = Seed;
		return Config;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeStartPlacesTest, "HeroesOfLiberty.MapSize.StartPlaces", UE::HeroesOfLiberty::Tests::MapSizeStart::Flags)
bool FHOLMapSizeStartPlacesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapSizeStart;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	// A map of four places (Medium), each start at its own spot.
	TArray<AHOLStartSlot*> Starts;
	for (int32 Place = 0; Place < 4; ++Place)
	{
		AHOLStartSlot* Start = World->SpawnActor<AHOLStartSlot>(FVector(1000.0 * Place, 0.0, 100.0), FRotator::ZeroRotator);
		if (Start)
		{
			Start->SetSlotIndex(Place);
		}
		Starts.Add(Start);
	}
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	World->SetGameState(State);
	if (TestNotNull(TEXT("game state"), State) && TestFalse(TEXT("four starts"), Starts.Contains(nullptr)))
	{
		// Before the configuration (a client before its replication): no place known, slot i at place i.
		TestFalse(TEXT("no place known before the configuration"), State->HasStartPlaces());

		// The test path: no size, fixed places, slot i at place i.
		State->InitVictory(MakeConfig({ true, false, true }, EHOLMapSize::None, false, 7));
		TestTrue(TEXT("places known after InitVictory"), State->HasStartPlaces());
		TestTrue(TEXT("fixed: slot 0 at start 0"), AHOLStartSlot::FindForSlot(World, 0) == Starts[0]);
		TestTrue(TEXT("fixed: slot 2 at start 2"), AHOLStartSlot::FindForSlot(World, 2) == Starts[2]);
		TestNull(TEXT("fixed: closed slot 1 has no start"), AHOLStartSlot::FindForSlot(World, 1));

		// The menu: drawn from the seed on the four places of Medium, as FHOLMapSizeRules::GetPlaces says.
		bool bSomeMoved = false;
		for (int32 Seed = 1; Seed <= 20; ++Seed)
		{
			const FHOLMatchConfig Config = MakeConfig({ true, false, true }, EHOLMapSize::Medium, true, Seed);
			State->InitVictory(Config);
			const TArray<int32> Places = FHOLMapSizeRules::GetPlaces(State->GetLaunchConfig());
			for (const int32 Slot : { 0, 2 })
			{
				TestTrue(FString::Printf(TEXT("seed %d: slot %d at its drawn place %d"), Seed, Slot, Places[Slot]),
					Places[Slot] != INDEX_NONE && State->GetStartPlace(Slot) == Places[Slot] && AHOLStartSlot::FindForSlot(World, Slot) == Starts[Places[Slot]]);
				bSomeMoved |= Places[Slot] != Slot;
			}
			TestTrue(FString::Printf(TEXT("seed %d: two different starts"), Seed), AHOLStartSlot::FindForSlot(World, 0) != AHOLStartSlot::FindForSlot(World, 2));
			TestNull(FString::Printf(TEXT("seed %d: closed slot 1 has no start"), Seed), AHOLStartSlot::FindForSlot(World, 1));
		}
		TestTrue(TEXT("the draw moves someone off the fixed places"), bSomeMoved);

		// Refused: three players on Small (2 places), or slot 3 fixed on Small: nobody gets a start, nothing breaks.
		State->InitVictory(MakeConfig({ true, true, true }, EHOLMapSize::Small, true, 3));
		TestTrue(TEXT("refused (3 players on Small): places known"), State->HasStartPlaces());
		TestNull(TEXT("refused (3 players on Small): no start for slot 0"), AHOLStartSlot::FindForSlot(World, 0));
		State->InitVictory(MakeConfig({ true, false, false, true }, EHOLMapSize::Small, false, 3));
		TestEqual(TEXT("refused (slot 3 fixed on Small): no place for slot 0"), State->GetStartPlace(0), static_cast<int32>(INDEX_NONE));
		TestNull(TEXT("refused (slot 3 fixed on Small): no start for slot 3"), AHOLStartSlot::FindForSlot(World, 3));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
