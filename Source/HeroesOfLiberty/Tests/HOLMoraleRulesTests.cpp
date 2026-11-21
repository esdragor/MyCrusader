// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLSimulationFragments.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLHeroAbilityRules.h"
#include "Simulation/HOLMoraleProcessor.h"
#include "Simulation/HOLMoraleRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLMoraleTableRows.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// PLAN-MORAL-1120 step 2: the pure morale rule in C++ (the single source, D836). Pure tests, no world: each posts effects (from the rows of
// HOLMoraleTableRows.h or made by hand) through Apply / Tick and reads the result from Total / Multiplier.
namespace UE::HeroesOfLiberty::Tests::MoraleRules
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	// Ids of hand-made effects, out of the table.
	constexpr int32 IdA = 100;
	constexpr int32 IdB = 101;
	constexpr int32 IdC = 102;
	constexpr int32 IdD = 103;
}

// The caps: 50 % taken (both signs), 40 % dealt bonus but 30 % dealt malus, 30 % rate; bonus and malus offset after their caps.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleCapsTest, "HeroesOfLiberty.Morale.CapsAndOffset", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleCapsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleRules;
	HOLMorale::FHOLMoraleState State;
	// Three taken bonuses of 25: 75 asked, 50 kept; the multiplier of the damage taken is 100 - 50.
	HOLMorale::Apply(State, HOLMorale::MakeEffect(IdA, 0, 25, 0, 1000, false));
	HOLMorale::Apply(State, HOLMorale::MakeEffect(IdB, 0, 25, 0, 1000, false));
	HOLMorale::Apply(State, HOLMorale::MakeEffect(IdC, 0, 25, 0, 1000, false));
	TestEqual(TEXT("taken bonus capped at 50"), HOLMorale::Total(State, HOLMorale::LeverTaken), 50);
	TestEqual(TEXT("damage taken multiplier"), HOLMorale::Multiplier(State, HOLMorale::LeverTaken), 50);
	TestEqual(TEXT("an untouched lever is 0"), HOLMorale::Total(State, HOLMorale::LeverDealt), 0);
	TestEqual(TEXT("an untouched lever multiplies by 100"), HOLMorale::Multiplier(State, HOLMorale::LeverRate), 100);

	HOLMorale::FHOLMoraleState Dealt;
	HOLMorale::Apply(Dealt, HOLMorale::MakeEffect(IdA, 30, 0, 20, 1000, false));
	HOLMorale::Apply(Dealt, HOLMorale::MakeEffect(IdB, 30, 0, 20, 1000, false));
	TestEqual(TEXT("dealt bonus capped at 40"), HOLMorale::Total(Dealt, HOLMorale::LeverDealt), 40);
	TestEqual(TEXT("rate bonus capped at 30"), HOLMorale::Total(Dealt, HOLMorale::LeverRate), 30);
	TestEqual(TEXT("dealt multiplier"), HOLMorale::Multiplier(Dealt, HOLMorale::LeverDealt), 140);
	// Two dealt malus of -20: 40 asked, 30 kept; with the 40 bonus above they offset to +10.
	HOLMorale::Apply(Dealt, HOLMorale::MakeEffect(IdC, -20, 0, 0, 1000, false));
	HOLMorale::Apply(Dealt, HOLMorale::MakeEffect(IdD, -20, 0, 0, 1000, false));
	TestEqual(TEXT("bonus 40 offset by malus capped at 30"), HOLMorale::Total(Dealt, HOLMorale::LeverDealt), 10);

	HOLMorale::FHOLMoraleState Malus;
	HOLMorale::Apply(Malus, HOLMorale::MakeEffect(IdA, 0, -30, 0, 1000, false));
	HOLMorale::Apply(Malus, HOLMorale::MakeEffect(IdB, 0, -30, 0, 1000, false));
	TestEqual(TEXT("taken malus capped at 50"), HOLMorale::Total(Malus, HOLMorale::LeverTaken), -50);
	TestEqual(TEXT("damage taken multiplier of a malus"), HOLMorale::Multiplier(Malus, HOLMorale::LeverTaken), 150);

	TestEqual(TEXT("BonusCap dealt"), HOLMorale::BonusCap(HOLMorale::LeverDealt), 40);
	TestEqual(TEXT("MalusCap dealt"), HOLMorale::MalusCap(HOLMorale::LeverDealt), 30);
	return true;
}

