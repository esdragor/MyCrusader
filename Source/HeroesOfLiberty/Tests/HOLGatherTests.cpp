// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLPlayerOrders.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLSiteView.h"
#include "Simulation/HOLSiteSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceLayout.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLGatherRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// J5 checks (D299): the gathering rules (D296) and the provisional values of the tables (D296, D297).
namespace UE::HeroesOfLiberty::Tests::Gather
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The fixed step of D217 (20 Hz). */
	constexpr float TestStepSeconds = 0.05f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherTablesTest, "HeroesOfLiberty.Gather.Tables", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherTablesTest::RunTest(const FString& Parameters)
{
	struct FExpectedSource
	{
		FName Name;
		EHOLResource Resource;
		int32 Amount;
	};
	const FExpectedSource Expected[] = {
		{ HOLSourceTypeNames::Tree, EHOLResource::Wood, 100 },
		{ HOLSourceTypeNames::BerryBush, EHOLResource::Food, 300 },
		{ HOLSourceTypeNames::StoneVein, EHOLResource::Stone, 3000 },
		{ HOLSourceTypeNames::GoldVein, EHOLResource::Gold, 3000 },
		// M-Q9: the veins in other sizes (small 500, medium 1 000, big 5 000), the same behaviour as the standard ones.
		{ HOLSourceTypeNames::StoneVeinSmall, EHOLResource::Stone, 2000 },
		{ HOLSourceTypeNames::StoneVeinMedium, EHOLResource::Stone, 2500 },
		{ HOLSourceTypeNames::StoneVeinBig, EHOLResource::Stone, 4000 },
		{ HOLSourceTypeNames::GoldVeinSmall, EHOLResource::Gold, 2000 },
		{ HOLSourceTypeNames::GoldVeinMedium, EHOLResource::Gold, 2500 },
		{ HOLSourceTypeNames::GoldVeinBig, EHOLResource::Gold, 4000 },
	};
	for (const FExpectedSource& Source : Expected)
	{
		const FString Name = Source.Name.ToString();
		const FHOLSourceTypeRow* Row = UHOLSourceSettings::FindType(Source.Name);
		if (TestNotNull(*FString::Printf(TEXT("%s in the source type table"), *Name), Row))
		{
			TestEqual(*FString::Printf(TEXT("%s: resource"), *Name), int32(Row->Resource), int32(Source.Resource));
			TestEqual(*FString::Printf(TEXT("%s: amount (D297)"), *Name), Row->Amount, Source.Amount);
			TestTrue(*FString::Printf(TEXT("%s: a body"), *Name), Row->Radius > 0.f);
			// The map's sources keep their behaviour (IA1.2): places all around, own rate, used up, placed by the map.
			TestEqual(*FString::Printf(TEXT("%s: places all around"), *Name), Row->Places, 0);
			TestEqual(*FString::Printf(TEXT("%s: rate scale 1"), *Name), Row->RateScale, 1.f);
			TestFalse(*FString::Printf(TEXT("%s: used up"), *Name), Row->Endless);
			TestFalse(*FString::Printf(TEXT("%s: not linked"), *Name), Row->Linked);
		}
	}

	// The farm's field (IA1.2): food, one place, endless, linked to its building, 0.55/s for the Peasant (his 0.7/s
	// through the row's GatherRate, the one rate function of the gathering, F3).
	const FHOLSourceTypeRow* Field = UHOLSourceSettings::FindType(HOLSourceTypeNames::FarmField);
	if (TestNotNull(TEXT("FarmField in the source type table"), Field))
	{
		TestEqual(TEXT("FarmField: resource"), int32(Field->Resource), int32(EHOLResource::Food));
		TestTrue(TEXT("FarmField: a body"), Field->Radius > 0.f);
		TestEqual(TEXT("FarmField: one place"), Field->Places, 1);
		TestTrue(TEXT("FarmField: endless"), Field->Endless);
		TestTrue(TEXT("FarmField: linked"), Field->Linked);
		const UDataTable* Units = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		const FHOLUnitTypeRow* Farmer = Units ? Units->FindRow<FHOLUnitTypeRow>(TEXT("Peasant"), TEXT("Gather test")) : nullptr;
		if (TestNotNull(TEXT("Peasant for the farm rate"), Farmer))
		{
			const FHOLUnitTypeConstants Constants = Farmer->ToConstants(TEXT("Peasant"));
			TestEqual(TEXT("FarmField: the Peasant gathers 0.55/s, exactly"),
				Field->GatherRate(Constants.GatherRates[EHOLResource::Food]), 0.55f, 0.f);
		}
	}

	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (TestNotNull(TEXT("unit type table"), UnitTypes))
	{
		const FHOLUnitTypeRow* Peasant = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Peasant"), TEXT("Gather test"));
		if (TestNotNull(TEXT("Peasant in the unit type table"), Peasant))
		{
			TestEqual(TEXT("Peasant carries 10 (D296)"), Peasant->CarryCapacity, 10);
			TestTrue(TEXT("Peasant gathers the 4 resources at 0.7/s (D296)"), Peasant->GatherRates == FHOLResourceRates(0.7f, 0.7f, 0.7f, 0.7f));
			const FHOLUnitTypeConstants Constants = Peasant->ToConstants(TEXT("Peasant"));
			TestEqual(TEXT("carry reaches the shared fragment"), Constants.CarryCapacity, 10);
			TestEqual(TEXT("wood rate reaches the shared fragment"), Constants.GatherRates[EHOLResource::Wood], 0.7f);
		}
		const FHOLUnitTypeRow* Infantry = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("LightInfantry"), TEXT("Gather test"));
		if (TestNotNull(TEXT("LightInfantry in the unit type table"), Infantry))
		{
			TestEqual(TEXT("a soldier carries nothing"), Infantry->CarryCapacity, 0);
			TestTrue(TEXT("a soldier gathers nothing"), Infantry->GatherRates == FHOLResourceRates());
		}
	}

	const FHOLBuildingTypeRow* MainCentre = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	if (TestNotNull(TEXT("main centre in the building type table"), MainCentre))
	{
		TestTrue(TEXT("the main centre takes the four resources (D295)"), MainCentre->AcceptsResource(EHOLResource::Food)
			&& MainCentre->AcceptsResource(EHOLResource::Wood) && MainCentre->AcceptsResource(EHOLResource::Stone)
			&& MainCentre->AcceptsResource(EHOLResource::Gold));
	}
	return true;
}

