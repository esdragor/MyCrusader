// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLProductionCommand.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "Match/HOLFactionRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLStartRules.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// L0 (LegionsInProgress.md, LQ-7, D677): the multi-faction base, on the real chain. The roles and the overrides come from the
// tables' text (Tools/Data/Factions.csv and FactionOverrides.csv; the cases that need a faction to differ from the Dawn use a
// fixture text with made-up types and figures, the real tables hold no Legions content before L1 and L2), the rows from the
// real unit and building type tables, the effects through the real subsystems (cost, start, production order).
namespace UE::HeroesOfLiberty::Tests::Factions
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Fixture tables: the Legions field a Spearman as their Worker and a ManAtArms as their Hero, and override the Spearman's name and cost (made-up figures). */
	const TCHAR* FixtureRoles = TEXT(R"csv(Name,Faction,Role,Type
Dawn_Worker,Dawn,Worker,Peasant
Dawn_Hero,Dawn,Hero,Hero
Dawn_MainCentre,Dawn,MainCentre,MainCentre
Legions_Worker,Legions,Worker,Spearman
Legions_Hero,Legions,Hero,ManAtArms
Legions_MainCentre,Legions,MainCentre,MainCentre
)csv");

	const TCHAR* FixtureOverrides = TEXT(R"csv(Name,Faction,Type,Field,Value
Legions_Spearman_Name,Legions,Spearman,DisplayName,Fixture Legion Spearman
Legions_Spearman_Cost,Legions,Spearman,Cost,"(Food=0,Wood=7,Stone=0,Gold=9)"
)csv");

	/** L2: the fixture's overrides plus a health and an attack of the Legions' Spearman (made-up figures), for the births. */
	const TCHAR* FixtureBirthOverrides = TEXT(R"csv(Name,Faction,Type,Field,Value
Legions_Spearman_Name,Legions,Spearman,DisplayName,Fixture Legion Spearman
Legions_Spearman_Cost,Legions,Spearman,Cost,"(Food=0,Wood=7,Stone=0,Gold=9)"
Legions_Spearman_Health,Legions,Spearman,MaxHealth,777
)csv");

	bool LoadData(FAutomationTestBase& Test, const TCHAR* File, FString& OutText)
	{
		const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Data"), File);
		return Test.TestTrue(FString::Printf(TEXT("%s is there"), *Path), FFileHelper::LoadFileToString(OutText, *Path));
	}

	/** The fixture tables, read by the real parsers, handed to the rules for the life of the object. */
	struct FFixtureTable
	{
		HOLFactionRules::FTable Table;
		TArray<FString> Problems;

		explicit FFixtureTable(const TCHAR* OverridesText = FixtureOverrides)
		{
			HOLFactionRules::ParseRoles(FixtureRoles, Table.Roles, Problems);
			HOLFactionRules::ParseOverrides(OverridesText, Table.Overrides, Problems);
			HOLFactionRules::SetTableForTests(&Table);
		}

		~FFixtureTable()
		{
			HOLFactionRules::SetTableForTests(nullptr);
		}
	};

	const FName Dawn(TEXT("Dawn"));
	const FName Legions(TEXT("Legions"));
	const FName Spearman(TEXT("Spearman"));
	const FName Peasant(TEXT("Peasant"));
	const FName Monk(TEXT("Monk"));
	const FName Chapel(TEXT("Chapel"));

	/** Slot 0 an AI of the Dawn, slot 1 an AI of the Legions, the rest closed. */
	FHOLMatchConfig MakeConfig()
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index <= 1 ? EHOLSlotState::AI : EHOLSlotState::Closed;
			Config.Slots[Index].ColorIndex = Index;
			Config.Slots[Index].Faction = Index == 1 ? EHOLFaction::BlackLegions : EHOLFaction::DawnOrder;
		}
		return Config;
	}

	/** A match's server for two AI slots (Dawn 0, Legions 1), each with a finished main centre, the construction and the production bound to the steps. */
	struct FWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* State = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLCommandSubsystem* Commands = nullptr;
		UHOLProductionSubsystem* Production = nullptr;
		AHOLBuilding* Centre[2] = { nullptr, nullptr };

		FWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			State = World->SpawnActor<AHOLGameState>();
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Commands = World->GetSubsystem<UHOLCommandSubsystem>();
			Production = World->GetSubsystem<UHOLProductionSubsystem>();
			UHOLConstructionSubsystem* Construction = World->GetSubsystem<UHOLConstructionSubsystem>();
			UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			if (!State || !Units || !Commands || !Production || !Construction || !Sources || !World->GetSubsystem<UHOLSimulationSubsystem>())
			{
				return;
			}
			const FHOLMatchConfig Config = MakeConfig();
			State->InitVictory(Config);
			AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
			Sources->OnWorldBeginPlay(*World);
			Construction->OnWorldBeginPlay(*World);
			Production->OnWorldBeginPlay(*World);
			for (int32 Slot = 0; Slot < 2; ++Slot)
			{
				Centre[Slot] = World->SpawnActor<AHOLBuilding>(FVector(Slot * -20000.0, 0.0, 0.0), FRotator::ZeroRotator);
				if (Centre[Slot])
				{
					Centre[Slot]->Setup(FHOLStartRules::GetMainCentreTypeName(Config, Slot), static_cast<uint8>(Slot), static_cast<uint8>(Slot));
				}
			}
		}

		~FWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Centre[0] && Centre[1]; }

		void RunStep(const int64 StepIndex = 1) { World->GetSubsystem<UHOLSimulationSubsystem>()->GetOnStepStarted().Broadcast(StepIndex); }

		AHOLPlayerState* FindPlayerState(const int32 Slot) const
		{
			for (APlayerState* Each : State->PlayerArray)
			{
				AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
				if (HOLEach && HOLEach->GetPlayerIndex() == Slot)
				{
					return HOLEach;
				}
			}
			return nullptr;
		}

		int32 CountUnits(const FName Type, const int32 Owner) const
		{
			TArray<uint32> Ids;
			Units->GetAllUnitIds(Ids);
			int32 Count = 0;
			for (const uint32 Id : Ids)
			{
				FHOLUnitView View;
				Count += Units->GetUnitView(Id, View) && View.OwnerPlayer == Owner && View.TypeName == Type ? 1 : 0;
			}
			return Count;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFactionTablesTest, "HeroesOfLiberty.Match.Factions.Tables", UE::HeroesOfLiberty::Tests::Factions::Flags)
