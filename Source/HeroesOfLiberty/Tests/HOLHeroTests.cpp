// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/AnimSequenceTransformProviderData.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// J8a.1 checks (J8a-Q1, D17): the Paladin-Commander's type, read from the real unit type table (Tools/Data/UnitTypes.csv
// imported by Tools/Editor/make_unit_types.py), and his resistance to the troops' hits (x0.3).
namespace UE::HeroesOfLiberty::Tests::Hero
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FHOLUnitTypeRow* FindRow(const TCHAR* Name)
	{
		const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		return UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(Name, TEXT("Hero test"), false) : nullptr;
	}

	/** D17 as written in J8a-Q1, independent of the code: the troop's hit times 0.3, exactly (health is a float). */
	float ExpectedTroopHitOnHero(const int32 PlainDamage)
	{
		return static_cast<float>(PlainDamage) * 0.3f;
	}

	/** A hit's damage as a float (the cast keeps the test readable whatever GetDamage returns). */
	float Hit(const FHOLUnitTypeConstants& Attacker, const FHOLUnitTypeConstants& Defender)
	{
		return static_cast<float>(HOLCombat::GetDamage(Attacker, Defender));
	}

	constexpr float Tolerance = 1.e-4f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroTypeTest, "HeroesOfLiberty.Hero.Type", UE::HeroesOfLiberty::Tests::Hero::Flags)