// M-Q9: each size of vein, placed by a map on the real source registry, holds its amount, is gathered load by load (the
// Peasant's 10) down to nothing and then leaves the map: the whole amount comes out, no more.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherVeinSizesTest, "HeroesOfLiberty.Gather.VeinSizes", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherVeinSizesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Gather;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLSourceSubsystem* Registry = World->GetSubsystem<UHOLSourceSubsystem>();
	AHOLSourceLayout* Layout = World->SpawnActor<AHOLSourceLayout>();
	const FArrayProperty* LayoutSources = FindFProperty<FArrayProperty>(AHOLSourceLayout::StaticClass(), TEXT("Sources"));
	if (TestNotNull(TEXT("source registry"), Registry) && TestNotNull(TEXT("map layout"), Layout) && TestNotNull(TEXT("the layout's source list"), LayoutSources))
	{
		struct FSize
		{
			FName Type;
			EHOLResource Resource;
			int32 Amount;
		};
		const FSize Sizes[] = {
			{ HOLSourceTypeNames::StoneVeinSmall, EHOLResource::Stone, 2000 }, { HOLSourceTypeNames::StoneVeinMedium, EHOLResource::Stone, 2500 },
			{ HOLSourceTypeNames::StoneVein, EHOLResource::Stone, 3000 }, { HOLSourceTypeNames::StoneVeinBig, EHOLResource::Stone, 4000 },
			{ HOLSourceTypeNames::GoldVeinSmall, EHOLResource::Gold, 2000 }, { HOLSourceTypeNames::GoldVeinMedium, EHOLResource::Gold, 2500 },
			{ HOLSourceTypeNames::GoldVein, EHOLResource::Gold, 3000 }, { HOLSourceTypeNames::GoldVeinBig, EHOLResource::Gold, 4000 },
		};
		TArray<FHOLPlacedSource>& Placed = *LayoutSources->ContainerPtrToValuePtr<TArray<FHOLPlacedSource>>(Layout);
		for (int32 Number = 0; Number < UE_ARRAY_COUNT(Sizes); ++Number)
		{
			Placed.Add(FHOLPlacedSource{ Sizes[Number].Type, FVector(3000.0 * Number, 0.0, 0.0) });
		}
		Registry->OnWorldBeginPlay(*World);
		TestEqual(TEXT("every vein placed"), Registry->GetSources().Num(), int32(UE_ARRAY_COUNT(Sizes)));
		for (int32 Number = 0; Number < Registry->GetSources().Num() && Number < UE_ARRAY_COUNT(Sizes); ++Number)
		{
			const FHOLSource& Vein = Registry->GetSources()[Number];
			const FString Name = Sizes[Number].Type.ToString();
			TestEqual(*FString::Printf(TEXT("%s: resource"), *Name), int32(Vein.Resource), int32(Sizes[Number].Resource));
			TestEqual(*FString::Printf(TEXT("%s: amount"), *Name), Vein.Remaining, Sizes[Number].Amount);
			TestTrue(*FString::Printf(TEXT("%s stands"), *Name), Vein.IsStanding());
			TestTrue(*FString::Printf(TEXT("%s: a bigger pile has a bigger body, up to the standard's"), *Name), Vein.Radius > 0.f);

			// Gathered by loads of 10 (a Peasant), then by the processor's rule: the whole amount comes out, and the vein is used up.
			int32 Taken = 0;
			while (Registry->GetSources()[Number].IsStanding() && Taken <= Sizes[Number].Amount + 20)
			{
				Taken += Registry->Take(Number, 10);
			}
			TestEqual(*FString::Printf(TEXT("%s: all of it taken, no more"), *Name), Taken, Sizes[Number].Amount);
			TestFalse(*FString::Printf(TEXT("%s: used up"), *Name), Registry->GetSources()[Number].IsStanding());

			FHOLGatherLoad Load;
			HOLGather::StartGathering(Load, Sizes[Number].Resource);
			int32 Remaining = Sizes[Number].Amount;
			int32 Gathered = 0;
			for (int32 Step = 0; Step < 2000000 && Remaining > 0; ++Step)
			{
				Gathered += HOLGather::Gather(Load, Sizes[Number].Resource, 0.7f, TestStepSeconds, 10, Remaining);
				if (HOLGather::IsFull(Load, 10))
				{
					Load = FHOLGatherLoad();
					HOLGather::StartGathering(Load, Sizes[Number].Resource);
				}
			}
			TestEqual(*FString::Printf(TEXT("%s: a Peasant gathers it to its end"), *Name), Gathered, Sizes[Number].Amount);
			TestEqual(*FString::Printf(TEXT("%s: nothing left"), *Name), Remaining, 0);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// IA1.2 F2: a farm field on the real source registry, in a test world. Added by code with its owner, it has no shape and
// no hole in the navmesh (its farm stands for it), the map never places one, gathering never counts it down, and
// RemoveSource takes it off the map (its index kept, skipped like a used-up source). The map's tree is untouched.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherLinkedSourceTest, "HeroesOfLiberty.Gather.LinkedSource", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherLinkedSourceTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLSourceSubsystem* Registry = World->GetSubsystem<UHOLSourceSubsystem>();
	AHOLSourceLayout* Layout = World->SpawnActor<AHOLSourceLayout>();
	const FArrayProperty* LayoutSources = FindFProperty<FArrayProperty>(AHOLSourceLayout::StaticClass(), TEXT("Sources"));
	if (TestNotNull(TEXT("source registry"), Registry) && TestNotNull(TEXT("map layout"), Layout)
		&& TestNotNull(TEXT("the layout's source list"), LayoutSources))
	{
		// The map asks for a tree and a farm field: only the tree is placed.
		TArray<FHOLPlacedSource>& Placed = *LayoutSources->ContainerPtrToValuePtr<TArray<FHOLPlacedSource>>(Layout);
		Placed.Add(FHOLPlacedSource{ HOLSourceTypeNames::Tree, FVector(0.0, 0.0, 0.0) });
		Placed.Add(FHOLPlacedSource{ HOLSourceTypeNames::FarmField, FVector(600.0, 0.0, 0.0) });
		Registry->OnWorldBeginPlay(*World);
		TestEqual(TEXT("the map placed its tree, not its farm field"), Registry->GetSources().Num(), 1);
		const int32 Tree = 0;
		const int32 TreeAmount = Registry->GetSources().IsValidIndex(Tree) ? Registry->GetSources()[Tree].Remaining : 0;
		TestTrue(TEXT("the map's tree belongs to no one"), Registry->GetSources().IsValidIndex(Tree)
			&& Registry->GetSources()[Tree].OwnerSlot == FHOLSource::NoOwner);

		const int32 Farm = Registry->AddSource(HOLSourceTypeNames::FarmField, FVector(2000.0, 0.0, 0.0), INDEX_NONE, 2);
		if (TestTrue(TEXT("a farm field added by code"), Registry->GetSources().IsValidIndex(Farm)))
		{
			const FHOLSource& Field = Registry->GetSources()[Farm];
			TestEqual(TEXT("the farm field's owner"), static_cast<int32>(Field.OwnerSlot), 2);
			TestTrue(TEXT("the farm field stands"), Field.IsStanding());
			TestEqual(TEXT("two standing"), Registry->GetNumStanding(), 2);
			const FString Report = Registry->MakeReport();
			AddInfo(Report);
			TestTrue(TEXT("the farm field has no shape and no hole: shown 1, navmesh 1 (the tree alone)"),
				Report.EndsWith(TEXT("; shown 1, navmesh 1")));
			TestTrue(TEXT("the report says it is endless"), Report.Contains(TEXT("FarmField 1: endless")));
			TestEqual(TEXT("the food nearest the field: the field"), Registry->FindNearestStanding(FVector(2000.0, 0.0, 0.0), EHOLResource::Food), Farm);

			// Gathering takes from it many times: it gives all that is asked and never goes down.
			const int32 Before = Field.Remaining;
			int32 Taken = 0;
			for (int32 Load = 0; Load < 100; ++Load)
			{
				Taken += Registry->Take(Farm, 10);
			}
			TestEqual(TEXT("100 loads of 10 taken from the endless field"), Taken, 1000);
			TestEqual(TEXT("its remaining unchanged"), Registry->GetSources()[Farm].Remaining, Before);
			TestTrue(TEXT("still standing"), Registry->GetSources()[Farm].IsStanding());
			TestEqual(TEXT("a huge take given whole, not counted down"), Registry->Take(Farm, MAX_int32), MAX_int32);
			TestTrue(TEXT("still standing after it"), Registry->GetSources()[Farm].IsStanding());

			// The farm dies: its field leaves the map.
			TestTrue(TEXT("the field removed"), Registry->RemoveSource(Farm));
			TestFalse(TEXT("no longer standing"), Registry->GetSources()[Farm].IsStanding());
			TestEqual(TEXT("its index kept"), Registry->GetSources().Num(), 2);
			TestEqual(TEXT("one standing: the tree"), Registry->GetNumStanding(), 1);
			TestEqual(TEXT("no food left near it"), Registry->FindNearestStanding(FVector(2000.0, 0.0, 0.0), EHOLResource::Food), INDEX_NONE);
			TestEqual(TEXT("the nearest source: the tree"), Registry->FindNearestStanding(FVector(2000.0, 0.0, 0.0)), Tree);
			TestEqual(TEXT("nothing taken from a removed field"), Registry->Take(Farm, 10), 0);
			TestFalse(TEXT("removed twice: no"), Registry->RemoveSource(Farm));
			const FString After = Registry->MakeReport();
			AddInfo(After);
			TestTrue(TEXT("the report: one standing of two, the tree still shown"), After.Contains(TEXT(": 1 standing of 2 ("))
				&& After.EndsWith(TEXT("; shown 1, navmesh 1")) && !After.Contains(TEXT("FarmField")));
			TestTrue(TEXT("the map's tree untouched"), Registry->GetSources()[Tree].IsStanding()
				&& Registry->GetSources()[Tree].Remaining == TreeAmount && TreeAmount > 0);
		}
		TestFalse(TEXT("nothing removed at a bad index"), Registry->RemoveSource(99));
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// IA1.2 F3: farm fields on the real chain, as a match's server: a test world, the order queue (HOLPlayerOrders, what the
// controller's RPCs and the AI call), the order processor then the gathering at the fixed step (no movement runs: the
// Peasants are set down touching their sources).
namespace UE::HeroesOfLiberty::Tests::GatherFarm
{
	const FName Peasant(TEXT("Peasant"));

	/** TestSteps the gatherers work (10 s at 20 Hz): a Peasant's load of 10 is not full yet, at a field nor at the berries. */
	constexpr int32 TestSteps = 200;

	/** The test world: a game state (slot 0 human, slot 1 AI), the subsystems, slot 0's mill and the step's processors. */
	struct FFarmWorld
	{
		UWorld* World = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLSourceSubsystem* Sources = nullptr;
		TSharedPtr<FMassEntityManager> EntityManager;
		UHOLGatherProcessor* Gather = nullptr;
		UHOLUnitOrderProcessor* Orders = nullptr;
		AHOLBuilding* Mill = nullptr;

		FFarmWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			AHOLGameState* State = World->SpawnActor<AHOLGameState>();
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
			if (!State || !Units || !Sources || !Simulation || !EntitySubsystem)
			{
				return;
			}
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
			{
				Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			}
			State->InitVictory(Config);
			Sources->OnWorldBeginPlay(*World);
			// Slot 0's deposit of food, finished: its farms' spots face it.
			Mill = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
			if (Mill)
			{
				Mill->Setup(HOLBuildingTypeNames::Mill, 0, 0);
			}
			EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			Gather = NewObject<UHOLGatherProcessor>(Simulation);
			Gather->CallInitialize(Simulation, EntityManager.ToSharedRef());
			Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
			Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		}

		~FFarmWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Gather && Orders && Mill; }

		/** One fixed step: the order processor, then the gathering (the pipeline's order). */
		void RunStep()
		{
			const float TestStepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
			{
				FMassProcessingContext ProcessingContext(EntityManager, TestStepSeconds);
				UE::Mass::Executor::Run(*Orders, ProcessingContext);
			}
			{
				FMassProcessingContext ProcessingContext(EntityManager, TestStepSeconds);
				UE::Mass::Executor::Run(*Gather, ProcessingContext);
			}
		}

		/** A Peasant born for Owner (far from every source); its identifier, 0 if none was born. */
		uint32 PlacePeasant(const int32 Owner)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = Peasant;
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Owner);
			Request.Center = FVector(-20000.0, -20000.0, 0.0);
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
			return After.Num() == 1 ? After[0] : 0;
		}

		/** Sets the unit down at Location (its height kept). */
		void SetDown(const uint32 UnitId, const FVector2D& Location)
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			if (FTransformFragment* Transform = EntityManager->IsEntityValid(Entity) ? EntityManager->GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr)
			{
				const double Z = Transform->GetTransform().GetLocation().Z;
				Transform->GetMutableTransform().SetLocation(FVector(Location, Z));
			}
		}

		const FHOLGatherFragment* GetGather(const uint32 UnitId) const
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			return EntityManager->IsEntityValid(Entity) ? EntityManager->GetFragmentDataPtr<FHOLGatherFragment>(Entity) : nullptr;
		}

		const FHOLUnitTypeConstants* GetConstants(const uint32 UnitId) const
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			return EntityManager->IsEntityValid(Entity) ? &EntityManager->GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity) : nullptr;
		}

		/** The unit gathers at SourceIndex now (touching it, taking from it). */
		bool IsGatheringAt(const uint32 UnitId, const int32 SourceIndex) const
		{
			const FHOLGatherFragment* Fragment = GetGather(UnitId);
			return Fragment && Fragment->Phase == EHOLGatherPhase::Gathering && Fragment->SourceIndex == SourceIndex;
		}

		/** Food the unit gathered: whole units carried and the share of the next one. */
		double GetFoodGathered(const uint32 UnitId) const
		{
			const FHOLGatherFragment* Fragment = GetGather(UnitId);
			if (!Fragment)
			{
				return 0.0;
			}
			const double Whole = Fragment->Load.Resource == EHOLResource::Food ? Fragment->Load.Amount : 0.0;
			const double Share = Fragment->Load.ProgressResource == EHOLResource::Food ? Fragment->Load.Progress : 0.0;
			return Whole + Share;
		}
	};
}

