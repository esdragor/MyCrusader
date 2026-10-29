// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAISummary.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionRules.h"
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
	// IA1.5: all its Peasants, idle or not (the opening's builders), the dying, the soldier and slot 1's left out.
	TestEqual(TEXT("slot 0: its six Peasants, ascending"), JoinIds(Slot0.PeasantIds), FString(TEXT("3 4 5 6 7 8")));
	const FHOLAISummary Slot1 = HOLAISummary::Make(1, Units, {}, {}, {});
	TestEqual(TEXT("slot 1: its own idle Peasant"), JoinIds(Slot1.IdlePeasants), FString(TEXT("9")));
	TestEqual(TEXT("slot 1: its own Peasant"), JoinIds(Slot1.PeasantIds), FString(TEXT("9")));
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

/**
 * IA0.4 B4: the summary keeps only the sources its slot may gather (FHOLSource::IsGatherableBy, IA1.2). A world as
 * AI.Summary.World: slot 0 (human) and slot 1 (AI), each a finished centre; farms set up finished by the real path
 * (AHOLBuilding::Setup -> its field added to the registry, its owner's, as Buildings.FarmField). Slot 0's farm stands
 * beside slot 1's centre, nearer than anything; slot 1's own farm and berries farther. Slot 1's food: the berries, then
 * its own field, never slot 0's; slot 0's food: the berries, then its own field, never slot 1's. Red while Make keeps
 * every standing source (the enemy field would be slot 1's nearest food: the gather would refuse it, HOLGatherProcessor).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISummaryGatherableOnlyTest, "HeroesOfLiberty.AI.Summary.GatherableOnly", UE::HeroesOfLiberty::Tests::AISummary::Flags)
bool FHOLAISummaryGatherableOnlyTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AISummary;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLSourceSubsystem* SourceSystem = World->GetSubsystem<UHOLSourceSubsystem>();
	AHOLBuilding* AICentre = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* HumanCentre = World->SpawnActor<AHOLBuilding>(FVector(-10000.0, 0.0, 0.0), FRotator::ZeroRotator);
	// Slot 0's farm right beside slot 1's centre; slot 1's own farm farther south. Axis-aligned offsets from slot 1's
	// centre: 1800 (enemy field), 2600 (berries), 3000 (own field): the order holds whatever the centre's body size.
	AHOLBuilding* EnemyFarm = World->SpawnActor<AHOLBuilding>(FVector(1800.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* OwnFarm = World->SpawnActor<AHOLBuilding>(FVector(0.0, -3000.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("sources"), SourceSystem) && TestNotNull(TEXT("buildings"), AICentre)
		&& TestNotNull(TEXT("buildings"), HumanCentre) && TestNotNull(TEXT("buildings"), EnemyFarm) && TestNotNull(TEXT("buildings"), OwnFarm))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
		}
		State->InitVictory(Config);
		AICentre->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
		HumanCentre->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);

		// As the match starts: the registry takes sources from now on; then the farms, set up finished (their fields).
		SourceSystem->OnWorldBeginPlay(*World);
		const int32 Berries = SourceSystem->AddSource(HOLSourceTypeNames::BerryBush, FVector(0.0, 2600.0, 0.0));
		EnemyFarm->Setup(HOLBuildingTypeNames::Farm, 0, 0);
		OwnFarm->Setup(HOLBuildingTypeNames::Farm, 1, 1);
		const int32 EnemyField = EnemyFarm->GetLinkedSource();
		const int32 OwnField = OwnFarm->GetLinkedSource();
		const TArray<FHOLSource>& All = SourceSystem->GetSources();
		if (TestTrue(TEXT("the berries placed"), Berries != INDEX_NONE)
			&& TestTrue(TEXT("slot 0's farm: its field"), All.IsValidIndex(EnemyField) && All[EnemyField].IsStanding())
			&& TestTrue(TEXT("slot 1's farm: its field"), All.IsValidIndex(OwnField) && All[OwnField].IsStanding()))
		{
			TestEqual(TEXT("slot 0's field is slot 0's"), static_cast<int32>(All[EnemyField].OwnerSlot), 0);
			TestEqual(TEXT("slot 1's field is slot 1's"), static_cast<int32>(All[OwnField].OwnerSlot), 1);
			TestFalse(TEXT("slot 1 may not gather slot 0's field"), All[EnemyField].IsGatherableBy(1));
			TestTrue(TEXT("the berries are everyone's"), All[Berries].IsGatherableBy(0) && All[Berries].IsGatherableBy(1));

			// Slot 1 (AI), read through the library as its brain reads it.
			const FHOLAISummary Slot1 = UHOLAISummaryLibrary::ReadAISummary(World, 1);
			const bool bSlot1HasEnemyField = Slot1.Food.Sources.ContainsByPredicate([EnemyField](const FHOLAISourceNear& Near) { return Near.SourceIndex == EnemyField; });
			TestFalse(TEXT("slot 1's food: slot 0's field left out, though the nearest"), bSlot1HasEnemyField);
			TestEqual(TEXT("slot 1's food: the berries, then its own field"), JoinSources(Slot1.Food),
				FString::Printf(TEXT("%d %d"), Berries, OwnField));
			TestTrue(TEXT("slot 1's nearest food: one it may gather"), Slot1.Food.Sources.Num() > 0
				&& All.IsValidIndex(Slot1.Food.Sources[0].SourceIndex) && All[Slot1.Food.Sources[0].SourceIndex].IsGatherableBy(1));

			// Slot 0 (human): its own field kept, slot 1's left out.
			const FHOLAISummary Slot0 = HOLAISummary::Read(*World, 0);
			const bool bSlot0HasOwnField = Slot0.Food.Sources.ContainsByPredicate([EnemyField](const FHOLAISourceNear& Near) { return Near.SourceIndex == EnemyField; });
			const bool bSlot0HasAIField = Slot0.Food.Sources.ContainsByPredicate([OwnField](const FHOLAISourceNear& Near) { return Near.SourceIndex == OwnField; });
			TestTrue(TEXT("slot 0's food: its own field kept"), bSlot0HasOwnField);
			TestFalse(TEXT("slot 0's food: slot 1's field left out"), bSlot0HasAIField);
			TestEqual(TEXT("slot 0's food: the berries, then its own field"), JoinSources(Slot0.Food),
				FString::Printf(TEXT("%d %d"), Berries, EnemyField));
			AddInfo(FString::Printf(TEXT("%s | %s"), *Slot1.ToString(), *Slot0.ToString()));
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// IA1.3, the rule alone: the slot's own buildings and sites (by type name, then position), its queue, the population
// figures and the Peasants by resource of its PlayerState, the centre.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISummaryExtendedRuleTest, "HeroesOfLiberty.AI.Summary.ExtendedRule", UE::HeroesOfLiberty::Tests::AISummary::Flags)
bool FHOLAISummaryExtendedRuleTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AISummary;
	auto MakeBuilding = [](const int32 Owner, const FName Type, const EHOLConstructionState State, const FVector& Where, TArray<FName> Queue = {})
	{
		FHOLAIBuilding Building;
		Building.OwnerPlayer = Owner;
		Building.TypeName = Type;
		Building.State = State;
		Building.Location = Where;
		Building.Queue = MoveTemp(Queue);
		return Building;
	};
	const TArray<FHOLAIBuilding> Buildings = {
		MakeBuilding(0, HOLBuildingTypeNames::LumberCamp, EHOLConstructionState::Foundation, FVector(4000, 0, 0)),
		MakeBuilding(0, HOLBuildingTypeNames::MainCentre, EHOLConstructionState::Finished, FVector(0, 0, 0), { Peasant, Peasant, Spearman }),
		MakeBuilding(1, HOLBuildingTypeNames::MainCentre, EHOLConstructionState::Finished, FVector(-9000, 0, 0), { Peasant }),
		MakeBuilding(0, HOLBuildingTypeNames::LumberCamp, EHOLConstructionState::UnderConstruction, FVector(-3000, 0, 0)),
	};
	FHOLAIPlayerFigures Figures;
	Figures.PopulationUsed = 12;
	Figures.PopulationCap = 25;
	Figures.Gatherers = FHOLResources(3, 4, 0, 1);

	const FHOLAISummary Slot0 = HOLAISummary::Make(0, {}, {}, {}, {}, HOLAISummary::DefaultMaxSourcesPerResource, Buildings, Figures);
	TestEqual(TEXT("slot 0: its three buildings, not slot 1's"), Slot0.Buildings.Num(), 3);
	if (Slot0.Buildings.Num() == 3)
	{
		TestEqual(TEXT("by type name (LumberCamp before MainCentre), then by X"), Slot0.Buildings[0].Location.X, -3000.0);
		TestEqual(TEXT("the site under construction"), Slot0.Buildings[0].State, EHOLConstructionState::UnderConstruction);
		TestEqual(TEXT("the foundation"), Slot0.Buildings[1].State, EHOLConstructionState::Foundation);
		TestEqual(TEXT("the centre last"), Slot0.Buildings[2].TypeName, HOLBuildingTypeNames::MainCentre);
		TestEqual(TEXT("its queue"), Slot0.Buildings[2].Queue.Num(), 3);
	}
	TestTrue(TEXT("its centre"), Slot0.bHasCentre && Slot0.CentreLocation == FVector::ZeroVector);
	TestEqual(TEXT("queued: its centre's three units"), Slot0.PopulationQueued, 3);
	TestTrue(TEXT("population found"), Slot0.bHasPopulation);
	TestEqual(TEXT("used"), Slot0.PopulationUsed, 12);
	TestEqual(TEXT("cap"), Slot0.PopulationCap, 25);
	TestEqual(TEXT("food Peasants"), Slot0.Food.Peasants, 3);
	TestEqual(TEXT("wood Peasants"), Slot0.Wood.Peasants, 4);
	TestEqual(TEXT("stone Peasants"), Slot0.Stone.Peasants, 0);
	TestEqual(TEXT("gold Peasants"), Slot0.Gold.Peasants, 1);

	const FHOLAISummary Slot1 = HOLAISummary::Make(1, {}, {}, {}, {}, HOLAISummary::DefaultMaxSourcesPerResource, Buildings, {});
	TestEqual(TEXT("slot 1: its centre"), Slot1.Buildings.Num(), 1);
	TestTrue(TEXT("slot 1: centre at -9000"), Slot1.bHasCentre && Slot1.CentreLocation.X == -9000.0);
	TestFalse(TEXT("slot 1: no PlayerState, no population"), Slot1.bHasPopulation);
	TestEqual(TEXT("slot 1: zeros"), Slot1.PopulationUsed + Slot1.PopulationCap + Slot1.Wood.Peasants, 0);

	const FHOLAISummary Slot2 = HOLAISummary::Make(2, {}, {}, {}, {}, HOLAISummary::DefaultMaxSourcesPerResource, Buildings, {});
	TestFalse(TEXT("slot 2: no centre"), Slot2.bHasCentre);
	TestEqual(TEXT("slot 2: no building"), Slot2.Buildings.Num(), 0);
	return true;
}

/**
 * IA1.3 on the real chain: a test world, real buildings (Setup), a real production queue and a real PlayerState (its
 * figures set by the setters the production and gather steps call), read by ReadAISummary.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAISummaryExtendedWorldTest, "HeroesOfLiberty.AI.Summary.ExtendedWorld", UE::HeroesOfLiberty::Tests::AISummary::Flags)
bool FHOLAISummaryExtendedWorldTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AISummary;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Player = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* Centre = World->SpawnActor<AHOLBuilding>(FVector(100.0, 200.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Camp = World->SpawnActor<AHOLBuilding>(FVector(4000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Other = World->SpawnActor<AHOLBuilding>(FVector(-10000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("state"), State) && TestNotNull(TEXT("player"), Player) && TestNotNull(TEXT("centre"), Centre)
		&& TestNotNull(TEXT("camp"), Camp) && TestNotNull(TEXT("other"), Other))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
		}
		State->InitVictory(Config);
		Player->SetMatchSlot(0, 0);
		Centre->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Camp->Setup(HOLBuildingTypeNames::LumberCamp, 0, 0, EHOLConstructionState::Foundation);
		Other->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
		Centre->GetMutableProductionQueue().Add(FHOLProductionItem{ Peasant, FHOLProductionItem::NotStarted });
		Centre->GetMutableProductionQueue().Add(FHOLProductionItem{ Peasant, FHOLProductionItem::NotStarted });
		Player->SetPopulation(7, 20);
		Player->SetGatherers(FHOLResources(2, 3, 0, 0));

		const FHOLAISummary Summary = UHOLAISummaryLibrary::ReadAISummary(World, 0);
		TestEqual(TEXT("two buildings of slot 0"), Summary.Buildings.Num(), 2);
		TestTrue(TEXT("its centre at its place"), Summary.bHasCentre && Summary.CentreLocation.Equals(Centre->GetActorLocation(), 1.0));
		const FHOLAIBuilding* Site = Summary.Buildings.FindByPredicate([](const FHOLAIBuilding& B) { return B.TypeName == HOLBuildingTypeNames::LumberCamp; });
		TestTrue(TEXT("the camp site is a foundation, at 0 progress"), Site && Site->State == EHOLConstructionState::Foundation && Site->Progress == 0.f);
		const FHOLAIBuilding* Main = Summary.Buildings.FindByPredicate([](const FHOLAIBuilding& B) { return B.TypeName == HOLBuildingTypeNames::MainCentre; });
		TestTrue(TEXT("the centre is finished with two queued Peasants, none started"),
			Main && Main->State == EHOLConstructionState::Finished && Main->Progress == 1.f && Main->Queue.Num() == 2 && !Main->bProducing);
		TestEqual(TEXT("queued"), Summary.PopulationQueued, 2);
		TestTrue(TEXT("population"), Summary.bHasPopulation && Summary.PopulationUsed == 7 && Summary.PopulationCap == 20);
		TestEqual(TEXT("Peasants on food"), Summary.Food.Peasants, 2);
		TestEqual(TEXT("Peasants on wood"), Summary.Wood.Peasants, 3);
		AddInfo(Summary.ToString());
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
