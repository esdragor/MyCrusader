// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAISummary.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA0.3 checks (IA-Q2, IA-Q3): the world summary of an AI slot, the rule alone (idle Peasants, sources near the slot's
// deposits, stock), then read from a test world's subsystems.
namespace UE::HeroesOfLiberty::Tests::AISummary
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Peasant(TEXT("Peasant"));
	const FName Spearman(TEXT("Spearman"));

	FHOLUnitView MakeUnit(const uint32 Id, const uint8 Owner, const FName Type, const EHOLUnitActivity Activity)
	{
		FHOLUnitView Unit;
		Unit.UnitId = Id;
		Unit.OwnerPlayer = Owner;
		Unit.TypeName = Type;
		Unit.CarryCapacity = Type == Peasant ? 10 : 0;
		Unit.Activity = Activity;
		return Unit;
	}

	FHOLSource MakeSource(const double X, const double Y, const EHOLResource Resource, const int32 Remaining = 100,
		const uint8 TypeIndex = 0)
	{
		FHOLSource Source;
		Source.TypeIndex = TypeIndex;
		Source.Resource = Resource;
		Source.Location = FVector(X, Y, 0.0);
		Source.Radius = 60.f;
		Source.Remaining = Remaining;
		return Source;
	}

	FHOLAIDeposit MakeDeposit(const double X, const double Y, const double HalfExtent, const uint8 Owner, TArray<EHOLResource> Accepts)
	{
		FHOLAIDeposit Deposit;
		Deposit.Footprint.Centre = FVector2D(X, Y);
		Deposit.Footprint.HalfExtent = FVector2D(HalfExtent, HalfExtent);
		Deposit.OwnerPlayer = Owner;
		Deposit.Accepts = MoveTemp(Accepts);
		return Deposit;
	}

	const TArray<EHOLResource> AllResources = { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold };

	/** "3 0 5": the source indices of a list, in order. */
	FString JoinSources(const FHOLAIResourceSummary& Part)
	{
		return FString::JoinBy(Part.Sources, TEXT(" "), [](const FHOLAISourceNear& Source) { return FString::FromInt(Source.SourceIndex); });
	}

	FString JoinIds(const TArray<int32>& Ids)
	{
		return FString::JoinBy(Ids, TEXT(" "), [](const int32 Id) { return FString::FromInt(Id); });
	}

	/** Units of Type born for Owner around Location (no walk: the test world runs no movement); their identifiers. */
	TArray<uint32> PlaceUnits(UHOLUnitSubsystem& Units, const FName Type, const int32 Owner, const FVector& Location, const int32 Count)
	{
		TArray<uint32> Before;
		Units.GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Type;
		Request.Count = Count;
		Request.OwnerPlayer = static_cast<uint8>(Owner);
		Request.Center = Location;
		Units.RequestUnits(Request);
		Units.ServeBirthRequests();
		TArray<uint32> After;
		Units.GetAllUnitIds(After);
		After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
		After.Sort();
		return After;
	}
}

// Idle Peasants: the slot's own workers standing with nothing to do (D307), by identifier; neither a gatherer on its
// round trip, a builder, a walker, another slot's Peasant, a soldier standing (no CarryCapacity) nor a dying one.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISummaryIdlePeasantsTest, "HeroesOfLiberty.AI.Summary.IdlePeasants", UE::HeroesOfLiberty::Tests::AISummary::Flags)
bool FHOLAISummaryIdlePeasantsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AISummary;
	const TArray<FHOLUnitView> Units = {
		MakeUnit(7, 0, Peasant, EHOLUnitActivity::Idle),
		MakeUnit(3, 0, Peasant, EHOLUnitActivity::Idle),
		MakeUnit(4, 0, Peasant, EHOLUnitActivity::Gathering),
		MakeUnit(5, 0, Peasant, EHOLUnitActivity::GatherTrip),
		MakeUnit(6, 0, Peasant, EHOLUnitActivity::Building),
		MakeUnit(8, 0, Peasant, EHOLUnitActivity::Moving),
		MakeUnit(9, 1, Peasant, EHOLUnitActivity::Idle),
		MakeUnit(10, 0, Spearman, EHOLUnitActivity::Idle),
		MakeUnit(11, 0, Peasant, EHOLUnitActivity::Dying),
	};
	const FHOLAISummary Slot0 = HOLAISummary::Make(0, Units, {}, {}, {});
	TestEqual(TEXT("slot 0's summary"), Slot0.PlayerIndex, 0);
	TestEqual(TEXT("slot 0: its two idle Peasants, ascending"), JoinIds(Slot0.IdlePeasants), FString(TEXT("3 7")));
	const FHOLAISummary Slot1 = HOLAISummary::Make(1, Units, {}, {}, {});
	TestEqual(TEXT("slot 1: its own idle Peasant"), JoinIds(Slot1.IdlePeasants), FString(TEXT("9")));
	const FHOLAISummary Slot2 = HOLAISummary::Make(2, Units, {}, {}, {});
	TestEqual(TEXT("slot 2: none"), Slot2.IdlePeasants.Num(), 0);
	return true;
}

