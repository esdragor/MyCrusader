// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Match/HOLResources.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLHeroSheet.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// J8b.1 checks (J8b-Q1 to Q3, Q5): the hero's XP and levels, numbers written from the decisions, not from the code.
namespace UE::HeroesOfLiberty::Tests::HeroXP
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	constexpr float Tolerance = 1.e-3f;

	const FHOLUnitTypeRow* FindRow(const TCHAR* Name)
	{
		const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		return UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(Name, TEXT("Hero XP test"), false) : nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroXPSourcesTest, "HeroesOfLiberty.HeroXP.Sources", UE::HeroesOfLiberty::Tests::HeroXP::Flags)

bool FHOLHeroXPSourcesTest::RunTest(const FString& Parameters)
{
	// J8b-Q2: military > economy > exploration.
	TestEqual(TEXT("Combat x1"), HOLHeroXP::GetWeightPercent(EHOLXPSource::Combat), 100);
	TestEqual(TEXT("Command x0.75"), HOLHeroXP::GetWeightPercent(EHOLXPSource::Command), 75);
	TestEqual(TEXT("Development x0.5"), HOLHeroXP::GetWeightPercent(EHOLXPSource::Development), 50);
	TestEqual(TEXT("Exploration x0.25"), HOLHeroXP::GetWeightPercent(EHOLXPSource::Exploration), 25);

	// The value is the cost, the four resources added up.
	TestEqual(TEXT("Man-at-arms 100 food + 20 gold = 120"), HOLHeroXP::GetValue(FHOLResources(100, 0, 0, 20)), 120);
	TestEqual(TEXT("Spearman 60 food + 20 wood = 80"), HOLHeroXP::GetValue(FHOLResources(60, 20, 0, 0)), 80);
	TestEqual(TEXT("Nothing is worth 0"), HOLHeroXP::GetValue(FHOLResources()), 0);

	// Examples of J8b-Q2.
	TestEqual(TEXT("Man-at-arms killed by the hero: 120"), HOLHeroXP::GetXP(120, EHOLXPSource::Combat), 120);
	TestEqual(TEXT("... by his troops near him: 90"), HOLHeroXP::GetXP(120, EHOLXPSource::Command), 90);
	TestEqual(TEXT("Barracks finished: 75"), HOLHeroXP::GetXP(150, EHOLXPSource::Development), 75);
	TestEqual(TEXT("House finished: 25"), HOLHeroXP::GetXP(50, EHOLXPSource::Development), 25);
	TestEqual(TEXT("Farm (75 wood) finished: 37, rounded down"), HOLHeroXP::GetXP(75, EHOLXPSource::Development), 37);
	TestEqual(TEXT("Peasant by his troops: 37"), HOLHeroXP::GetXP(50, EHOLXPSource::Command), 37);
	TestEqual(TEXT("Nothing gives nothing"), HOLHeroXP::GetXP(0, EHOLXPSource::Combat), 0);

	// J8b-Q5: a hero killed, 300 x (victim's level / killer's level) x the weight.
	TestEqual(TEXT("Same level, by the hero: 300"), HOLHeroXP::GetHeroKillXP(1, 1, EHOLXPSource::Combat), 300);
	TestEqual(TEXT("Level 4 killed by a level 2: 600"), HOLHeroXP::GetHeroKillXP(4, 2, EHOLXPSource::Combat), 600);
	TestEqual(TEXT("Level 2 killed by a level 6: 100"), HOLHeroXP::GetHeroKillXP(2, 6, EHOLXPSource::Combat), 100);
	TestEqual(TEXT("Level 3 by the troops of a level 3: 225"), HOLHeroXP::GetHeroKillXP(3, 3, EHOLXPSource::Command), 225);
	TestEqual(TEXT("Level 1 by the troops of a level 6: 37"), HOLHeroXP::GetHeroKillXP(1, 6, EHOLXPSource::Command), 37);

	// D06: only the civilisation's sources while dead.
	TestFalse(TEXT("No combat XP while dead"), HOLHeroXP::CountsWhileDead(EHOLXPSource::Combat));
	TestFalse(TEXT("No command XP while dead"), HOLHeroXP::CountsWhileDead(EHOLXPSource::Command));
	TestTrue(TEXT("Development XP while dead"), HOLHeroXP::CountsWhileDead(EHOLXPSource::Development));
	TestTrue(TEXT("Exploration XP while dead"), HOLHeroXP::CountsWhileDead(EHOLXPSource::Exploration));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroXPKillSourceTest, "HeroesOfLiberty.HeroXP.KillSource", UE::HeroesOfLiberty::Tests::HeroXP::Flags)

