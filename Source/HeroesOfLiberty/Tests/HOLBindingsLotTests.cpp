// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIGarrisonOrders.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLSpecializationReadLibrary.h"
#include "Match/HOLSpecializationSubsystem.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Tests/HOLTestWorldLibrary.h"
#include "AI/HOLAISummary.h"
#include "Buildings/HOLBuilding.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLCommandGrid.h"
#include "Player/HOLCommandGridReadLibrary.h"
#include "Simulation/HOLUnitScriptLibrary.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// ELROND-1123e: the grouped C++ bindings (D839). One test per binding that reads a value: the AI summary's garrison kind (S4-b), the grid's LightSeal
// (the two enums keep the same order), the unit owner and type of a UnitId unknown to the machine, the AI garrison order refused without a world.
namespace UE::HeroesOfLiberty::Tests::BindingsLot
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FHOLUnitView MakeUnit(const uint32 Id, const FName Type, const bool bSiege, const bool bRanged, const bool bHero)
	{
		FHOLUnitView Unit;
		Unit.UnitId = Id;
		Unit.OwnerPlayer = 0;
		Unit.TypeName = Type;
		Unit.bSiege = bSiege;
		Unit.bRanged = bRanged;
		Unit.bHero = bHero;
		Unit.Activity = EHOLUnitActivity::Idle;
		return Unit;
	}

	/** A unit of Type born for slot 0 in the world through the real unit table, its identifier (0 if none). */
	uint32 Place(UHOLUnitSubsystem* Units, const FName Type, const FVector& Location)
	{
		TArray<uint32> Before;
		Units->GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Type;
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = Location;
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> After;
		Units->GetAllUnitIds(After);
		After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
		return After.Num() == 1 ? After[0] : 0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsAIGarrisonKindTest, "HeroesOfLiberty.AI.Summary.GarrisonKind", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsAIGarrisonKindTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BindingsLot;
	const TArray<FHOLUnitView> Units = {
		MakeUnit(1, FName(TEXT("Spearman")), false, false, false), MakeUnit(2, FName(TEXT("Archer")), false, true, false) };
	const FHOLAISummary Slot0 = HOLAISummary::Make(0, Units, {}, {}, {});
	TMap<int32, int32> KindById;
	for (const FHOLAIUnit& Unit : Slot0.Army)
	{
		KindById.Add(Unit.Id, Unit.GarrisonKind);
	}
	// HOLGarrison::EKind: 0 melee, 1 ranged. A unit left out of the Army reads as INDEX_NONE and fails.
	TestEqual(TEXT("Spearman: melee infantry"), KindById.FindRef(1), 0);
	TestEqual(TEXT("Archer: ranged infantry"), KindById.FindRef(2), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsLightSealTest, "HeroesOfLiberty.Player.CommandGrid.LightSealEnum", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsLightSealTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("LightSeal has the same number in the rule's enum and the script's UENUM"), static_cast<uint8>(EHOLGridCommandKind::LightSeal), static_cast<uint8>(EHOLGridCommand::LightSeal));
	TestEqual(TEXT("LightSeal follows UngarrisonAll"), static_cast<int32>(EHOLGridCommand::LightSeal), static_cast<int32>(EHOLGridCommand::UngarrisonAll) + 1);
	FHOLGridContext Context;
	TestFalse(TEXT("bMonkUnit false by default"), Context.bMonkUnit);
	FHOLGridContextView View;
	TestFalse(TEXT("the view's bMonkUnit false by default"), View.bMonkUnit);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsUnitOwnerTypeTest, "HeroesOfLiberty.Simulation.UnitScriptLibrary.OwnerType", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsUnitOwnerTypeTest::RunTest(const FString& Parameters)
{
	int32 Owner = 5;
	TestFalse(TEXT("no world: the owner is not found"), UHOLUnitScriptLibrary::GetUnitOwner(nullptr, 7, Owner));
	TestEqual(TEXT("no world: the owner slot is INDEX_NONE"), Owner, static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("no world: the type is none"), UHOLUnitScriptLibrary::GetUnitType(nullptr, 7), FName());
	TestFalse(TEXT("no world: the AI garrison order is refused"), UHOLAIGarrisonOrderLibrary::IssueAIGarrisonOrder(nullptr, 0, TArray<int32>{ 7 }, nullptr));
	return true;
}

// ELROND-1123g B2: the player controller reads each selected unit through HOLCommandGrid::NoteSelectedUnitType; a real Monk, Damned Warrior and
// Spearman born in a world (the real unit table, HOLUnitView::Find) must set bMonkUnit / bRageUnit as the Sceau de lumiere and rage cells need.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsSelectionKindsTest, "HeroesOfLiberty.Player.CommandGrid.SelectionKinds", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsSelectionKindsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BindingsLot;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->SetGameState(World->SpawnActor<AHOLGameState>());
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	if (TestNotNull(TEXT("units"), Units))
	{
		const uint32 Monk = Place(Units, FName(TEXT("Monk")), FVector::ZeroVector);
		const uint32 Warrior = Place(Units, FName(TEXT("DamnedWarrior")), FVector(300., 0., 0.));
		const uint32 Spearman = Place(Units, FName(TEXT("Spearman")), FVector(600., 0., 0.));
		if (TestTrue(TEXT("a Monk, a Damned Warrior and a Spearman born"), Monk != 0 && Warrior != 0 && Spearman != 0))
		{
			const auto Read = [World](const uint32 Id)
			{
				FHOLGridContext Context;
				FHOLUnitView View;
				if (HOLUnitView::Find(*World, Id, View))
				{
					HOLCommandGrid::NoteSelectedUnitType(Context, View.TypeName);
				}
				return Context;
			};
			const FHOLGridContext MonkOnly = Read(Monk);
			TestTrue(TEXT("a selected Monk: bMonkUnit"), MonkOnly.bMonkUnit);
			TestFalse(TEXT("a selected Monk: not bRageUnit"), MonkOnly.bRageUnit);
			const FHOLGridContext WarriorOnly = Read(Warrior);
			TestTrue(TEXT("a selected Damned Warrior: bRageUnit"), WarriorOnly.bRageUnit);
			TestFalse(TEXT("a selected Damned Warrior: not bMonkUnit"), WarriorOnly.bMonkUnit);
			const FHOLGridContext SpearmanOnly = Read(Spearman);
			TestFalse(TEXT("a selected Spearman: not bMonkUnit"), SpearmanOnly.bMonkUnit);
			TestFalse(TEXT("a selected Spearman: not bRageUnit"), SpearmanOnly.bRageUnit);
			// A mixed selection accumulates (|=), as the controller's loop does.
			FHOLGridContext Mixed;
			for (const uint32 Id : { Spearman, Monk, Warrior })
			{
				FHOLUnitView View;
				if (HOLUnitView::Find(*World, Id, View))
				{
					HOLCommandGrid::NoteSelectedUnitType(Mixed, View.TypeName);
				}
			}
			TestTrue(TEXT("a mixed selection holding a Monk: bMonkUnit"), Mixed.bMonkUnit);
			TestTrue(TEXT("a mixed selection holding a Damned Warrior: bRageUnit"), Mixed.bRageUnit);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// ELROND-1123g G1 / G3: the AI summary reports the cavalry (kind 2), the siege (3) and the hero (4); a LightCavalry born in a world carries bCavalry from its type row.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsGarrisonKindCavalryTest, "HeroesOfLiberty.AI.Summary.GarrisonKindCavalry", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsGarrisonKindCavalryTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BindingsLot;
	FHOLUnitView Horse = MakeUnit(1, FName(TEXT("LightCavalry")), false, false, false);
	Horse.bCavalry = true;
	const TArray<FHOLUnitView> Units = {
		Horse, MakeUnit(2, FName(TEXT("Ram")), true, false, false), MakeUnit(3, FName(TEXT("Hero")), false, false, true) };
	const FHOLAISummary Slot0 = HOLAISummary::Make(0, Units, {}, {}, {});
	TMap<int32, int32> KindById;
	for (const FHOLAIUnit& Unit : Slot0.Army)
	{
		KindById.Add(Unit.Id, Unit.GarrisonKind);
	}
	// HOLGarrison::EKind: 2 cavalry, 3 siege. The hero is left out of the army (IA2.1), so he is not checked here.
	TestEqual(TEXT("a cavalryman: kind 2"), KindById.FindRef(1), 2);
	TestEqual(TEXT("a siege unit: kind 3"), KindById.FindRef(2), 3);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->SetGameState(World->SpawnActor<AHOLGameState>());
	UHOLUnitSubsystem* Subsystem = World->GetSubsystem<UHOLUnitSubsystem>();
	if (TestNotNull(TEXT("units"), Subsystem))
	{
		const uint32 Cavalry = Place(Subsystem, FName(TEXT("LightCavalry")), FVector::ZeroVector);
		const uint32 Spearman = Place(Subsystem, FName(TEXT("Spearman")), FVector(300., 0., 0.));
		FHOLUnitView CavalryView;
		FHOLUnitView SpearmanView;
		if (TestTrue(TEXT("both born and found"), Cavalry != 0 && Spearman != 0 && HOLUnitView::Find(*World, Cavalry, CavalryView) && HOLUnitView::Find(*World, Spearman, SpearmanView)))
		{
			TestTrue(TEXT("a LightCavalry reads bCavalry from its type"), CavalryView.bCavalry);
			TestFalse(TEXT("a Spearman does not"), SpearmanView.bCavalry);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// ELROND-1123g S4-b: the AI summary gives each building the seconds since its last hit (what the script's garrison rule reads): a hit House reads >= 0, an untouched one < 0.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsSecondsSinceHitTest, "HeroesOfLiberty.AI.Summary.SecondsSinceHit", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsSecondsSinceHitTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->SetGameState(World->SpawnActor<AHOLGameState>());
	AHOLBuilding* Hit = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Spared = World->SpawnActor<AHOLBuilding>(FVector(-3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("two buildings"), Hit) && TestNotNull(TEXT("two buildings"), Spared))
	{
		Hit->Setup(HOLBuildingTypeNames::House, 0, 0);
		Spared->Setup(HOLBuildingTypeNames::House, 0, 0);
		Hit->TakeDamagePoints(1);
		TArray<FHOLAIBuilding> Buildings;
		HOLAISummary::GatherBuildings(*World, Buildings);
		float HitSeconds = -99.f;
		float SparedSeconds = -99.f;
		for (const FHOLAIBuilding& Building : Buildings)
		{
			if (Building.Actor == Hit)
			{
				HitSeconds = Building.SecondsSinceHit;
			}
			else if (Building.Actor == Spared)
			{
				SparedSeconds = Building.SecondsSinceHit;
			}
		}
		TestTrue(TEXT("the building that took a hit reads >= 0 s since the hit"), HitSeconds >= 0.f && HitSeconds < 5.f);
		TestTrue(TEXT("the building never hit reads a negative value"), SparedSeconds < 0.f && SparedSeconds > -50.f);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}
// ELROND-1123pf (lot of bindings 2, D839). (A) The SP3 button reads "Forum complete" through the server's own calculation (a standard seat true, a Nomad
// seat false until its first Forum is complete, no world false). Model: HeroesOfLiberty.Specialization.NomadForum.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsForumCompleteForSlotTest, "HeroesOfLiberty.Specialization.ForumCompleteForSlot", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsForumCompleteForSlotTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);

	UHOLSpecializationSubsystem* Board = UHOLSpecializationSubsystem::Find(World);
	if (TestNotNull(TEXT("the script board of specializations"), Board))
	{
		Board->SetNomadNow(1, true);
		TestTrue(TEXT("a standard seat has its Forum"), UHOLSpecializationReadLibrary::IsForumCompleteForSlot(World, 0));
		TestFalse(TEXT("a Nomad seat without a Forum has none"), UHOLSpecializationReadLibrary::IsForumCompleteForSlot(World, 1));
		Board->NoteForumCompleteNow(1);
		TestTrue(TEXT("its first Forum complete: it has one"), UHOLSpecializationReadLibrary::IsForumCompleteForSlot(World, 1));
		TestFalse(TEXT("a negative slot has none"), UHOLSpecializationReadLibrary::IsForumCompleteForSlot(World, -1));
	}
	TestFalse(TEXT("no world, no Forum"), UHOLSpecializationReadLibrary::IsForumCompleteForSlot(nullptr, 0));
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// (B) HOLTestWorld::SpawnTestHero makes the slot's hero BORN ALIVE: the unit table knows it as the slot's hero, and the real PlantBanner of the hero
// subsystem (the one PlantHeroBanner calls) works on it: refused at level 1, accepted at level 4 inside 16 m, refused beyond.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsSpawnTestHeroTest, "HeroesOfLiberty.TestWorld.SpawnTestHero", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsSpawnTestHeroTest::RunTest(const FString& Parameters)
{
	UHOLTestWorldHandle* Handle = UHOLTestWorldLibrary::CreateTestWorld();
	if (TestNotNull(TEXT("the test world"), Handle))
	{
		UWorld* World = Handle->GetTestWorld();
		UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
		UHOLHeroSubsystem* Heroes = World ? World->GetSubsystem<UHOLHeroSubsystem>() : nullptr;
		const int32 HeroUnit = UHOLTestWorldLibrary::SpawnTestHero(Handle, 0, FVector(0.0, 0.0, 0.0));
		TestTrue(TEXT("a hero unit was born"), HeroUnit > 0);
		if (TestNotNull(TEXT("the unit table"), Units) && TestNotNull(TEXT("the hero subsystem"), Heroes))
		{
			TestEqual(TEXT("it is the slot's hero"), static_cast<int32>(Units->FindHeroUnit(0)), HeroUnit);
			TestFalse(TEXT("no banner at level 1"), Heroes->PlantBanner(0, FVector(1600.0, 0.0, 0.0)));
			Heroes->SetXP(0, HOLHeroXP::GetLevelXP(4));
			TestFalse(TEXT("no banner 16.5 m away"), Heroes->PlantBanner(0, FVector(1650.0, 0.0, 0.0)));
			TestTrue(TEXT("a banner 16 m away"), Heroes->PlantBanner(0, FVector(1600.0, 0.0, 0.0)));
		}
		TestEqual(TEXT("an unknown slot gets no hero"), UHOLTestWorldLibrary::SpawnTestHero(Handle, -1, FVector::ZeroVector), 0);
		UHOLTestWorldLibrary::DestroyTestWorld(Handle);
	}
	return true;
}

// (C) HOLTestWorld::SetPlayerSpecialization (AHOLPlayerState::SetSpecialization, for the tests only) puts the view on the player: what GetSpecialization
// (a UFUNCTION, the SP3 ReadView's source) then reads back.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBindingsSetPlayerSpecializationTest, "HeroesOfLiberty.TestWorld.SetPlayerSpecialization", UE::HeroesOfLiberty::Tests::BindingsLot::Flags)
bool FHOLBindingsSetPlayerSpecializationTest::RunTest(const FString& Parameters)
{
	UHOLTestWorldHandle* Handle = UHOLTestWorldLibrary::CreateTestWorld();
	if (TestNotNull(TEXT("the test world"), Handle))
	{
		AHOLPlayerState* Player = UHOLTestWorldLibrary::SpawnPlayerState(Handle, 0, 0, TEXT("Spec"));
		if (TestNotNull(TEXT("a player state"), Player))
		{
			TestEqual(TEXT("nothing runs at first"), Player->GetSpecialization().PendingTier, 0);
			FHOLSpecializationView View;
			View.PendingTier = 1;
			View.PendingOption = 1;
			View.RemainingMs = 5000;
			TestTrue(TEXT("the view changed: the server's answer is true"), UHOLTestWorldLibrary::SetPlayerSpecialization(Player, View));
			TestEqual(TEXT("the pending tier reads back"), Player->GetSpecialization().PendingTier, 1);
			TestEqual(TEXT("the pending option reads back"), Player->GetSpecialization().PendingOption, 1);
			TestEqual(TEXT("the remaining time reads back"), Player->GetSpecialization().RemainingMs, 5000);
		}
		TestFalse(TEXT("no player, nothing set"), UHOLTestWorldLibrary::SetPlayerSpecialization(nullptr, FHOLSpecializationView()));
		UHOLTestWorldLibrary::DestroyTestWorld(Handle);
	}
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