// Sources: per resource, the standing ones, nearest the slot's own deposit taking that resource first; a used-up or unused
// source left out, another slot's deposit ignored, a deposit not taking the resource ignored, a tie by index, the cap.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISummarySourcesTest, "HeroesOfLiberty.AI.Summary.Sources", UE::HeroesOfLiberty::Tests::AISummary::Flags)
bool FHOLAISummarySourcesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AISummary;
	const TArray<FHOLAIDeposit> Deposits = {
		// Slot 0's centre: 1600 cm square, takes all; its lumber camp (wood only); slot 1's centre, west.
		MakeDeposit(0.0, 0.0, 800.0, 0, AllResources),
		MakeDeposit(5000.0, 0.0, 300.0, 0, { EHOLResource::Wood }),
		MakeDeposit(-6500.0, 0.0, 800.0, 1, AllResources),
	};
	TArray<FHOLSource> Sources;
	Sources.Add(MakeSource(1500.0, 0.0, EHOLResource::Wood));                  // 0: 700 from the centre
	Sources.Add(MakeSource(5600.0, 0.0, EHOLResource::Wood));                  // 1: 300 from the camp (4800 from the centre)
	Sources.Add(MakeSource(0.0, -2000.0, EHOLResource::Wood));                 // 2: 1200 from the centre
	Sources.Add(MakeSource(1000.0, 0.0, EHOLResource::Wood, 0));               // 3: used up
	Sources.Add(MakeSource(900.0, 0.0, EHOLResource::Wood, 50, FHOLSource::NoType)); // 4: an index not used
	Sources.Add(MakeSource(-6000.0, 0.0, EHOLResource::Wood));                 // 5: beside slot 1's centre, 5200 from slot 0's
	Sources.Add(MakeSource(5000.0, 600.0, EHOLResource::Food));                // 6: beside the camp, which takes no food: 4200
	Sources.Add(MakeSource(0.0, 1100.0, EHOLResource::Food));                  // 7: 300 from the centre
	Sources.Add(MakeSource(0.0, -1000.0, EHOLResource::Stone));                // 8: 200, a tie with 9
	Sources.Add(MakeSource(0.0, 1000.0, EHOLResource::Stone));                 // 9: 200
	// 10 to 19: gold 1000 cm to 100 cm from the centre, the farthest first; 8 kept, the nearest.
	for (int32 Index = 0; Index < 10; ++Index)
	{
		Sources.Add(MakeSource(0.0, 1800.0 - 100.0 * Index, EHOLResource::Gold));
	}

	const FHOLAISummary Summary = HOLAISummary::Make(0, {}, Sources, Deposits, {});
	TestEqual(TEXT("wood: camp's tree, then by distance; used-up, unused left out; slot 1's deposit ignored"), JoinSources(Summary.Wood), FString(TEXT("1 0 2 5")));
	if (Summary.Wood.Sources.Num() == 4)
	{
		TestEqual(TEXT("wood 1: 300 cm from the camp"), Summary.Wood.Sources[0].DepositDistance, 300.f, 1.f);
		TestEqual(TEXT("wood 0: 700 cm from the centre"), Summary.Wood.Sources[1].DepositDistance, 700.f, 1.f);
		TestEqual(TEXT("wood 2: 1200 cm"), Summary.Wood.Sources[2].DepositDistance, 1200.f, 1.f);
		TestEqual(TEXT("wood 5: 5200 cm from slot 0's centre, not slot 1's"), Summary.Wood.Sources[3].DepositDistance, 5200.f, 1.f);
		TestEqual(TEXT("its location"), Summary.Wood.Sources[0].Location, FVector(5600.0, 0.0, 0.0));
		TestEqual(TEXT("what it holds"), Summary.Wood.Sources[0].Remaining, 100);
	}
	TestEqual(TEXT("food: the camp takes none"), JoinSources(Summary.Food), FString(TEXT("7 6")));
	if (Summary.Food.Sources.Num() == 2)
	{
		TestEqual(TEXT("food 6: 4200 cm from the centre"), Summary.Food.Sources[1].DepositDistance, 4200.f, 1.f);
	}
	TestEqual(TEXT("stone: a tie, the lower index first"), JoinSources(Summary.Stone), FString(TEXT("8 9")));
	TestEqual(TEXT("gold: the 8 nearest"), JoinSources(Summary.Gold), FString(TEXT("19 18 17 16 15 14 13 12")));
	if (Summary.Gold.Sources.Num() == 8)
	{
		TestEqual(TEXT("gold 19: 100 cm"), Summary.Gold.Sources[0].DepositDistance, 100.f, 1.f);
	}

	// Slot 1: its own centre only.
	const FHOLAISummary Slot1 = HOLAISummary::Make(1, {}, Sources, Deposits, {});
	TestTrue(TEXT("slot 1: its tree first"), Slot1.Wood.Sources.Num() > 0 && Slot1.Wood.Sources[0].SourceIndex == 5);
	// A smaller cap.
	const FHOLAISummary Capped = HOLAISummary::Make(0, {}, Sources, Deposits, {}, 2);
	TestEqual(TEXT("a cap of 2"), JoinSources(Capped.Wood), FString(TEXT("1 0")));
	return true;
}