// The rows of the table go through the rule: each table effect, posed alone, gives its row's percents; a banner is the hero's aura.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleTableRowsTest, "HeroesOfLiberty.Morale.TableRowsThroughTheRule", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleTableRowsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleRules;
	for (int32 RowId = 0; RowId < HOLMoraleTableRows::HOLMoraleRowCount; RowId++)
	{
		const HOLMoraleTableRows::FHOLMoraleTableRow* Row = HOLMoraleTableRows::Find(RowId);
		if (!TestNotNull(TEXT("row exists"), Row))
		{
			continue;
		}
		if (Row->Gives >= 0)
		{
			continue; // a banner: tested below
		}
		HOLMorale::FHOLMoraleState State;
		const int32 Result = HOLMorale::Apply(State, HOLMorale::MakeEffectFromRow(RowId), 0);
		const bool bHasIntensity = Row->Dealt != 0 || Row->Taken != 0 || Row->Rate != 0;
		const bool bHasTime = Row->bAura || Row->DurationMs > 0;
		TestEqual(*FString::Printf(TEXT("%hs pose result"), Row->Key), Result, (bHasIntensity && bHasTime) ? HOLMorale::ApplyPosed : HOLMorale::ApplyNothing);
		TestEqual(*FString::Printf(TEXT("%hs dealt"), Row->Key), HOLMorale::Total(State, HOLMorale::LeverDealt), bHasTime ? Row->Dealt : 0);
		TestEqual(*FString::Printf(TEXT("%hs taken"), Row->Key), HOLMorale::Total(State, HOLMorale::LeverTaken), bHasTime ? Row->Taken : 0);
		TestEqual(*FString::Printf(TEXT("%hs rate"), Row->Key), HOLMorale::Total(State, HOLMorale::LeverRate), bHasTime ? Row->Rate : 0);
	}

	// The banner gives the Dawn Aura's levers under the Dawn Aura's id: the hero's aura and a banner are one effect, 15 % and not 30 %.
	const HOLMorale::FHOLMoraleEffect BannerEffect = HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleBanner);
	TestEqual(TEXT("banner effect id is the aura's"), BannerEffect.Id, static_cast<int32>(HOLMoraleTableRows::HOLMoraleDawnAura));
	TestTrue(TEXT("banner effect is an aura"), BannerEffect.bAura);
	HOLMorale::FHOLMoraleState State;
	TestEqual(TEXT("aura posed"), HOLMorale::Apply(State, HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleDawnAura)), HOLMorale::ApplyPosed);
	TestEqual(TEXT("banner refreshes the aura"), HOLMorale::Apply(State, BannerEffect), HOLMorale::ApplyRefreshed);
	TestEqual(TEXT("one effect, not two"), State.Num, 1);
	TestEqual(TEXT("aura and banner do not add up"), HOLMorale::Total(State, HOLMorale::LeverTaken), HOLMoraleTableRows::Find(HOLMoraleTableRows::HOLMoraleDawnAura)->Taken);
	return true;
}

// Posing: the same effect is never added to itself (strongest intensity, longest time), no intensity or no time poses nothing, a full unit
// takes no new effect.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleApplyTest, "HeroesOfLiberty.Morale.ApplyNotCumulated", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleApplyTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleRules;
	HOLMorale::FHOLMoraleState State;
	TestEqual(TEXT("first pose"), HOLMorale::Apply(State, HOLMorale::MakeEffect(IdA, 0, 10, 0, 4000, false)), HOLMorale::ApplyPosed);
	TestEqual(TEXT("same id again"), HOLMorale::Apply(State, HOLMorale::MakeEffect(IdA, 0, 25, 0, 2000, false)), HOLMorale::ApplyRefreshed);
	TestEqual(TEXT("still one effect"), State.Num, 1);
	TestEqual(TEXT("strongest intensity kept"), HOLMorale::Total(State, HOLMorale::LeverTaken), 25);
	TestEqual(TEXT("longest time kept"), State.Effects[0].DurationMs, 4000);
	TestTrue(TEXT("Has"), HOLMorale::Has(State, IdA));
	TestFalse(TEXT("Has not"), HOLMorale::Has(State, IdB));

	TestEqual(TEXT("no intensity"), HOLMorale::Apply(State, HOLMorale::MakeEffect(IdB, 0, 0, 0, 4000, false)), HOLMorale::ApplyNothing);
	TestEqual(TEXT("no time"), HOLMorale::Apply(State, HOLMorale::MakeEffect(IdB, 0, 10, 0, 0, false)), HOLMorale::ApplyNothing);
	TestEqual(TEXT("an aura needs no time"), HOLMorale::Apply(State, HOLMorale::MakeEffect(IdB, 0, 10, 0, 0, true)), HOLMorale::ApplyPosed);
	// A malus keeps its sign when refreshed: the larger magnitude wins.
	TestEqual(TEXT("malus pose"), HOLMorale::Apply(State, HOLMorale::MakeEffect(IdC, -5, 0, 0, 1000, false)), HOLMorale::ApplyPosed);
	TestEqual(TEXT("malus refresh"), HOLMorale::Apply(State, HOLMorale::MakeEffect(IdC, -15, 0, 0, 1000, false)), HOLMorale::ApplyRefreshed);
	TestEqual(TEXT("larger malus kept"), HOLMorale::Total(State, HOLMorale::LeverDealt), -15);

	TestEqual(TEXT("four slots used"), State.Num, HOLMorale::MaxEffects - 1);
	TestEqual(TEXT("fourth effect"), HOLMorale::Apply(State, HOLMorale::MakeEffect(IdD, 5, 0, 0, 1000, false)), HOLMorale::ApplyPosed);
	TestEqual(TEXT("fifth effect has no room"), HOLMorale::Apply(State, HOLMorale::MakeEffect(IdD + 1, 5, 0, 0, 1000, false)), HOLMorale::ApplyNoRoom);
	TestEqual(TEXT("a full unit still refreshes"), HOLMorale::Apply(State, HOLMorale::MakeEffect(IdD, 9, 0, 0, 1000, false)), HOLMorale::ApplyRefreshed);
	return true;
}

