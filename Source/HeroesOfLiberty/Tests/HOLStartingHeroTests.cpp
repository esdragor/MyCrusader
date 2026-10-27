// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLStartRules.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// J8a.2 checks (J8a-Q2): the hero of every occupied slot by its main forum at the start of a real match, clear of the
// starting Peasants and of the King, outside the population; none in the test scenarios unless their .json asks.
namespace UE::HeroesOfLiberty::Tests::StartingHero
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A body's radius from the real unit type table (Tools/Data/UnitTypes.csv); -1 when the row is missing. */
	float GetRadius(const FName Type)
	{
		const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		const FHOLUnitTypeRow* Row = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(Type, TEXT("Starting hero test"), false) : nullptr;
		return Row ? Row->Radius : -1.f;
	}

	/** Two bodies clear of each other: their radii and half a metre between them. */
	constexpr double Clearance = 50.0;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartingHeroLocationTest, "HeroesOfLiberty.Match.StartingHero.Location",
	UE::HeroesOfLiberty::Tests::StartingHero::Flags)

bool FHOLStartingHeroLocationTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::StartingHero;
	const float HeroRadius = GetRadius(FHOLStartRules::HeroTypeName);
	const float KingRadius = GetRadius(FHOLStartRules::KingTypeName);
	const float PeasantRadius = GetRadius(FHOLStartRules::PeasantTypeName);
	TestEqual(TEXT("The hero's radius: 72 cm (J8a-Q3)"), HeroRadius, 72.f);
	if (!TestTrue(TEXT("King and Peasant rows found"), KingRadius > 0.f && PeasantRadius > 0.f))
	{
		return false;
	}
	const int32 NumPeasants = GetDefault<UHOLMatchSettings>()->StartingPeasants;
	// Eight forums around the middle of a map, as Plain's eight starts (about 150 m out), two body sizes.
	for (const float HalfBody : { 600.f, 800.f })
	{
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			const double Angle = Slot * UE_DOUBLE_PI / 4.0;
			const FVector Centre(15000.0 * FMath::Cos(Angle), 15000.0 * FMath::Sin(Angle), 120.0);
			const FVector Hero = FHOLStartRules::GetHeroLocation(Centre, HalfBody);
			const FVector King = FHOLStartRules::GetKingLocation(Centre, HalfBody);
			const FString Where = FString::Printf(TEXT("slot %d, half body %.0f"), Slot, HalfBody);
			// In front (-X, the side of the births, of the Peasants and of the King), off the body (a square HalfBody
			// around the centre), by the forum.
			TestTrue(*FString::Printf(TEXT("%s: in front of the forum, off its body"), *Where), Hero.X <= Centre.X - HalfBody - HeroRadius);
			TestTrue(*FString::Printf(TEXT("%s: by the forum (within 5 m of its edge, across its width)"), *Where),
				Hero.X >= Centre.X - HalfBody - 500.0 && FMath::Abs(Hero.Y - Centre.Y) <= HalfBody);
			TestEqual(*FString::Printf(TEXT("%s: at the forum's height"), *Where), Hero.Z, Centre.Z);
			TestTrue(*FString::Printf(TEXT("%s: clear of the King"), *Where),
				FVector::Dist2D(Hero, King) >= HeroRadius + KingRadius + Clearance);
			// Clear of the starting Peasants, for the rule's number and others around it.
			for (const int32 Num : { NumPeasants, 1, 3, 8 })
			{
				for (const FVector& Peasant : FHOLStartRules::GetStartingPeasantLocations(Centre, HalfBody, Num))
				{
					TestTrue(*FString::Printf(TEXT("%s: clear of the %d starting Peasants"), *Where, Num),
						FVector::Dist2D(Hero, Peasant) >= HeroRadius + PeasantRadius + Clearance);
				}
			}
			TestEqual(*FString::Printf(TEXT("%s: the same place each time"), *Where), FHOLStartRules::GetHeroLocation(Centre, HalfBody), Hero);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartingHeroCountTest, "HeroesOfLiberty.Match.StartingHero.Count",
	UE::HeroesOfLiberty::Tests::StartingHero::Flags)