// A slot without deposit (none, a foundation is not given to the rule, another's only): no source near it, its idle
// Peasants and stock still read. A resource without source: an empty list.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISummaryNoDepositTest, "HeroesOfLiberty.AI.Summary.NoDeposit", UE::HeroesOfLiberty::Tests::AISummary::Flags)
bool FHOLAISummaryNoDepositTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AISummary;
	const TArray<FHOLSource> Sources = { MakeSource(100.0, 0.0, EHOLResource::Wood), MakeSource(0.0, 100.0, EHOLResource::Gold) };
	const TArray<FHOLUnitView> Units = { MakeUnit(2, 3, Peasant, EHOLUnitActivity::Idle) };
	const TArray<FHOLAIDeposit> OthersOnly = { MakeDeposit(0.0, 0.0, 800.0, 0, AllResources) };
	const FHOLAISummary Summary = HOLAISummary::Make(3, Units, Sources, OthersOnly, FHOLResources(10, 20, 30, 40));
	for (const EHOLResource Resource : AllResources)
	{
		TestEqual(FString::Printf(TEXT("no deposit of its own: no source (resource %d)"), static_cast<int32>(Resource)), Summary.Get(Resource).Sources.Num(), 0);
	}
	TestEqual(TEXT("its idle Peasant still"), JoinIds(Summary.IdlePeasants), FString(TEXT("2")));
	TestTrue(TEXT("its stock still"), Summary.bHasStock && Summary.GetStock() == FHOLResources(10, 20, 30, 40));

	// Slot 0 has a deposit, but no stone stands: that list is empty, the others not.
	const FHOLAISummary Slot0 = HOLAISummary::Make(0, Units, Sources, OthersOnly, {});
	TestEqual(TEXT("no stone source: empty"), Slot0.Stone.Sources.Num(), 0);
	TestEqual(TEXT("no food source: empty"), Slot0.Food.Sources.Num(), 0);
	TestEqual(TEXT("its wood"), JoinSources(Slot0.Wood), FString(TEXT("0")));
	TestEqual(TEXT("its gold"), JoinSources(Slot0.Gold), FString(TEXT("1")));
	return true;
}