// Attenuation (a malus only: time for a temporary one, intensity for an aura), the 75 % cap and the combination of sources, the immunity.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleAttenuationTest, "HeroesOfLiberty.Morale.AttenuationAndImmunity", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleAttenuationTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleRules;
	const HOLMorale::FHOLMoraleEffect Demoralised = HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleDemoralised);
	const HOLMorale::FHOLMoraleEffect Shorter = HOLMorale::Attenuated(Demoralised, 50);
	TestEqual(TEXT("temporary malus loses half its time"), Shorter.DurationMs, Demoralised.DurationMs / 2);
	TestEqual(TEXT("temporary malus keeps its intensity"), Shorter.Taken, Demoralised.Taken);
	TestEqual(TEXT("attenuation never passes 75"), HOLMorale::Attenuated(Demoralised, 100).DurationMs, Demoralised.DurationMs / 4);

	const HOLMorale::FHOLMoraleEffect AuraMalus = HOLMorale::MakeEffect(IdA, 0, -10, 0, 0, true);
	TestEqual(TEXT("aura malus loses intensity (-5 at 50 %)"), HOLMorale::Attenuated(AuraMalus, 50).Taken, -5);
	TestEqual(TEXT("aura malus time unchanged"), HOLMorale::Attenuated(AuraMalus, 50).DurationMs, 0);
	TestEqual(TEXT("rounding is half away from zero (-5 x 75 % = -4)"), HOLMorale::Attenuated(HOLMorale::MakeEffect(IdA, 0, -5, 0, 0, true), 25).Taken, -4);
	const HOLMorale::FHOLMoraleEffect Bonus = HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleCloseRanks);
	TestEqual(TEXT("a bonus is never attenuated"), HOLMorale::Attenuated(Bonus, 75).DurationMs, Bonus.DurationMs);
	TestEqual(TEXT("no attenuation"), HOLMorale::Attenuated(Demoralised, 0).DurationMs, Demoralised.DurationMs);

	// Sources: the strongest of one nature counts alone, natures add, 75 at most.
	const int32 NaturesTwo[] = { 1, 1, 2 };
	const int32 PercentsTwo[] = { 30, 50, 20 };
	TestEqual(TEXT("strongest of a nature + another nature"), HOLMorale::CombineAttenuation(NaturesTwo, PercentsTwo, 3), 70);
	const int32 PercentsBig[] = { 30, 50, 40 };
	TestEqual(TEXT("capped at 75"), HOLMorale::CombineAttenuation(NaturesTwo, PercentsBig, 3), HOLMorale::AttenuationCap);
	const int32 PercentsNegative[] = { -20, 10, 0 };
	TestEqual(TEXT("a negative source counts for nothing"), HOLMorale::CombineAttenuation(NaturesTwo, PercentsNegative, 3), 10);
	TestEqual(TEXT("no source"), HOLMorale::CombineAttenuation(nullptr, nullptr, 0), 0);

	// Apply with an attenuation shortens the posed malus.
	HOLMorale::FHOLMoraleState State;
	TestEqual(TEXT("attenuated malus is posed"), HOLMorale::Apply(State, Demoralised, 50), HOLMorale::ApplyPosed);
	TestEqual(TEXT("posed with half the time"), State.Effects[0].DurationMs, Demoralised.DurationMs / 2);

	// Immunity: a malus is refused, a bonus is not; the immunity runs down.
	HOLMorale::FHOLMoraleState Immune;
	HOLMorale::GrantImmunity(Immune, HOLMorale::SacredLightImmunityMs);
	HOLMorale::GrantImmunity(Immune, 1000);
	TestEqual(TEXT("a shorter grant never cuts a longer one"), Immune.ImmunityMs, HOLMorale::SacredLightImmunityMs);
	TestEqual(TEXT("malus blocked by the immunity"), HOLMorale::Apply(Immune, Demoralised), HOLMorale::ApplyBlockedByImmunity);
	TestEqual(TEXT("nothing posed"), Immune.Num, 0);
	TestEqual(TEXT("bonus passes the immunity"), HOLMorale::Apply(Immune, Bonus), HOLMorale::ApplyPosed);
	HOLMorale::Tick(Immune, HOLMorale::SacredLightImmunityMs);
	TestEqual(TEXT("immunity over"), Immune.ImmunityMs, 0);
	TestEqual(TEXT("malus posed after the immunity"), HOLMorale::Apply(Immune, Demoralised), HOLMorale::ApplyPosed);
	return true;
}

