// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Match/HOLPlayerState.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "Match/HOLGameState.h"
#include "Match/HOLSpecializationSubsystem.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLStartRules.h"
#include "Map/HOLStartSlot.h"
#include "Map/HOLMapBounds.h"
#include "Player/HOLCommandGrid.h"
#include "HAL/IConsoleManager.h"
#include "Match/HOLFactionRules.h"
#include "Simulation/HOLSiteSubsystem.h"
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
	constexpr double TestClearance = 50.0;
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
				FVector::Dist2D(Hero, King) >= HeroRadius + KingRadius + TestClearance);
			// Clear of the starting Peasants, for the rule's number and others around it.
			for (const int32 Num : { NumPeasants, 1, 3, 8 })
			{
				for (const FVector& Peasant : FHOLStartRules::GetStartingPeasantLocations(Centre, HalfBody, Num))
				{
					TestTrue(*FString::Printf(TEXT("%s: clear of the %d starting Peasants"), *Where, Num),
						FVector::Dist2D(Hero, Peasant) >= HeroRadius + PeasantRadius + TestClearance);
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
						FVector::Dist2D(Hero.Location, Other.Location) >= Hero.Radius + Other.Radius + TestClearance);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartingUnitsAtPointTest, "HeroesOfLiberty.Match.StartingHero.AtPoint",
	UE::HeroesOfLiberty::Tests::StartingHero::Flags)