bool FHOLFactionTablesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Factions;

	// 1. The real files read without a problem; they hold the Dawn's three roles and the Legions' (provisional until L1 and L8a).
	FString RolesText;
	FString OverridesText;
	if (!LoadData(*this, TEXT("Factions.csv"), RolesText) || !LoadData(*this, TEXT("FactionOverrides.csv"), OverridesText))
	{
		return false;
	}
	HOLFactionRules::FTable Real;
	TArray<FString> Problems;
	HOLFactionRules::ParseRoles(RolesText, Real.Roles, Problems);
	HOLFactionRules::ParseOverrides(OverridesText, Real.Overrides, Problems);
	for (const FString& Problem : Problems)
	{
		AddError(Problem);
	}
	TestEqual(TEXT("Dawn Worker"), Real.FindRoleType(Dawn, HOLFactionRules::Roles::Worker), Peasant);
	TestEqual(TEXT("Dawn Hero"), Real.FindRoleType(Dawn, HOLFactionRules::Roles::Hero), FName(TEXT("Hero")));
	TestEqual(TEXT("Dawn MainCentre"), Real.FindRoleType(Dawn, HOLFactionRules::Roles::MainCentre), FName(TEXT("MainCentre")));
	TestTrue(TEXT("the Legions have rows of their own"), Real.HasFaction(Legions));
	// L1: the Legions' Worker is the Zombie; their Hero stays the Dawn's until L8a (Damned Lord).
	TestEqual(TEXT("Legions Worker: the Zombie (L1)"), Real.FindRoleType(Legions, HOLFactionRules::Roles::Worker), FName(TEXT("Zombie")));
	TestEqual(TEXT("Legions Hero, provisional until L8a"), Real.FindRoleType(Legions, HOLFactionRules::Roles::Hero), FName(TEXT("Hero")));

	// 2. The figures built in (a packaged game) are the files'.
	const HOLFactionRules::FTable BuiltIn = HOLFactionRules::GetBuiltInTable();
	TestEqual(TEXT("built-in roles: the same number of rows"), BuiltIn.Roles.Num(), Real.Roles.Num());
	for (const HOLFactionRules::FRoleRow& Row : Real.Roles)
	{
		TestEqual(*FString::Printf(TEXT("built-in %s"), *Row.Name.ToString()), BuiltIn.FindRoleType(Row.Faction, Row.Role), Row.Type);
	}
	TestEqual(TEXT("built-in overrides: the files'"), BuiltIn.Overrides.Num(), Real.Overrides.Num());
	for (const HOLFactionRules::FOverrideRow& Row : Real.Overrides)
	{
		const bool bSame = BuiltIn.Overrides.ContainsByPredicate([&Row](const HOLFactionRules::FOverrideRow& Each)
			{ return Each.Name == Row.Name && Each.Faction == Row.Faction && Each.Type == Row.Type && Each.Field == Row.Field && Each.Value == Row.Value; });
		TestTrue(*FString::Printf(TEXT("built-in override %s is the file's (run Tools/Data/make_builtin_overrides.py)"), *Row.Name.ToString()), bSame);
	}

	// 3. The fixture, by the same parsers: the Legions differ from the Dawn, a faction with no rows reads as the Dawn.
	{
		FFixtureTable Fixture;
		for (const FString& Problem : Fixture.Problems)
		{
			AddError(Problem);
		}
		const HOLFactionRules::FTable& Table = HOLFactionRules::GetTable();
		TestEqual(TEXT("fixture: Dawn Worker"), Table.FindRoleType(Dawn, HOLFactionRules::Roles::Worker), Peasant);
		TestEqual(TEXT("fixture: Legions Worker"), Table.FindRoleType(Legions, HOLFactionRules::Roles::Worker), Spearman);
		const FName Templars(TEXT("Templars"));
		TestEqual(TEXT("a faction with no rows reads as the Dawn"), Table.ResolveKey(Templars), Dawn);
		TestEqual(TEXT("... for its Worker too"), Table.FindRoleType(Templars, HOLFactionRules::Roles::Worker), Peasant);
		TestTrue(TEXT("Common is for everyone"), Table.Allows(HOLFactionRules::Common, Legions));
		TestFalse(TEXT("a Dawn-only type is not the Legions'"), Table.Allows(Dawn, Legions));
		TestTrue(TEXT("a Dawn-only type is the Dawn's"), Table.Allows(Dawn, Dawn));
		TestTrue(TEXT("a list names several"), Table.Allows(FName(TEXT("Legions;Dawn")), Legions));
		TestTrue(TEXT("a faction with no rows reads as the Dawn: it has the Dawn-only type"), Table.Allows(Dawn, Templars));
		TestTrue(TEXT("the Legions override the Spearman"), Table.HasOverride(Legions, Spearman));
		TestFalse(TEXT("the Dawn does not"), Table.HasOverride(Dawn, Spearman));

		// The same through the roles of a match: a Legions slot starts with the Legions' Worker.
		const FHOLMatchConfig Config = MakeConfig();
		TestEqual(TEXT("slot 0 (Dawn) starts with Peasants"), FHOLStartRules::GetWorkerTypeName(Config, 0), Peasant);
		TestEqual(TEXT("slot 1 (Legions) starts with the Legions' Worker"), FHOLStartRules::GetWorkerTypeName(Config, 1), Spearman);
		TestEqual(TEXT("slot 1 (Legions) starts with the Legions' Hero"), FHOLStartRules::GetHeroTypeName(Config, 1), FName(TEXT("ManAtArms")));
		TestEqual(TEXT("slot 0 (Dawn) starts with its Hero"), FHOLStartRules::GetHeroTypeName(Config, 0), FHOLStartRules::HeroTypeName);
		TestEqual(TEXT("slot 5 (closed, Dawn by default) reads as the Dawn"), FHOLStartRules::GetWorkerTypeName(Config, 5), Peasant);
	}

	// 4. The shape: a bad header, a repeated role and an unknown role are problems, not silently dropped; so is a field that does not exist.
	{
		TArray<HOLFactionRules::FRoleRow> Rows;
		TArray<FString> Wrong;
		HOLFactionRules::ParseRoles(TEXT("Name,Faction,Kind,Type\nA,Dawn,Worker,Peasant\n"), Rows, Wrong);
		TestFalse(TEXT("a wrong header is a problem"), Wrong.IsEmpty());
		Wrong.Reset();
		HOLFactionRules::ParseRoles(TEXT("Name,Faction,Role,Type\nA,Dawn,Worker,Peasant\nB,Dawn,Worker,Zombie\nC,Dawn,Cook,Peasant\n"), Rows, Wrong);
		TestEqual(TEXT("a repeated role and an unknown role: two problems"), Wrong.Num(), 2);
		TestEqual(TEXT("... and the good row is kept"), Rows.Num(), 1);

		FFixtureTable Fixture;
		FHOLUnitTypeRow Row;
		TArray<FString> FieldProblems;
		HOLFactionRules::FTable Bad;
		Bad.Overrides.Add({ FName(TEXT("X")), Legions, Spearman, FName(TEXT("NoSuchField")), TEXT("1") });
		Bad.Overrides.Add({ FName(TEXT("Y")), Legions, Spearman, FName(TEXT("MaxHealth")), TEXT("123") });
		TestEqual(TEXT("one override applies, the unknown field does not"), Bad.ApplyOverrides(Legions, Spearman, FHOLUnitTypeRow::StaticStruct(), &Row, &FieldProblems), 1);
		TestEqual(TEXT("... and is named"), FieldProblems.Num(), 1);
		TestEqual(TEXT("the field is written"), Row.MaxHealth, 123);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFactionRealChainTest, "HeroesOfLiberty.Match.Factions.RealChain", UE::HeroesOfLiberty::Tests::Factions::Flags)