// Stock: the slot's stock given, each resource's; none (an AI slot without PlayerState until IA0.1): zeros, bHasStock false.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISummaryStockTest, "HeroesOfLiberty.AI.Summary.Stock", UE::HeroesOfLiberty::Tests::AISummary::Flags)
bool FHOLAISummaryStockTest::RunTest(const FString& Parameters)
{
	const FHOLAISummary Summary = HOLAISummary::Make(1, {}, {}, {}, FHOLResources(100, 200, 50, 25));
	TestTrue(TEXT("a stock"), Summary.bHasStock);
	TestEqual(TEXT("food"), Summary.Food.Stock, 100);
	TestEqual(TEXT("wood"), Summary.Wood.Stock, 200);
	TestEqual(TEXT("stone"), Summary.Stone.Stock, 50);
	TestEqual(TEXT("gold"), Summary.Gold.Stock, 25);
	TestTrue(TEXT("the four together"), Summary.GetStock() == FHOLResources(100, 200, 50, 25));

	const FHOLAISummary None = HOLAISummary::Make(1, {}, {}, {}, {});
	TestFalse(TEXT("no stock"), None.bHasStock);
	TestTrue(TEXT("zeros"), None.GetStock() == FHOLResources());
	TestEqual(TEXT("its slot all the same"), None.PlayerIndex, 1);
	return true;
}