// (a) A Peasant of slot 0 sent to its own farm field, by its mill, gathers 0.55 food a second: the field's RateScale times
// its own rate, both read from the real tables; the endless field never sends it away. (b) A second Peasant sent with it
// never works the field at the same time (one place): it goes to the free farm 6 m away (its massif); at a farm alone, it
// stands idle. (c) A Peasant of slot 1 never gathers slot 0's field: its order to it is refused, and the berries of the
// map it used up do not send it to slot 0's farm beside them. (d) The map's berries keep the Peasant's own rate (0.7).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherFarmFieldTest, "HeroesOfLiberty.Gather.FarmField", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherFarmFieldTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::GatherFarm;
	FFarmWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	UWorld& World = *Test.World;
	UHOLSourceSubsystem& Registry = *Test.Sources;
	// Every group 70 m or more from the others (beyond a nearby grove's 20 m, D309 point 6).
	const int32 Farm = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, 0.0, 0.0), INDEX_NONE, 0);
	const int32 NextFarm = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, 600.0, 0.0), INDEX_NONE, 0);
	const int32 LoneFarm = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, 8000.0, 0.0), INDEX_NONE, 0);
	const int32 GuardedFarm = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, -8000.0, 0.0), INDEX_NONE, 0);
	const int32 LastBerries = Registry.AddSource(HOLSourceTypeNames::BerryBush, FVector(-8000.0, 0.0, 0.0), 1);
	const int32 FarmByBerries = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(-8000.0, 600.0, 0.0), INDEX_NONE, 0);
	const int32 Berries = Registry.AddSource(HOLSourceTypeNames::BerryBush, FVector(8000.0, 0.0, 0.0));
	const TArray<FHOLSource>& Sources = Registry.GetSources();
	if (!TestTrue(TEXT("five farm fields of slot 0 and two bushes of the map"), Sources.Num() == 7 && Farm != INDEX_NONE && NextFarm != INDEX_NONE
		&& LoneFarm != INDEX_NONE && GuardedFarm != INDEX_NONE && LastBerries != INDEX_NONE && FarmByBerries != INDEX_NONE && Berries != INDEX_NONE))
	{
		return false;
	}
	const FHOLSourceTypeRow* FieldRow = Registry.GetType(Sources[Farm].TypeIndex);
	const FHOLSourceTypeRow* BushRow = Registry.GetType(Sources[Berries].TypeIndex);
	if (!TestNotNull(TEXT("the farm field's row"), FieldRow) || !TestNotNull(TEXT("the berry bush's row"), BushRow))
	{
		return false;
	}

	// Slot 0: two Peasants for the farm by the mill, two for the lone farm, one for the berries; slot 1: one sent to slot
	// 0's farm, one to the last berries.
	const uint32 First = Test.PlacePeasant(0);
	const uint32 Second = Test.PlacePeasant(0);
	const uint32 LoneFirst = Test.PlacePeasant(0);
	const uint32 LoneSecond = Test.PlacePeasant(0);
	const uint32 Picker = Test.PlacePeasant(0);
	const uint32 Intruder = Test.PlacePeasant(1);
	const uint32 Passer = Test.PlacePeasant(1);
	const FHOLUnitTypeConstants* Constants = Test.GetConstants(First);
	if (!TestTrue(TEXT("seven Peasants born"), First && Second && LoneFirst && LoneSecond && Picker && Intruder && Passer)
		|| !TestNotNull(TEXT("the Peasant's constants"), Constants))
	{
		return false;
	}
	const float TestStepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	const float UnitRate = Constants->GatherRates[EHOLResource::Food];
	const float FieldRate = FieldRow->GatherRate(UnitRate);
	const float BushRate = BushRow->GatherRate(UnitRate);
	TestTrue(TEXT("the field gives less than the Peasant's own rate (RateScale of the table)"), UnitRate > 0.f && FieldRate < UnitRate);
	TestEqual(TEXT("the berries give the Peasant's own rate"), BushRate, UnitRate);
	TestEqual(TEXT("one place at a field (the table)"), FieldRow->Places, 1);

	// Each one touching its source (within the contact gap, D296): no walk needed, none is run.
	const double BodyRadius = Constants->Radius;
	const auto Touching = [&Sources, BodyRadius](const int32 SourceIndex, const FVector2D& Direction)
	{
		return FVector2D(Sources[SourceIndex].Location) + Direction * (Sources[SourceIndex].Radius + BodyRadius + 10.0);
	};
	Test.SetDown(First, Touching(Farm, FVector2D(-1.0, 0.0)));         // between the field and the mill
	Test.SetDown(Second, Touching(Farm, FVector2D(1.0, 0.0)));         // the other side, 3.4 m from the first
	Test.SetDown(LoneFirst, Touching(LoneFarm, FVector2D(-1.0, 0.0)));
	Test.SetDown(LoneSecond, Touching(LoneFarm, FVector2D(1.0, 0.0)));
	Test.SetDown(Picker, Touching(Berries, FVector2D(-1.0, 0.0)));
	Test.SetDown(Intruder, Touching(GuardedFarm, FVector2D(-1.0, 0.0)));
	Test.SetDown(Passer, Touching(LastBerries, FVector2D(0.0, -1.0))); // the side away from slot 0's farm

	TestTrue(TEXT("slot 0's order to its farm"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ First, Second }), Farm));
	TestTrue(TEXT("slot 0's order to its lone farm"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ LoneFirst, LoneSecond }), LoneFarm));
	TestTrue(TEXT("slot 0's order to the berries"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ Picker }), Berries));
	TestTrue(TEXT("slot 1's order to slot 0's farm enters the queue (the gathering decides)"),
		HOLPlayerOrders::Gather(World, 1, TArray<uint32>({ Intruder }), GuardedFarm));
	TestTrue(TEXT("slot 1's order to the last berries"), HOLPlayerOrders::Gather(World, 1, TArray<uint32>({ Passer }), LastBerries));

	// The first step: the orders applied, each one touching its source starts at once, or looks elsewhere.
	Test.RunStep();
	const bool bFirstHolds = Test.IsGatheringAt(First, Farm);
	const uint32 Holder = bFirstHolds ? First : Second;
	const uint32 Other = bFirstHolds ? Second : First;
	TestTrue(TEXT("(a) one Peasant gathers the farm by the mill"), Test.IsGatheringAt(Holder, Farm));
	TestFalse(TEXT("(b) the other does not gather it with him"), Test.IsGatheringAt(Other, Farm));
	const FHOLGatherFragment* OtherGather = Test.GetGather(Other);
	TestTrue(TEXT("(b) the other goes to the free farm of its massif, 6 m away"), OtherGather && OtherGather->IsGathering()
		&& OtherGather->SourceIndex == NextFarm);
	const bool bLoneFirstHolds = Test.IsGatheringAt(LoneFirst, LoneFarm);
	const uint32 LoneOther = bLoneFirstHolds ? LoneSecond : LoneFirst;
	TestTrue(TEXT("(b) one Peasant gathers the lone farm"), Test.IsGatheringAt(bLoneFirstHolds ? LoneFirst : LoneSecond, LoneFarm));
	const FHOLGatherFragment* LoneOtherGather = Test.GetGather(LoneOther);
	TestTrue(TEXT("(b) the other one, no free farm near, stands idle"), LoneOtherGather && !LoneOtherGather->IsGathering());
	const FHOLGatherFragment* IntruderGather = Test.GetGather(Intruder);
	TestTrue(TEXT("(c) slot 1's Peasant refused at slot 0's farm: not gathering"), IntruderGather && !IntruderGather->IsGathering());

	// The steps after: at most one at a time on each field; slot 1 never at slot 0's fields.
	int32 MostOnFarm = 0;
	int32 MostOnLoneFarm = 0;
	bool bIntruderGathered = false;
	bool bPasserToFarm = false;
	for (int32 Step = 1; Step < TestSteps; ++Step)
	{
		Test.RunStep();
		MostOnFarm = FMath::Max(MostOnFarm, (Test.IsGatheringAt(First, Farm) ? 1 : 0) + (Test.IsGatheringAt(Second, Farm) ? 1 : 0));
		MostOnLoneFarm = FMath::Max(MostOnLoneFarm, (Test.IsGatheringAt(LoneFirst, LoneFarm) ? 1 : 0) + (Test.IsGatheringAt(LoneSecond, LoneFarm) ? 1 : 0));
		const FHOLGatherFragment* Intruding = Test.GetGather(Intruder);
		bIntruderGathered |= !Intruding || Intruding->IsGathering() || Test.GetFoodGathered(Intruder) > 0.0;
		const FHOLGatherFragment* Passing = Test.GetGather(Passer);
		bPasserToFarm |= !Passing || Passing->SourceIndex == FarmByBerries;
	}

	// (a) 0.55 a second at the field, give or take the first step (spent starting).
	const double Seconds = TestSteps * TestStepSeconds;
	TestTrue(TEXT("(a) the holder still gathers its field (endless: never used up)"), Test.IsGatheringAt(Holder, Farm));
	TestEqual(TEXT("(a) food gathered at the field: the field's rate"), Test.GetFoodGathered(Holder), FieldRate * Seconds, FieldRate * TestStepSeconds + 1e-3);
	TestTrue(TEXT("(a) the field still stands"), Sources[Farm].IsStanding());
	// (b)
	TestEqual(TEXT("(b) never two at once at the farm by the mill"), MostOnFarm, 1);
	TestEqual(TEXT("(b) never two at once at the lone farm"), MostOnLoneFarm, 1);
	// (c)
	TestFalse(TEXT("(c) slot 1's Peasant never gathered slot 0's farm"), bIntruderGathered);
	TestFalse(TEXT("(c) the last berries used up"), Sources[LastBerries].IsStanding());
	TestEqual(TEXT("(c) slot 1's Peasant took the last berry"), Test.GetFoodGathered(Passer), 1.0, 1e-3);
	TestFalse(TEXT("(c) ... and was never sent to slot 0's farm beside them"), bPasserToFarm);
	// (d)
	TestTrue(TEXT("(d) the berries' Peasant still gathers"), Test.IsGatheringAt(Picker, Berries));
	TestEqual(TEXT("(d) food gathered at the berries: the Peasant's own rate"), Test.GetFoodGathered(Picker), BushRate * Seconds, BushRate * TestStepSeconds + 1e-3);
	return true;
}