bool FHOLHeroTypeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Hero;
	const FHOLUnitTypeRow* Hero = FindRow(TEXT("Hero"));
	const FHOLUnitTypeRow* ManAtArms = FindRow(TEXT("ManAtArms"));
	const FHOLUnitTypeRow* Spearman = FindRow(TEXT("Spearman"));
	if (!TestNotNull(TEXT("The table has a Hero"), Hero) || !TestNotNull(TEXT("... a Man-at-arms"), ManAtArms)
		|| !TestNotNull(TEXT("... a Spearman"), Spearman))
	{
		return false;
	}
	// Combat (J8a-Q1).
	TestEqual(TEXT("450 HP"), Hero->MaxHealth, 450);
	TestEqual(TEXT("30 of attack"), Hero->Attack, 30);
	TestTrue(TEXT("A hero attack (D345)"), Hero->AttackType == EHOLAttackType::Hero);
	TestEqual(TEXT("Every 1.25 s"), Hero->AttackInterval, 1.25f);
	TestEqual(TEXT("Melee reach 0.7 m"), Hero->AttackRange, 70.f);
	TestEqual(TEXT("A melee wind-up (the Man-at-arms')"), Hero->AttackWindUpPercent, ManAtArms->AttackWindUpPercent);
	TestTrue(TEXT("No armour (D17: the resistance is the x0.3)"),
		Hero->MeleeArmor == 0 && Hero->RangedArmor == 0 && Hero->SiegeArmor == 0 && Hero->HeroArmor == 0);
	TestEqual(TEXT("No bonus"), Hero->Bonuses.Num(), 0);
	TestEqual(TEXT("Aggression radius of 4 tiles (the Spearman's)"), Hero->AggressionRadius, Spearman->AggressionRadius);
	TestEqual(TEXT("... 16 m"), Hero->AggressionRadius, 1600.f);
	TestTrue(TEXT("It ripostes"), Hero->Ripostes);
	TestTrue(TEXT("Of the Hero category (D17), alone"), Hero->HasArmorCategory(EHOLArmorCategory::Hero) && Hero->ArmorCategories.Num() == 1);
	// Walk.
	TestEqual(TEXT("1.74 tiles/s, the Man-at-arms' speed"), Hero->Speed, ManAtArms->Speed);
	TestEqual(TEXT("... 697"), Hero->Speed, 697.f);
	// Buildings.
	TestTrue(TEXT("A melee building attack"), Hero->BuildingAttackType == EHOLAttackType::Melee);
	TestEqual(TEXT("10 against buildings"), Hero->BuildingAttack, 10);
	TestEqual(TEXT("... every 2.1 s"), Hero->BuildingAttackInterval, 2.1f);
	TestEqual(TEXT("... from a melee building reach"), Hero->BuildingAttackRange, ManAtArms->BuildingAttackRange);
	// Outside population, produced nowhere (J8a-Q2).
	TestEqual(TEXT("Outside the population"), Hero->Population, 0);
	TestTrue(TEXT("Free"), Hero->Cost.Food == 0 && Hero->Cost.Wood == 0 && Hero->Cost.Stone == 0 && Hero->Cost.Gold == 0);
	TestTrue(TEXT("No gathering, nothing carried"), Hero->GatherRates.Food <= 0.f && Hero->GatherRates.Wood <= 0.f
		&& Hero->GatherRates.Stone <= 0.f && Hero->GatherRates.Gold <= 0.f && Hero->CarryCapacity == 0);
	TestEqual(TEXT("No building gesture"), Hero->BuildSequence, -1);
	// Body: 1.2 times the others' (J8a-Q3).
	TestEqual(TEXT("Body radius 72 cm (1.2 x 60)"), Hero->Radius, 72.f);
	TestEqual(TEXT("Mesh scale 1.8 (1.2 x 1.5)"), Hero->MeshScale, 1.8f);
	TestEqual(TEXT("... the Man-at-arms' times 1.2"), Hero->MeshScale, ManAtArms->MeshScale * 1.2f, 1.e-4f);
	// His own skinned mesh (J8a.2: SKM_Hero, the paladin pieces of soldier_props.py) and provider (TP_Hero), playing the
	// Man-at-arms' gestures; the walk slowed by the scale (D234).
	TestEqual(TEXT("His own skinned mesh"), Hero->SkinnedMesh.ToSoftObjectPath(),
		FSoftObjectPath(TEXT("/Game/Units/Mannequin/SKM_Hero.SKM_Hero")));
	TestEqual(TEXT("... and his own animation provider"), Hero->AnimationProvider.ToSoftObjectPath(),
		FSoftObjectPath(TEXT("/Game/Units/Mannequin/TP_Hero.TP_Hero")));
	const USkeletalMesh* HeroMesh = Hero->SkinnedMesh.LoadSynchronous();
	const USkeletalMesh* ManAtArmsMesh = ManAtArms->SkinnedMesh.LoadSynchronous();
	if (TestNotNull(TEXT("SKM_Hero loads"), HeroMesh) && TestNotNull(TEXT("... SKM_ManAtArms too"), ManAtArmsMesh))
	{
		TestEqual(TEXT("SKM_Hero: the Man-at-arms' LODs"), HeroMesh->GetLODNum(), ManAtArmsMesh->GetLODNum());
		TestEqual(TEXT("... the same skeleton"), HeroMesh->GetSkeleton(), ManAtArmsMesh->GetSkeleton());
	}
	const UAnimSequenceTransformProviderData* HeroProvider = Hero->AnimationProvider.LoadSynchronous();
	const UAnimSequenceTransformProviderData* ManAtArmsProvider = ManAtArms->AnimationProvider.LoadSynchronous();
	if (TestNotNull(TEXT("TP_Hero loads"), HeroProvider) && TestNotNull(TEXT("... TP_ManAtArms too"), ManAtArmsProvider))
	{
		TestTrue(TEXT("TP_Hero drives SKM_Hero"), HeroProvider->GetSkinnedAsset() == HeroMesh);
		const TArray<FAnimSequenceTransformProviderSequence>& Own = HeroProvider->GetSequences();
		const TArray<FAnimSequenceTransformProviderSequence>& Theirs = ManAtArmsProvider->GetSequences();
		bool bSame = Own.Num() == Theirs.Num() && Own.Num() == 6;
		for (int32 Index = 0; bSame && Index < Own.Num(); ++Index)
		{
			bSame = Own[Index].Sequence != nullptr && Own[Index].Sequence == Theirs[Index].Sequence;
		}
		TestTrue(TEXT("TP_Hero: the Man-at-arms' 6 sequences, same order"), bSame);
	}
	TestTrue(TEXT("... his idle, walk, strike and fall"), Hero->IdleSequence == ManAtArms->IdleSequence
		&& Hero->WalkSequence == ManAtArms->WalkSequence && Hero->AttackSequence == ManAtArms->AttackSequence
		&& Hero->FallSequence == ManAtArms->FallSequence);
	TestEqual(TEXT("Walk play rate: the Man-at-arms' / 1.2 (longer stride, same speed)"), Hero->WalkPlayRate,
		ManAtArms->WalkPlayRate / 1.2f, 0.01f);

	const FHOLUnitTypeConstants Constants = Hero->ToConstants(TEXT("Hero"));
	TestTrue(TEXT("Its constants carry the Hero category"), HOLIsHero(Constants.ArmorCategoryMask));
	TestFalse(TEXT("A Man-at-arms is no hero"), HOLIsHero(ManAtArms->ToConstants(TEXT("ManAtArms")).ArmorCategoryMask));
	TestTrue(TEXT("The hero can be healed (only the King cannot)"), HOLCanBeHealed(Constants.ArmorCategoryMask));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroResistanceTest, "HeroesOfLiberty.Hero.Resistance", UE::HeroesOfLiberty::Tests::Hero::Flags)