// N2 (c)+(d), first step: the King, the starting Peasants and the hero born from a POINT and a half body (what a Nomad
// seat without a forum will use) land exactly where the forum's calls put them (the standard start does not change):
// two worlds, one through the forum, one through the point; the born units compared one by one (type, owner, place).
bool FHOLStartingUnitsAtPointTest::RunTest(const FString& Parameters)
{
	constexpr int32 Slot = 1;
	const int32 NumPeasants = GetDefault<UHOLMatchSettings>()->StartingPeasants;
	const FVector ForumLocation(5000.0, 2000.0, 0.0);
	TArray<FHOLUnitView> Born[2];
	for (int32 Way = 0; Way < 2; ++Way)
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
		AHOLBuilding* Forum = World->SpawnActor<AHOLBuilding>(ForumLocation, FRotator::ZeroRotator);
		if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("forum"), Forum))
		{
			Forum->Setup(HOLBuildingTypeNames::MainCentre, Slot, Slot);
			if (Way == 0)
			{
				TestTrue(TEXT("Forum: King"), AHeroesOfLibertyGameModeBase::SpawnKing(*World, Slot, *Forum));
				TestTrue(TEXT("Forum: Peasants"), AHeroesOfLibertyGameModeBase::SpawnStartingPeasants(*World, Slot, *Forum, NumPeasants));
				TestTrue(TEXT("Forum: hero"), AHeroesOfLibertyGameModeBase::SpawnStartingHero(*World, Slot, *Forum));
			}
			else
			{
				// The point's half body read from the main centre's type, as a forum of that type would have it.
				const float HalfBody = AHeroesOfLibertyGameModeBase::GetStartHalfBody(HOLBuildingTypeNames::MainCentre);
				Forum->Destroy();
				TestTrue(TEXT("Point: King"), AHeroesOfLibertyGameModeBase::SpawnKingAt(*World, Slot, ForumLocation, HalfBody));
				TestTrue(TEXT("Point: Peasants"), AHeroesOfLibertyGameModeBase::SpawnStartingPeasantsAt(*World, Slot, ForumLocation, HalfBody, NumPeasants));
				TestTrue(TEXT("Point: hero"), AHeroesOfLibertyGameModeBase::SpawnStartingHeroAt(*World, Slot, ForumLocation, HalfBody));
			}
			Units->ServeBirthRequests();
			TArray<uint32> Ids;
			Units->GetAllUnitIds(Ids);
			for (const uint32 Id : Ids)
			{
				FHOLUnitView View;
				if (Units->GetUnitView(Id, View))
				{
					Born[Way].Add(View);
				}
			}
		}
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	TestEqual(TEXT("Forum: King, Peasants and hero born"), Born[0].Num(), NumPeasants + 2);
	if (TestEqual(TEXT("As many born from the point as from the forum"), Born[1].Num(), Born[0].Num()))
	{
		for (int32 Index = 0; Index < Born[0].Num(); ++Index)
		{
			const FHOLUnitView& A = Born[0][Index];
			const FHOLUnitView& B = Born[1][Index];
			const FString Where = FString::Printf(TEXT("unit %d (%s)"), Index, *A.TypeName.ToString());
			TestEqual(*FString::Printf(TEXT("%s: same type"), *Where), B.TypeName, A.TypeName);
			TestEqual(*FString::Printf(TEXT("%s: same owner"), *Where), static_cast<int32>(B.OwnerPlayer), static_cast<int32>(A.OwnerPlayer));
			TestTrue(*FString::Printf(TEXT("%s: same place (within 1 cm)"), *Where), FVector::Dist2D(A.Location, B.Location) <= 1.0);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartingNomadSeatTest, "HeroesOfLiberty.Match.StartingHero.NomadSeat",
	UE::HeroesOfLiberty::Tests::StartingHero::Flags)

// N2 (c)+(d): a slot's start through the game mode's own start (SpawnSlotStart, what SpawnMainCentres calls for each start),
// two worlds from the same configuration, standard then Nomad (bNomad):
// - standard: one building (the primary forum) of the slot, a complete Forum on the script board;
// - Nomad: no building at all, the seat Nomad on the board (no complete Forum), and the very same units born (type, owner)
//   as the standard start's by its forum; their places are dispersed (BUG nomad_start point 2, test NomadScatter below).
bool FHOLStartingNomadSeatTest::RunTest(const FString& Parameters)
{
	constexpr int32 Slot = 0;
	const FTransform Start(FRotator(0.f, 90.f, 0.f), FVector(5000.0, 2000.0, 0.0));
	TArray<FHOLUnitView> Born[2];
	int32 Used[2] = { INDEX_NONE, INDEX_NONE };
	for (int32 Way = 0; Way < 2; ++Way)
	{
		const bool bNomad = Way == 1;
		const TCHAR* Name = bNomad ? TEXT("Nomad") : TEXT("Standard");
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
		World->SetGameState(GameState);
		UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
		if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("the script board"), UHOLSpecializationSubsystem::Find(World)))
		{
			// The menu's configuration (one human in slot 0, the rule's starting Peasants and hero), Nomad or not.
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			Config.bNomad = bNomad;
			AHOLBuilding* Centre = AHeroesOfLibertyGameModeBase::SpawnSlotStart(*World, Config, Slot, Start);
			Units->ServeBirthRequests();
			int32 NumBuildings = 0;
			for (TActorIterator<AHOLBuilding> It(World); It; ++It)
			{
				NumBuildings += It->IsActorBeingDestroyed() ? 0 : 1;
			}
			TestEqual(*FString::Printf(TEXT("%s: buildings at the start"), Name), NumBuildings, bNomad ? 0 : 1);
			TestEqual(*FString::Printf(TEXT("%s: a centre returned"), Name), Centre != nullptr, !bNomad);
			if (Centre)
			{
				TestTrue(TEXT("Standard: the centre is the slot's primary forum"), Centre->IsPrimaryForum() && Centre->GetOwnerPlayer() == Slot);
			}
			TestEqual(*FString::Printf(TEXT("%s: a complete Forum on the board"), Name), UHOLSpecializationSubsystem::IsForumComplete(World, Slot), !bNomad);
			TArray<uint32> Ids;
			Units->GetAllUnitIds(Ids);
			for (const uint32 Id : Ids)
			{
				FHOLUnitView View;
				if (Units->GetUnitView(Id, View))
				{
					Born[Way].Add(View);
				}
			}
			// The population the top bar reads (AHOLPlayerState), written by the production at the start of each step: the
			// Nomad's starting units count as the standard start's, even with no building at all (seen at the trial of
			// nomad_start: "Population 0 / 0" with 5 Peasants alive).
			AHOLPlayerState* Player = World->SpawnActor<AHOLPlayerState>();
			UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
			UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			if (TestNotNull(TEXT("player state"), Player) && TestNotNull(TEXT("production"), Production) && TestNotNull(TEXT("simulation"), Simulation))
			{
				Player->SetMatchSlot(Slot, Slot);
				Production->OnWorldBeginPlay(*World);
				Simulation->GetOnStepStarted().Broadcast(1);
				Used[Way] = Player->GetPopulationUsed();
			}
		}
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	const int32 NumPeasants = FHOLStartRules::GetNumStartingPeasants(FHOLMatchConfig::MakeDefault(), GetDefault<UHOLMatchSettings>()->StartingPeasants);
	TestTrue(TEXT("Standard: its starting Peasants (and hero) born"), NumPeasants > 0 && Born[0].Num() >= NumPeasants);
	TestTrue(TEXT("Standard: the top bar counts its starting units"), Used[0] > 0);
	TestEqual(TEXT("Nomad: the top bar counts the same population as the standard start's (no building needed)"), Used[1], Used[0]);
	if (TestEqual(TEXT("Nomad: as many born as the standard start"), Born[1].Num(), Born[0].Num()))
	{
		for (int32 Index = 0; Index < Born[0].Num(); ++Index)
		{
			const FHOLUnitView& A = Born[0][Index];
			const FHOLUnitView& B = Born[1][Index];
			const FString Where = FString::Printf(TEXT("unit %d (%s)"), Index, *A.TypeName.ToString());
			TestEqual(*FString::Printf(TEXT("%s: same type"), *Where), B.TypeName, A.TypeName);
			TestEqual(*FString::Printf(TEXT("%s: same owner (slot 0)"), *Where), static_cast<int32>(B.OwnerPlayer), static_cast<int32>(A.OwnerPlayer));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartingNomadScatterTest, "HeroesOfLiberty.Match.StartingHero.NomadScatter",
	UE::HeroesOfLiberty::Tests::StartingHero::Flags)

// BUG nomad_start point 2 (D207, D786, D702): a Nomad seat's start through the game mode's own start (SpawnSlotStart), the units
// born by the unit subsystem, another camp's start 90 m away and an active sacred site 50 m away:
// - its Peasants dispersed (groups of one, D786): no two of them within 10 m (the standard row is 1.8 m apart);
// - the King with a Peasant (D207: "the King appears with a worker"), the hero with another one, King and hero not together;
// - every unit at least 40 tiles (4000 cm, D786) from the other camp's start, and outside the site's zone plus the Nomad's
//   margin (1200 cm zone + 3200 cm, D702, HOLSiteOption::NomadMarginCm).
bool FHOLStartingNomadScatterTest::RunTest(const FString& Parameters)
{
	constexpr int32 Slot = 0;
	constexpr double Apart = 1000.0;
	constexpr double EnemyCm = 4000.0;
	constexpr float SiteRadius = 1200.f;
	constexpr double SiteCm = 1200.0 + 3200.0;
	const FVector Point(5000.0, 2000.0, 0.0);
	const FTransform Start(FRotator(0.f, 90.f, 0.f), Point);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSiteSubsystem* Sites = World->GetSubsystem<UHOLSiteSubsystem>();
	const AHOLStartSlot* Enemy = World->SpawnActor<AHOLStartSlot>(Point + FVector(9000.0, 0.0, 0.0), FRotator::ZeroRotator);
	// D797: the groups are cast over the whole map; a map of 400 m a side (AHOLMapBounds' own box), the constraints as before.
	World->SpawnActor<AHOLMapBounds>(FVector::ZeroVector, FRotator::ZeroRotator);
	const FVector2D SiteAt(Point.X, Point.Y + 5000.0);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("sites"), Sites) && TestNotNull(TEXT("another camp's start"), Enemy)
		&& TestNotNull(TEXT("the script board"), UHOLSpecializationSubsystem::Find(World)))
	{
		Sites->AddSite(SiteAt, SiteRadius);
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		Config.bNomad = true;
		Config.bRegicide = true;
		Config.bSacredSites = true;
		AHeroesOfLibertyGameModeBase::SpawnSlotStart(*World, Config, Slot, Start);
		Units->ServeBirthRequests();
		const FName Worker = HOLFactionRules::GetRoleTypeName(World, Slot, HOLFactionRules::Roles::Worker);
		TArray<FVector> Peasants;
		TArray<FVector> Kings;
		TArray<FVector> Others;
		TArray<uint32> Ids;
		Units->GetAllUnitIds(Ids);
		for (const uint32 Id : Ids)
		{
			FHOLUnitView View;
			if (Units->GetUnitView(Id, View) && static_cast<int32>(View.OwnerPlayer) == Slot)
			{
				(View.TypeName == Worker ? Peasants : (View.TypeName == FHOLStartRules::KingTypeName ? Kings : Others)).Add(View.Location);
			}
		}
		const int32 NumPeasants = FHOLStartRules::GetNumStartingPeasants(Config, GetDefault<UHOLMatchSettings>()->StartingPeasants);
		TestTrue(TEXT("at least two starting Peasants"), NumPeasants >= 2);
		TestEqual(TEXT("the starting Peasants born"), Peasants.Num(), NumPeasants);
		TestEqual(TEXT("the King born (Regicide)"), Kings.Num(), 1);
		for (int32 A = 0; A < Peasants.Num(); ++A)
		{
			for (int32 B = A + 1; B < Peasants.Num(); ++B)
			{
				const double Distance = FVector::Dist2D(Peasants[A], Peasants[B]);
				TestTrue(*FString::Printf(TEXT("Peasants %d and %d dispersed (%.0f cm apart, at least %.0f)"), A, B, Distance, Apart), Distance >= Apart);
			}
		}
		auto ClosestPeasant = [&Peasants](const FVector& At, double& OutDistance)
		{
			int32 Best = INDEX_NONE;
			OutDistance = TNumericLimits<double>::Max();
			for (int32 Index = 0; Index < Peasants.Num(); ++Index)
			{
				const double Distance = FVector::Dist2D(At, Peasants[Index]);
				if (Distance < OutDistance)
				{
					OutDistance = Distance;
					Best = Index;
				}
			}
			return Best;
		};
		double KingGap = 0.0;
		const int32 KingPeasant = Kings.Num() > 0 ? ClosestPeasant(Kings[0], KingGap) : INDEX_NONE;
		TestTrue(*FString::Printf(TEXT("the King with a Peasant (%.0f cm)"), KingGap), KingPeasant != INDEX_NONE && KingGap < Apart);
		for (const FVector& Hero : Others)
		{
			double HeroGap = 0.0;
			const int32 HeroPeasant = ClosestPeasant(Hero, HeroGap);
			TestTrue(*FString::Printf(TEXT("the hero with a Peasant (%.0f cm)"), HeroGap), HeroPeasant != INDEX_NONE && HeroGap < Apart);
			TestTrue(TEXT("the hero with another Peasant than the King's"), HeroPeasant != KingPeasant || Peasants.Num() < 2);
			if (Kings.Num() > 0)
			{
				TestTrue(*FString::Printf(TEXT("King and hero not together (%.0f cm)"), FVector::Dist2D(Hero, Kings[0])), FVector::Dist2D(Hero, Kings[0]) >= Apart);
			}
		}
		TArray<FVector> All = Peasants;
		All.Append(Kings);
		All.Append(Others);
		const FVector EnemyAt = Enemy->GetMainCentreLocation();
		for (const FVector& Each : All)
		{
			const double ToEnemy = FVector::Dist2D(Each, EnemyAt);
			const double ToSite = FVector2D::Distance(FVector2D(Each), SiteAt);
			TestTrue(*FString::Printf(TEXT("unit at (%.0f, %.0f) 40 tiles from the other camp (%.0f cm)"), Each.X, Each.Y, ToEnemy), ToEnemy >= EnemyCm);
			TestTrue(*FString::Printf(TEXT("unit at (%.0f, %.0f) outside the site's zone and margin (%.0f cm)"), Each.X, Each.Y, ToSite), ToSite >= SiteCm);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartingNomadWholeMapTest, "HeroesOfLiberty.Match.StartingHero.NomadWholeMap",
	UE::HeroesOfLiberty::Tests::StartingHero::Flags)

// D797 (the user's setting on nomad_start, 2026-11-14: "dispersé = dispersé sur TOUTE la carte", the Nomad's principle): a Nomad seat's
// start through the game mode's own start (SpawnSlotStart) on a map of 400 m a side (AHOLMapBounds' own box), its units born by the
// unit subsystem:
// - hol.Nomad.ScatterRadiusCm 0 (the default: the whole map): every unit inside the map's playable area, and the groups spread
//   over it: one at least 100 m from the start's point (the former disk was 60 m);
// - hol.Nomad.ScatterRadiusCm 6000 (the optional disk): every unit within 60 m (+ a group's 7 m reach) of the start's point.
bool FHOLStartingNomadWholeMapTest::RunTest(const FString& Parameters)
{
	constexpr int32 Slot = 0;
	constexpr double FarCm = 10000.0;
	constexpr double DiskCm = 6000.0;
	constexpr double ReachCm = 700.0;
	const FVector Point(5000.0, 2000.0, 0.0);
	const FTransform Start(FRotator(0.f, 90.f, 0.f), Point);
	IConsoleVariable* Radius = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Nomad.ScatterRadiusCm"));
	if (!TestNotNull(TEXT("hol.Nomad.ScatterRadiusCm"), Radius))
	{
		return false;
	}
	const float Saved = Radius->GetFloat();
	for (int32 Way = 0; Way < 2; ++Way)
	{
		const bool bDisk = Way == 1;
		const TCHAR* Name = bDisk ? TEXT("disk of 60 m") : TEXT("whole map");
		Radius->Set(bDisk ? static_cast<float>(DiskCm) : 0.f, ECVF_SetByCode);
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
		World->SetGameState(GameState);
		UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
		const AHOLMapBounds* Bounds = World->SpawnActor<AHOLMapBounds>(FVector::ZeroVector, FRotator::ZeroRotator);
		if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("map bounds"), Bounds) && TestNotNull(TEXT("the script board"), UHOLSpecializationSubsystem::Find(World)))
		{
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			Config.bNomad = true;
			Config.bRegicide = true;
			AHeroesOfLibertyGameModeBase::SpawnSlotStart(*World, Config, Slot, Start);
			Units->ServeBirthRequests();
			const FBox2D Playable = Bounds->GetPlayableArea();
			TArray<uint32> Ids;
			Units->GetAllUnitIds(Ids);
			int32 Count = 0;
			double Farthest = 0.0;
			for (const uint32 Id : Ids)
			{
				FHOLUnitView View;
				if (Units->GetUnitView(Id, View) && static_cast<int32>(View.OwnerPlayer) == Slot)
				{
					++Count;
					const double Distance = FVector::Dist2D(View.Location, Point);
					Farthest = FMath::Max(Farthest, Distance);
					TestTrue(*FString::Printf(TEXT("%s: unit at (%.0f, %.0f) inside the map's playable area"), Name, View.Location.X, View.Location.Y),
						Playable.IsInside(FVector2D(View.Location)));
					if (bDisk)
					{
						TestTrue(*FString::Printf(TEXT("%s: unit at (%.0f, %.0f) within the disk (%.0f cm)"), Name, View.Location.X, View.Location.Y, Distance),
							Distance <= DiskCm + ReachCm);
					}
				}
			}
			TestTrue(*FString::Printf(TEXT("%s: units born (%d)"), Name, Count), Count >= 3);
			if (!bDisk)
			{
				TestTrue(*FString::Printf(TEXT("%s: a group spread far over the map (farthest %.0f cm from the start, at least %.0f)"), Name, Farthest, FarCm),
					Farthest >= FarCm);
			}
		}
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	Radius->Set(Saved, ECVF_SetByCode);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartingNomadFirstForumTest, "HeroesOfLiberty.Match.StartingHero.NomadFirstForum",
	UE::HeroesOfLiberty::Tests::StartingHero::Flags)

// BUG nomad_start point 1 (the user's trial, 2026-11-14: "je ne pouvais pas construire de forum, je n'avais pas l'option"): a Nomad seat
// builds its first forum from the start (an exception to tier 1, the secondary centre's), for the secondary centre's price (D783: 150
// wood + 400 stone, HOLSecondaryCentreRules.as). Two worlds from the game mode's own start (SpawnSlotStart), standard then Nomad:
// - the builders' list of the slot (what the command grid shows): the main centre at age I for the Nomad only;
// - a placement through the players' orders (HOLPlayerOrders::Place, then the construction's step): a forum foundation of the slot
//   and the price taken from its stock for the Nomad; nothing placed, nothing paid for the standard seat (its forum stands);
// - once its foundation stands, the Nomad's list no longer shows it (one first forum).
bool FHOLStartingNomadFirstForumTest::RunTest(const FString& Parameters)
{
	constexpr int32 Slot = 0;
	const FTransform Start(FRotator(0.f, 90.f, 0.f), FVector(5000.0, 2000.0, 0.0));
	const FVector Spot(11000.0, 2000.0, 0.0);
	const auto Lists = [](const TArray<FHOLBuildable>& List)
	{
		// D797 (the user's setting, 2026-11-14: "le centre doit apparaître dans le palier 2"): page « Palier II » (age index 1, where
		// the secondary centre opens, GDD 11.1), cell C (10), defaults of hol.Nomad.ForumAge / hol.Nomad.ForumCell.
		return List.ContainsByPredicate([](const FHOLBuildable& Each) { return Each.TypeName == HOLBuildingTypeNames::MainCentre && Each.Age == 1 && Each.Cell == 10; });
	};
	for (int32 Way = 0; Way < 2; ++Way)
	{
		const bool bNomad = Way == 1;
		const TCHAR* Name = bNomad ? TEXT("Nomad") : TEXT("Standard");
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
		World->SetGameState(GameState);
		UHOLConstructionSubsystem* Construction = World->GetSubsystem<UHOLConstructionSubsystem>();
		UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
		if (TestNotNull(TEXT("construction"), Construction) && TestNotNull(TEXT("simulation"), Simulation)
			&& TestNotNull(TEXT("the script board"), UHOLSpecializationSubsystem::Find(World)))
		{
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			Config.bNomad = bNomad;
			AHeroesOfLibertyGameModeBase::SpawnSlotStart(*World, Config, Slot, Start);
			AHOLPlayerState* Player = World->SpawnActor<AHOLPlayerState>();
			if (TestNotNull(TEXT("player state"), Player))
			{
				Player->SetMatchSlot(Slot, Slot);
				Player->SetResources(FHOLResources(1000, 1000, 1000, 1000));
				const FHOLResources Before = Player->GetResources();
				TArray<FHOLBuildable> List;
				UHOLBuildingSettings::GetBuildableTypesForSlot(World, Slot, List);
				TestEqual(*FString::Printf(TEXT("%s: the forum in the builders' list, page Palier II, cell C"), Name), Lists(List), bNomad);
				// D797: the keys a player presses (HOLCommandGrid::PressKey, the builders' grid of a seat at tier 0): Z opens page II for
				// the Nomad only (its forum always buildable there), then C asks to build the forum; the Market (II A) stays locked.
				FHOLGridContext Grid;
				Grid.bBuilders = true;
				Grid.bOwnUnits = true;
				Grid.ReachedAge = 0;
				Grid.Buildable = List;
				const FHOLGridKeyResult PageTwo = HOLCommandGrid::PressKey(Grid, INDEX_NONE, 1);
				TestEqual(*FString::Printf(TEXT("%s: Z opens the page Palier II at tier 0"), Name), PageTwo.ChosenAge, bNomad ? 1 : INDEX_NONE);
				const FHOLGridKeyResult KeyC = HOLCommandGrid::PressKey(Grid, 1, 10);
				TestEqual(*FString::Printf(TEXT("%s: II C builds the forum"), Name), KeyC.Target == EHOLGridKeyTarget::Build && KeyC.TypeName == HOLBuildingTypeNames::MainCentre, bNomad);
				const FHOLGridKeyResult KeyA = HOLCommandGrid::PressKey(Grid, 1, 0);
				TestFalse(*FString::Printf(TEXT("%s: II A (the Market) still locked at tier 0"), Name), KeyA.Target == EHOLGridKeyTarget::Build);
				const FHOLGridKeyResult Clicked = HOLCommandGrid::ClickAge(Grid, INDEX_NONE, 1, 10);
				TestEqual(*FString::Printf(TEXT("%s: a click on II C builds the forum"), Name), Clicked.Target == EHOLGridKeyTarget::Build && Clicked.TypeName == HOLBuildingTypeNames::MainCentre, bNomad);
				const auto CountForums = [World]()
				{
					int32 Count = 0;
					for (TActorIterator<AHOLBuilding> It(World); It; ++It)
					{
						Count += !It->IsActorBeingDestroyed() && It->GetTypeName() == HOLBuildingTypeNames::MainCentre && It->GetOwnerPlayer() == Slot ? 1 : 0;
					}
					return Count;
				};
				const int32 ForumsBefore = CountForums();
				Construction->OnWorldBeginPlay(*World);
				HOLPlayerOrders::Place(*World, Slot, HOLBuildingTypeNames::MainCentre, Spot, {}, false);
				Simulation->GetOnStepStarted().Broadcast(1);
				const FHOLResources After = Player->GetResources();
				TestEqual(*FString::Printf(TEXT("%s: forums of the slot after the placement"), Name), CountForums(), ForumsBefore + (bNomad ? 1 : 0));
				TestEqual(*FString::Printf(TEXT("%s: wood paid"), Name), Before.Wood - After.Wood, bNomad ? 150 : 0);
				TestEqual(*FString::Printf(TEXT("%s: stone paid"), Name), Before.Stone - After.Stone, bNomad ? 400 : 0);
				TestEqual(*FString::Printf(TEXT("%s: food and gold untouched"), Name), Before.Food - After.Food + Before.Gold - After.Gold, 0);
				TestEqual(*FString::Printf(TEXT("%s: the forum's cost the grid shows"), Name),
					UHOLBuildingSettings::GetBuildCost(World, HOLBuildingTypeNames::MainCentre, Slot).ToString(),
					bNomad ? FHOLResources(Before.Food - After.Food, Before.Wood - After.Wood, Before.Stone - After.Stone, Before.Gold - After.Gold).ToString()
						: UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre)->Cost.ToString());
				UHOLBuildingSettings::GetBuildableTypesForSlot(World, Slot, List);
				TestFalse(*FString::Printf(TEXT("%s: its foundation placed, the forum leaves the list"), Name), Lists(List));
			}
		}
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartingNomadReplicatedTest, "HeroesOfLiberty.Match.StartingHero.NomadReplicated",
	UE::HeroesOfLiberty::Tests::StartingHero::Flags)

// D799 (answer A of Q-nomad-net): the server replicates each seat's Nomad state (one bit per seat in the game state), so a remote
// player's grid shows the forum cell open and its true price as on the host. Two worlds:
// - the server: the game mode's own start (SpawnSlotStart, Nomad option) for slot 0 marks the seat Nomad in its game state;
// - a client: its own game state and script board, never told by the game mode; it receives the server's bits as the replication
//   does (AHOLGameState::ApplyNomadSeats, the body of OnRep_NomadSeats). Before them: no forum in its builders' grid. After them:
//   for slot 0, page « Palier II » cell C builds the forum, not greyed, for the price the server computes (150 wood + 400 stone);
//   for slot 1 (not Nomad, no centre either): no forum cell, the table's price.
bool FHOLStartingNomadReplicatedTest::RunTest(const FString& Parameters)
{
	const auto MakeWorld = []()
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
		World->SetGameState(GameState);
		return World;
	};
	const auto ForumCell = [](UWorld* World, const int32 Slot)
	{
		TArray<FHOLBuildable> List;
		UHOLBuildingSettings::GetBuildableTypesForSlot(World, Slot, List);
		FHOLGridContext Grid;
		Grid.bBuilders = true;
		Grid.bOwnUnits = true;
		Grid.ReachedAge = 0;
		Grid.Buildable = List;
		return HOLCommandGrid::GetAgeBuilding(Grid, 1, 10);
	};
	UWorld* Server = MakeWorld();
	UWorld* Client = MakeWorld();
	AHOLGameState* ServerState = Server->GetGameState<AHOLGameState>();
	AHOLGameState* ClientState = Client->GetGameState<AHOLGameState>();
	if (TestNotNull(TEXT("server game state"), ServerState) && TestNotNull(TEXT("client game state"), ClientState)
		&& TestNotNull(TEXT("server board"), UHOLSpecializationSubsystem::Find(Server)) && TestNotNull(TEXT("client board"), UHOLSpecializationSubsystem::Find(Client)))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		Config.bNomad = true;
		AHeroesOfLibertyGameModeBase::SpawnSlotStart(*Server, Config, 0, FTransform(FRotator::ZeroRotator, FVector(5000.0, 2000.0, 0.0)));
		TestTrue(TEXT("server: slot 0 Nomad in the game state"), ServerState->IsSeatNomad(0));
		TestFalse(TEXT("server: slot 1 not Nomad"), ServerState->IsSeatNomad(1));
		const FHOLResources ServerCost = UHOLBuildingSettings::GetBuildCost(Server, HOLBuildingTypeNames::MainCentre, 0);
		TestEqual(TEXT("server: the forum's price, wood"), ServerCost.Wood, 150);
		TestEqual(TEXT("server: the forum's price, stone"), ServerCost.Stone, 400);

		TestFalse(TEXT("client before the replication: no forum in cell II C"), ForumCell(Client, 0).TypeName == HOLBuildingTypeNames::MainCentre);
		ClientState->ApplyNomadSeats(ServerState->GetNomadSeats(), ServerState->GetNomadForumSeats());
		TestTrue(TEXT("client: slot 0 Nomad"), ClientState->IsSeatNomad(0));
		const FHOLGridCommand Nomad = ForumCell(Client, 0);
		TestTrue(TEXT("client, slot 0: cell II C builds the forum"), Nomad.Command == EHOLGridCommand::Build && Nomad.TypeName == HOLBuildingTypeNames::MainCentre);
		TestFalse(TEXT("client, slot 0: the forum cell active (not greyed)"), Nomad.bGreyed);
		const FHOLResources ClientCost = UHOLBuildingSettings::GetBuildCost(Client, HOLBuildingTypeNames::MainCentre, 0);
		TestEqual(TEXT("client, slot 0: the forum's price shown, the server's"), ClientCost.ToString(), ServerCost.ToString());
		TestEqual(TEXT("client, slot 0: 150 wood"), ClientCost.Wood, 150);
		TestEqual(TEXT("client, slot 0: 400 stone"), ClientCost.Stone, 400);
		const FHOLGridCommand Standard = ForumCell(Client, 1);
		TestFalse(TEXT("client, slot 1 (standard): no forum in cell II C"), Standard.TypeName == HOLBuildingTypeNames::MainCentre);
		TestEqual(TEXT("client, slot 1: the table's forum price"), UHOLBuildingSettings::GetBuildCost(Client, HOLBuildingTypeNames::MainCentre, 1).ToString(),
			UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre)->Cost.ToString());
		// D354 ("from the first forum complete, the normal rules"; the forum then falls to ruins and is rebuilt in place, D417-D420):
		// the server notes slot 0's first forum complete (what HOLBuildProcessor calls when it finishes); the client receives the bits
		// as the replication does: its board knows the forum complete, the cell II C closed, the forum's price back to the table's.
		ServerState->NoteSeatForumComplete(0);
		TestTrue(TEXT("server: slot 0's first forum complete"), UHOLSpecializationSubsystem::IsForumComplete(Server, 0));
		const FHOLResources ServerAfter = UHOLBuildingSettings::GetBuildCost(Server, HOLBuildingTypeNames::MainCentre, 0);
		ClientState->ApplyNomadSeats(ServerState->GetNomadSeats(), ServerState->GetNomadForumSeats());
		TestTrue(TEXT("client: slot 0 still Nomad"), ClientState->IsSeatNomad(0));
		TestTrue(TEXT("client: slot 0's first forum complete"), UHOLSpecializationSubsystem::IsForumComplete(Client, 0));
		TestFalse(TEXT("client: slot 0's forum cell closed"), UHOLBuildingSettings::IsFirstForumOpen(Client, 0));
		TestFalse(TEXT("client, slot 0 after its first forum: no forum in cell II C"), ForumCell(Client, 0).TypeName == HOLBuildingTypeNames::MainCentre);
		const FHOLResources ClientAfter = UHOLBuildingSettings::GetBuildCost(Client, HOLBuildingTypeNames::MainCentre, 0);
		TestEqual(TEXT("client, slot 0 after its first forum: the server's price"), ClientAfter.ToString(), ServerAfter.ToString());
		TestEqual(TEXT("client, slot 0 after its first forum: the table's price"), ClientAfter.ToString(),
			UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre)->Cost.ToString());
		TestFalse(TEXT("client, slot 0 after its first forum: no longer the first forum's 150 wood + 400 stone"), ClientAfter.Wood == 150 && ClientAfter.Stone == 400);
		// The server takes the seat back (development command hol.Nomad.SetSeat): the client's grid follows.
		ServerState->SetSeatNomad(0, false);
		TestEqual(TEXT("server: the seat's forum bit cleared with it"), static_cast<int32>(ServerState->GetNomadForumSeats()), 0);
		ClientState->ApplyNomadSeats(ServerState->GetNomadSeats(), ServerState->GetNomadForumSeats());
		TestFalse(TEXT("client, slot 0 no longer Nomad: no forum cell"), ForumCell(Client, 0).TypeName == HOLBuildingTypeNames::MainCentre);
	}
	for (UWorld* World : { Server, Client })
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	return true;
}

#endif