// D602: the farm fields within 30 m of a finished Sanctuary of the Dawn of their owner give 15 % more food (the pure rule, then
// the real chain: a Peasant at a field in the zone, one at a field out of it, one at a field by a rival's Sanctuary).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherSanctuaryTest, "HeroesOfLiberty.Gather.Sanctuary", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherSanctuaryTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::GatherFarm;
	// The rule: the zone's edge, nothing without a Sanctuary, no stacking.
	TArray<FVector2D> Sanctuaries;
	TestEqual(TEXT("no Sanctuary: no bonus"), HOLGather::GetSanctuaryFarmPercent(Sanctuaries, FVector2D(100.0, 0.0)), 0);
	Sanctuaries.Add(FVector2D(0.0, 0.0));
	TestEqual(TEXT("a field 29 m away"), HOLGather::GetSanctuaryFarmPercent(Sanctuaries, FVector2D(2900.0, 0.0)), 15);
	TestEqual(TEXT("a field at 30 m"), HOLGather::GetSanctuaryFarmPercent(Sanctuaries, FVector2D(3000.0, 0.0)), 15);
	TestEqual(TEXT("a field 31 m away"), HOLGather::GetSanctuaryFarmPercent(Sanctuaries, FVector2D(3100.0, 0.0)), 0);
	Sanctuaries.Add(FVector2D(1000.0, 0.0));
	TestEqual(TEXT("two Sanctuaries do not stack"), HOLGather::GetSanctuaryFarmPercent(Sanctuaries, FVector2D(500.0, 0.0)), 15);

	FFarmWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	UWorld& World = *Test.World;
	UHOLSourceSubsystem& Registry = *Test.Sources;
	const int32 Near = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, 0.0, 0.0), INDEX_NONE, 0);
	const int32 Far = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, 8000.0, 0.0), INDEX_NONE, 0);
	const int32 Rival = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, -8000.0, 0.0), INDEX_NONE, 0);
	AHOLBuilding* Own = World.SpawnActor<AHOLBuilding>(FVector(1000.0, 2500.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Enemy = World.SpawnActor<AHOLBuilding>(FVector(1000.0, -6000.0, 0.0), FRotator::ZeroRotator);
	if (!TestTrue(TEXT("three fields and two Sanctuaries"), Near != INDEX_NONE && Far != INDEX_NONE && Rival != INDEX_NONE && Own && Enemy))
	{
		return false;
	}
	Own->Setup(HOLBuildingTypeNames::Sanctuary, 0, 0);
	Enemy->Setup(HOLBuildingTypeNames::Sanctuary, 1, 1);
	const TArray<FHOLSource>& Sources = Registry.GetSources();
	const FHOLSourceTypeRow* FieldRow = Registry.GetType(Sources[Near].TypeIndex);
	const uint32 InZone = Test.PlacePeasant(0);
	const uint32 OutOfZone = Test.PlacePeasant(0);
	const uint32 ByRival = Test.PlacePeasant(0);
	const FHOLUnitTypeConstants* Constants = Test.GetConstants(InZone);
	if (!TestNotNull(TEXT("the field's row"), FieldRow) || !TestTrue(TEXT("three Peasants born"), InZone && OutOfZone && ByRival) || !TestNotNull(TEXT("constants"), Constants))
	{
		return false;
	}
	const float TestStepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	const float FieldRate = FieldRow->GatherRate(Constants->GatherRates[EHOLResource::Food]);
	const double BodyRadius = Constants->Radius;
	const auto Touching = [&Sources, BodyRadius](const int32 SourceIndex)
	{
		return FVector2D(Sources[SourceIndex].Location) + FVector2D(-1.0, 0.0) * (Sources[SourceIndex].Radius + BodyRadius + 10.0);
	};
	Test.SetDown(InZone, Touching(Near));
	Test.SetDown(OutOfZone, Touching(Far));
	Test.SetDown(ByRival, Touching(Rival));
	TestTrue(TEXT("order to the field in the zone"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ InZone }), Near));
	TestTrue(TEXT("order to the field out of the zone"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ OutOfZone }), Far));
	TestTrue(TEXT("order to the field by the rival's Sanctuary"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ ByRival }), Rival));
	for (int32 Step = 0; Step < TestSteps; ++Step)
	{
		Test.RunStep();
	}
	const double Seconds = TestSteps * TestStepSeconds;
	const double Slack = FieldRate * TestStepSeconds * 1.15 + 1e-3;
	TestEqual(TEXT("in the zone of its Sanctuary: the field's rate + 15 %"), Test.GetFoodGathered(InZone), FieldRate * 1.15 * Seconds, Slack);
	TestEqual(TEXT("out of the zone: the field's rate"), Test.GetFoodGathered(OutOfZone), FieldRate * Seconds, Slack);
	TestEqual(TEXT("by a rival's Sanctuary: the field's rate"), Test.GetFoodGathered(ByRival), FieldRate * Seconds, Slack);
	return true;
}