bool FHOLHeroResistanceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Hero;
	const TCHAR* Names[] = { TEXT("Hero"), TEXT("ManAtArms"), TEXT("Spearman"), TEXT("Archer"), TEXT("Peasant"), TEXT("King") };
	TMap<FString, FHOLUnitTypeConstants> Types;
	for (const TCHAR* Name : Names)
	{
		const FHOLUnitTypeRow* Row = FindRow(Name);
		if (!TestNotNull(FString::Printf(TEXT("The table has a %s"), Name), Row))
		{
			return false;
		}
		Types.Add(Name, Row->ToConstants(FName(Name)));
	}
	const FHOLUnitTypeConstants& Hero = Types[TEXT("Hero")];
	const FHOLUnitTypeConstants& ManAtArms = Types[TEXT("ManAtArms")];
	const FHOLUnitTypeConstants& Spearman = Types[TEXT("Spearman")];
	const FHOLUnitTypeConstants& Archer = Types[TEXT("Archer")];
	const FHOLUnitTypeConstants& Peasant = Types[TEXT("Peasant")];
	const FHOLUnitTypeConstants& King = Types[TEXT("King")];

	// D17: a troop's hit on the hero, x0.3 (after its bonuses and the hero's armour, all 0), rounded to nearest, 1 at least.
	for (const FHOLUnitTypeConstants* Troop : { &Spearman, &ManAtArms, &Archer, &Peasant, &King })
	{
		const int32 Plain = FMath::Max(Troop->Attack - HOLCombat::GetArmor(Hero, Troop->AttackType), 1);
		TestEqual(FString::Printf(TEXT("%s on the hero: x0.3 of %d"), *Troop->TypeName.ToString(), Plain),
			Hit(*Troop, Hero), ExpectedTroopHitOnHero(Plain), Tolerance);
	}
	// The same, written out (the table's values at J8a.1).
	TestEqual(TEXT("Man-at-arms 12 -> 3.6"), Hit(ManAtArms, Hero), 3.6f, Tolerance);
	TestEqual(TEXT("Spearman 8 -> 2.4"), Hit(Spearman, Hero), 2.4f, Tolerance);
	TestEqual(TEXT("Archer 5 -> 1.5"), Hit(Archer, Hero), 1.5f, Tolerance);
	TestEqual(TEXT("Peasant 3 -> 0.9 (below 1: the x0.3 comes after the 1 at least)"), Hit(Peasant, Hero), 0.9f, Tolerance);
	TestEqual(TEXT("King 50 -> 15 (not a hero: a troop)"), Hit(King, Hero), 15.f, Tolerance);

	// The hero's own hits are not reduced: on a Man-at-arms, 30 minus his hero armour (2).
	TestEqual(TEXT("Hero on a Man-at-arms: 30 - 2"), Hit(Hero, ManAtArms), static_cast<float>(30 - ManAtArms.HeroArmor), Tolerance);
	TestEqual(TEXT("... 28"), Hit(Hero, ManAtArms), 28.f, Tolerance);
	TestEqual(TEXT("Hero on a Spearman: 30"), Hit(Hero, Spearman), 30.f, Tolerance);
	// Hero against hero: no resistance (a duel, full damage).
	TestEqual(TEXT("Hero on a hero: full, 30"), Hit(Hero, Hero), 30.f, Tolerance);

	// The others unchanged.
	TestEqual(TEXT("Man-at-arms on a Spearman: 12"), Hit(ManAtArms, Spearman), 12.f, Tolerance);
	TestEqual(TEXT("Spearman on a Man-at-arms: 8 - 4"), Hit(Spearman, ManAtArms), 4.f, Tolerance);
	TestEqual(TEXT("Archer on a Spearman: 5 + 5 (light infantry)"), Hit(Archer, Spearman), 10.f, Tolerance);
	TestEqual(TEXT("Peasant on a Man-at-arms: 1 at least"), Hit(Peasant, ManAtArms), 1.f, Tolerance);
	TestEqual(TEXT("Man-at-arms on the King: 12"), Hit(ManAtArms, King), 12.f, Tolerance);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroShownHealthTest, "HeroesOfLiberty.Hero.ShownHealth", UE::HeroesOfLiberty::Tests::Hero::Flags)

