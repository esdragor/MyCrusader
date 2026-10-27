// Copyright Epic Games, Inc. All Rights Reserved.

#include "Algo/Reverse.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Presentation/HOLArrowSubsystem.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// J7b.1 checks (J7-Q4): the pure rules of the melee (damage, cadence, reach, target choice), on the table's numbers.
namespace UE::HeroesOfLiberty::Tests::Combat
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A type of the unit table as the simulation sees it (its shared fragment); false when the row is missing. */
	bool GetType(FAutomationTestBase& Test, const FName Name, FHOLUnitTypeConstants& OutConstants)
	{
		const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		const FHOLUnitTypeRow* Row = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(Name, TEXT("Combat test")) : nullptr;
		if (!Test.TestNotNull(*FString::Printf(TEXT("%s in the unit type table"), *Name.ToString()), Row))
		{
			return false;
		}
		OutConstants = Row->ToConstants(Name);
		return true;
	}

	int32 StepMs()
	{
		return HOLCombat::ToMilliseconds(GetDefault<UHOLSimulationSettings>()->GetStepSeconds());
	}

	/** Steps from the start of an attack to each of the first HitCount hits. */
	TArray<int32> HitSteps(const int32 IntervalMs, const int32 HitCount, const int32 WindUpPercent = HOLCombat::DefaultWindUpPercent)
	{
		TArray<int32> Steps;
		int32 Cooldown = HOLCombat::StartAttack(0, IntervalMs, WindUpPercent);
		for (int32 Step = 1; Steps.Num() < HitCount && Step < 100000; ++Step)
		{
			if (HOLCombat::TickAttack(Cooldown, StepMs(), IntervalMs))
			{
				Steps.Add(Step);
			}
		}
		return Steps;
	}

	HOLCombat::FCandidate Enemy(const uint64 Id, const double X, const double Y = 0.)
	{
		HOLCombat::FCandidate Candidate;
		Candidate.Id = Id;
		Candidate.Position = FVector2D(X, Y);
		return Candidate;
	}

	/** A soldier at the origin: body 60 cm, reach 50 cm, aggression 12 m (pure-rule numbers, not the table's). */
	HOLCombat::FSeeker Soldier(const HOLCombat::ESeekMode Mode = HOLCombat::ESeekMode::Auto, const float Aggression = 1200.f)
	{
		HOLCombat::FSeeker Seeker;
		Seeker.AggressionRadius = Aggression;
		Seeker.Mode = Mode;
		return Seeker;
	}

	/** The Id chosen, 0 for none. */
	uint64 Chosen(const HOLCombat::FSeeker& Seeker, TConstArrayView<HOLCombat::FCandidate> Candidates)
	{
		const int32 Index = HOLCombat::ChooseTarget(Seeker, Candidates);
		return Index == INDEX_NONE ? 0 : Candidates[Index].Id;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatComeToAidTest, "HeroesOfLiberty.Combat.ComeToAid", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatComeToAidTest::RunTest(const FString& Parameters)
{
	// MV.4, choice C (AoE4): an idle melee soldier comes to the aid of a friend fighting within the aid radius.
	using HOLCombat::ESeekMode;
	constexpr float Aid = HOLCombat::DefaultAidRadius;
	constexpr float Aggression = 1600.f;
	TestTrue(TEXT("a friend striking: fighting"), HOLCombat::IsFriendFighting(true, 0, false));
	TestTrue(TEXT("a friend taken on by an enemy: fighting"), HOLCombat::IsFriendFighting(false, 1, false));
	TestTrue(TEXT("a friend hit (an arrow): fighting"), HOLCombat::IsFriendFighting(false, 0, true));
	TestFalse(TEXT("a friend idle: not fighting"), HOLCombat::IsFriendFighting(false, 0, false));
	TestTrue(TEXT("an idle melee, a fighting friend 18 m off: comes to its aid"), HOLCombat::ShouldComeToAid(ESeekMode::Auto, false, Aggression, false, 1800., Aid));
	TestTrue(TEXT("a fighting friend at the radius: comes to its aid"), HOLCombat::ShouldComeToAid(ESeekMode::Auto, false, Aggression, false, Aid, Aid));
	TestFalse(TEXT("a fighting friend beyond the radius: stays"), HOLCombat::ShouldComeToAid(ESeekMode::Auto, false, Aggression, false, Aid + 1., Aid));
	TestFalse(TEXT("no fighting friend: stays"), HOLCombat::ShouldComeToAid(ESeekMode::Auto, false, Aggression, false, -1., Aid));
	TestFalse(TEXT("aid off (radius 0): stays"), HOLCombat::ShouldComeToAid(ESeekMode::Auto, false, Aggression, false, 0., 0.f));
	TestFalse(TEXT("on Hold: stays"), HOLCombat::ShouldComeToAid(ESeekMode::Hold, false, Aggression, false, 500., Aid));
	TestFalse(TEXT("on an attack-move: its own search"), HOLCombat::ShouldComeToAid(ESeekMode::AttackMove, false, Aggression, false, 500., Aid));
	TestFalse(TEXT("a ranged unit: stays"), HOLCombat::ShouldComeToAid(ESeekMode::Auto, true, Aggression, false, 500., Aid));
	TestFalse(TEXT("a Peasant (no aggression): stays"), HOLCombat::ShouldComeToAid(ESeekMode::Auto, false, 0.f, false, 500., Aid));
	TestFalse(TEXT("a target already: keeps it"), HOLCombat::ShouldComeToAid(ESeekMode::Auto, false, Aggression, true, 500., Aid));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatDamageTest, "HeroesOfLiberty.Combat.Damage", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatDamageTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Combat;
	FHOLUnitTypeConstants Spearman, ManAtArms, Peasant, Archer;
	if (!GetType(*this, TEXT("Spearman"), Spearman) || !GetType(*this, TEXT("ManAtArms"), ManAtArms)
		|| !GetType(*this, TEXT("Peasant"), Peasant) || !GetType(*this, TEXT("Archer"), Archer))
	{
		return false;
	}

	// The table reaches the shared fragment (J7a.1 numbers, D345 speeds, reaches and aggression).
	TestEqual(TEXT("Spearman: 80 HP"), Spearman.MaxHealth, 80);
	TestEqual(TEXT("Spearman: attack 8"), Spearman.Attack, 8);
	TestEqual(TEXT("Spearman: every 1.875 s"), Spearman.AttackInterval, 1.875f);
	TestEqual(TEXT("Spearman: one bonus"), Spearman.Bonuses.Num(), 1);
	if (Spearman.Bonuses.Num() == 1)
	{
		TestEqual(TEXT("Spearman: +17"), Spearman.Bonuses[0].Damage, 17);
		TestEqual(TEXT("Spearman: against cavalry"), int32(Spearman.Bonuses[0].Category), int32(EHOLArmorCategory::Cavalry));
	}
	TestEqual(TEXT("Spearman: armours 0 / 0 / 0 / 0"), Spearman.MeleeArmor + Spearman.RangedArmor + Spearman.SiegeArmor + Spearman.HeroArmor, 0);
	TestEqual(TEXT("Spearman: light infantry only (D345)"), Spearman.ArmorCategoryMask, HOLArmorCategoryBit(EHOLArmorCategory::LightInfantry));
	TestEqual(TEXT("Spearman: melee attack (D345)"), int32(Spearman.AttackType), int32(EHOLAttackType::Melee));
	TestEqual(TEXT("Spearman: reach 1.3 m, its spear (J7c.4)"), Spearman.AttackRange, 130.f);
	TestEqual(TEXT("Spearman: aggression 16 m"), Spearman.AggressionRadius, 1600.f);
	TestEqual(TEXT("Man-at-arms: 155 HP"), ManAtArms.MaxHealth, 155);
	TestEqual(TEXT("Man-at-arms: attack 12"), ManAtArms.Attack, 12);
	TestEqual(TEXT("Man-at-arms: every 1.375 s"), ManAtArms.AttackInterval, 1.375f);
	TestEqual(TEXT("Man-at-arms: aggression 16 m"), ManAtArms.AggressionRadius, 1600.f);
	TestEqual(TEXT("Man-at-arms: melee armour 4"), ManAtArms.MeleeArmor, 4);
	TestEqual(TEXT("Man-at-arms: ranged armour 4"), ManAtArms.RangedArmor, 4);
	TestEqual(TEXT("Man-at-arms: siege armour 0"), ManAtArms.SiegeArmor, 0);
	TestEqual(TEXT("Man-at-arms: hero armour 2 (D345)"), ManAtArms.HeroArmor, 2);
	TestEqual(TEXT("Man-at-arms: heavy infantry and Heavy (D428)"), ManAtArms.ArmorCategoryMask, HOLArmorCategoryBit(EHOLArmorCategory::HeavyInfantry) | HOLArmorCategoryBit(EHOLArmorCategory::Heavy));
	TestEqual(TEXT("Man-at-arms: melee attack"), int32(ManAtArms.AttackType), int32(EHOLAttackType::Melee));
	TestEqual(TEXT("Man-at-arms: no bonus"), ManAtArms.Bonuses.Num(), 0);
	TestEqual(TEXT("Peasant: Peasant only"), Peasant.ArmorCategoryMask, HOLArmorCategoryBit(EHOLArmorCategory::Peasant));
	TestEqual(TEXT("Peasant: melee attack"), int32(Peasant.AttackType), int32(EHOLAttackType::Melee));
	TestEqual(TEXT("Archer: Ranged only"), Archer.ArmorCategoryMask, HOLArmorCategoryBit(EHOLArmorCategory::Ranged));
	TestEqual(TEXT("Archer: ranged attack (D345)"), int32(Archer.AttackType), int32(EHOLAttackType::Ranged));
	TestEqual(TEXT("Archer: two bonuses"), Archer.Bonuses.Num(), 2);
	TestEqual(TEXT("Archer: siege and hero armours 0"), Archer.SiegeArmor + Archer.HeroArmor, 0);
	TestEqual(TEXT("Peasant: attack 3"), Peasant.Attack, 3);
	TestEqual(TEXT("Peasant: every 3.7 s"), Peasant.AttackInterval, 3.7f);
	TestEqual(TEXT("Peasant: reach 1.16 m"), Peasant.AttackRange, 116.f);
	TestEqual(TEXT("Peasant: no aggression"), Peasant.AggressionRadius, 0.f);
	TestEqual(TEXT("Archer: reach 20 m"), Archer.AttackRange, 2000.f);
	TestEqual(TEXT("Archer: aggression 21.33 m"), Archer.AggressionRadius, 2133.f);
	// The wind-up per type (J7c, retour d'essai 8: hit and run): the Archer looses at 12 % (~0.2 s), the melee at half.
	TestEqual(TEXT("Archer: wind-up 12 %"), Archer.AttackWindUpPercent, 12);
	TestEqual(TEXT("Spearman: wind-up 50 %"), Spearman.AttackWindUpPercent, 50);
	TestEqual(TEXT("Man-at-arms: wind-up 50 %"), ManAtArms.AttackWindUpPercent, 50);
	TestEqual(TEXT("Peasant: wind-up 50 %"), Peasant.AttackWindUpPercent, 50);

	// Damage = attack + the bonuses on the target's categories - its armour of the attack's type, 1 at least (D21, D345).
	TestEqual(TEXT("Spearman on Man-at-arms: 8 - 4"), HOLCombat::GetDamage(Spearman, ManAtArms), 4.f);
	TestEqual(TEXT("Man-at-arms on Spearman: 12 - 0"), HOLCombat::GetDamage(ManAtArms, Spearman), 12.f);
	TestEqual(TEXT("Spearman on Spearman: no bonus"), HOLCombat::GetDamage(Spearman, Spearman), 8.f);
	FHOLUnitTypeConstants Horseman;
	Horseman.ArmorCategoryMask = HOLArmorCategoryBit(EHOLArmorCategory::Cavalry);
	Horseman.MeleeArmor = 2;
	Horseman.RangedArmor = 5;
	Horseman.SiegeArmor = 7;
	Horseman.HeroArmor = 3;
	TestEqual(TEXT("Spearman on cavalry: 8 + 17 - 2"), HOLCombat::GetDamage(Spearman, Horseman), 23.f);
	TestEqual(TEXT("Peasant on Man-at-arms: 3 - 4, 1 at least"), HOLCombat::GetDamage(Peasant, ManAtArms), 1.f);
	TestEqual(TEXT("Archer on Spearman: 5 + 5 (light infantry), ranged armour 0"), HOLCombat::GetDamage(Archer, Spearman), 10.f);
	TestEqual(TEXT("Archer on Man-at-arms: 5 - 4 (ranged armour)"), HOLCombat::GetDamage(Archer, ManAtArms), 1.f);
	TestEqual(TEXT("Archer on Archer: no bonus on Ranged"), HOLCombat::GetDamage(Archer, Archer), 5.f);

	// The armour taken off is the one of the attack's type (D345), whatever the reach.
	FHOLUnitTypeConstants Typed = Spearman;
	TestEqual(TEXT("melee hit: melee armour"), HOLCombat::GetDamage(Typed, Horseman), static_cast<float>(8 + 17 - 2));
	Typed.AttackType = EHOLAttackType::Ranged;
	TestEqual(TEXT("ranged hit: ranged armour"), HOLCombat::GetDamage(Typed, Horseman), static_cast<float>(8 + 17 - 5));
	Typed.AttackType = EHOLAttackType::Siege;
	TestEqual(TEXT("siege hit: siege armour"), HOLCombat::GetDamage(Typed, Horseman), static_cast<float>(8 + 17 - 7));
	Typed.AttackType = EHOLAttackType::Hero;
	TestEqual(TEXT("hero hit: hero armour"), HOLCombat::GetDamage(Typed, Horseman), static_cast<float>(8 + 17 - 3));
	TestEqual(TEXT("hero hit on a Man-at-arms: 8 - 2 (hero armour 2)"), HOLCombat::GetDamage(Typed, ManAtArms), 6.f);
	TestEqual(TEXT("GetArmor: hero"), HOLCombat::GetArmor(ManAtArms, EHOLAttackType::Hero), 2);
	TestEqual(TEXT("GetArmor: siege"), HOLCombat::GetArmor(Horseman, EHOLAttackType::Siege), 7);
	FHOLUnitTypeConstants LongMelee = Spearman;
	LongMelee.AttackRange = 2000.f;
	TestEqual(TEXT("a long reach does not make it ranged: melee armour"), HOLCombat::GetDamage(LongMelee, ManAtArms), 4.f);
	Typed.Attack = 1;
	Typed.Bonuses.Reset();
	TestEqual(TEXT("1 at least against every armour"), HOLCombat::GetDamage(Typed, Horseman), 1.f);

	// Several categories (D345): every bonus aimed at one of them adds up.
	FHOLUnitTypeConstants Musketeer;
	Musketeer.ArmorCategoryMask = HOLArmorCategoryBit(EHOLArmorCategory::LightInfantry) | HOLArmorCategoryBit(EHOLArmorCategory::GunpowderInfantry);
	TestTrue(TEXT("two categories: light infantry"), Musketeer.HasArmorCategory(EHOLArmorCategory::LightInfantry));
	TestTrue(TEXT("two categories: gunpowder infantry"), Musketeer.HasArmorCategory(EHOLArmorCategory::GunpowderInfantry));
	TestFalse(TEXT("two categories: not cavalry"), Musketeer.HasArmorCategory(EHOLArmorCategory::Cavalry));
	TestEqual(TEXT("Archer on light + gunpowder infantry: 5 + 5 + 5"), HOLCombat::GetDamage(Archer, Musketeer), 15.f);
	Musketeer.RangedArmor = 3;
	TestEqual(TEXT("... minus its ranged armour"), HOLCombat::GetDamage(Archer, Musketeer), 12.f);
	FHOLUnitTypeConstants Gunner;
	Gunner.ArmorCategoryMask = HOLArmorCategoryBit(EHOLArmorCategory::Ranged) | HOLArmorCategoryBit(EHOLArmorCategory::GunpowderInfantry);
	TestEqual(TEXT("Archer on ranged + gunpowder infantry: one bonus"), HOLCombat::GetDamage(Archer, Gunner), 10.f);
	FHOLUnitTypeConstants NoBonus = Spearman;
	NoBonus.Bonuses.Reset();
	TestEqual(TEXT("no bonus: no bonus"), HOLCombat::GetDamage(NoBonus, Horseman), 6.f);
	if (Spearman.Bonuses.Num() == 1)
	{
		FHOLUnitTypeConstants ZeroBonus = Spearman;
		ZeroBonus.Bonuses[0].Damage = 0;
		TestEqual(TEXT("a bonus of 0: no bonus"), HOLCombat::GetDamage(ZeroBonus, Horseman), 6.f);
	}

	// Hits to kill (the matchup the user will see first).
	TestEqual(TEXT("Spearman kills a Man-at-arms in 39 hits"), HOLCombat::GetHitsToKill(ManAtArms.MaxHealth, HOLCombat::GetDamage(Spearman, ManAtArms)), 39);
	TestEqual(TEXT("Man-at-arms kills a Spearman in 7 hits"), HOLCombat::GetHitsToKill(Spearman.MaxHealth, HOLCombat::GetDamage(ManAtArms, Spearman)), 7);
	TestEqual(TEXT("Spearman kills a Spearman in 10 hits"), HOLCombat::GetHitsToKill(80, 8), 10);
	TestEqual(TEXT("one more HP, one more hit"), HOLCombat::GetHitsToKill(81, 8), 11);
	TestEqual(TEXT("dead: no hit"), HOLCombat::GetHitsToKill(0, 8), 0);
	TestEqual(TEXT("zero damage counts as 1"), HOLCombat::GetHitsToKill(3, 0), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatCrossbowmanTest, "HeroesOfLiberty.Combat.Crossbowman", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatCrossbowmanTest::RunTest(const FString& Parameters)
{
	// AR8 (AR-Q1): the Crossbowman, AoE4's numbers at our scales, read from the real row of the unit table.
	using namespace UE::HeroesOfLiberty::Tests::Combat;
	FHOLUnitTypeConstants Crossbowman, ManAtArms, Spearman;
	if (!GetType(*this, TEXT("Crossbowman"), Crossbowman) || !GetType(*this, TEXT("ManAtArms"), ManAtArms)
		|| !GetType(*this, TEXT("Spearman"), Spearman))
	{
		return false;
	}
	const double Tile = UHOLBuildingSettings::GetTileSize();
	TestEqual(TEXT("Crossbowman: 80 HP"), Crossbowman.MaxHealth, 80);
	TestEqual(TEXT("Crossbowman: Ranged only"), Crossbowman.ArmorCategoryMask, HOLArmorCategoryBit(EHOLArmorCategory::Ranged));
	TestEqual(TEXT("Crossbowman: ranged attack"), int32(Crossbowman.AttackType), int32(EHOLAttackType::Ranged));
	TestEqual(TEXT("Crossbowman: attack 12"), Crossbowman.Attack, 12);
	TestEqual(TEXT("Crossbowman: every 2.125 s"), Crossbowman.AttackInterval, 2.125f);
	TestEqual(TEXT("Crossbowman: wind-up 12 %, as the Archer"), Crossbowman.AttackWindUpPercent, 12);
	TestTrue(TEXT("Crossbowman: reach 5 tiles (20 m)"), FMath::IsNearlyEqual(double(Crossbowman.AttackRange), 5. * Tile));
	TestTrue(TEXT("Crossbowman: aggression = reach + 1/3 tile"),
		FMath::IsNearlyEqual(double(Crossbowman.AggressionRadius), double(Crossbowman.AttackRange) + Tile / 3., 1.));
	TestEqual(TEXT("Crossbowman: armours 0 / 0 / 0 / 0"), Crossbowman.MeleeArmor + Crossbowman.RangedArmor + Crossbowman.SiegeArmor + Crossbowman.HeroArmor, 0);
	TestTrue(TEXT("Crossbowman: 1.8 tiles/s (720 cm/s)"), FMath::IsNearlyEqual(double(Crossbowman.Speed), 1.8 * Tile));
	TestEqual(TEXT("Crossbowman: one bonus"), Crossbowman.Bonuses.Num(), 1);
	if (Crossbowman.Bonuses.Num() == 1)
	{
		TestEqual(TEXT("Crossbowman: +10"), Crossbowman.Bonuses[0].Damage, 10);
		TestEqual(TEXT("Crossbowman: against Heavy (D428)"), int32(Crossbowman.Bonuses[0].Category), int32(EHOLArmorCategory::Heavy));
	}

	// Damage by the real rule (D21, D345): attack + the bonus on the target's categories - its ranged armour.
	TestEqual(TEXT("Crossbowman on Man-at-arms: 12 + 10 - its ranged armour"), HOLCombat::GetDamage(Crossbowman, ManAtArms),
		float(12 + 10 - ManAtArms.RangedArmor));
	TestEqual(TEXT("Crossbowman on Man-at-arms: 18"), HOLCombat::GetDamage(Crossbowman, ManAtArms), 18.f);
	TestEqual(TEXT("Crossbowman on Spearman: 12 - its ranged armour, no bonus"), HOLCombat::GetDamage(Crossbowman, Spearman), float(12 - Spearman.RangedArmor));
	TestEqual(TEXT("Crossbowman on Crossbowman: 12, no bonus on Ranged"), HOLCombat::GetDamage(Crossbowman, Crossbowman), 12.f);

	// Against the buildings: the same attack, 1 left after their ranged armour.
	TestTrue(TEXT("Crossbowman's building attack: ranged, 12, 2.125 s, its reach"), Crossbowman.BuildingAttackType == EHOLAttackType::Ranged
		&& Crossbowman.BuildingAttack == 12 && FMath::IsNearlyEqual(Crossbowman.BuildingAttackInterval, 2.125f)
		&& FMath::IsNearlyEqual(Crossbowman.BuildingAttackRange, Crossbowman.AttackRange));
	const FHOLBuildingTypeRow* House = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::House);
	if (TestNotNull(TEXT("House in the building table"), House))
	{
		TestEqual(TEXT("A bolt does 1 to a building"), HOLConstruction::GetBuildingDamage(Crossbowman.BuildingAttack, Crossbowman.BuildingAttackType,
			House->RangedArmor), 1);
	}

	// Production (AR-Q1) and presentation (AR6), on the row itself (its provider's sequences: HeroesOfLiberty.Animation).
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* Row = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Crossbowman"), TEXT("Combat test")) : nullptr;
	if (!TestNotNull(TEXT("Crossbowman row"), Row))
	{
		return false;
	}
	TestTrue(TEXT("Crossbowman: 80 food + 40 gold"), Row->Cost.Food == 80 && Row->Cost.Wood == 0 && Row->Cost.Stone == 0 && Row->Cost.Gold == 40);
	TestEqual(TEXT("Crossbowman: 22 s"), Row->BuildTime, 22.f);
	TestEqual(TEXT("Crossbowman: population 1"), Row->Population, 1);
	TestTrue(TEXT("Crossbowman: ripostes, as the Archer"), Row->Ripostes);
	TestEqual(TEXT("Crossbowman: walk play rate 4.28 (720 / 168.2)"), Row->WalkPlayRate, 4.28f);
	TestEqual(TEXT("Crossbowman: mesh scale as the Archer"), Row->MeshScale, 1.5f);
	return true;
}

// AR9 (AR-Q5): each shooter's projectile, from the Projectile column of the real table (SM_Arrow for the Archer, SM_Bolt
// for the Crossbowman, empty for the melee types), and a shot launched in the arrow display drawn with its shooter type's
// mesh: an instance in that mesh's own component. The flight itself is the same (HOLCombat::GetArrowSpeed).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatProjectilesTest, "HeroesOfLiberty.Combat.Projectiles", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatProjectilesTest::RunTest(const FString& Parameters)
{
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* Archer = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Archer"), TEXT("Combat test")) : nullptr;
	const FHOLUnitTypeRow* Crossbowman = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Crossbowman"), TEXT("Combat test")) : nullptr;
	const FHOLUnitTypeRow* ManAtArms = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("ManAtArms"), TEXT("Combat test")) : nullptr;
	if (!TestNotNull(TEXT("Archer row"), Archer) || !TestNotNull(TEXT("Crossbowman row"), Crossbowman) || !TestNotNull(TEXT("ManAtArms row"), ManAtArms))
	{
		return false;
	}
	TestEqual(TEXT("Archer's projectile: SM_Arrow"), Archer->Projectile.ToSoftObjectPath().ToString(), FString(TEXT("/Game/Units/Props/SM_Arrow.SM_Arrow")));
	TestEqual(TEXT("Crossbowman's projectile: SM_Bolt"), Crossbowman->Projectile.ToSoftObjectPath().ToString(), FString(TEXT("/Game/Units/Props/SM_Bolt.SM_Bolt")));
	TestTrue(TEXT("Man-at-arms: no projectile"), ManAtArms->Projectile.IsNull());

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	UHOLArrowSubsystem* Display = World->GetSubsystem<UHOLArrowSubsystem>();
	if (TestNotNull(TEXT("arrow display in a game world"), Display))
	{
		const UStaticMesh* Arrow = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Units/Props/SM_Arrow.SM_Arrow"));
		const UStaticMesh* Bolt = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Units/Props/SM_Bolt.SM_Bolt"));
		TestNotNull(TEXT("SM_Arrow"), Arrow);
		TestNotNull(TEXT("SM_Bolt"), Bolt);
		TestTrue(TEXT("an Archer shoots SM_Arrow"), Display->GetProjectileMesh(TEXT("Archer")) == Arrow);
		TestTrue(TEXT("a Crossbowman shoots SM_Bolt"), Display->GetProjectileMesh(TEXT("Crossbowman")) == Bolt);
		TestTrue(TEXT("an empty column: SM_Arrow"), Display->GetProjectileMesh(TEXT("ManAtArms")) == Arrow);

		// Two bolts and an arrow in flight: drawn at the next tick, each in its mesh's own component, without shadow (D258).
		FHOLArrowLaunch Launch;
		Launch.From = FVector(0.0, 0.0, 0.0);
		Launch.To = FVector(2000.0, 0.0, 0.0);
		Launch.FlightMs = HOLCombat::GetArrowFlightMilliseconds(FVector2D(Launch.From), FVector2D(Launch.To), HOLCombat::GetArrowSpeed());
		Launch.ShooterType = TEXT("Crossbowman");
		Display->Launch(Launch);
		Display->Launch(Launch);
		Launch.ShooterType = TEXT("Archer");
		Display->Launch(Launch);
		Display->Tick(0.01f);
		const UInstancedStaticMeshComponent* Bolts = Display->GetInstances(Bolt);
		const UInstancedStaticMeshComponent* Arrows = Display->GetInstances(Arrow);
		TestTrue(TEXT("two bolts drawn as SM_Bolt"), Bolts && Bolts->GetInstanceCount() == 2);
		TestTrue(TEXT("one arrow drawn as SM_Arrow"), Arrows && Arrows->GetInstanceCount() == 1);
		TestTrue(TEXT("no shadow (D258)"), (!Bolts || !Bolts->CastShadow) && (!Arrows || !Arrows->CastShadow));
		TestEqual(TEXT("three in flight"), Display->GetInFlightCount(), 3);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatCadenceTest, "HeroesOfLiberty.Combat.Cadence", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatCadenceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Combat;
	TestEqual(TEXT("fixed step of 50 ms"), StepMs(), 50);
	TestEqual(TEXT("1.875 s"), HOLCombat::ToMilliseconds(1.875f), 1875);
	TestEqual(TEXT("1.375 s"), HOLCombat::ToMilliseconds(1.375f), 1375);
	TestEqual(TEXT("wind-up: half the interval"), HOLCombat::GetWindUpMilliseconds(1875, 50), 937);
	TestEqual(TEXT("wind-up: 12 % of the Archer's 1.625 s"), HOLCombat::GetWindUpMilliseconds(1625, 12), 195);
	TestEqual(TEXT("wind-up: 1 % at least"), HOLCombat::GetWindUpMilliseconds(1000, 0), 10);
	TestEqual(TEXT("wind-up: the whole interval at most"), HOLCombat::GetWindUpMilliseconds(1000, 250), 1000);

	// Spearman: first hit after the wind-up (937 ms -> 19 steps), then 37 or 38 steps apart, exactly 1.875 s on average.
	const TArray<int32> Spear = HitSteps(1875, 9);
	if (TestEqual(TEXT("9 hits"), Spear.Num(), 9))
	{
		TestEqual(TEXT("Spearman: first hit at step 19 (0.95 s)"), Spear[0], 19);
		for (int32 Hit = 1; Hit < Spear.Num(); ++Hit)
		{
			const int32 Gap = Spear[Hit] - Spear[Hit - 1];
			TestTrue(FString::Printf(TEXT("Spearman: gap %d is 37 or 38 steps (%d)"), Hit, Gap), Gap == 37 || Gap == 38);
		}
		TestEqual(TEXT("Spearman: 8 intervals = 15 s = 300 steps, no drift"), Spear[8] - Spear[0], 300);
	}

	// Man-at-arms kills a Spearman: 10 hits, 0.6875 + 9 x 1.375 = 13.06 s, within one step.
	const TArray<int32> Arms = HitSteps(1375, 10);
	if (TestEqual(TEXT("10 hits"), Arms.Num(), 10))
	{
		const int32 Ms = Arms.Last() * StepMs();
		TestTrue(FString::Printf(TEXT("Man-at-arms kills a Spearman in 13.06 s (%d ms)"), Ms), Ms >= 13062 && Ms < 13062 + 50);
	}

	// A switch of target keeps the rhythm, never a free hit; idle runs the cool-down down to 0.
	TestEqual(TEXT("start: wind-up at least"), HOLCombat::StartAttack(100, 1375, 50), 687);
	TestEqual(TEXT("start: keeps a longer wait"), HOLCombat::StartAttack(1000, 1375, 50), 1000);
	TestEqual(TEXT("start: the type's wind-up"), HOLCombat::StartAttack(100, 1625, 12), 195);
	int32 Cooldown = 120;
	HOLCombat::TickIdle(Cooldown, 50);
	TestEqual(TEXT("idle: runs down"), Cooldown, 70);
	HOLCombat::TickIdle(Cooldown, 50);
	HOLCombat::TickIdle(Cooldown, 50);
	TestEqual(TEXT("idle: stays at 0"), Cooldown, 0);

	// One hit per step at most, even with an interval shorter than the step.
	Cooldown = 0;
	int32 Hits = 0;
	for (int32 Step = 0; Step < 10; ++Step)
	{
		Hits += HOLCombat::TickAttack(Cooldown, 50, 10) ? 1 : 0;
	}
	TestEqual(TEXT("short interval: one hit per step"), Hits, 10);
	return true;
}

// J7c, retour d'essai 8 (hit and run): the Archer looses 12 % of its cadence after it stops (the table's wind-up),
// walks off while it reloads (TickIdle, as a player's move leaves its cool-down, Disengage), and stops again: its next
// arrow then falls when the cool-down left runs out, the wind-up at least, never sooner than had it stood.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatHitAndRunTest, "HeroesOfLiberty.Combat.HitAndRun", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatHitAndRunTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Combat;
	FHOLUnitTypeConstants Archer, Spearman;
	if (!GetType(*this, TEXT("Archer"), Archer) || !GetType(*this, TEXT("Spearman"), Spearman))
	{
		return false;
	}
	const int32 Step = StepMs();
	const int32 IntervalMs = HOLCombat::ToMilliseconds(Archer.AttackInterval);
	TestEqual(TEXT("Archer: 1625 ms"), IntervalMs, 1625);

	// First arrow: the wind-up (195 ms -> 4 steps, 0.2 s), a fifth of the melee's (Spearman: 937 ms -> 19 steps).
	const TArray<int32> Shots = HitSteps(IntervalMs, 2, Archer.AttackWindUpPercent);
	if (TestEqual(TEXT("2 arrows"), Shots.Num(), 2))
	{
		TestEqual(TEXT("Archer: first arrow at step 4 (0.2 s)"), Shots[0], 4);
		const int32 Gap = Shots[1] - Shots[0];
		TestTrue(FString::Printf(TEXT("Archer: second 32 or 33 steps later, 1.625 s (%d)"), Gap), Gap == 32 || Gap == 33);
	}
	const TArray<int32> Strikes = HitSteps(HOLCombat::ToMilliseconds(Spearman.AttackInterval), 1, Spearman.AttackWindUpPercent);
	TestEqual(TEXT("Spearman: first hit still at step 19 (0.95 s)"), Strikes.Num() == 1 ? Strikes[0] : 0, 19);

	// Shoot, walk WalkSteps (idle), stop and shoot again: the step of the next arrow, counted from the first step.
	const auto NextShot = [Step, IntervalMs, &Archer](const int32 WalkSteps, int32& OutFirst)
	{
		int32 Cooldown = HOLCombat::StartAttack(0, IntervalMs, Archer.AttackWindUpPercent);
		int32 At = 0;
		OutFirst = 0;
		while (OutFirst == 0 && At < 1000)
		{
			++At;
			OutFirst = HOLCombat::TickAttack(Cooldown, Step, IntervalMs) ? At : 0;
		}
		for (int32 Walk = 0; Walk < WalkSteps; ++Walk)
		{
			++At;
			HOLCombat::TickIdle(Cooldown, Step);
		}
		Cooldown = HOLCombat::StartAttack(Cooldown, IntervalMs, Archer.AttackWindUpPercent);
		while (At < 10000)
		{
			++At;
			if (HOLCombat::TickAttack(Cooldown, Step, IntervalMs))
			{
				return At;
			}
		}
		return 0;
	};
	int32 First = 0;
	const int32 Stood = NextShot(0, First);
	TestEqual(TEXT("first arrow at step 4"), First, 4);
	TestEqual(TEXT("standing: the next arrow one interval later, step 37"), Stood, 37);
	// A short walk (10 steps, 0.5 s): the cool-down left (1120 ms) is longer than the wind-up: the same step as standing.
	TestEqual(TEXT("short walk: the next arrow when the cool-down runs out, as had it stood"), NextShot(10, First), 37);
	// A walk of 25 steps (1.25 s): 370 ms left, still longer than the wind-up (195 ms): the same step as standing.
	TestEqual(TEXT("walk of 1.25 s: still as had it stood"), NextShot(25, First), 37);
	// A walk of 30 steps (1.5 s): 120 ms left, the wind-up longer: 4 steps after it stops (step 34 + 4).
	TestEqual(TEXT("walk of 1.5 s: the wind-up after it stops"), NextShot(30, First), 4 + 30 + 4);
	// A long walk (3 s): reloaded, the wind-up only.
	TestEqual(TEXT("long walk: the wind-up after it stops"), NextShot(60, First), 4 + 60 + 4);
	// Never sooner than had it stood.
	for (int32 Walk = 0; Walk <= 40; ++Walk)
	{
		const int32 Next = NextShot(Walk, First);
		TestTrue(*FString::Printf(TEXT("walk of %d steps: next arrow at %d, not before %d"), Walk, Next, Stood), Next >= Stood && Next >= 4 + Walk + 4);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatRangeTest, "HeroesOfLiberty.Combat.Range", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatRangeTest::RunTest(const FString& Parameters)
{
	const FVector2D Origin(0., 0.);
	TestEqual(TEXT("edge distance: 200 - 60 - 60"), HOLCombat::GetEdgeDistance(Origin, 60.f, FVector2D(200., 0.), 60.f), 80., 1e-9);
	TestTrue(TEXT("reach 50: edges 50 apart, in"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(170., 0.), 60.f, 50.f));
	TestFalse(TEXT("reach 50: edges 50.1 apart, out"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(170.1, 0.), 60.f, 50.f));
	TestTrue(TEXT("pushed into each other: in"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(100., 0.), 60.f, 50.f));
	TestTrue(TEXT("diagonal: in"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(120., 120.), 60.f, 50.f));
	TestFalse(TEXT("diagonal: out"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(121., 121.), 60.f, 50.f));
	TestTrue(TEXT("larger body reached from further"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(400., 0.), 300.f, 50.f));
	TestTrue(TEXT("Archer: 15 m edge to edge"), HOLCombat::IsInRange(Origin, 52.f, FVector2D(1612., 0.), 60.f, 1500.f));
	TestFalse(TEXT("Archer: beyond 15 m"), HOLCombat::IsInRange(Origin, 52.f, FVector2D(1613., 0.), 60.f, 1500.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatPushTest, "HeroesOfLiberty.Combat.Push", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatPushTest::RunTest(const FString& Parameters)
{
	// The user's trial of J7b: two enemies never push each other back; one player's units keep the soft push (D292).
	for (const bool bWalkingA : { false, true })
	{
		for (const bool bWalkingB : { false, true })
		{
			TestEqual(*FString::Printf(TEXT("allies (walking %d, %d): half each"), bWalkingA, bWalkingB), HOLCombat::GetOverlapShare(false, bWalkingA, bWalkingB), 0.5);
		}
	}
	TestEqual(TEXT("walker against a standing enemy: it takes it all"), HOLCombat::GetOverlapShare(true, true, false), 1.);
	TestEqual(TEXT("pushed by its own side into a standing enemy: it takes it all"), HOLCombat::GetOverlapShare(true, false, false), 1.);
	TestEqual(TEXT("an enemy walker against a standing unit: the walker takes it all"), HOLCombat::GetOverlapShare(true, false, true), 0.);
	TestEqual(TEXT("two enemies walking into each other: half each"), HOLCombat::GetOverlapShare(true, true, true), 0.5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatTargetTest, "HeroesOfLiberty.Combat.Target", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatTargetTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Combat;
	using HOLCombat::ESeekMode;
	using HOLCombat::FCandidate;

	// Search radius.
	TestEqual(TEXT("search: aggression radius"), HOLCombat::GetSearchRadius(Soldier()), 1200.f);
	TestEqual(TEXT("search: none without aggression"), HOLCombat::GetSearchRadius(Soldier(ESeekMode::Auto, 0.f)), 0.f);
	TestEqual(TEXT("search: attack-move gives 16 m"), HOLCombat::GetSearchRadius(Soldier(ESeekMode::AttackMove, 0.f)), HOLCombat::AttackMoveRadius);
	TestEqual(TEXT("search: hold, the reach only"), HOLCombat::GetSearchRadius(Soldier(ESeekMode::Hold)), 50.f);

	TestEqual(TEXT("nobody: none"), Chosen(Soldier(), {}), uint64(0));
	TestEqual(TEXT("nearest edge to edge"), Chosen(Soldier(), { Enemy(1, 400.), Enemy(2, 300.) }), uint64(2));
	{
		TArray<FCandidate> Set = { Enemy(1, 160.), Enemy(2, 250.) };
		Set[1].bAttacksMe = true;
		TestEqual(TEXT("in reach before one attacking me out of reach"), Chosen(Soldier(), Set), uint64(1));
		Set[0].Position = FVector2D(300., 0.);
		Set[1].Position = FVector2D(600., 0.);
		TestEqual(TEXT("out of reach: one attacking me before a nearer one"), Chosen(Soldier(), Set), uint64(2));
	}
	{
		// Tie-break: same edge distance, smallest id, in any order.
		const TArray<FCandidate> Set = { Enemy(7, 300.), Enemy(3, -300.), Enemy(5, 0., 300.) };
		const TArray<FCandidate> Reversed = { Set[2], Set[1], Set[0] };
		TestEqual(TEXT("tie: smallest id"), Chosen(Soldier(), Set), uint64(3));
		TestEqual(TEXT("tie: same in reverse order"), Chosen(Soldier(), Reversed), uint64(3));
	}
	{
		// Aggression radius: 1380 cm edge to edge is beyond 12 m, unless it attacks me (riposte).
		TArray<FCandidate> Set = { Enemy(1, 1500.) };
		TestEqual(TEXT("beyond the aggression radius: none"), Chosen(Soldier(), Set), uint64(0));
		Set[0].bAttacksMe = true;
		TestEqual(TEXT("beyond, but attacking me: riposte"), Chosen(Soldier(), Set), uint64(1));
		Set[0].Position = FVector2D(150., 0.);
		TestEqual(TEXT("Peasant (no aggression): even attacked, none"), Chosen(Soldier(ESeekMode::Auto, 0.f), Set), uint64(0));
		TestEqual(TEXT("Peasant on an attack-move: takes it on"), Chosen(Soldier(ESeekMode::AttackMove, 0.f), Set), uint64(1));
	}
	{
		// Hold position: in reach only, never a chase, even when attacked from further.
		TArray<FCandidate> Set = { Enemy(1, 300.) };
		Set[0].bAttacksMe = true;
		TestEqual(TEXT("hold: out of reach, attacking me, none"), Chosen(Soldier(ESeekMode::Hold), Set), uint64(0));
		Set.Add(Enemy(2, 165.));
		TestEqual(TEXT("hold: the one in reach"), Chosen(Soldier(ESeekMode::Hold), Set), uint64(2));
		TestEqual(TEXT("hold: Peasant hits back in reach"), Chosen(Soldier(ESeekMode::Hold, 0.f), Set), uint64(2));
	}
	{
		// The current target in reach is kept; out of reach, the choice is made again.
		TArray<FCandidate> Set = { Enemy(1, 130.), Enemy(2, 165.) };
		Set[1].bIsCurrentTarget = true;
		TestEqual(TEXT("current target in reach: kept"), Chosen(Soldier(), Set), uint64(2));
		Set[1].Position = FVector2D(400., 0.);
		TestEqual(TEXT("current target out of reach: the one in reach"), Chosen(Soldier(), Set), uint64(1));
	}
	{
		// A unit before a building (hook), even a building in reach.
		TArray<FCandidate> Set = { Enemy(1, 150.), Enemy(2, 600.) };
		Set[0].bIsBuilding = true;
		TestEqual(TEXT("unit before building"), Chosen(Soldier(), Set), uint64(2));
		Set.RemoveAt(1);
		TestEqual(TEXT("a building alone is taken on"), Chosen(Soldier(), Set), uint64(1));
	}
	{
		// Spreading: out of reach, a crowded enemy comes after a free one further away; in reach, never passed over.
		TArray<FCandidate> Set = { Enemy(1, 300.), Enemy(2, 500.) };
		Set[0].AttackerCount = HOLCombat::MaxAttackers;
		TestEqual(TEXT("spreading: the free one"), Chosen(Soldier(), Set), uint64(2));
		Set[0].AttackerCount = HOLCombat::MaxAttackers - 1;
		TestEqual(TEXT("spreading: room left, the nearest"), Chosen(Soldier(), Set), uint64(1));
		Set[0].AttackerCount = HOLCombat::MaxAttackers + 2;
		Set[0].Position = FVector2D(160., 0.);
		TestEqual(TEXT("spreading: crowded but in reach, taken"), Chosen(Soldier(), Set), uint64(1));
	}
	{
		// Determinism: 40 enemies (ties included), shuffled 20 times, always the same one.
		FRandomStream Random(7);
		TArray<FCandidate> Set;
		for (int32 Index = 0; Index < 40; ++Index)
		{
			FCandidate Candidate = Enemy(100 + Index, 200. + 100. * Random.RandRange(0, 9), 100. * Random.RandRange(-5, 5));
			Candidate.bAttacksMe = Random.RandRange(0, 3) == 0;
			Candidate.AttackerCount = Random.RandRange(0, 8);
			Set.Add(Candidate);
		}
		const uint64 First = Chosen(Soldier(), Set);
		TestTrue(TEXT("determinism: one is chosen"), First != 0);
		bool bSame = true;
		for (int32 Shuffle = 0; Shuffle < 20; ++Shuffle)
		{
			for (int32 Index = Set.Num() - 1; Index > 0; --Index)
			{
				Set.Swap(Index, Random.RandRange(0, Index));
			}
			bSame &= Chosen(Soldier(), Set) == First;
		}
		TestTrue(TEXT("determinism: same choice in any order"), bSame);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatArrowTest, "HeroesOfLiberty.Combat.Arrow", UE::HeroesOfLiberty::Tests::Combat::Flags)

// J7c.2 checks (J7c-Q1 to Q4, Q6): the flight of an arrow by the distance, its landing, a shooter's target.
bool FHOLCombatArrowTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Combat;
	FHOLUnitTypeConstants Archer;
	FHOLUnitTypeConstants Spearman;
	if (!GetType(*this, TEXT("Archer"), Archer) || !GetType(*this, TEXT("Spearman"), Spearman))
	{
		return false;
	}
	// Shooters: the Archer's 20 m, not a melee's reach.
	TestTrue(TEXT("the Archer shoots"), HOLCombat::IsRangedAttack(Archer.AttackRange));
	TestFalse(TEXT("the Spearman does not"), HOLCombat::IsRangedAttack(Spearman.AttackRange));
	TestEqual(TEXT("default arrow speed 37.5 m/s (D345 trial)"), HOLCombat::DefaultArrowSpeed, 3750.f);

	// The flight: centre-to-centre distance over the speed (a round 25 m/s here): 15 m in 600 ms, 5 m in 200 ms, 0 at point-blank.
	const float Speed = 2500.f;
	TestEqual(TEXT("15 m in 600 ms"), HOLCombat::GetArrowFlightMilliseconds(FVector2D(0, 0), FVector2D(1500, 0), Speed), 600);
	TestEqual(TEXT("5 m (3, 4) in 200 ms"), HOLCombat::GetArrowFlightMilliseconds(FVector2D(100, 100), FVector2D(400, 500), Speed), 200);
	TestEqual(TEXT("point-blank in 0 ms"), HOLCombat::GetArrowFlightMilliseconds(FVector2D(0, 0), FVector2D(0, 0), Speed), 0);
	TestEqual(TEXT("twice the speed, half the flight"), HOLCombat::GetArrowFlightMilliseconds(FVector2D(0, 0), FVector2D(1500, 0), 2.f * Speed), 300);

	// The landing step: the flight in whole steps rounded up, 1 at least (never in the release's own step).
	TestEqual(TEXT("600 ms = 12 steps of 50 ms"), HOLCombat::GetArrowFlightSteps(600, 50), 12);
	TestEqual(TEXT("601 ms = 13 steps"), HOLCombat::GetArrowFlightSteps(601, 50), 13);
	TestEqual(TEXT("0 ms = 1 step"), HOLCombat::GetArrowFlightSteps(0, 50), 1);
	TestEqual(TEXT("10 ms = 1 step"), HOLCombat::GetArrowFlightSteps(10, 50), 1);
	const int32 Step = StepMs();
	TestEqual(TEXT("an Archer's full-reach shot at the table's step"), HOLCombat::GetArrowFlightSteps(HOLCombat::GetArrowFlightMilliseconds(FVector2D(0, 0),
		FVector2D(Archer.AttackRange + Archer.Radius + Spearman.Radius, 0), Speed), Step), FMath::DivideAndRoundUp(FMath::RoundToInt32((Archer.AttackRange + Archer.Radius + Spearman.Radius) * 1000.f / Speed), Step));

	// The landing: always hits a standing target (no dodge, J7c-Q1); lost on a dead or gone one (J7c-Q2).
	const float Damage = HOLCombat::GetDamage(Archer, Spearman);
	float Health = static_cast<float>(Spearman.MaxHealth);
	TestEqual(TEXT("hits a standing target"), HOLCombat::ResolveArrow(true, Health, Damage), HOLCombat::EArrowOutcome::Hit);
	TestEqual(TEXT("its damage off the health"), Health, static_cast<float>(Spearman.MaxHealth) - Damage);
	Health = Damage;
	TestEqual(TEXT("kills a target at its damage"), HOLCombat::ResolveArrow(true, Health, Damage), HOLCombat::EArrowOutcome::Kill);
	TestTrue(TEXT("at 0 or less"), Health <= 0);
	float Dead = 0.f;
	TestEqual(TEXT("lost on a target already at 0 (died in flight)"), HOLCombat::ResolveArrow(true, Dead, Damage), HOLCombat::EArrowOutcome::Lost);
	TestEqual(TEXT("no damage on it"), Dead, 0.f);
	float Gone = 40.f;
	TestEqual(TEXT("lost on a target gone (destroyed)"), HOLCombat::ResolveArrow(false, Gone, Damage), HOLCombat::EArrowOutcome::Lost);
	TestEqual(TEXT("its health untouched"), Gone, 40.f);

	// The target of a shooter (J7c-Q6): no spreading, the nearest even crowded, the current one kept while in reach.
	HOLCombat::FSeeker Shooter;
	Shooter.Radius = Archer.Radius;
	Shooter.AttackRange = Archer.AttackRange;
	Shooter.AggressionRadius = Archer.AggressionRadius;
	Shooter.bRanged = true;
	{
		// Out of reach both (edges 20.2 and 20.9 m: beyond the 20 m reach, within the 21.33 m aggression): a melee seeker spreads, a shooter does not.
		HOLCombat::FCandidate Near = Enemy(1, 2130.);
		Near.AttackerCount = HOLCombat::MaxAttackers;
		const HOLCombat::FCandidate Far = Enemy(2, 2200.);
		const HOLCombat::FCandidate Pair[] = { Near, Far };
		HOLCombat::FSeeker Melee = Shooter;
		Melee.bRanged = false;
		TestEqual(TEXT("a melee seeker passes the crowded nearest over"), Chosen(Melee, Pair), uint64(2));
		TestEqual(TEXT("a shooter takes the nearest, crowded or not"), Chosen(Shooter, Pair), uint64(1));
	}
	{
		// Its current target at 14 m kept while in reach, a nearer one at 3 m notwithstanding.
		HOLCombat::FCandidate Current = Enemy(5, 1400.);
		Current.bIsCurrentTarget = true;
		const HOLCombat::FCandidate Nearer = Enemy(6, 300.);
		const HOLCombat::FCandidate Pair[] = { Nearer, Current };
		TestEqual(TEXT("current target kept while in reach"), Chosen(Shooter, Pair), uint64(5));
		// Out of reach (22 m, edge 20.9 m): the nearest again.
		Current.Position = FVector2D(2200., 0.);
		const HOLCombat::FCandidate Away[] = { Nearer, Current };
		TestEqual(TEXT("out of reach: the nearest"), Chosen(Shooter, Away), uint64(6));
	}
	return true;
}

// D345.2b: units attack buildings. A unit before a building; a building only with no enemy unit (the processor asks
// ChooseBuilding only then); the Peasant never at rest; reach measured to the body's box; the table's building attacks.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatBuildingAttackTest, "HeroesOfLiberty.Combat.BuildingAttack", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatBuildingAttackTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Combat;
	FHOLUnitTypeConstants Peasant, Spearman, ManAtArms, Archer, Light;
	if (!GetType(*this, TEXT("Peasant"), Peasant) || !GetType(*this, TEXT("Spearman"), Spearman) || !GetType(*this, TEXT("ManAtArms"), ManAtArms)
		|| !GetType(*this, TEXT("Archer"), Archer) || !GetType(*this, TEXT("LightInfantry"), Light))
	{
		return false;
	}
	// The table's columns.
	TestEqual(TEXT("Peasant building attack"), Peasant.BuildingAttack, 10);
	TestEqual(TEXT("Spearman building attack"), Spearman.BuildingAttack, 20);
	TestEqual(TEXT("Man-at-arms building attack"), ManAtArms.BuildingAttack, 20);
	TestEqual(TEXT("Archer building attack"), Archer.BuildingAttack, 5);
	TestTrue(TEXT("Archer's building attack is ranged"), HOLCombat::IsBuildingShot(Archer) && !HOLCombat::IsBuildingShot(Spearman));
	TestTrue(TEXT("Spearman's building attack is melee, 1 tile"), Spearman.BuildingAttackType == EHOLAttackType::Melee && FMath::IsNearlyEqual(Spearman.BuildingAttackRange, 400.f));
	TestEqual(TEXT("Light infantry (stress tests) never attacks a building"), Light.BuildingAttack, 0);
	TestEqual(TEXT("An Archer's arrow does 1 to a building (ranged armour 40)"), HOLConstruction::GetBuildingDamage(Archer.BuildingAttack, Archer.BuildingAttackType, 40), 1);
	TestEqual(TEXT("A Spearman's blow does 20 to a building"), HOLConstruction::GetBuildingDamage(Spearman.BuildingAttack, Spearman.BuildingAttackType, 40), 20);

	// A unit before a building, whatever the order and the distance.
	HOLCombat::FSeeker Seeker;
	Seeker.Radius = Spearman.Radius;
	Seeker.AttackRange = Spearman.AttackRange;
	Seeker.AggressionRadius = Spearman.AggressionRadius;
	TArray<HOLCombat::FCandidate> Candidates;
	HOLCombat::FCandidate& Building = Candidates.AddDefaulted_GetRef();
	Building.Id = 1;
	Building.Position = FVector2D(100.0, 0.0);
	Building.bIsBuilding = true;
	HOLCombat::FCandidate& Unit = Candidates.AddDefaulted_GetRef();
	Unit.Id = 2;
	Unit.Position = FVector2D(1200.0, 0.0);
	TestEqual(TEXT("An enemy unit is taken before a building"), HOLCombat::ChooseTarget(Seeker, Candidates), 1);

	// An Archer shooting a building (trial 5): an enemy unit within its aggression, even beyond its reach, is taken (it
	// drops the building); one beyond its aggression is not. Edge to edge: 21 m within 21.33 m, 22 m beyond.
	HOLCombat::FSeeker ArcherSeeker;
	ArcherSeeker.Radius = Archer.Radius;
	ArcherSeeker.AttackRange = Archer.AttackRange;
	ArcherSeeker.AggressionRadius = Archer.AggressionRadius;
	ArcherSeeker.bRanged = HOLCombat::IsRangedAttack(Archer.AttackRange);
	TArray<HOLCombat::FCandidate> ArcherCandidates;
	HOLCombat::FCandidate& Near = ArcherCandidates.AddDefaulted_GetRef();
	Near.Id = 3;
	Near.Radius = Spearman.Radius;
	Near.Position = FVector2D(static_cast<double>(Archer.Radius) + Spearman.Radius + 2100.0, 0.0);
	TestEqual(TEXT("An Archer takes an enemy 21 m away, beyond its reach, within its aggression"), HOLCombat::ChooseTarget(ArcherSeeker, ArcherCandidates), 0);
	Near.Position.X += 100.0;
	TestEqual(TEXT("An Archer takes no enemy 22 m away, beyond its aggression"), HOLCombat::ChooseTarget(ArcherSeeker, ArcherCandidates), INDEX_NONE);
	// D345: that search runs for a building taken on by itself, at rest or on an attack-move (it drops it for the enemy 21 m
	// away); never for an ordered one (a right click, Shift or not): struck until it falls. An ordered unit target as before.
	TestTrue(TEXT("Its own building, at rest: it looks for enemy units (drops it)"), HOLCombat::MaySeek(false, false, false, false));
	TestTrue(TEXT("Its own building, on an attack-move: it looks for enemy units (drops it)"), HOLCombat::MaySeek(true, true, false, false));
	TestFalse(TEXT("An ordered building: no enemy unit taken on (never dropped)"), HOLCombat::MaySeek(false, false, false, true));
	TestFalse(TEXT("An ordered unit target: none taken on either"), HOLCombat::MaySeek(false, false, true, false));
	TestFalse(TEXT("A plain walk: none taken on"), HOLCombat::MaySeek(true, false, false, false));

	// Search radii: at rest the aggression, the Peasant none; attack-move; hold its reach.
	TestTrue(TEXT("A Spearman at rest looks for buildings within its aggression"), FMath::IsNearlyEqual(HOLCombat::GetBuildingSearchRadius(Seeker, Spearman), Spearman.AggressionRadius));
	HOLCombat::FSeeker PeasantSeeker;
	PeasantSeeker.AggressionRadius = Peasant.AggressionRadius;
	TestEqual(TEXT("A Peasant at rest never takes on a building"), HOLCombat::GetBuildingSearchRadius(PeasantSeeker, Peasant), 0.f);
	PeasantSeeker.Mode = HOLCombat::ESeekMode::AttackMove;
	TestTrue(TEXT("A Peasant on an attack-move looks within AttackMoveRadius"), FMath::IsNearlyEqual(HOLCombat::GetBuildingSearchRadius(PeasantSeeker, Peasant), HOLCombat::AttackMoveRadius));
	HOLCombat::FSeeker HoldSeeker = Seeker;
	HoldSeeker.Mode = HOLCombat::ESeekMode::Hold;
	TestTrue(TEXT("On hold, only its building reach"), FMath::IsNearlyEqual(HOLCombat::GetBuildingSearchRadius(HoldSeeker, Spearman), Spearman.BuildingAttackRange));
	HOLCombat::FSeeker LightSeeker;
	LightSeeker.AggressionRadius = Light.AggressionRadius;
	TestEqual(TEXT("Light infantry never looks for buildings"), HOLCombat::GetBuildingSearchRadius(LightSeeker, Light), 0.f);

	// Reach to the body's box (D324), edge to edge.
	const FBox2D Box(FVector2D(0.0, 0.0), FVector2D(400.0, 400.0));
	TestTrue(TEXT("Edge distance to the box's side"), FMath::IsNearlyEqual(HOLCombat::GetEdgeDistanceToBox(FVector2D(600.0, 200.0), 60.f, Box), 140.0));
	TestTrue(TEXT("Edge distance to the box's corner"), FMath::IsNearlyEqual(HOLCombat::GetEdgeDistanceToBox(FVector2D(700.0, 800.0), 0.f, Box), 500.0));
	TestTrue(TEXT("Walking in, a melee strikes once beside the wall"), HOLCombat::IsBuildingInReach(Spearman, 140.0, false, false, false));
	TestFalse(TEXT("Walking in, not yet from 3 m"), HOLCombat::IsBuildingInReach(Spearman, 300.0, false, false, false));
	TestTrue(TEXT("Striking, it keeps its tile of reach"), HOLCombat::IsBuildingInReach(Spearman, 390.0, true, false, false));
	TestTrue(TEXT("Its walk ended short (the footprint), it strikes within its tile"), HOLCombat::IsBuildingInReach(Spearman, 300.0, false, true, false));
	TestFalse(TEXT("Beyond its tile, never"), HOLCombat::IsBuildingInReach(Spearman, 410.0, true, true, true));
	TestTrue(TEXT("An Archer shoots from 19 m"), HOLCombat::IsBuildingInReach(Archer, 1900.0, false, false, false));

	// The nearest building within the radius, smallest id on a tie.
	TArray<HOLCombat::FBuildingCandidate> Buildings;
	Buildings.Add({ 7, FBox2D(FVector2D(1000.0, 0.0), FVector2D(1400.0, 400.0)) });
	Buildings.Add({ 5, FBox2D(FVector2D(-1400.0, 0.0), FVector2D(-1000.0, 400.0)) });
	Buildings.Add({ 9, FBox2D(FVector2D(0.0, 3000.0), FVector2D(400.0, 3400.0)) });
	TestEqual(TEXT("Nearest building, smallest id on a tie"), HOLCombat::ChooseBuilding(FVector2D(0.0, 200.0), 60.f, 1600.f, Buildings), 1);
	TestEqual(TEXT("None within the radius"), HOLCombat::ChooseBuilding(FVector2D(0.0, 200.0), 60.f, 500.f, Buildings), INDEX_NONE);
	TestEqual(TEXT("No radius, none"), HOLCombat::ChooseBuilding(FVector2D(0.0, 200.0), 60.f, 0.f, Buildings), INDEX_NONE);

	// The approach point: off the nearest side by its radius and the gap.
	const FVector2D Point = HOLCombat::GetBuildingApproachPoint(FVector2D(1000.0, 200.0), 60.f, Box, 40.0);
	TestTrue(TEXT("Approach point beside the nearest side"), Point.Equals(FVector2D(500.0, 200.0), 0.01));
	return true;
}

// EtapeDefini J: one place per unit of a group ordered to strike a building, off the visible wall by a radius and
// BuildingPlaceVisibleGap. Loop 11b: at contact, a continuous arc centred in front of the group (corners passed without a
// hole); loop 12: its places 2.2 radii apart (10 % air); a ring full: evenly round, the rest on a second ring. And the chain of places of GetBuildingPlaceChain.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatBuildingPlacesTest, "HeroesOfLiberty.Combat.BuildingPlaces", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatBuildingPlacesTest::RunTest(const FString& Parameters)
{
	// The visible wall's box (GetVisibleBodyBounds), a Man-at-arms' radius, the gap of the game.
	const FBox2D Box(FVector2D(0.0, 0.0), FVector2D(400.0, 400.0));
	const float Radius = 60.f;
	const double Gap = HOLCombat::BuildingPlaceVisibleGap;
	const double D = Radius + Gap;
	// Loop 12: the arc's places BuildingPlaceArcRadii (2.2) radii apart: 132 cm for a Man-at-arms.
	const double ArcStep = HOLCombat::BuildingPlaceArcRadii * Radius;
	const double Arc = 0.5 * UE_DOUBLE_PI * D;
	const double Length = 4.0 * 400.0 + 4.0 * Arc;
	const auto MinPair = [](const TArray<FVector2D>& Places)
	{
		double Least = TNumericLimits<double>::Max();
		for (int32 A = 0; A < Places.Num(); ++A)
		{
			for (int32 B = A + 1; B < Places.Num(); ++B)
			{
				Least = FMath::Min(Least, FVector2D::Distance(Places[A], Places[B]));
			}
		}
		return Least;
	};
	// Where a point of the ring (off Box by Ring) lies along it, from the Min corner along +X, quarter arcs at the corners.
	const auto RingS = [&Box](const FVector2D& P, const double Ring) -> double
	{
		const double W = Box.Max.X - Box.Min.X, H = Box.Max.Y - Box.Min.Y, A = 0.5 * UE_DOUBLE_PI * Ring;
		const bool bLeft = P.X < Box.Min.X, bRight = P.X > Box.Max.X, bDown = P.Y < Box.Min.Y, bUp = P.Y > Box.Max.Y;
		if (bRight && bDown)
		{
			const FVector2D V = P - FVector2D(Box.Max.X, Box.Min.Y);
			return W + FMath::Atan2(V.X, -V.Y) * Ring;
		}
		if (bRight && bUp)
		{
			const FVector2D V = P - Box.Max;
			return W + A + H + FMath::Atan2(V.Y, V.X) * Ring;
		}
		if (bLeft && bUp)
		{
			const FVector2D V = P - FVector2D(Box.Min.X, Box.Max.Y);
			return 2.0 * W + 2.0 * A + H + FMath::Atan2(-V.X, V.Y) * Ring;
		}
		if (bLeft && bDown)
		{
			const FVector2D V = P - Box.Min;
			return 2.0 * (W + H) + 3.0 * A + FMath::Atan2(-V.Y, -V.X) * Ring;
		}
		if (bDown)
		{
			return P.X - Box.Min.X;
		}
		if (bRight)
		{
			return W + A + (P.Y - Box.Min.Y);
		}
		if (bUp)
		{
			return W + 2.0 * A + H + (Box.Max.X - P.X);
		}
		return 2.0 * W + 3.0 * A + H + (Box.Max.Y - P.Y);
	};
	// Evenly round the ring: the gaps between places one after the other along it (the last one round to the first) all equal.
	const auto Even = [&RingS](const TArray<FVector2D>& Places, const double Ring, const double RingLength)
	{
		TArray<double> S;
		for (const FVector2D& Place : Places)
		{
			S.Add(RingS(Place, Ring));
		}
		S.Sort();
		bool bEven = S.Num() > 0;
		for (int32 Index = 0; bEven && Index < S.Num(); ++Index)
		{
			const double Next = Index + 1 < S.Num() ? S[Index + 1] : S[0] + RingLength;
			bEven = FMath::IsNearlyEqual(Next - S[Index], RingLength / S.Num(), 0.5);
		}
		return bEven;
	};
	const auto OnRing = [&Box, Radius](const FVector2D& Place, const double EdgeGap)
	{
		return FMath::IsNearlyEqual(HOLCombat::GetEdgeDistanceToBox(Place, Radius, Box), EdgeGap, 0.5);
	};
	// At contact: the places sorted along the ring, each two next ones ArcStep apart in a straight line (within 2 cm: round a
	// corner the place moves on by 1 cm steps), and the gap that closes the ring (the void) wider: one arc, no hole.
	const auto Contact = [&RingS, ArcStep](const TArray<FVector2D>& Places, const double Ring, const double RingLength)
	{
		TArray<TPair<double, FVector2D>> S;
		for (const FVector2D& Place : Places)
		{
			S.Emplace(RingS(Place, Ring), Place);
		}
		S.Sort([](const TPair<double, FVector2D>& A, const TPair<double, FVector2D>& B) { return A.Key < B.Key; });
		// Start after the widest gap along the ring (the void), then every next one at contact.
		int32 After = 0;
		double Widest = -1.0;
		for (int32 Index = 0; Index < S.Num(); ++Index)
		{
			const double Next = Index + 1 < S.Num() ? S[Index + 1].Key : S[0].Key + RingLength;
			if (Next - S[Index].Key > Widest)
			{
				Widest = Next - S[Index].Key;
				After = (Index + 1) % S.Num();
			}
		}
		bool bContact = S.Num() > 0 && Widest > ArcStep + 2.0;
		for (int32 Step = 0; bContact && Step + 1 < S.Num(); ++Step)
		{
			const double Apart = FVector2D::Distance(S[(After + Step) % S.Num()].Value, S[(After + Step + 1) % S.Num()].Value);
			bContact = Apart >= ArcStep - 0.5 && Apart <= ArcStep + 2.0;
		}
		return bContact;
	};
	TArray<FVector2D> Places;

	// A group of 1: the ring's point nearest it, off the visible wall.
	HOLCombat::GetBuildingPlaces({ FVector2D(1000.0, 200.0) }, { 7u }, Radius, Box, Gap, Places);
	TestTrue(TEXT("Alone: beside the nearest side, off the visible wall"), Places.Num() == 1 && Places[0].Equals(FVector2D(400.0 + D, 200.0), 0.01));

	// A group of 4 in front of the bottom side (its middle at x = 200): 4 places side by side, 2.2 radii apart, centred in
	// front of it (x = 2, 134, 266, 398), the units from left to right in the same order (no crossing).
	TArray<FVector2D> Positions;
	TArray<uint32> Ids;
	for (int32 Index = 0; Index < 4; ++Index)
	{
		Positions.Add(FVector2D(20.0 + Index * 120.0, -800.0));
		Ids.Add(50u + Index);
	}
	HOLCombat::GetBuildingPlaces(Positions, Ids, Radius, Box, Gap, Places);
	bool bFour = Places.Num() == 4;
	for (int32 Index = 0; bFour && Index < 4; ++Index)
	{
		bFour = Places[Index].Equals(FVector2D(2.0 + Index * ArcStep, -D), 0.01);
	}
	TestTrue(TEXT("Four: side by side 2.2 radii apart, centred in front of the group, in the units' order"), bFour);
	TestTrue(TEXT("Four: at contact, one arc"), Contact(Places, D, Length));

	// A tight group of 6 in a bias towards the top right corner, symmetric about its diagonal (middle 980, 980): an arc through
	// the corner, 3 on each side, no hole there. Loop 12 (2.2 radii): the two by the corner 79 cm along the ring either side
	// of its middle (the chord across the corner 133 cm), then 211 and 343: on the right side y = 376, 244, 112 (8 would not
	// fit a 400 cm side any more: the 4th would round the next corner).
	Positions = { FVector2D(900.0, 900.0), FVector2D(1020.0, 900.0), FVector2D(900.0, 1020.0), FVector2D(1140.0, 900.0), FVector2D(900.0, 1140.0),
		FVector2D(1020.0, 1020.0) };
	Ids = { 100u, 101u, 102u, 103u, 104u, 105u };
	HOLCombat::GetBuildingPlaces(Positions, Ids, Radius, Box, Gap, Places);
	TestEqual(TEXT("Bias: a place each"), Places.Num(), 6);
	TestTrue(FString::Printf(TEXT("Bias: 2.2 radii apart (%.0f cm)"), MinPair(Places)), MinPair(Places) >= ArcStep - 0.5);
	bool bAllOnRing = true;
	int32 Down = 0, Right = 0, Up = 0, Left = 0;
	for (const FVector2D& Place : Places)
	{
		bAllOnRing &= OnRing(Place, Gap);
		Down += Place.Y < Box.Min.Y ? 1 : 0;
		Right += Place.X > Box.Max.X ? 1 : 0;
		Up += Place.Y > Box.Max.Y ? 1 : 0;
		Left += Place.X < Box.Min.X ? 1 : 0;
	}
	TestTrue(TEXT("Bias: all off the visible wall by a radius and the gap"), bAllOnRing);
	TestTrue(FString::Printf(TEXT("Bias: through the corner, half on each side (%d down, %d right, %d up, %d left)"), Down, Right, Up, Left),
		Down == 0 && Left == 0 && Right == 3 && Up == 3);
	TestTrue(TEXT("Bias: at contact through the corner, one arc"), Contact(Places, D, Length));

	// Stable: the same place for each unit whatever the order of the list, and on every call.
	TArray<FVector2D> ReversedPositions(Positions);
	TArray<uint32> ReversedIds(Ids);
	Algo::Reverse(ReversedPositions);
	Algo::Reverse(ReversedIds);
	TArray<FVector2D> Reversed;
	HOLCombat::GetBuildingPlaces(ReversedPositions, ReversedIds, Radius, Box, Gap, Reversed);
	bool bStable = Reversed.Num() == Places.Num();
	for (int32 Index = 0; bStable && Index < Places.Num(); ++Index)
	{
		bStable = Reversed[Places.Num() - 1 - Index].Equals(Places[Index], 0.001);
	}
	TestTrue(TEXT("Stable: whatever the order of the list"), bStable);

	// All on one spot: ties by Id, still a place each, at contact (the two ends round the bottom corners).
	TArray<FVector2D> Same;
	Same.Init(FVector2D(200.0, -800.0), 5);
	HOLCombat::GetBuildingPlaces(Same, { 5u, 4u, 3u, 2u, 1u }, Radius, Box, Gap, Places);
	TestTrue(FString::Printf(TEXT("One spot: a place each (%.0f cm apart)"), MinPair(Places)), Places.Num() == 5 && MinPair(Places) >= ArcStep - 0.5);
	TestTrue(TEXT("One spot: at contact, one arc"), Contact(Places, D, Length));

	// Bigger than the ring (2040 cm round: 16 places along it, 14 for the chords round the corners): the first ring full,
	// evenly round; the rest on a second ring, 2 radii further, in an arc at contact; never two places nearer than 2 radii.
	Positions.Reset();
	Ids.Reset();
	for (int32 Index = 0; Index < 25; ++Index)
	{
		Positions.Add(FVector2D(-200.0 + (Index % 5) * 120.0, -600.0 - (Index / 5) * 120.0));
		Ids.Add(200u + Index);
	}
	HOLCombat::GetBuildingPlaces(Positions, Ids, Radius, Box, Gap, Places);
	TArray<FVector2D> FirstRing;
	TArray<FVector2D> SecondRing;
	for (const FVector2D& Place : Places)
	{
		(OnRing(Place, Gap) ? FirstRing : SecondRing).Add(Place);
	}
	bool bSecondOnRing = true;
	for (const FVector2D& Place : SecondRing)
	{
		bSecondOnRing &= OnRing(Place, Gap + 2.0 * Radius);
	}
	const double SecondD = D + 2.0 * Radius;
	TestTrue(FString::Printf(TEXT("Full: 13 to 16 on the first ring (%d)"), FirstRing.Num()), FirstRing.Num() >= 13 && FirstRing.Num() <= 16 && SecondRing.Num() > 0);
	TestTrue(FString::Printf(TEXT("Full: the rest on the second ring, 2 radii further (%d)"), SecondRing.Num()),
		bSecondOnRing && FirstRing.Num() + SecondRing.Num() == 25);
	TestTrue(TEXT("Full: the first ring evenly round"), Even(FirstRing, D, Length));
	TestTrue(TEXT("Full: the second ring at contact, one arc"), Contact(SecondRing, SecondD, 4.0 * 400.0 + 2.0 * UE_DOUBLE_PI * SecondD));
	TestTrue(FString::Printf(TEXT("Full: 2 radii apart (%.0f cm)"), MinPair(Places)), MinPair(Places) >= 2.0 * Radius - 0.5);
	TestTrue(TEXT("Full: the second ring's units came from the back"), OnRing(Places[24], Gap + 2.0 * Radius));

	// GetBuildingPlaceChain: a unit at x = 30 below the box, its place at x = 380 on the first ring; three friends' places on
	// the same ring between (x = 300, 100, 200), one on the second ring and one behind the unit (left side): the chain gives
	// the three in order along the way, nearest the unit first.
	const TArray<FVector2D> Others = { FVector2D(300.0, -D), FVector2D(100.0, -D), FVector2D(200.0, -D - 2.0 * Radius), FVector2D(200.0, -D),
		FVector2D(-D, 200.0) };
	TArray<int32> Chain;
	HOLCombat::GetBuildingPlaceChain(FVector2D(30.0, -D - 5.0), FVector2D(380.0, -D), Others, Box, Chain);
	TestTrue(FString::Printf(TEXT("Chain: the three between, nearest first (%d found)"), Chain.Num()),
		Chain.Num() == 3 && Chain[0] == 1 && Chain[1] == 3 && Chain[2] == 0);
	return true;
}

#endif