// Time and removal: temporary effects end at 0, auras stay, ClearAuras takes the auras, RemoveMalus the malus and keeps the bonus.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleTickTest, "HeroesOfLiberty.Morale.TickClearRemove", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleTickTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleRules;
	HOLMorale::FHOLMoraleState State;
	HOLMorale::Apply(State, HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleCloseRanks));
	HOLMorale::Apply(State, HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleDawnAura));
	const int32 CloseRanksMs = HOLMoraleTableRows::Find(HOLMoraleTableRows::HOLMoraleCloseRanks)->DurationMs;
	HOLMorale::Tick(State, CloseRanksMs - 1000);
	TestTrue(TEXT("the cry is still on"), HOLMorale::Has(State, HOLMoraleTableRows::HOLMoraleCloseRanks));
	TestEqual(TEXT("cry and aura add up (25 + 15)"), HOLMorale::Total(State, HOLMorale::LeverTaken), 40);
	HOLMorale::Tick(State, 1000);
	TestFalse(TEXT("the cry is over at 0"), HOLMorale::Has(State, HOLMoraleTableRows::HOLMoraleCloseRanks));
	TestTrue(TEXT("the aura does not run down"), HOLMorale::Has(State, HOLMoraleTableRows::HOLMoraleDawnAura));
	TestEqual(TEXT("only the aura left"), HOLMorale::Total(State, HOLMorale::LeverTaken), 15);

	HOLMorale::ClearAuras(State);
	TestEqual(TEXT("no effect after ClearAuras"), State.Num, 0);

	// Removal keeps the bonus, takes every malus, and keeps the order of the rest.
	HOLMorale::Apply(State, HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleDemoralised));
	HOLMorale::Apply(State, HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleTriumph));
	HOLMorale::Apply(State, HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleHesitation));
	TestEqual(TEXT("three effects"), State.Num, 3);
	TestEqual(TEXT("two malus removed"), HOLMorale::RemoveMalus(State), 2);
	TestEqual(TEXT("the bonus stays"), State.Num, 1);
	TestTrue(TEXT("it is the Triumph"), HOLMorale::Has(State, HOLMoraleTableRows::HOLMoraleTriumph));
	TestEqual(TEXT("its percents are intact"), HOLMorale::Total(State, HOLMorale::LeverDealt), HOLMoraleTableRows::Find(HOLMoraleTableRows::HOLMoraleTriumph)->Dealt);
	TestEqual(TEXT("nothing left to remove"), HOLMorale::RemoveMalus(State), 0);
	return true;
}