bool FHOLFactionRealChainTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Factions;
	FWorld Test;
	if (!TestTrue(TEXT("the test world stands"), Test.IsReady()))
	{
		return false;
	}
	const FHOLUnitTypeRow* BaseSpearman = Test.Units->FindTypeRow(Spearman);
	if (!TestNotNull(TEXT("the real unit type table has a Spearman"), BaseSpearman))
	{
		return false;
	}
	const FHOLResources FixtureCost(0, 7, 0, 9);
	TestTrue(TEXT("the fixture's cost is not the table's (the test can tell them apart)"), BaseSpearman->Cost != FixtureCost);

	// (3) FindTypeRow(Spearman, Legions): the override's name and cost; the Dawn's row is the table's own.
	{
		FFixtureTable Fixture;
		const FHOLUnitTypeRow* LegionsSpearman = Test.Units->FindTypeRow(Spearman, Legions);
		if (TestNotNull(TEXT("the Legions' Spearman"), LegionsSpearman))
		{
			TestEqual(TEXT("the Legions' Spearman: the override's name"), LegionsSpearman->DisplayName, FString(TEXT("Fixture Legion Spearman")));
			TestTrue(TEXT("the Legions' Spearman: the override's cost"), LegionsSpearman->Cost == FixtureCost);
			TestEqual(TEXT("the Legions' Spearman: what is not overridden stays the table's (health)"), LegionsSpearman->MaxHealth, BaseSpearman->MaxHealth);
		}
		TestTrue(TEXT("the Dawn's Spearman is the table's own row"), Test.Units->FindTypeRow(Spearman, Dawn) == BaseSpearman);
		TestTrue(TEXT("a faction with no rows reads as the Dawn"), Test.Units->FindTypeRow(Spearman, FName(TEXT("Templars"))) == BaseSpearman);
		TestFalse(TEXT("the table's own row is untouched by the Legions'"), BaseSpearman->Cost == FixtureCost);

		// The cost a player pays: the production's, by the payer's faction (slot 0 Dawn, slot 1 Legions).
		TestTrue(TEXT("slot 1 (Legions) pays the override's cost"), Test.Production->GetCost(Spearman, 1).Resources == FixtureCost);
		TestTrue(TEXT("slot 0 (Dawn) pays the table's"), Test.Production->GetCost(Spearman, 0).Resources == BaseSpearman->Cost);
		TestTrue(TEXT("no payer: the table's"), Test.Production->GetCost(Spearman).Resources == BaseSpearman->Cost);

		// (1) The start: slot 1's workers are the Legions' Worker (the fixture's Spearman), slot 0's the Dawn's Peasant.
		TestTrue(TEXT("Peasants asked for slot 0"), AHeroesOfLibertyGameModeBase::SpawnStartingPeasants(*Test.World, 0, *Test.Centre[0], 3));
		TestTrue(TEXT("workers asked for slot 1"), AHeroesOfLibertyGameModeBase::SpawnStartingPeasants(*Test.World, 1, *Test.Centre[1], 3));
		Test.Units->ServeBirthRequests();
		TestEqual(TEXT("slot 0 starts with 3 Peasants"), Test.CountUnits(Peasant, 0), 3);
		TestEqual(TEXT("slot 1 starts with 3 of the Legions' Worker"), Test.CountUnits(Spearman, 1), 3);
		TestEqual(TEXT("slot 1 starts with no Peasant"), Test.CountUnits(Peasant, 1), 0);
	}

	// With the real tables (L2, Q-Legions-RosterValues = A): the 7 types of the roster differ for the Legions, by a name and a cost of
	// their own (the cost under the Dawn's), and by nothing else; the others are the table's own rows.
	{
		TArray<FName> Names;
		Test.Units->GetTypeNames(Names);
		int32 Different = 0;
		for (const FName Name : Names)
		{
			Different += Test.Units->FindTypeRow(Name, Legions) != Test.Units->FindTypeRow(Name) ? 1 : 0;
		}
		TestEqual(TEXT("seven unit types differ for the Legions (the roster)"), Different, 7);
		const TCHAR* const RosterNames[][2] = { { TEXT("Spearman"), TEXT("Legion Spearman") }, { TEXT("ManAtArms"), TEXT("Black Guard") },
			{ TEXT("Archer"), TEXT("Legion Archer") }, { TEXT("Crossbowman"), TEXT("Legion Crossbowman") }, { TEXT("LightCavalry"), TEXT("Dread Rider") },
			{ TEXT("Ram"), TEXT("Bone Ram") }, { TEXT("Mangonel"), TEXT("Bone Mangonel") } };
		for (const TCHAR* const* Pair : RosterNames)
		{
			const FName Type(Pair[0]);
			const FHOLUnitTypeRow* Dawns = Test.Units->FindTypeRowFor(Type, 0);
			const FHOLUnitTypeRow* Theirs = Test.Units->FindTypeRowFor(Type, 1);
			if (TestNotNull(*FString::Printf(TEXT("%s: both rows"), Pair[0]), Dawns) && TestNotNull(*FString::Printf(TEXT("%s: the Legions' row"), Pair[0]), Theirs))
			{
				TestEqual(*FString::Printf(TEXT("%s: the Legions' name"), Pair[0]), Theirs->DisplayName, FString(Pair[1]));
				TestTrue(*FString::Printf(TEXT("%s: the Dawn keeps its name"), Pair[0]), Dawns->DisplayName != Theirs->DisplayName);
				const auto Total = [](const FHOLResources& Cost) { return Cost.Food + Cost.Wood + Cost.Stone + Cost.Gold; };
				TestTrue(*FString::Printf(TEXT("%s: the Legions' cost is under the Dawn's"), Pair[0]), Total(Theirs->Cost) < Total(Dawns->Cost));
				TestEqual(*FString::Printf(TEXT("%s: the Legions' health is the Dawn's"), Pair[0]), Theirs->MaxHealth, Dawns->MaxHealth);
			}
		}
		TestEqual(TEXT("slot 1 (Legions) has the Zombie as Worker, by the game state's configuration"), HOLFactionRules::GetRoleTypeName(Test.World, 1, HOLFactionRules::Roles::Worker), FName(TEXT("Zombie")));
		TestEqual(TEXT("slot 1 (Legions, provisional Hero until L8a) has the Dawn's"), HOLFactionRules::GetRoleTypeName(Test.World, 1, HOLFactionRules::Roles::Hero), FName(TEXT("Hero")));
	}

	// (2) The grid: the Chapel's Monk is the Dawn's; the Chapel and the Sanctuary are built by the Dawn.
	{
		const FHOLBuildingTypeRow* ChapelRow = UHOLBuildingSettings::FindType(Chapel);
		if (TestNotNull(TEXT("the real building type table has a Chapel"), ChapelRow))
		{
			TestTrue(TEXT("the Chapel produces the Monk"), ChapelRow->Produces.Contains(Monk));
			TestTrue(TEXT("the Dawn's Chapel grid shows the Monk"), HOLFactionRules::FilterProduces(Test.World, ChapelRow->Produces, 0).Contains(Monk));
			TestFalse(TEXT("the Legions' Chapel grid does not show the Monk"), HOLFactionRules::FilterProduces(Test.World, ChapelRow->Produces, 1).Contains(Monk));
		}
		TArray<FHOLBuildable> DawnBuilds;
		TArray<FHOLBuildable> LegionsBuilds;
		UHOLBuildingSettings::GetBuildableTypes(DawnBuilds, Dawn);
		UHOLBuildingSettings::GetBuildableTypes(LegionsBuilds, Legions);
		const auto Has = [](const TArray<FHOLBuildable>& List, const FName Name)
		{
			return List.ContainsByPredicate([Name](const FHOLBuildable& Each) { return Each.TypeName == Name; });
		};
		for (const FName Name : { Chapel, FName(TEXT("Sanctuary")) })
		{
			TestTrue(*FString::Printf(TEXT("the Dawn builds the %s"), *Name.ToString()), Has(DawnBuilds, Name));
			TestFalse(*FString::Printf(TEXT("the Legions' builders' grid has no %s"), *Name.ToString()), Has(LegionsBuilds, Name));
		}
		TestTrue(TEXT("the Legions' builders' grid keeps the House"), Has(LegionsBuilds, HOLBuildingTypeNames::House));
		TestTrue(TEXT("no faction given: everything buildable"), [&]() { TArray<FHOLBuildable> All; UHOLBuildingSettings::GetBuildableTypes(All); return All.Num() == DawnBuilds.Num(); }());
		TestFalse(TEXT("the Legions do not build the Chapel (server rule)"), HOLFactionRules::IsBuildingOpenTo(Test.World, Chapel, 1));
		TestTrue(TEXT("the Dawn does"), HOLFactionRules::IsBuildingOpenTo(Test.World, Chapel, 0));
	}

	// (4) The server refuses a unit type of another faction, whatever a client asked: a Monk at slot 1's Chapel is not queued; at slot 0's it is.
	{
		AHOLBuilding* Chapels[2] = { nullptr, nullptr };
		for (int32 Slot = 0; Slot < 2; ++Slot)
		{
			Chapels[Slot] = Test.World->SpawnActor<AHOLBuilding>(FVector(Slot * -20000.0 + 4000.0, 0.0, 0.0), FRotator::ZeroRotator);
			if (!TestNotNull(TEXT("a Chapel"), Chapels[Slot]))
			{
				return false;
			}
			Chapels[Slot]->Setup(Chapel, static_cast<uint8>(Slot), static_cast<uint8>(Slot));
			AHOLPlayerState* Payer = Test.FindPlayerState(Slot);
			if (!TestNotNull(TEXT("the slot's player state"), Payer))
			{
				return false;
			}
			Payer->SetResources(FHOLResources(5000, 5000, 5000, 5000));
			FHOLProductionCommand Command;
			Command.PlayerIndex = Slot;
			Command.Building = Chapels[Slot];
			Command.TypeName = Monk;
			Test.Commands->IssueProductionOrder(Command);
		}
		Test.RunStep();
		TestEqual(TEXT("the Dawn's Chapel queued the Monk"), Chapels[0]->GetProductionQueue().Num(), 1);
		TestEqual(TEXT("the Legions' Chapel refused it"), Chapels[1]->GetProductionQueue().Num(), 0);
	}
	return true;
}

