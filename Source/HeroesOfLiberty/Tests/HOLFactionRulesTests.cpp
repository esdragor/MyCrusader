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

		FFixtureTable()
		{
			HOLFactionRules::ParseRoles(FixtureRoles, Table.Roles, Problems);
			HOLFactionRules::ParseOverrides(FixtureOverrides, Table.Overrides, Problems);
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

		void RunStep() { World->GetSubsystem<UHOLSimulationSubsystem>()->GetOnStepStarted().Broadcast(1); }

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
	// Until L1 (Zombie) and L8a (Damned Lord) a Legions slot starts as the Dawn does: no crash, nothing missing.
	TestEqual(TEXT("Legions Worker, provisional"), Real.FindRoleType(Legions, HOLFactionRules::Roles::Worker), Peasant);

	// 2. The figures built in (a packaged game) are the files'.
	const HOLFactionRules::FTable BuiltIn = HOLFactionRules::GetBuiltInTable();
	TestEqual(TEXT("built-in roles: the same number of rows"), BuiltIn.Roles.Num(), Real.Roles.Num());
	for (const HOLFactionRules::FRoleRow& Row : Real.Roles)
	{
		TestEqual(*FString::Printf(TEXT("built-in %s"), *Row.Name.ToString()), BuiltIn.FindRoleType(Row.Faction, Row.Role), Row.Type);
	}
	TestEqual(TEXT("built-in overrides: the files'"), BuiltIn.Overrides.Num(), Real.Overrides.Num());

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

	// With the real tables (no Legions content yet): nothing differs from the Dawn, the type tables' rows are the same rows.
	{
		TArray<FName> Names;
		Test.Units->GetTypeNames(Names);
		int32 Different = 0;
		for (const FName Name : Names)
		{
			Different += Test.Units->FindTypeRow(Name, Legions) != Test.Units->FindTypeRow(Name) ? 1 : 0;
		}
		TestEqual(TEXT("no unit type differs for the Legions before L2 (same rows)"), Different, 0);
		TestEqual(TEXT("slot 1 (Legions, provisional Worker) starts as the Dawn: Peasant"), HOLFactionRules::GetRoleTypeName(Test.World, 1, HOLFactionRules::Roles::Worker), Peasant);
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

#endif // WITH_DEV_AUTOMATION_TESTS