// PLAN-MORAL-1120 step 3: the morale fragment of a unit through the rule: pose, refresh (no doubling), expiry, removal of the malus; the fingerprint
// key is 0 for an empty unit (left out of the hash) and follows what the unit carries.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleFragmentTest, "HeroesOfLiberty.Morale.FragmentPosesRefreshesExpiresRemoves", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleFragmentTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleRules;
	FHOLMoraleFragment Fragment;
	TestTrue(TEXT("a new fragment is empty"), HOLMorale::IsEmpty(Fragment.State));
	TestEqual(TEXT("an empty unit mixes nothing into the fingerprint"), HOLMorale::FingerprintKey(Fragment.State), static_cast<uint64>(0));

	// A temporary taken bonus of 25 for one second: Total(Taken) rises, a second pose of the same effect refreshes it and does not double it.
	TestEqual(TEXT("first pose"), HOLMorale::Apply(Fragment.State, HOLMorale::MakeEffect(IdA, 0, 25, 0, 1000, false)), HOLMorale::ApplyPosed);
	TestEqual(TEXT("Total(Taken) rises"), HOLMorale::Total(Fragment.State, HOLMorale::LeverTaken), 25);
	const uint64 KeyWithBonus = HOLMorale::FingerprintKey(Fragment.State);
	TestTrue(TEXT("a unit under an effect has a key"), KeyWithBonus != 0);
	TestEqual(TEXT("second pose refreshes"), HOLMorale::Apply(Fragment.State, HOLMorale::MakeEffect(IdA, 0, 25, 0, 1000, false)), HOLMorale::ApplyRefreshed);
	TestEqual(TEXT("the refresh does not double"), HOLMorale::Total(Fragment.State, HOLMorale::LeverTaken), 25);
	TestEqual(TEXT("the key does not change with a refresh"), HOLMorale::FingerprintKey(Fragment.State), KeyWithBonus);

	// A malus beside the bonus: the removal takes the malus and leaves the bonus; the key follows.
	HOLMorale::Apply(Fragment.State, HOLMorale::MakeEffect(IdB, 0, -20, 0, 500, false));
	TestEqual(TEXT("bonus and malus offset"), HOLMorale::Total(Fragment.State, HOLMorale::LeverTaken), 5);
	const uint64 KeyWithBoth = HOLMorale::FingerprintKey(Fragment.State);
	TestTrue(TEXT("a different state has a different key"), KeyWithBoth != KeyWithBonus);
	TestEqual(TEXT("RemoveMalus counts one"), HOLMorale::RemoveMalus(Fragment.State), 1);
	TestEqual(TEXT("the bonus stays"), HOLMorale::Total(Fragment.State, HOLMorale::LeverTaken), 25);
	TestEqual(TEXT("the key is back to the bonus alone"), HOLMorale::FingerprintKey(Fragment.State), KeyWithBonus);

	// After the time, the total falls to 0 and the unit leaves the fingerprint again.
	HOLMorale::Tick(Fragment.State, 1000);
	TestEqual(TEXT("expired: Total(Taken) is 0"), HOLMorale::Total(Fragment.State, HOLMorale::LeverTaken), 0);
	TestTrue(TEXT("expired: the fragment is empty"), HOLMorale::IsEmpty(Fragment.State));
	TestEqual(TEXT("expired: the key is 0"), HOLMorale::FingerprintKey(Fragment.State), static_cast<uint64>(0));
	return true;
}
// PLAN-MORAL-1120 step 4 (D866): the fragment is on the unit ARCHETYPE (UHOLUnitTrait::BuildTemplate), not only a struct the rule can use. A unit born
// through the real chain (RequestUnits -> ServeBirthRequests -> the type's template) carries FHOLMoraleFragment, empty at birth, and the morale rule
// writes into the entity's own fragment: a pose is read back from the entity. RED until the trait adds the fragment.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleUnitArchetypeTest, "HeroesOfLiberty.Morale.UnitEntityCarriesTheFragment", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleUnitArchetypeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleRules;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
		// A fighter (ManAtArms) and a Peasant: every unit type shares UHOLUnitTrait, so the fragment is on all of them (the rule leaves the Peasants
		// out by the recipients, D705, not by the archetype).
		for (const TCHAR* TypeName : { TEXT("ManAtArms"), TEXT("Peasant") })
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(TypeName);
			Request.Count = 1;
			Request.OwnerPlayer = 0;
			Request.Center = FVector(0.0, 0.0, 0.0);
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			uint32 Born = 0;
			for (const uint32 Id : After)
			{
				Born = Before.Contains(Id) ? Born : Id;
			}
			if (!TestTrue(*FString::Printf(TEXT("%s was born"), TypeName), Born != 0))
			{
				continue;
			}
			const FMassEntityHandle Entity = Units->FindEntity(Born);
			FHOLMoraleFragment* Morale = Entity.IsSet() ? EntityManager.GetFragmentDataPtr<FHOLMoraleFragment>(Entity) : nullptr;
			if (!TestNotNull(*FString::Printf(TEXT("%s carries FHOLMoraleFragment"), TypeName), Morale))
			{
				continue;
			}
			TestTrue(*FString::Printf(TEXT("%s: empty at birth"), TypeName), HOLMorale::IsEmpty(Morale->State));
			// The effect: a pose through the rule is read back from the entity's own fragment, and the fingerprint key follows.
			TestEqual(TEXT("pose on the entity"), HOLMorale::Apply(Morale->State, HOLMorale::MakeEffect(IdA, 0, 25, 0, 1000, false)), HOLMorale::ApplyPosed);
			const FHOLMoraleFragment* Again = EntityManager.GetFragmentDataPtr<FHOLMoraleFragment>(Entity);
			TestEqual(TEXT("the entity's total rises"), Again ? HOLMorale::Total(Again->State, HOLMorale::LeverTaken) : -1, 25);
			TestTrue(TEXT("the entity now has a fingerprint key"), Again && HOLMorale::FingerprintKey(Again->State) != 0);
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// PLAN-MORAL-1120 step 4 (D866), the processor's first rule: each fixed step UHOLMoraleProcessor lets the time pass on the fragment of every unit
// (HOLMorale::Tick). A unit born through the real chain carries a temporary effect and an aura; the processor runs on the entity manager as a
// match runs it; the entity's own fragment is read back. The expected values are CALCULATED by the rule on a copy (never typed). RED until the
// processor's Execute applies the rule (its stub does nothing).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleProcessorTest, "HeroesOfLiberty.Morale.ProcessorTicksTheFragment", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleProcessorTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleRules;
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
		UHOLMoraleProcessor* Morale = NewObject<UHOLMoraleProcessor>(Simulation);
		Morale->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const int32 StepMs = HOLCombat::ToMilliseconds(StepSeconds);

		TArray<uint32> Before;
		Units->GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = FName(TEXT("ManAtArms"));
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = FVector(0.0, 0.0, 0.0);
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> After;
		Units->GetAllUnitIds(After);
		uint32 Born = 0;
		for (const uint32 Id : After)
		{
			Born = Before.Contains(Id) ? Born : Id;
		}
		const FMassEntityHandle Entity = TestTrue(TEXT("a unit was born"), Born != 0) ? Units->FindEntity(Born) : FMassEntityHandle();
		FHOLMoraleFragment* Fragment = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLMoraleFragment>(Entity) : nullptr;
		if (TestNotNull(TEXT("the unit carries FHOLMoraleFragment"), Fragment))
		{
			// A temporary effect (1000 ms, 25 % less damage taken) and an aura (10 % more damage dealt, does not run down).
			HOLMorale::Apply(Fragment->State, HOLMorale::MakeEffect(IdA, 0, 25, 0, 1000, false));
			HOLMorale::Apply(Fragment->State, HOLMorale::MakeEffect(IdB, 10, 0, 0, 0, true));
			// What the rule leaves after ONE step, calculated on a copy.
			HOLMorale::FHOLMoraleState Expected = Fragment->State;
			HOLMorale::ClearAuras(Expected);
			HOLMorale::Tick(Expected, StepMs);

			const auto RunStep = [&EntityManager, &Morale, StepSeconds]
			{
				FMassProcessingContext ProcessingContext(*EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Morale, ProcessingContext);
			};
			RunStep();
			const FHOLMoraleFragment* Now = EntityManager->GetFragmentDataPtr<FHOLMoraleFragment>(Entity);
			TestTrue(TEXT("one step: the temporary effect lost one step of time"), Now && Now->State.Effects[0].DurationMs == Expected.Effects[0].DurationMs);
			TestTrue(TEXT("one step: the time left is less than it was"), Now && Now->State.Effects[0].DurationMs < 1000);

			// Enough steps for the temporary effect to end: it leaves, and the aura nobody lays again is already gone (the unit is empty).
			for (int32 Steps = 1; Steps <= 1000 / FMath::Max(StepMs, 1) + 1; ++Steps)
			{
				RunStep();
			}
			Now = EntityManager->GetFragmentDataPtr<FHOLMoraleFragment>(Entity);
			TestFalse(TEXT("the temporary effect has ended"), Now && HOLMorale::Has(Now->State, IdA));
			TestFalse(TEXT("the aura is gone"), Now && HOLMorale::Has(Now->State, IdB));
			TestTrue(TEXT("the unit is empty again"), Now && HOLMorale::IsEmpty(Now->State));
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}
// Slice 2 of the processor (plan step 4): every step the unit's auras are taken off (HOLMorale::ClearAuras) before the engine lays
// the ones in range again, so an aura nobody lays again ends; the temporary effects are not touched by it. Red while Execute only ticks.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleProcessorClearsAurasTest, "HeroesOfLiberty.Morale.ProcessorClearsAurasEachStep", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleProcessorClearsAurasTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleRules;
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
		UHOLMoraleProcessor* Morale = NewObject<UHOLMoraleProcessor>(Simulation);
		Morale->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const int32 StepMs = HOLCombat::ToMilliseconds(StepSeconds);

		TArray<uint32> Before;
		Units->GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = FName(TEXT("ManAtArms"));
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = FVector(0.0, 0.0, 0.0);
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> After;
		Units->GetAllUnitIds(After);
		uint32 Born = 0;
		for (const uint32 Id : After)
		{
			Born = Before.Contains(Id) ? Born : Id;
		}
		const FMassEntityHandle Entity = TestTrue(TEXT("a unit was born"), Born != 0) ? Units->FindEntity(Born) : FMassEntityHandle();
		FHOLMoraleFragment* Fragment = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLMoraleFragment>(Entity) : nullptr;
		if (TestNotNull(TEXT("the unit carries FHOLMoraleFragment"), Fragment))
		{
			HOLMorale::Apply(Fragment->State, HOLMorale::MakeEffect(IdA, 0, 25, 0, 1000, false));
			HOLMorale::Apply(Fragment->State, HOLMorale::MakeEffect(IdB, 10, 0, 0, 0, true));
			// What the rule leaves after ONE step (auras off, then the time passes), calculated on a copy.
			HOLMorale::FHOLMoraleState Expected = Fragment->State;
			HOLMorale::ClearAuras(Expected);
			HOLMorale::Tick(Expected, StepMs);

			FMassProcessingContext ProcessingContext(*EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Morale, ProcessingContext);
			const FHOLMoraleFragment* Now = EntityManager->GetFragmentDataPtr<FHOLMoraleFragment>(Entity);
			TestFalse(TEXT("one step: the aura nobody laid again has ended"), Now && HOLMorale::Has(Now->State, IdB));
			TestEqual(TEXT("one step: the dealt lever no longer reads the aura"), Now ? HOLMorale::Total(Now->State, HOLMorale::LeverDealt) : -1, HOLMorale::Total(Expected, HOLMorale::LeverDealt));
			TestTrue(TEXT("one step: the temporary effect stays, one step older"), Now && HOLMorale::Has(Now->State, IdA) && Now->State.Num == Expected.Num);
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}