// L1 (LegionsInProgress.md, LQ-8, D66): the Zombie replaces the Peasant for the Legions, on the real tables (no fixture): the Legions
// start with Zombies, their centre produces Zombies and no Peasant, the Dawn's the opposite; the Zombie is cheaper than the Peasant,
// costs no food, gathers slower and takes one population place. The figures are the provisional ones of the CSV (Q-Legions-ZombieValues).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZombieWorkerTest, "HeroesOfLiberty.Match.Factions.Zombie", UE::HeroesOfLiberty::Tests::Factions::Flags)
bool FHOLZombieWorkerTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Factions;
	const FName Zombie(TEXT("Zombie"));
	FWorld Test;
	if (!TestTrue(TEXT("the test world stands"), Test.IsReady()))
	{
		return false;
	}
	const FHOLUnitTypeRow* ZombieRow = Test.Units->FindTypeRow(Zombie);
	const FHOLUnitTypeRow* PeasantRow = Test.Units->FindTypeRow(Peasant);
	if (!TestNotNull(TEXT("the unit type table has a Zombie"), ZombieRow) || !TestNotNull(TEXT("... and a Peasant"), PeasantRow))
	{
		return false;
	}

	// The row: a worker (it carries), of the Legions only, cheaper, no food, slower, one place.
	TestTrue(TEXT("the Zombie carries what it gathers (a worker)"), ZombieRow->CarryCapacity > 0);
	TestEqual(TEXT("the Zombie is the Legions' only"), ZombieRow->Faction, Legions);
	TestEqual(TEXT("the Peasant is the Dawn's only"), PeasantRow->Faction, Dawn);
	TestEqual(TEXT("the Zombie costs no food (LQ-8)"), ZombieRow->Cost.Food, 0);
	TestTrue(TEXT("the Zombie is free (the user's answer of 2026-11-05 to Q-Legions-ZombieValues)"), ZombieRow->Cost == FHOLResources());
	TestTrue(TEXT("the Zombie is cheaper than the Peasant (D66)"), ZombieRow->Cost.Food + ZombieRow->Cost.Wood + ZombieRow->Cost.Stone + ZombieRow->Cost.Gold
		< PeasantRow->Cost.Food + PeasantRow->Cost.Wood + PeasantRow->Cost.Stone + PeasantRow->Cost.Gold);
	TestEqual(TEXT("the Zombie has 35 health (the user's answer)"), ZombieRow->MaxHealth, 35);
	TestEqual(TEXT("the Zombie walks at the Peasant's speed"), ZombieRow->Speed, PeasantRow->Speed);
	TestEqual(TEXT("the Zombie takes 20 s to produce"), ZombieRow->BuildTime, 20.f);
	TestEqual(TEXT("the Zombie takes one population place (D66)"), ZombieRow->Population, 1);
	TestTrue(TEXT("the Zombie gathers wood slower than the Peasant"), ZombieRow->GatherRates.Wood < PeasantRow->GatherRates.Wood);
	TestTrue(TEXT("... and stone"), ZombieRow->GatherRates.Stone < PeasantRow->GatherRates.Stone);
	TestTrue(TEXT("... and gold"), ZombieRow->GatherRates.Gold < PeasantRow->GatherRates.Gold);
	TestTrue(TEXT("... and food"), ZombieRow->GatherRates.Food < PeasantRow->GatherRates.Food);

	// The roles of the real table, by the real start of a match.
	const FHOLMatchConfig Config = MakeConfig();
	TestEqual(TEXT("slot 1 (Legions) has the Zombie as Worker"), FHOLStartRules::GetWorkerTypeName(Config, 1), Zombie);
	TestEqual(TEXT("slot 0 (Dawn) has the Peasant as Worker"), FHOLStartRules::GetWorkerTypeName(Config, 0), Peasant);
	TestTrue(TEXT("Peasants asked for slot 0"), AHeroesOfLibertyGameModeBase::SpawnStartingPeasants(*Test.World, 0, *Test.Centre[0], 3));
	TestTrue(TEXT("workers asked for slot 1"), AHeroesOfLibertyGameModeBase::SpawnStartingPeasants(*Test.World, 1, *Test.Centre[1], 3));
	Test.Units->ServeBirthRequests();
	TestEqual(TEXT("the Legions start with 3 Zombies"), Test.CountUnits(Zombie, 1), 3);
	TestEqual(TEXT("... and no Peasant"), Test.CountUnits(Peasant, 1), 0);
	TestEqual(TEXT("the Dawn starts with 3 Peasants"), Test.CountUnits(Peasant, 0), 3);
	TestEqual(TEXT("... and no Zombie"), Test.CountUnits(Zombie, 0), 0);

	// The centre's grid and the server's rule: Zombie for the Legions, Peasant for the Dawn.
	const FHOLBuildingTypeRow* CentreRow = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	if (TestNotNull(TEXT("the main centre's row"), CentreRow))
	{
		TestTrue(TEXT("the Legions' centre grid: the Zombie alone"), HOLFactionRules::FilterProduces(Test.World, CentreRow->Produces, 1) == TArray<FName>({ Zombie }));
		TestTrue(TEXT("the Dawn's centre grid: the Peasant alone"), HOLFactionRules::FilterProduces(Test.World, CentreRow->Produces, 0) == TArray<FName>({ Peasant }));
	}
	for (int32 Slot = 0; Slot < 2; ++Slot)
	{
		AHOLPlayerState* Payer = Test.FindPlayerState(Slot);
		if (!TestNotNull(TEXT("the slot's player state"), Payer))
		{
			return false;
		}
		Payer->SetResources(FHOLResources(5000, 5000, 5000, 5000));
		for (const FName Type : { Peasant, Zombie })
		{
			FHOLProductionCommand Command;
			Command.PlayerIndex = Slot;
			Command.Building = Test.Centre[Slot];
			Command.TypeName = Type;
			Test.Commands->IssueProductionOrder(Command);
		}
	}
	Test.RunStep();
	TestEqual(TEXT("the Dawn's centre queued one worker (the Peasant)"), Test.Centre[0]->GetProductionQueue().Num(), 1);
	TestEqual(TEXT("the Legions' centre queued one worker (the Zombie)"), Test.Centre[1]->GetProductionQueue().Num(), 1);
	if (Test.Centre[0]->GetProductionQueue().Num() == 1 && Test.Centre[1]->GetProductionQueue().Num() == 1)
	{
		TestEqual(TEXT("... which is a Peasant for the Dawn"), Test.Centre[0]->GetProductionQueue()[0].TypeName, Peasant);
		TestEqual(TEXT("... and a Zombie for the Legions"), Test.Centre[1]->GetProductionQueue()[0].TypeName, Zombie);
	}

	// A free worker is no unlimited one: with an empty stock the Legions' centre takes the free orders (nothing is paid), but its queue
	// stops at its length (MaxQueue), and the population cap holds at each step (HOLProduction::Advance, whatever the cost).
	Test.FindPlayerState(1)->SetResources(FHOLResources());
	for (int32 Index = 0; Index < 12; ++Index)
	{
		FHOLProductionCommand Command;
		Command.PlayerIndex = 1;
		Command.Building = Test.Centre[1];
		Command.TypeName = Zombie;
		Test.Commands->IssueProductionOrder(Command);
	}
	Test.RunStep();
	if (CentreRow)
	{
		TestEqual(TEXT("free Zombies: the queue stops at the centre's MaxQueue"), Test.Centre[1]->GetProductionQueue().Num(), CentreRow->MaxQueue);
	}
	FHOLResources Stock;
	TestTrue(TEXT("free Zombies: nothing was paid"), AHOLGameState::GetSlotStock(*Test.World, 1, Stock) && Stock == FHOLResources());
	return true;
}

