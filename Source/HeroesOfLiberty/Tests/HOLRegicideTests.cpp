// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Match/HOLStartRules.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLCommandGrid.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "Engine/DataTable.h"

#if WITH_DEV_AUTOMATION_TESTS

// V5.1 checks (Regicide, D206 revised on 2026-10-25): the King's type, its riposte without aggression, its birth.
namespace UE::HeroesOfLiberty::Tests::Regicide
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FHOLUnitTypeRow* FindRow(const TCHAR* Name)
	{
		const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		return UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(Name, TEXT("Regicide test"), false) : nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRegicideKingTypeTest, "HeroesOfLiberty.Victory.Regicide.KingType", UE::HeroesOfLiberty::Tests::Regicide::Flags)

bool FHOLRegicideKingTypeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Regicide;
	const FHOLUnitTypeRow* King = FindRow(TEXT("King"));
	const FHOLUnitTypeRow* Spearman = FindRow(TEXT("Spearman"));
	const FHOLUnitTypeRow* ManAtArms = FindRow(TEXT("ManAtArms"));
	const FHOLUnitTypeRow* Peasant = FindRow(TEXT("Peasant"));
	if (!TestNotNull(TEXT("The table has a King"), King) || !TestNotNull(TEXT("... a Spearman"), Spearman)
		|| !TestNotNull(TEXT("... a Man-at-arms"), ManAtArms) || !TestNotNull(TEXT("... a Peasant"), Peasant))
	{
		return false;
	}
	TestEqual(TEXT("250 HP"), King->MaxHealth, 250);
	TestEqual(TEXT("A Spearman's speed (777)"), King->Speed, Spearman->Speed);
	TestEqual(TEXT("50 of attack"), King->Attack, 50);
	TestTrue(TEXT("A melee attack"), King->AttackType == EHOLAttackType::Melee);
	TestEqual(TEXT("A Man-at-arms' interval"), King->AttackInterval, ManAtArms->AttackInterval);
	TestEqual(TEXT("A Man-at-arms' reach"), King->AttackRange, ManAtArms->AttackRange);
	TestTrue(TEXT("No armour"), King->MeleeArmor == 0 && King->RangedArmor == 0 && King->SiegeArmor == 0 && King->HeroArmor == 0);
	TestEqual(TEXT("Outside the population (GetPopulationByOwner counts the row's Population)"), King->Population, 0);
	TestEqual(TEXT("No building attack (D345)"), King->BuildingAttack, 0);
	TestEqual(TEXT("No aggression radius"), King->AggressionRadius, 0.f);
	TestTrue(TEXT("It ripostes"), King->Ripostes);
	TestFalse(TEXT("The Peasant does not (unchanged)"), Peasant->Ripostes);
	TestTrue(TEXT("No gathering, nothing carried"), King->GatherRates.Food <= 0.f && King->GatherRates.Wood <= 0.f
		&& King->GatherRates.Stone <= 0.f && King->GatherRates.Gold <= 0.f && King->CarryCapacity == 0);
	TestEqual(TEXT("No building gesture"), King->BuildSequence, -1);
	const FHOLUnitTypeConstants KingConstants = King->ToConstants(TEXT("King"));
	TestTrue(TEXT("Its constants carry the riposte"), KingConstants.bRipostes);
	TestFalse(TEXT("Never healed"), HOLCanBeHealed(KingConstants.ArmorCategoryMask));
	TestTrue(TEXT("A Spearman can be"), HOLCanBeHealed(Spearman->ToConstants(TEXT("Spearman")).ArmorCategoryMask));
	TestEqual(TEXT("Its building search radius is 0, even on an attack-move"), HOLCombat::GetBuildingSearchRadius(
		HOLCombat::FSeeker{ FVector2D::ZeroVector, 60.f, 70.f, 0.f, HOLCombat::ESeekMode::AttackMove }, KingConstants), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRegicideRiposteTest, "HeroesOfLiberty.Victory.Regicide.Riposte", UE::HeroesOfLiberty::Tests::Regicide::Flags)