bool FHOLStartingHeroCountTest::RunTest(const FString& Parameters)
{
	// The rule of the game, from DefaultGame.ini.
	const bool bSetting = GetDefault<UHOLMatchSettings>()->bStartingHero;
	TestTrue(TEXT("The setting: a hero"), bSetting);
	const auto CountHeroes = [bSetting](const FHOLMatchConfig& Config)
	{
		int32 Num = 0;
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots + 1; ++Slot)
		{
			Num += FHOLStartRules::ShouldSpawnHero(Config, Slot, bSetting) ? 1 : 0;
		}
		return Num;
	};

	// A real match: the menu (its first configuration is the default one: one human) and the lobby (a file without the
	// field: a human, an AI, the rest closed): one hero per occupied slot, AI included.
	TestEqual(TEXT("Menu: one hero for the human"), CountHeroes(FHOLMatchConfig::MakeDefault()), 1);
	FHOLMatchConfig Lobby;
	TestTrue(TEXT("Lobby file read"), FHOLMatchConfig::FromJson(
		TEXT(R"({ "version": 2, "map": "/Game/Maps/Plain.Plain", "slots": [ { "state": "Human", "colorIndex": 0, "playerName": "Bob" }, { "state": "AI", "colorIndex": 1 } ] })"),
		Lobby));
	TestEqual(TEXT("Lobby: one hero for each of the 2 occupied slots"), CountHeroes(Lobby), 2);
	TestTrue(TEXT("Lobby: the AI slot has one"), FHOLStartRules::ShouldSpawnHero(Lobby, 1, bSetting));
	TestFalse(TEXT("Lobby: a closed slot none"), FHOLStartRules::ShouldSpawnHero(Lobby, 2, bSetting));
	TestFalse(TEXT("The rule off (quick_check's -ini): none"), FHOLStartRules::ShouldSpawnHero(Lobby, 0, false));

	// Every scenario, through the reading of hol.Scenario: none, unless its .json asks for one with "startingHero".
	const FString ScenarioDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Scenarios")));
	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *FPaths::Combine(ScenarioDir, TEXT("*.json")), true, false);
	TestTrue(TEXT("Scenarios with a .json found"), Files.Num() >= 10);
	for (const FString& File : Files)
	{
		FString Json;
		FHOLMatchConfig Config;
		TestTrue(*FString::Printf(TEXT("%s read"), *File), FFileHelper::LoadFileToString(Json, *FPaths::Combine(ScenarioDir, File))
			&& HOLTestHooks::MakeScenarioConfig(&Json, Config));
		if (!Json.Contains(TEXT("\"startingHero\""), ESearchCase::IgnoreCase))
		{
			TestEqual(*FString::Printf(TEXT("%s: no hero"), *File), CountHeroes(Config), 0);
		}
	}
	FHOLMatchConfig NoFile;
	TestTrue(TEXT("Scenario without .json"), HOLTestHooks::MakeScenarioConfig(nullptr, NoFile));
	TestEqual(TEXT("Scenario without .json: no hero"), CountHeroes(NoFile), 0);

	// A scenario (or a test script's configuration) that asks, as a JSON boolean or a number.
	const TCHAR* Format = TEXT(R"({ "map": "/Game/Maps/Plain.Plain", "startingHero": VALUE, "slots": [ { "state": "Human", "colorIndex": 0 }, { "state": "AI", "colorIndex": 1 } ] })");
	for (const TPair<const TCHAR*, int32>& Asked : { TPair<const TCHAR*, int32>(TEXT("true"), 2), TPair<const TCHAR*, int32>(TEXT("1"), 2),
		TPair<const TCHAR*, int32>(TEXT("false"), 0), TPair<const TCHAR*, int32>(TEXT("0"), 0) })
	{
		const FString Json = FString(Format).Replace(TEXT("VALUE"), Asked.Key);
		FHOLMatchConfig Scenario;
		TestTrue(*FString::Printf(TEXT("Scenario asking %s read"), Asked.Key), HOLTestHooks::MakeScenarioConfig(&Json, Scenario));
		TestEqual(*FString::Printf(TEXT("Scenario asking %s: %d heroes"), Asked.Key, Asked.Value), CountHeroes(Scenario), Asked.Value);
		FHOLMatchConfig Script;
		TestTrue(*FString::Printf(TEXT("Script configuration %s read"), Asked.Key), FHOLMatchConfig::FromJson(Json, Script));
		TestEqual(*FString::Printf(TEXT("Script configuration %s: %d heroes, whatever the rule"), Asked.Key, Asked.Value),
			CountHeroes(Script), Asked.Value);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartingHeroSpawnTest, "HeroesOfLiberty.Match.StartingHero.Spawn",
	UE::HeroesOfLiberty::Tests::StartingHero::Flags)