bool FHOLHeroXPKillSourceTest::RunTest(const FString& Parameters)
{
	using namespace HOLHeroXP;
	const TOptional<EHOLXPSource> ByHero = GetKillSource(true, true, 0.f);
	TestTrue(TEXT("Killed by the hero: combat"), ByHero.IsSet() && ByHero.GetValue() == EHOLXPSource::Combat);
	const TOptional<EHOLXPSource> Near = GetKillSource(false, true, 1100.f);
	TestTrue(TEXT("By a troop 11 m from him: command"), Near.IsSet() && Near.GetValue() == EHOLXPSource::Command);
	const TOptional<EHOLXPSource> Edge = GetKillSource(false, true, 1200.f);
	TestTrue(TEXT("... 12 m: command still"), Edge.IsSet() && Edge.GetValue() == EHOLXPSource::Command);
	TestFalse(TEXT("... 13 m: nothing"), GetKillSource(false, true, 1300.f).IsSet());
	TestFalse(TEXT("By a troop while he is dead: nothing (D06)"), GetKillSource(false, false, 100.f).IsSet());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroXPLevelsTest, "HeroesOfLiberty.HeroXP.Levels", UE::HeroesOfLiberty::Tests::HeroXP::Flags)

bool FHOLHeroXPLevelsTest::RunTest(const FString& Parameters)
{
	using namespace HOLHeroXP;
	// J8b-Q2: totals 200, 600, 1 300, 2 200, 3 300.
	const int32 Expected[] = {0, 200, 600, 1300, 2200, 3300};
	for (int32 Level = 1; Level <= 6; ++Level)
	{
		TestEqual(FString::Printf(TEXT("Level %d at %d XP"), Level, Expected[Level - 1]), GetLevelXP(Level), Expected[Level - 1]);
	}
	TestEqual(TEXT("Below level 1: level 1's"), GetLevelXP(0), 0);
	TestEqual(TEXT("Above 6: level 6's"), GetLevelXP(7), 3300);

	TestEqual(TEXT("0 XP: level 1"), GetLevel(0), 1);
	TestEqual(TEXT("199: level 1"), GetLevel(199), 1);
	TestEqual(TEXT("200: level 2"), GetLevel(200), 2);
	TestEqual(TEXT("599: level 2"), GetLevel(599), 2);
	TestEqual(TEXT("600: level 3"), GetLevel(600), 3);
	TestEqual(TEXT("1 300: level 4"), GetLevel(1300), 4);
	TestEqual(TEXT("2 199: level 4"), GetLevel(2199), 4);
	TestEqual(TEXT("2 200: level 5"), GetLevel(2200), 5);
	TestEqual(TEXT("3 300: level 6"), GetLevel(3300), 6);
	TestEqual(TEXT("100 000: level 6, the prototype's last"), GetLevel(100000), 6);
	TestEqual(TEXT("Negative XP: level 1"), GetLevel(-5), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroXPStatsTest, "HeroesOfLiberty.HeroXP.Stats", UE::HeroesOfLiberty::Tests::HeroXP::Flags)

bool FHOLHeroXPStatsTest::RunTest(const FString& Parameters)
{
	using namespace HOLHeroXP;
	using namespace UE::HeroesOfLiberty::Tests::HeroXP;
	// J8b-Q3: +8 % a level on level 1's stats.
	TestEqual(TEXT("Level 1: 100 %"), GetStatPercent(1), 100);
	TestEqual(TEXT("Level 2: 108 %"), GetStatPercent(2), 108);
	TestEqual(TEXT("Level 6: 140 %"), GetStatPercent(6), 140);
	TestEqual(TEXT("Level 1 HP: 450"), GetLevelStat(450.f, 1), 450.f, Tolerance);
	TestEqual(TEXT("Level 2 HP: 486"), GetLevelStat(450.f, 2), 486.f, Tolerance);
	TestEqual(TEXT("Level 6 HP: 630"), GetLevelStat(450.f, 6), 630.f, Tolerance);
	TestEqual(TEXT("Level 2 attack: 32.4"), GetLevelStat(30.f, 2), 32.4f, Tolerance);
	TestEqual(TEXT("Level 6 attack: 42"), GetLevelStat(30.f, 6), 42.f, Tolerance);

	// The HP gained are added to the current health.
	TestEqual(TEXT("Wounded at 200, level 1 -> 2: 236"), GetHealthAfterLevelUp(200.f, 450.f, 1, 2), 236.f, Tolerance);
	TestEqual(TEXT("Full at 450, level 1 -> 2: full at 486"), GetHealthAfterLevelUp(450.f, 450.f, 1, 2), 486.f, Tolerance);
	TestEqual(TEXT("Two levels at once, 2 -> 4 from 100: 172"), GetHealthAfterLevelUp(100.f, 450.f, 2, 4), 172.f, Tolerance);
	TestEqual(TEXT("Dead stays dead (0)"), GetHealthAfterLevelUp(0.f, 450.f, 1, 2), 0.f, Tolerance);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroXPChainTest, "HeroesOfLiberty.HeroXP.Chain", UE::HeroesOfLiberty::Tests::HeroXP::Flags)

bool FHOLHeroXPChainTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroXP;
	// J8b.2: the level reaches the real chain, the table's Hero and Man-at-arms (J8b-Q3).
	const FHOLUnitTypeRow* HeroRow = FindRow(TEXT("Hero"));
	const FHOLUnitTypeRow* ManAtArmsRow = FindRow(TEXT("ManAtArms"));
	if (!TestNotNull(TEXT("The table has a Hero"), HeroRow) || !TestNotNull(TEXT("... a Man-at-arms"), ManAtArmsRow))
	{
		return false;
	}
	const FHOLUnitTypeConstants Hero = HeroRow->ToConstants(TEXT("Hero"));
	const FHOLUnitTypeConstants ManAtArms = ManAtArmsRow->ToConstants(TEXT("ManAtArms"));
	const float Level1 = HOLCombat::GetDamage(Hero, ManAtArms);
	TestEqual(TEXT("Level 1 hit unchanged by the level (1 by default)"), HOLCombat::GetDamage(Hero, ManAtArms, 1), Level1, Tolerance);
	TestEqual(TEXT("Level 2: +2.4 (30 -> 32.4)"), HOLCombat::GetDamage(Hero, ManAtArms, 2), Level1 + 2.4f, Tolerance);
	TestEqual(TEXT("Level 6: +12 (30 -> 42)"), HOLCombat::GetDamage(Hero, ManAtArms, 6), Level1 + 12.f, Tolerance);
	TestEqual(TEXT("A troop's hit on him does not depend on his level"), HOLCombat::GetDamage(ManAtArms, Hero, 1),
		static_cast<float>(FMath::Max(ManAtArms.Attack - HOLCombat::GetArmor(Hero, ManAtArms.AttackType), 1)) * 0.3f, Tolerance);

	TestEqual(TEXT("Max health level 1: 450"), HOLCombat::GetMaxHealth(Hero, 1), 450);
	TestEqual(TEXT("... level 2: 486"), HOLCombat::GetMaxHealth(Hero, 2), 486);
	TestEqual(TEXT("... level 6: 630"), HOLCombat::GetMaxHealth(Hero, 6), 630);
	TestEqual(TEXT("A troop at level 1: its table's"), HOLCombat::GetMaxHealth(ManAtArms, 1), ManAtArms.MaxHealth);

	// The sheet's level comes from the XP (J8b-Q2).
	FHOLHeroSheets Previous;
	Previous.Public.State = EHOLHeroState::Alive;
	Previous.Public.Level = 1;
	Previous.Private.XP = 600;
	FHOLHeroFacts Facts;
	Facts.bPresent = true;
	Facts.Health = 400.f;
	TestEqual(TEXT("Alive with 600 XP: level 3"), static_cast<int32>(HOLHeroSheet::Make(Facts, Previous).Public.Level), 3);
	Facts.DeathStep = 10;
	TestEqual(TEXT("Dying with 600 XP: level 3"), static_cast<int32>(HOLHeroSheet::Make(Facts, Previous).Public.Level), 3);
	// His body gone, dead: the XP of development still raises him (D06).
	Previous.Public.State = EHOLHeroState::Dead;
	Previous.Private.XP = 1300;
	TestEqual(TEXT("Dead, body gone, 1 300 XP: level 4"), static_cast<int32>(HOLHeroSheet::Make(FHOLHeroFacts(), Previous).Public.Level), 4);
	TestEqual(TEXT("No hero ever: level 0"), static_cast<int32>(HOLHeroSheet::Make(FHOLHeroFacts(), FHOLHeroSheets()).Public.Level), 0);
	return true;
}

#endif