bool FHOLRegicideRiposteTest::RunTest(const FString& Parameters)
{
	HOLCombat::FSeeker King;
	King.Radius = 60.f;
	King.AttackRange = 70.f;
	King.AggressionRadius = 0.f;
	King.bRipostes = true;
	TestEqual(TEXT("No search radius of its own"), HOLCombat::GetSearchRadius(King), 0.f);

	HOLCombat::FCandidate Near;
	Near.Id = 1;
	Near.Position = FVector2D(150., 0.); // in reach (60 + 60 + 70)
	HOLCombat::FCandidate FarAttacker;
	FarAttacker.Id = 2;
	FarAttacker.Position = FVector2D(3000., 0.);
	FarAttacker.bAttacksMe = true;
	HOLCombat::FCandidate Building;
	Building.Id = 3;
	Building.Position = FVector2D(100., 0.);
	Building.bIsBuilding = true;
	Building.bAttacksMe = true;

	TestEqual(TEXT("An enemy in reach not attacking it is left alone (no aggression)"), HOLCombat::ChooseTarget(King, { Near }), INDEX_NONE);
	TestEqual(TEXT("The enemy attacking it is taken on, even far away (riposte)"), HOLCombat::ChooseTarget(King, { Near, FarAttacker }), 1);
	TestEqual(TEXT("Never a building"), HOLCombat::ChooseTarget(King, { Building }), INDEX_NONE);

	HOLCombat::FSeeker Peasant = King;
	Peasant.bRipostes = false;
	TestEqual(TEXT("The Peasant (no riposte) takes on nobody by itself, as before"), HOLCombat::ChooseTarget(Peasant, { Near, FarAttacker }), INDEX_NONE);

	HOLCombat::FSeeker Soldier = King;
	Soldier.bRipostes = false;
	Soldier.AggressionRadius = 1600.f;
	TestEqual(TEXT("A soldier with an aggression radius takes the one in reach first, as before"), HOLCombat::ChooseTarget(Soldier, { Near, FarAttacker }), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRegicideBirthTest, "HeroesOfLiberty.Victory.Regicide.Birth", UE::HeroesOfLiberty::Tests::Regicide::Flags)

bool FHOLRegicideBirthTest::RunTest(const FString& Parameters)
{
	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	for (FHOLMatchSlot& Slot : Config.Slots)
	{
		Slot.State = EHOLSlotState::Closed;
	}
	Config.Slots[0].State = EHOLSlotState::Human;
	Config.Slots[1].State = EHOLSlotState::AI;

	Config.bRegicide = false;
	TestFalse(TEXT("Without the option, no King"), FHOLStartRules::ShouldSpawnKing(Config, 0) || FHOLStartRules::ShouldSpawnKing(Config, 1));
	Config.bRegicide = true;
	TestTrue(TEXT("With it, the human slot gets one"), FHOLStartRules::ShouldSpawnKing(Config, 0));
	TestTrue(TEXT("... and the AI slot"), FHOLStartRules::ShouldSpawnKing(Config, 1));
	TestFalse(TEXT("... a closed slot none"), FHOLStartRules::ShouldSpawnKing(Config, 2));
	TestFalse(TEXT("... nor a slot beyond the configuration"), FHOLStartRules::ShouldSpawnKing(Config, 99));
	TestEqual(TEXT("The King's type is King"), FHOLStartRules::KingTypeName, FName(TEXT("King")));

	const FVector Location = FHOLStartRules::GetKingLocation(FVector(1000., 2000., 50.), 600.f);
	TestEqual(TEXT("By its forum, below it on the screen"), Location, FVector(1000. - 600. - FHOLStartRules::KingGap, 2000., 50.));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRegicideDeathTest, "HeroesOfLiberty.Victory.Regicide.Death", UE::HeroesOfLiberty::Tests::Regicide::Flags)

bool FHOLRegicideDeathTest::RunTest(const FString& Parameters)
{
	// V5.2 (D355): the King dead, its slot defeated (King) at the end of the step, after a KingDied event to all.
	TestFalse(TEXT("A Spearman is no King"), HOLIsKing(HOLArmorCategoryBit(EHOLArmorCategory::LightInfantry)));
	TestTrue(TEXT("The King is"), HOLIsKing(HOLArmorCategoryBit(EHOLArmorCategory::King) | HOLArmorCategoryBit(EHOLArmorCategory::LightInfantry)));

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("simulation"), Simulation))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index < 4 ? (Index == 0 ? EHOLSlotState::Human : EHOLSlotState::AI) : EHOLSlotState::Closed;
			Config.Slots[Index].Team = Index < 2 ? 1 : 2;
		}
		Config.bRegicide = true;
		State->InitVictory(Config);
		TArray<FHOLForumEvent> Events;
		State->OnForumEvent.AddLambda([&Events](const FHOLForumEvent& Event) { Events.Add(Event); });
		int64 Step = 0;
		const auto RunStep = [Simulation, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		RunStep();

		AddExpectedMessage(TEXT("Regicide: king of slot 2 died"), EAutomationExpectedMessageFlags::Contains, 1);
		AddExpectedMessage(TEXT("Victory: slot 2 defeated .King."), EAutomationExpectedMessageFlags::Contains, 1);
		TestTrue(TEXT("the King of slot 2 dies"), State->HandleKingDeath(2));
		TestFalse(TEXT("not yet defeated: at the end of the step"), State->IsSlotDefeated(2));
		TestEqual(TEXT("KingDied sent at once"), Events.Num(), 1);
		TestTrue(TEXT("... its kind"), Events.Num() > 0 && Events[0].Kind == EHOLForumEventKind::KingDied && Events[0].Slot == 2);
		TestFalse(TEXT("its death once"), State->HandleKingDeath(2));
		RunStep();
		TestTrue(TEXT("defeated by the King"), State->GetDefeatReason(2) == EHOLDefeatReason::King);
		TestTrue(TEXT("then the defeat's message"), Events.Num() == 2 && Events[1].Kind == EHOLForumEventKind::Defeated);
		TestFalse(TEXT("not over: its ally stands"), State->IsMatchOver());

		// The collapse of a defeated slot kills its King: nothing again.
		TestFalse(TEXT("a defeated slot's King dying: nothing"), State->HandleKingDeath(2));
		RunStep();
		TestEqual(TEXT("no message more"), Events.Num(), 2);

		// A second King, of another slot.
		AddExpectedMessage(TEXT("Regicide: king of slot 3 died"), EAutomationExpectedMessageFlags::Contains, 1);
		AddExpectedMessage(TEXT("Victory: slot 3 defeated .King."), EAutomationExpectedMessageFlags::Contains, 1);
		TestTrue(TEXT("the King of slot 3 dies"), State->HandleKingDeath(3));
		RunStep();
		TestTrue(TEXT("slot 3 defeated by the King"), State->GetDefeatReason(3) == EHOLDefeatReason::King);
		TestTrue(TEXT("its camp out: over"), State->IsMatchOver());
		TestFalse(TEXT("over: no King's death counts"), State->HandleKingDeath(0));
		TestFalse(TEXT("slot 0 never defeated"), State->IsSlotDefeated(0));
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRegicideMessageTest, "HeroesOfLiberty.Victory.Regicide.Message", UE::HeroesOfLiberty::Tests::Regicide::Flags)