// Through the game mode's spawning (SpawnMainCentres' calls): a main forum of slot 1, its King, its starting Peasants
// and its hero; the hero born once, of slot 1, at the rule's place, clear of the others, and outside the population.
bool FHOLStartingHeroSpawnTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::StartingHero;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	AHOLBuilding* Forum = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 2000.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("forum"), Forum))
	{
		constexpr int32 Slot = 1;
		Forum->Setup(HOLBuildingTypeNames::MainCentre, Slot, Slot);
		const int32 NumPeasants = GetDefault<UHOLMatchSettings>()->StartingPeasants;
		TestTrue(TEXT("King spawned"), AHeroesOfLibertyGameModeBase::SpawnKing(*World, Slot, *Forum));
		TestTrue(TEXT("Peasants spawned"), AHeroesOfLibertyGameModeBase::SpawnStartingPeasants(*World, Slot, *Forum, NumPeasants));
		TestTrue(TEXT("Hero spawned"), AHeroesOfLibertyGameModeBase::SpawnStartingHero(*World, Slot, *Forum));
		Units->ServeBirthRequests();

		TArray<uint32> Ids;
		Units->GetAllUnitIds(Ids);
		TArray<FHOLUnitView> Views;
		int32 NumHeroes = 0;
		FHOLUnitView Hero;
		for (const uint32 Id : Ids)
		{
			FHOLUnitView View;
			if (Units->GetUnitView(Id, View))
			{
				Views.Add(View);
				if (View.TypeName == FHOLStartRules::HeroTypeName)
				{
					++NumHeroes;
					Hero = View;
				}
			}
		}
		TestEqual(TEXT("King, Peasants and one hero born"), Views.Num(), NumPeasants + 2);
		if (TestEqual(TEXT("One hero"), NumHeroes, 1))
		{
			TestEqual(TEXT("The hero of slot 1"), static_cast<int32>(Hero.OwnerPlayer), Slot);
			TestEqual(TEXT("The hero's radius"), Hero.Radius, 72.f);
			const FHOLBuildingTypeRow* Type = Forum->GetType();
			const float HalfBody = 0.5f * static_cast<float>((Type ? UHOLBuildingSettings::GetBodySize(Type->Footprint) : FVector2D(1200.0)).GetMax());
			TestTrue(TEXT("The hero at the rule's place (within 1 m)"),
				FVector::Dist2D(Hero.Location, FHOLStartRules::GetHeroLocation(Forum->GetActorLocation(), HalfBody)) <= 100.0);
			for (const FHOLUnitView& Other : Views)
			{
				if (Other.UnitId != Hero.UnitId)
				{
					TestTrue(*FString::Printf(TEXT("The hero clear of the %s %u"), *Other.TypeName.ToString(), Other.UnitId),
						FVector::Dist2D(Hero.Location, Other.Location) >= Hero.Radius + Other.Radius + Clearance);
				}
			}
		}
		// The population the HUD reads (GetPopulationByOwner, the rows' Population): the Peasants only, not the King nor the hero.
		TArray<int32> Population;
		Units->GetPopulationByOwner(Population);
		TestEqual(TEXT("Slot 1's population: its Peasants only (the hero outside)"), Population.IsValidIndex(Slot) ? Population[Slot] : 0, NumPeasants);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