// Health is a float in the simulation; the view (HUD, bars) and the wire show it as an integer rounded up, 1 at least
// alive, 0 dead (HOLCombat::GetShownHealth).
bool FHOLHeroShownHealthTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("446.4 -> 447"), HOLCombat::GetShownHealth(446.4f), 447);
	TestEqual(TEXT("154.1 -> 155"), HOLCombat::GetShownHealth(154.1f), 155);
	TestEqual(TEXT("155 -> 155"), HOLCombat::GetShownHealth(155.f), 155);
	TestEqual(TEXT("0.4 alive -> 1"), HOLCombat::GetShownHealth(0.4f), 1);
	TestEqual(TEXT("0 -> 0 (dead)"), HOLCombat::GetShownHealth(0.f), 0);
	TestEqual(TEXT("-2.5 -> 0 (dead)"), HOLCombat::GetShownHealth(-2.5f), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroResistanceInCombatTest, "HeroesOfLiberty.Hero.ResistanceInCombat", UE::HeroesOfLiberty::Tests::Hero::Flags)

// The x0.3 through the combat processor, as in a match: a Man-at-arms of slot 1 beside slot 0's hero; the first hit on the
// hero takes 3.6 HP (12 x 0.3: 446.4, shown 447), the hero's first on the Man-at-arms 28.
bool FHOLHeroResistanceInCombatTest::RunTest(const FString& Parameters)
{
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Combat, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		// As the Regicide tests' PlaceUnit: the test world's step index stays 0 and a unit searches one step of 4 by its
		// identifier (UHOLCombatProcessor), so it is born again until its identifier is a multiple of 4.
		const auto Place = [Units](const FName Type, const int32 Owner, const FVector& Location)
		{
			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
			{
				TArray<uint32> Before;
				Units->GetAllUnitIds(Before);
				FHOLUnitSpawnRequest Request;
				Request.TypeName = Type;
				Request.Count = 1;
				Request.OwnerPlayer = static_cast<uint8>(Owner);
				Request.Center = Location;
				Units->RequestUnits(Request);
				Units->ServeBirthRequests();
				TArray<uint32> After;
				Units->GetAllUnitIds(After);
				uint32 Born = 0;
				for (const uint32 Id : After)
				{
					Born = Before.Contains(Id) ? Born : Id;
				}
				if (Born == 0 || Born % 4 == 0)
				{
					return Born;
				}
				const uint32 Again[] = { Born };
				Units->RequestDestroyUnits(Again);
				Units->FlushDestructions();
			}
			return 0u;
		};
		const auto GetHealth = [Units](const uint32 Id)
		{
			FHOLUnitView View;
			return Units->GetUnitView(Id, View) ? View.Health : -1;
		};

		// Radii 72 and 60, reach 70: 150 cm apart, in reach of each other.
		const uint32 Hero = Place(FName(TEXT("Hero")), 0, FVector(0.0, 0.0, 0.0));
		const uint32 Foe = Place(FName(TEXT("ManAtArms")), 1, FVector(150.0, 0.0, 0.0));
		if (TestTrue(TEXT("a hero and a Man-at-arms born"), Hero != 0 && Foe != 0))
		{
			const int32 HeroFull = GetHealth(Hero);
			const int32 FoeFull = GetHealth(Foe);
			TestEqual(TEXT("the hero born with 450 HP"), HeroFull, 450);
			int32 HeroLoss = 0;
			int32 FoeLoss = 0;
			for (int32 Steps = 0; Steps < FMath::CeilToInt(10.f / StepSeconds) && (HeroLoss == 0 || FoeLoss == 0); ++Steps)
			{
				RunStep();
				HeroLoss = HeroLoss == 0 ? HeroFull - GetHealth(Hero) : HeroLoss;
				FoeLoss = FoeLoss == 0 ? FoeFull - GetHealth(Foe) : FoeLoss;
			}
			// 450 - 3.6 = 446.4, shown rounded up (HOLCombat::GetShownHealth): 447.
			TestEqual(TEXT("the Man-at-arms' first hit on the hero: 12 x 0.3 = 3.6, 446.4 shown 447"), HeroLoss, 3);
			TestEqual(TEXT("the hero's first hit on the Man-at-arms: 30 - 2"), FoeLoss, 28);
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