// The real chain: a test world with real units, buildings and sources, read through the subsystems
// (UHOLAISummaryLibrary::ReadAISummary, HOLAISummary::Read). Slot 0: a PlayerState with its stock, a finished centre, a
// lumber camp still a foundation (no deposit, D312), three Peasants and a Spearman. Slot 1: a centre, a Peasant, no
// PlayerState, then its AI PlayerState born as at the match start (IA0.1 E3).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISummaryWorldTest, "HeroesOfLiberty.AI.Summary.World", UE::HeroesOfLiberty::Tests::AISummary::Flags)
bool FHOLAISummaryWorldTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AISummary;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSourceSubsystem* SourceSystem = World->GetSubsystem<UHOLSourceSubsystem>();
	AHOLPlayerState* Human = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* Centre = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Camp = World->SpawnActor<AHOLBuilding>(FVector(4000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* OtherCentre = World->SpawnActor<AHOLBuilding>(FVector(-10000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("sources"), SourceSystem)
		&& TestNotNull(TEXT("player state"), Human) && TestNotNull(TEXT("buildings"), Centre) && TestNotNull(TEXT("buildings"), Camp)
		&& TestNotNull(TEXT("buildings"), OtherCentre))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
		}
		State->InitVictory(Config);
		Human->SetMatchSlot(0, 0);
		Human->SetResources(FHOLResources(100, 200, 50, 25));
		Centre->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Camp->Setup(HOLBuildingTypeNames::LumberCamp, 0, 0, EHOLConstructionState::Foundation);
		OtherCentre->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);

		SourceSystem->OnWorldBeginPlay(*World);
		const int32 NearTree = SourceSystem->AddSource(HOLSourceTypeNames::Tree, FVector(2000.0, 0.0, 0.0));
		const int32 CampTree = SourceSystem->AddSource(HOLSourceTypeNames::Tree, FVector(4600.0, 0.0, 0.0));
		const int32 UsedTree = SourceSystem->AddSource(HOLSourceTypeNames::Tree, FVector(1600.0, 0.0, 0.0));
		const int32 EnemyTree = SourceSystem->AddSource(HOLSourceTypeNames::Tree, FVector(-9000.0, 0.0, 0.0));
		const int32 Berries = SourceSystem->AddSource(HOLSourceTypeNames::BerryBush, FVector(0.0, 1800.0, 0.0));
		const int32 GoldVein = SourceSystem->AddSource(HOLSourceTypeNames::GoldVein, FVector(0.0, -2500.0, 0.0));
		TestTrue(TEXT("six sources placed"), NearTree != INDEX_NONE && CampTree != INDEX_NONE && UsedTree != INDEX_NONE
			&& EnemyTree != INDEX_NONE && Berries != INDEX_NONE && GoldVein != INDEX_NONE);
		SourceSystem->Take(UsedTree, 1000000);
		TestFalse(TEXT("a tree used up"), SourceSystem->GetSources().IsValidIndex(UsedTree) && SourceSystem->GetSources()[UsedTree].IsStanding());

		const TArray<uint32> Peasants = PlaceUnits(*Units, Peasant, 0, FVector(1000.0, 1000.0, 0.0), 3);
		const TArray<uint32> Soldiers = PlaceUnits(*Units, Spearman, 0, FVector(-1000.0, 1000.0, 0.0), 1);
		const TArray<uint32> OtherPeasants = PlaceUnits(*Units, Peasant, 1, FVector(-9000.0, 1000.0, 0.0), 1);
		TestEqual(TEXT("three Peasants of slot 0 born"), Peasants.Num(), 3);
		TestEqual(TEXT("a Spearman of slot 0 born"), Soldiers.Num(), 1);
		TestEqual(TEXT("a Peasant of slot 1 born"), OtherPeasants.Num(), 1);

		const FHOLAISummary Slot0 = UHOLAISummaryLibrary::ReadAISummary(World, 0);
		const FString PeasantIds = FString::JoinBy(Peasants, TEXT(" "), [](const uint32 Id) { return FString::FromInt(static_cast<int32>(Id)); });
		TestEqual(TEXT("slot 0: its three Peasants idle, not its Spearman nor slot 1's Peasant"), JoinIds(Slot0.IdlePeasants), PeasantIds);
		TestTrue(TEXT("slot 0: its PlayerState's stock"), Slot0.bHasStock && Slot0.GetStock() == FHOLResources(100, 200, 50, 25));
		TestEqual(TEXT("slot 0's wood: nearest the centre first, the used-up tree left out, slot 1's far tree last"),
			JoinSources(Slot0.Wood), FString::Printf(TEXT("%d %d %d"), NearTree, CampTree, EnemyTree));
		if (Slot0.Wood.Sources.Num() == 3)
		{
			// The camp is a foundation: its tree is measured from the centre (its body edge, less than 800 cm from its middle).
			TestTrue(TEXT("the camp's tree measured from the centre, not the foundation"), Slot0.Wood.Sources[1].DepositDistance > 3700.f
				&& Slot0.Wood.Sources[1].DepositDistance < 4600.f);
			TestTrue(TEXT("the near tree: between 1100 and 2000 cm"), Slot0.Wood.Sources[0].DepositDistance > 1100.f
				&& Slot0.Wood.Sources[0].DepositDistance < 2000.f);
			TestEqual(TEXT("its remaining: a tree's amount"), Slot0.Wood.Sources[0].Remaining, SourceSystem->GetSources()[NearTree].Remaining);
		}
		TestEqual(TEXT("slot 0's food: the berries"), JoinSources(Slot0.Food), FString::FromInt(Berries));
		TestEqual(TEXT("slot 0's gold: the vein"), JoinSources(Slot0.Gold), FString::FromInt(GoldVein));
		TestEqual(TEXT("slot 0's stone: none placed"), Slot0.Stone.Sources.Num(), 0);

		const FHOLAISummary Slot1 = HOLAISummary::Read(*World, 1);
		TestEqual(TEXT("slot 1: its Peasant"), JoinIds(Slot1.IdlePeasants), FString::FromInt(static_cast<int32>(OtherPeasants.IsEmpty() ? 0 : OtherPeasants[0])));
		TestFalse(TEXT("slot 1: no PlayerState yet, no stock"), Slot1.bHasStock);
		TestTrue(TEXT("slot 1: its tree first"), Slot1.Wood.Sources.Num() > 0 && Slot1.Wood.Sources[0].SourceIndex == EnemyTree);
		AddInfo(FString::Printf(TEXT("%s | %s"), *Slot0.ToString(), *Slot1.ToString()));

		// IA0.1 E3: the AI slot's PlayerState, born as at the match start: the summary reads its stock.
		const FHOLResources Starting = GetDefault<UHOLMatchSettings>()->StartingResources;
		TestEqual(TEXT("slot 1: one AI player state born"), AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, Starting), 1);
		const FHOLAISummary Slot1Born = HOLAISummary::Read(*World, 1);
		TestTrue(TEXT("slot 1: its PlayerState's stock, the starting one"), Slot1Born.bHasStock && Slot1Born.GetStock() == Starting);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