// L2 (LegionsInProgress.md, LQ-7): the effect of a faction's overrides, through the real chain: a Spearman ordered at a Barracks costs
// its payer's faction's price (the Legions' 45 food 15 wood, the Dawn's own), takes its faction's time and places, and is BORN with its
// faction's health (a made-up 777 in the fixture; the Dawn's stays the table's), and the Legions' own template is built for it only.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFactionRosterTest, "HeroesOfLiberty.Match.Factions.Roster", UE::HeroesOfLiberty::Tests::Factions::Flags)
bool FHOLFactionRosterTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Factions;
	FWorld Test;
	if (!TestTrue(TEXT("the test world stands"), Test.IsReady()))
	{
		return false;
	}
	const FName Barracks(TEXT("Barracks"));
	const FHOLUnitTypeRow* BaseSpearman = Test.Units->FindTypeRow(Spearman);
	if (!TestNotNull(TEXT("the Spearman row"), BaseSpearman))
	{
		return false;
	}

	AHOLBuilding* Houses[2] = { nullptr, nullptr };
	AHOLPlayerState* Payers[2] = { nullptr, nullptr };
	for (int32 Slot = 0; Slot < 2; ++Slot)
	{
		Houses[Slot] = Test.World->SpawnActor<AHOLBuilding>(FVector(Slot * -20000.0 + 4000.0, 0.0, 0.0), FRotator::ZeroRotator);
		Payers[Slot] = Test.FindPlayerState(Slot);
		if (!TestNotNull(TEXT("a Barracks"), Houses[Slot]) || !TestNotNull(TEXT("the slot's player state"), Payers[Slot]))
		{
			return false;
		}
		Houses[Slot]->Setup(Barracks, static_cast<uint8>(Slot), static_cast<uint8>(Slot));
		Payers[Slot]->SetResources(FHOLResources(5000, 5000, 5000, 5000));
		FHOLProductionCommand Command;
		Command.PlayerIndex = Slot;
		Command.Building = Houses[Slot];
		Command.TypeName = Spearman;
		Test.Commands->IssueProductionOrder(Command);
	}

	// With the real tables: what each payer paid at the order.
	Test.RunStep(1);
	const FHOLResources Dawn0 = Payers[0]->GetResources();
	const FHOLResources Legions1 = Payers[1]->GetResources();
	TestEqual(TEXT("the Dawn's Barracks queued the Spearman"), Houses[0]->GetProductionQueue().Num(), 1);
	TestEqual(TEXT("the Legions' Barracks queued the Spearman"), Houses[1]->GetProductionQueue().Num(), 1);
	TestEqual(TEXT("the Dawn paid the table's food"), 5000 - Dawn0.Food, BaseSpearman->Cost.Food);
	TestEqual(TEXT("the Dawn paid the table's wood"), 5000 - Dawn0.Wood, BaseSpearman->Cost.Wood);
	TestEqual(TEXT("the Legions paid 45 food (-25 %, Q-Legions-RosterValues = A)"), 5000 - Legions1.Food, 45);
	TestEqual(TEXT("the Legions paid 15 wood"), 5000 - Legions1.Wood, 15);
	TestTrue(TEXT("... under what the Dawn paid"), 5000 - Legions1.Food < 5000 - Dawn0.Food);

	// With a fixture that gives the Legions' Spearman a health of 777: ordered, built, born with it.
	{
		FFixtureTable Fixture(FixtureBirthOverrides);
		for (const FString& Problem : Fixture.Problems)
		{
			AddError(Problem);
		}
		const FHOLUnitTypeRow* LegionsRow = Test.Units->FindTypeRowFor(Spearman, 1);
		if (!TestNotNull(TEXT("the Legions' Spearman row under the fixture"), LegionsRow))
		{
			return false;
		}
		TestEqual(TEXT("the fixture's health is the Legions' row's"), LegionsRow->MaxHealth, 777);
		TestTrue(TEXT("the test can tell it from the table's"), BaseSpearman->MaxHealth != 777);
		TestEqual(TEXT("the Legions' Spearman is told apart by template key"), Test.Units->GetTemplateKey(Spearman, Legions), FName(TEXT("Spearman@Legions")));
		TestEqual(TEXT("the Dawn's Spearman keeps the table's key"), Test.Units->GetTemplateKey(Spearman, Dawn), Spearman);

		// Both queues run to their end (the time of an item is its payer's faction's), the births are served.
		const int32 Steps = FMath::Max(Test.Production->GetCost(Spearman, 0).BuildSteps, Test.Production->GetCost(Spearman, 1).BuildSteps) + 3;
		for (int32 Step = 2; Step < Steps + 2; ++Step)
		{
			Test.RunStep(Step);
		}
		Test.Units->ServeBirthRequests();
		int32 NumBorn[2] = { 0, 0 };
		int32 Health[2] = { 0, 0 };
		TArray<uint32> Ids;
		Test.Units->GetAllUnitIds(Ids);
		for (const uint32 Id : Ids)
		{
			FHOLUnitView View;
			if (Test.Units->GetUnitView(Id, View) && View.TypeName == Spearman && View.OwnerPlayer >= 0 && View.OwnerPlayer < 2)
			{
				++NumBorn[View.OwnerPlayer];
				Health[View.OwnerPlayer] = View.MaxHealth;
			}
		}
		TestEqual(TEXT("the Dawn's Spearman was born"), NumBorn[0], 1);
		TestEqual(TEXT("the Legions' Spearman was born"), NumBorn[1], 1);
		TestEqual(TEXT("the Dawn's Spearman is born with the table's health"), Health[0], BaseSpearman->MaxHealth);
		TestEqual(TEXT("the Legions' Spearman is born with the Legions' health (777)"), Health[1], 777);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