bool FHOLRegicideMessageTest::RunTest(const FString& Parameters)
{
	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
	{
		Config.Slots[Index].State = Index < 3 ? EHOLSlotState::Human : EHOLSlotState::Closed;
		Config.Slots[Index].Team = Index < 2 ? 1 : 2;
	}
	const FHOLForumEvent Event = FHOLForumEvent::Make(EHOLForumEventKind::KingDied, 0);
	const FHOLForumMessage Own = HOLForumMessages::GetMessage(Event, Config, 0);
	TestEqual(TEXT("its own"), Own.Text.ToString(), FString(TEXT("Votre Roi est mort")));
	TestFalse(TEXT("never an alert of its own loss"), Own.bAlert);
	TestTrue(TEXT("no name"), Own.NamedSlots.IsEmpty());
	const FHOLForumMessage Ally = HOLForumMessages::GetMessage(Event, Config, 1);
	TestEqual(TEXT("an ally"), Ally.Text.ToString(), FString(TEXT("Le Roi de Joueur 1 est mort")));
	TestTrue(TEXT("an alert for its camp"), Ally.bAlert);
	TestEqual(TEXT("its name coloured"), Ally.NamedSlots, TArray<int32>{ 0 });
	const FHOLForumMessage Adversary = HOLForumMessages::GetMessage(Event, Config, 2);
	TestEqual(TEXT("an adversary"), Adversary.Text.ToString(), FString(TEXT("Le Roi de Joueur 1 est mort")));
	TestFalse(TEXT("no alert"), Adversary.bAlert);
	TestFalse(TEXT("a spectator: no alert"), HOLForumMessages::GetMessage(Event, Config, INDEX_NONE).bAlert);
	TestEqual(TEXT("its kind's name"), FString(HOLForumMessages::GetKindName(EHOLForumEventKind::KingDied)), FString(TEXT("KingDied")));
	TestEqual(TEXT("the reason's name"), FString(AHOLGameState::GetReasonName(EHOLDefeatReason::King)), FString(TEXT("King")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRegicideCommandsTest, "HeroesOfLiberty.Victory.Regicide.Commands", UE::HeroesOfLiberty::Tests::Regicide::Flags)

// V5.3: the King moves, stops and attacks on order; no attack-move nor hold of its own, and given one with others, it seeks nobody.
bool FHOLRegicideCommandsTest::RunTest(const FString& Parameters)
{
	FHOLGridContext Kings;
	Kings.bOwnUnits = true;
	Kings.bSoldiers = true;
	Kings.bOnlyKings = true;
	TestTrue(TEXT("Kings alone: no attack-move"), HOLCommandGrid::GetOrder(Kings, HOLCommandGrid::AttackMoveCell).Command == EHOLGridCommand::None);
	TestTrue(TEXT("... stop"), HOLCommandGrid::GetOrder(Kings, HOLCommandGrid::StopCell).Command == EHOLGridCommand::Stop);
	TestTrue(TEXT("... no hold (it would take on enemies in reach)"), HOLCommandGrid::GetOrder(Kings, HOLCommandGrid::HoldCell).Command == EHOLGridCommand::None);
	TestFalse(TEXT("... no builders' grid"), HOLCommandGrid::IsBuildGrid(Kings));
	FHOLGridContext Mixed = Kings;
	Mixed.bOnlyKings = false;
	TestTrue(TEXT("A King with soldiers: the attack-move stays"), HOLCommandGrid::GetOrder(Mixed, HOLCommandGrid::AttackMoveCell).Command == EHOLGridCommand::AttackMove);
	TestTrue(TEXT("... and the hold"), HOLCommandGrid::GetOrder(Mixed, HOLCommandGrid::HoldCell).Command == EHOLGridCommand::Hold);

	HOLCombat::FSeeker King;
	King.Radius = 60.f;
	King.AttackRange = 70.f;
	King.AggressionRadius = 0.f;
	King.bRipostes = true;
	King.Mode = HOLCombat::ESeekMode::AttackMove;
	TestEqual(TEXT("On attack-move, no search radius (a plain move)"), HOLCombat::GetSearchRadius(King), 0.f);
	HOLCombat::FCandidate Near;
	Near.Id = 1;
	Near.Position = FVector2D(150., 0.); // in reach (60 + 60 + 70)
	HOLCombat::FCandidate Attacker = Near;
	Attacker.Id = 2;
	Attacker.bAttacksMe = true;
	TestEqual(TEXT("... an enemy in its way left alone"), HOLCombat::ChooseTarget(King, { Near }), INDEX_NONE);
	TestEqual(TEXT("... the one attacking it taken on (riposte)"), HOLCombat::ChooseTarget(King, { Near, Attacker }), 1);
	King.Mode = HOLCombat::ESeekMode::Hold;
	TestEqual(TEXT("On hold (a mixed group's): an enemy in reach left alone"), HOLCombat::ChooseTarget(King, { Near }), INDEX_NONE);
	TestEqual(TEXT("... the one attacking it in reach taken on"), HOLCombat::ChooseTarget(King, { Near, Attacker }), 1);

	HOLCombat::FSeeker Peasant = King;
	Peasant.bRipostes = false;
	Peasant.Mode = HOLCombat::ESeekMode::AttackMove;
	TestEqual(TEXT("A Peasant on attack-move keeps the attack-move's radius, as before"), HOLCombat::GetSearchRadius(Peasant), HOLCombat::AttackMoveRadius);
	return true;
}

#endif