// Slice 3 of the processor (plan step 4, D705, row DawnAura of Effects.csv, SourceKind Hero): every step, after the auras were taken off
// (ClearAuras), a living hero lays the Dawn aura on the units of HIS OWN camp within the row's radius (a bonus goes to the owner alone, never to
// an enemy, never to a Peasant). Born through the real chain; the processor runs as a match runs it; the expected effect is CALCULATED by the rule
// (MakeEffectFromRow + Apply on an empty state, never typed). RED while Execute only clears and ticks.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleProcessorLaysDawnAuraTest, "HeroesOfLiberty.Morale.ProcessorLaysDawnAuraInRange", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleProcessorLaysDawnAuraTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MoraleRules;
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
		UHOLMoraleProcessor* Morale = NewObject<UHOLMoraleProcessor>(Simulation);
		Morale->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();

		// One unit born through the real chain at (X, 0): its entity.
		const auto Spawn = [this, Units](const TCHAR* TypeName, const uint8 Owner, const double X) -> FMassEntityHandle
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(TypeName);
			Request.Count = 1;
			Request.OwnerPlayer = Owner;
			Request.Center = FVector(X, 0.0, 0.0);
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			uint32 Born = 0;
			for (const uint32 Id : After)
			{
				Born = Before.Contains(Id) ? Born : Id;
			}
			return TestTrue(TEXT("a unit was born"), Born != 0) ? Units->FindEntity(Born) : FMassEntityHandle();
		};
		const FMassEntityHandle Hero = Spawn(TEXT("Hero"), 0, 0.0);
		const FMassEntityHandle AllyNear = Spawn(TEXT("ManAtArms"), 0, 500.0);
		const FMassEntityHandle AllyFar = Spawn(TEXT("ManAtArms"), 0, 9000.0);
		const FMassEntityHandle EnemyNear = Spawn(TEXT("ManAtArms"), 1, 500.0);
		const FMassEntityHandle PeasantNear = Spawn(TEXT("Peasant"), 0, 500.0);

		// What the rule leaves on a unit that stands in the aura after ONE step: the row's effect posed on an empty state.
		HOLMorale::FHOLMoraleState Expected;
		HOLMorale::Apply(Expected, HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleDawnAura));
		TestTrue(TEXT("the Dawn aura row gives a real lever"), HOLMorale::Total(Expected, HOLMorale::LeverTaken) > 0);

		const auto Read = [&EntityManager](const FMassEntityHandle Entity) -> const FHOLMoraleFragment*
		{
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLMoraleFragment>(Entity) : nullptr;
		};
		if (TestTrue(TEXT("every unit carries the fragment"), Read(Hero) && Read(AllyNear) && Read(AllyFar) && Read(EnemyNear) && Read(PeasantNear)))
		{
			// Two steps: the aura is taken off then laid again each step, so it is there and never doubled.
			for (int32 Step = 0; Step < 2; ++Step)
			{
				FMassProcessingContext ProcessingContext(*EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Morale, ProcessingContext);
			}
			const FHOLMoraleFragment* Near = Read(AllyNear);
			TestTrue(TEXT("the ally in range carries the Dawn aura"), HOLMorale::Has(Near->State, HOLMoraleTableRows::HOLMoraleDawnAura));
			TestEqual(TEXT("it reads the row's damage taken lever"), HOLMorale::Total(Near->State, HOLMorale::LeverTaken), HOLMorale::Total(Expected, HOLMorale::LeverTaken));
			TestEqual(TEXT("the aura is laid once, never doubled"), Near->State.Num, Expected.Num);
			TestTrue(TEXT("the ally out of range has nothing"), HOLMorale::IsEmpty(Read(AllyFar)->State));
			TestTrue(TEXT("the enemy in range has nothing (a bonus goes to the owner alone, D705)"), HOLMorale::IsEmpty(Read(EnemyNear)->State));
			TestTrue(TEXT("the Peasant in range has nothing (never a Peasant, D705)"), HOLMorale::IsEmpty(Read(PeasantNear)->State));
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// The combat reads the damage-taken lever of the morale fragment (PLAN-MORAL-1120 step 4, last slice): one strike on a unit under the Dawn aura does
// the row's percent less than the same strike on a bare unit, the cry adds to it under the cap, and the old constants of the combat agree with the row.
// Every expected value is CALCULATED (the row of the table, the rule's own GetReducedDamage), never typed. RED while TakenPercent is a stub (100).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleTakenLeverTest, "HeroesOfLiberty.Morale.CombatReadsTheTakenLever", UE::HeroesOfLiberty::Tests::MoraleRules::Flags)

bool FHOLMoraleTakenLeverTest::RunTest(const FString& Parameters)
{
	const HOLMoraleTableRows::FHOLMoraleTableRow* Row = HOLMoraleTableRows::Find(HOLMoraleTableRows::HOLMoraleDawnAura);
	if (!TestNotNull(TEXT("the Dawn aura row exists"), Row))
	{
		return true;
	}
	HOLMorale::FHOLMoraleState Bare;
	HOLMorale::FHOLMoraleState UnderAura;
	HOLMorale::Apply(UnderAura, HOLMorale::MakeEffectFromRow(HOLMoraleTableRows::HOLMoraleDawnAura));

	const float Strike = 200.f;
	const float OnBare = HOLHeroAbility::GetReducedDamage(Strike, HOLMorale::TakenPercent(Bare, 0));
	const float OnAura = HOLHeroAbility::GetReducedDamage(Strike, HOLMorale::TakenPercent(UnderAura, 0));
	TestEqual(TEXT("a bare unit takes the whole strike"), OnBare, Strike);
	TestEqual(TEXT("the same strike does the row's percent less under the aura"), OnAura, OnBare * static_cast<float>(100 - Row->Taken) / 100.f);

	// The cry (posed apart) adds to the aura, under the cap: the lever replaces the aura term of the old constants, so the two agree on every case.
	for (int32 Cry = 0; Cry < 2; Cry++)
	{
		const int32 Other = Cry ? HOLHeroAbility::CloseRanksReductionPercent : 0;
		TestEqual(*FString::Printf(TEXT("aura %d, cry %d: the lever reads as the old rule"), 1, Cry), HOLMorale::TakenPercent(UnderAura, Other), HOLHeroAbility::GetDamageTakenPercent(true, Cry != 0));
		TestEqual(*FString::Printf(TEXT("aura %d, cry %d: the lever reads as the old rule"), 0, Cry), HOLMorale::TakenPercent(Bare, Other), HOLHeroAbility::GetDamageTakenPercent(false, Cry != 0));
	}
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