// The top bar's income tooltip (Retours/Q-sites-gold-line.md, answer "other": every source of the resource, its Peasants counted on it).
// (a) Slot 0's Peasants at their sources each count their rate there, read from the real tables: one at its farm field (the field's
// RateScale), two at berry bushes (the Peasant's own rate); the food a minute is their sum times 60, nothing for the other resources,
// nothing for slot 1 (no gatherer). (b) The gold counts the sacred sites the player holds (its own views whose owner is its slot): two
// held and one seen held by slot 1 = 2 x hol.Site.GoldPerMinute, the food unchanged; slot 1 gets nothing from what slot 0 sees.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGatherIncomeTest, "HeroesOfLiberty.Gather.Income", UE::HeroesOfLiberty::Tests::Gather::Flags)
bool FHOLGatherIncomeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::GatherFarm;
	FFarmWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	UWorld& World = *Test.World;
	AHOLGameState* GameState = World.GetGameState<AHOLGameState>();
	if (!TestNotNull(TEXT("the game state"), GameState))
	{
		return false;
	}
	AHOLPlayerState* Players[2] = {};
	for (int32 Slot = 0; Slot < 2; ++Slot)
	{
		Players[Slot] = World.SpawnActor<AHOLPlayerState>();
		if (!TestNotNull(TEXT("a player state"), Players[Slot]))
		{
			return false;
		}
		Players[Slot]->SetMatchSlot(Slot, Slot);
		GameState->AddPlayerState(Players[Slot]);
	}
	UHOLSourceSubsystem& Registry = *Test.Sources;
	const int32 Farm = Registry.AddSource(HOLSourceTypeNames::FarmField, FVector(1000.0, 0.0, 0.0), INDEX_NONE, 0);
	const int32 Berries = Registry.AddSource(HOLSourceTypeNames::BerryBush, FVector(8000.0, 0.0, 0.0));
	const int32 OtherBerries = Registry.AddSource(HOLSourceTypeNames::BerryBush, FVector(8000.0, 8000.0, 0.0));
	const TArray<FHOLSource>& Sources = Registry.GetSources();
	if (!TestTrue(TEXT("a farm field of slot 0 and two bushes"), Farm != INDEX_NONE && Berries != INDEX_NONE && OtherBerries != INDEX_NONE))
	{
		return false;
	}
	const FHOLSourceTypeRow* FieldRow = Registry.GetType(Sources[Farm].TypeIndex);
	const FHOLSourceTypeRow* BushRow = Registry.GetType(Sources[Berries].TypeIndex);
	const uint32 Farmer = Test.PlacePeasant(0);
	const uint32 Picker = Test.PlacePeasant(0);
	const uint32 OtherPicker = Test.PlacePeasant(0);
	const FHOLUnitTypeConstants* Constants = Test.GetConstants(Farmer);
	if (!TestTrue(TEXT("the rows, three Peasants and their constants"), FieldRow && BushRow && Farmer && Picker && OtherPicker && Constants))
	{
		return false;
	}
	const double BodyRadius = Constants->Radius;
	const auto Touching = [&Sources, BodyRadius](const int32 SourceIndex, const FVector2D& Direction)
	{
		return FVector2D(Sources[SourceIndex].Location) + Direction * (Sources[SourceIndex].Radius + BodyRadius + 10.0);
	};
	Test.SetDown(Farmer, Touching(Farm, FVector2D(-1.0, 0.0)));
	Test.SetDown(Picker, Touching(Berries, FVector2D(-1.0, 0.0)));
	Test.SetDown(OtherPicker, Touching(OtherBerries, FVector2D(-1.0, 0.0)));
	TestTrue(TEXT("slot 0's order to its farm"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ Farmer }), Farm));
	TestTrue(TEXT("slot 0's order to the berries"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ Picker }), Berries));
	TestTrue(TEXT("slot 0's order to the other berries"), HOLPlayerOrders::Gather(World, 0, TArray<uint32>({ OtherPicker }), OtherBerries));
	Test.RunStep();
	Test.RunStep();
	TestTrue(TEXT("the three gather at their sources"), Test.IsGatheringAt(Farmer, Farm) && Test.IsGatheringAt(Picker, Berries)
		&& Test.IsGatheringAt(OtherPicker, OtherBerries));

	// (a) The Peasants (D788): each one's rate at its source, from the tables, slowed by its round trip to the nearest deposit.
	// The farmer's field faces the mill (a short trip: close to the field's rate); the two pickers' bushes are 80 m away
	// (a long trip: well under the Peasant's own rate). The sum is under the rates at the sources, not their sum (D788: ~+350,
	// not +600, for Peasants far from their deposit).
	const float UnitRate = Constants->GatherRates[EHOLResource::Food];
	const float FieldRate = FieldRow->GatherRate(UnitRate);
	const float AtSources = 60.f * (FieldRate + 2.f * BushRow->GatherRate(UnitRate));
	TestTrue(TEXT("the Peasant gathers food (the table)"), AtSources > 0.f);
	const FHOLResources Income = Players[0]->GetIncomePerMinute();
	TestTrue(*FString::Printf(TEXT("(a) slot 0's food a minute (%d) is under the rates at the sources (%.0f): the trips count"), Income.Food, AtSources),
		Income.Food > 0 && Income.Food <= FMath::RoundToInt(0.8f * AtSources));
	TestTrue(*FString::Printf(TEXT("(a) the farmer by its mill still counts most of its field's rate (%d >= %.0f)"), Income.Food, 0.8f * 60.f * FieldRate),
		Income.Food >= FMath::RoundToInt(0.8f * 60.f * FieldRate));
	TestEqual(TEXT("(a) the gatherers' part is the same"), Players[0]->GetGatherIncome().Food, Income.Food);
	const int32 FoodWithBushesFar = Income.Food;
	TestEqual(TEXT("(a) no wood"), Income.Wood, 0);
	TestEqual(TEXT("(a) no stone"), Income.Stone, 0);
	TestEqual(TEXT("(a) no gold"), Income.Gold, 0);
	TestTrue(TEXT("(a) slot 1, no gatherer: nothing"), Players[1]->GetIncomePerMinute() == FHOLResources());

	// (b) The sacred sites slot 0 holds pay it their gold a minute; the one it sees held by slot 1 does not.
	const int32 Rate = UHOLSiteSubsystem::GetGoldPerMinute();
	TestTrue(TEXT("a site pays some gold (hol.Site.GoldPerMinute)"), Rate > 0);
	TArray<FHOLSiteView> Views;
	for (int32 Index = 0; Index < 3; ++Index)
	{
		FHOLSiteView& View = Views.AddDefaulted_GetRef();
		View.Index = static_cast<uint8>(Index);
		View.Owner = Index < 2 ? 0 : 1;
	}
	Players[0]->SetSiteViews(Views);
	const FHOLResources WithSites = Players[0]->GetIncomePerMinute();
	TestEqual(TEXT("(b) two sites held: twice the rate in gold"), WithSites.Gold, 2 * Rate);
	TestEqual(TEXT("(b) the food unchanged"), WithSites.Food, FoodWithBushesFar);
	TestEqual(TEXT("(b) slot 1 gets nothing from slot 0's views"), Players[1]->GetIncomePerMinute().Gold, 0);
	return true;
}

#endif