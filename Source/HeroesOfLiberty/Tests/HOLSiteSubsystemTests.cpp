// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Map/HOLFogMode.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLPlayerState.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSiteSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// S5 of SacredSitesInProgress.md, through the real chain: the rule HOLSite::Step (S1, S2) wired to the units of a world. Real units born
// by the real unit subsystem stand in a site's zone, the sites subsystem walks them each step, the site's state is what the rule makes of
// them. The option is off by default (hol.Site.Enabled): then nothing changes whatever stands in a zone.
namespace UE::HeroesOfLiberty::Tests::SiteSubsystem
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The steps to hold a site: hol.Site.HoldSeconds 1 at the simulation's rate. */
	int32 HoldSteps()
	{
		return GetDefault<UHOLSimulationSettings>()->SimulationRateHz;
	}

	/** Sets a console variable for the life of the object and puts the old value back. */
	struct FScopedVariable
	{
		IConsoleVariable* Variable = nullptr;
		FString Old;

		FScopedVariable(const TCHAR* Name, const TCHAR* Value)
		{
			Variable = IConsoleManager::Get().FindConsoleVariable(Name);
			if (Variable)
			{
				Old = Variable->GetString();
				Variable->Set(Value, ECVF_SetByCode);
			}
		}

		~FScopedVariable()
		{
			if (Variable)
			{
				Variable->Set(*Old, ECVF_SetByCode);
			}
		}
	};

	/** A world with a game state (three human slots, each its own camp), the unit subsystem and the sites subsystem. */
	struct FFixture
	{
		UWorld* World = nullptr;
		AHOLGameState* GameState = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLSiteSubsystem* Sites = nullptr;
		int64 Step = 0;

		explicit FFixture(FAutomationTestBase& Test)
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			GameState = World->SpawnActor<AHOLGameState>();
			World->SetGameState(GameState);
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Sites = World->GetSubsystem<UHOLSiteSubsystem>();
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			Config.Mode = EHOLMatchMode::Standard;
			Config.Slots.SetNum(FHOLMatchConfig::MaxSlots);
			for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
			{
				Config.Slots[Slot].State = Slot < 3 ? EHOLSlotState::Human : EHOLSlotState::Closed;
				Config.Slots[Slot].Team = 0;
			}
			if (GameState)
			{
				GameState->InitVictory(Config);
			}
			Test.TestTrue(TEXT("a game state, the units and the sites subsystem"), GameState && Units && Sites);
		}

		~FFixture()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return GameState && Units && Sites; }

		/** A unit of Type born for Owner at Location; its identifier (0 if none). */
		uint32 Place(const TCHAR* Type, const int32 Owner, const FVector& Location) const
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(Type);
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Owner);
			Request.Center = Location;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
			return After.Num() == 1 ? After[0] : 0;
		}

		void Remove(const uint32 UnitId) const
		{
			Units->RequestDestroyUnits({ UnitId });
			Units->FlushDestructions();
		}

		void Tick(const int32 Count)
		{
			for (int32 Index = 0; Index < Count; ++Index)
			{
				Sites->OnStepEnded(++Step);
			}
		}

		const FHOLSiteState& State(const int32 Index) const { return Sites->GetSites()[Index].State; }
	};

	const FVector Here(0., 0., 0.);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSiteSubsystemViewsTest, "HeroesOfLiberty.Sites.SubsystemViews", UE::HeroesOfLiberty::Tests::SiteSubsystem::Flags)

// S5 (D701), through the real chain (real units, the real fog, the players' states): each player's state holds only the sites his camp may
// know. A site nobody saw is in nobody's view; the camp whose unit stands in a zone (a Spearman sees around it) knows the site and sees its
// state; a site another camp takes is NOT shown to a camp that never saw it; a site once seen stays as it was last seen (frozen) while the
// fog hides it; without a fog every seat knows every site, live.
bool FHOLSiteSubsystemViewsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SiteSubsystem;
	FScopedVariable Enabled(TEXT("hol.Site.Enabled"), TEXT("1"));
	FScopedVariable Hold(TEXT("hol.Site.HoldSeconds"), TEXT("1"));
	FFixture Fixture(*this);
	if (!Fixture.IsReady())
	{
		return false;
	}
	UHOLVisionSubsystem* Vision = Fixture.World->GetSubsystem<UHOLVisionSubsystem>();
	if (!TestNotNull(TEXT("the vision subsystem"), Vision))
	{
		return false;
	}
	AHOLPlayerState* Players[3] = { nullptr, nullptr, nullptr };
	for (int32 Slot = 0; Slot < 3; ++Slot)
	{
		Players[Slot] = Fixture.World->SpawnActor<AHOLPlayerState>();
		if (!TestNotNull(TEXT("a player state"), Players[Slot]))
		{
			return false;
		}
		Players[Slot]->SetMatchSlot(Slot, Slot);
		Fixture.GameState->AddPlayerState(Players[Slot]);
	}
	Vision->Start(Fixture.GameState->GetLaunchConfig(), EHOLFogMode::Normal);
	const int32 SiteA = Fixture.Sites->AddSite(FVector2D(0., 0.), 1500.f);
	const int32 SiteB = Fixture.Sites->AddSite(FVector2D(30000., 0.), 1500.f);
	const uint32 Watcher = Fixture.Place(TEXT("Spearman"), 0, FVector(0., 0., 0.));
	TestTrue(TEXT("a Spearman of slot 0 on site A"), Watcher != 0);
	TestTrue(TEXT("a Monk of slot 1 on site B"), Fixture.Place(TEXT("Monk"), 1, FVector(30000., 0., 0.)) != 0);
	auto Step = [&](const int32 Count)
	{
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Vision->Update();
			Fixture.Tick(1);
		}
	};
	auto Find = [](const AHOLPlayerState* Player, const int32 Index) -> const FHOLSiteView*
	{
		return Player->GetSiteViews().FindByPredicate([Index](const FHOLSiteView& View) { return View.Index == Index; });
	};

	Step(2);
	TestEqual(TEXT("slot 0 knows one site (the one its Spearman stands in)"), Players[0]->GetSiteViews().Num(), 1);
	TestTrue(TEXT("... site A, at its place"), Find(Players[0], SiteA) && Find(Players[0], SiteA)->X == 0.f && Find(Players[0], SiteA)->Radius == 1500.f);
	TestEqual(TEXT("slot 1 knows one site (its Monk's)"), Players[1]->GetSiteViews().Num(), 1);
	TestTrue(TEXT("... site B"), Find(Players[1], SiteB) != nullptr);
	TestEqual(TEXT("slot 2 saw nothing: no site in its view"), Players[2]->GetSiteViews().Num(), 0);

	// The Monk of slot 1 takes B: slot 1 sees it its own, slot 0 (never saw B) is shown nothing, slot 2 neither.
	Step(HoldSteps() + 5);
	TestTrue(TEXT("slot 1 owns site B in its view"), Find(Players[1], SiteB) && Find(Players[1], SiteB)->Owner == 1);
	TestEqual(TEXT("a site taken by another camp is not shown to a camp that never saw it"), Players[0]->GetSiteViews().Num(), 1);
	TestTrue(TEXT("... slot 0 still knows A only"), Find(Players[0], SiteA) != nullptr && Find(Players[0], SiteB) == nullptr);
	TestEqual(TEXT("slot 2 still knows nothing"), Players[2]->GetSiteViews().Num(), 0);

	// Slot 0's Spearman leaves and a Monk of slot 1 takes A: A is explored by slot 0 but out of its sight, so its view stays as last seen (nobody's).
	Fixture.Remove(Watcher);
	TestTrue(TEXT("a Monk of slot 1 on site A"), Fixture.Place(TEXT("Monk"), 1, FVector(0., 0., 0.)) != 0);
	Step(HoldSteps() + 5);
	TestTrue(TEXT("slot 1 owns A in its view"), Find(Players[1], SiteA) && Find(Players[1], SiteA)->Owner == 1);
	TestEqual(TEXT("slot 1 knows both sites"), Players[1]->GetSiteViews().Num(), 2);
	TestTrue(TEXT("slot 0 still has A (explored)"), Find(Players[0], SiteA) != nullptr);
	TestTrue(TEXT("... frozen as it last saw it: nobody's"), Find(Players[0], SiteA) && Find(Players[0], SiteA)->Owner == INDEX_NONE);

	// Without a fog every seat knows every site, as it is.
	Vision->Start(Fixture.GameState->GetLaunchConfig(), EHOLFogMode::Disabled);
	Step(2);
	TestEqual(TEXT("no fog: slot 2 knows both sites"), Players[2]->GetSiteViews().Num(), 2);
	TestTrue(TEXT("no fog: slot 0 sees A live (slot 1's)"), Find(Players[0], SiteA) && Find(Players[0], SiteA)->Owner == 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSiteSubsystemOffByDefaultTest, "HeroesOfLiberty.Sites.SubsystemOffByDefault", UE::HeroesOfLiberty::Tests::SiteSubsystem::Flags)

// The option is off by default: a Monk alone in a site's zone for three times the hold changes nothing (the game as it is does not change).
bool FHOLSiteSubsystemOffByDefaultTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SiteSubsystem;
	TestFalse(TEXT("hol.Site.Enabled is 0 by default"), UHOLSiteSubsystem::IsEnabled());
	FScopedVariable Hold(TEXT("hol.Site.HoldSeconds"), TEXT("1"));
	FFixture Fixture(*this);
	if (!Fixture.IsReady())
	{
		return false;
	}
	Fixture.Sites->AddSite(FVector2D(Here), 1500.f);
	TestTrue(TEXT("a Monk of slot 0 born in the zone"), Fixture.Place(TEXT("Monk"), 0, Here) != 0);
	Fixture.Tick(3 * HoldSteps());
	TestEqual(TEXT("off: nobody owns the site"), Fixture.State(0).Owner, static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("off: no one fills its count"), Fixture.State(0).Held, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSiteSubsystemTakeTest, "HeroesOfLiberty.Sites.SubsystemTake", UE::HeroesOfLiberty::Tests::SiteSubsystem::Flags)

// Option on, the real units: a Monk takes the site in about the hold time (not before), a Spearman alone does not, a hero does, a Peasant
// counts for nothing (neither takes nor freezes), a Spearman of another camp freezes the count, and the site then goes to the Monk.
bool FHOLSiteSubsystemTakeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SiteSubsystem;
	FScopedVariable Enabled(TEXT("hol.Site.Enabled"), TEXT("1"));
	FScopedVariable Hold(TEXT("hol.Site.HoldSeconds"), TEXT("1"));
	FFixture Fixture(*this);
	if (!Fixture.IsReady())
	{
		return false;
	}
	const int32 Hold1 = HoldSteps();
	const int32 MonkSite = Fixture.Sites->AddSite(FVector2D(0., 0.), 1500.f);
	const int32 SpearSite = Fixture.Sites->AddSite(FVector2D(4000., 0.), 1500.f);
	const int32 HeroSite = Fixture.Sites->AddSite(FVector2D(8000., 0.), 1500.f);
	const int32 PeasantSite = Fixture.Sites->AddSite(FVector2D(12000., 0.), 1500.f);
	const int32 FrozenSite = Fixture.Sites->AddSite(FVector2D(16000., 0.), 1500.f);
	TestTrue(TEXT("a Monk of slot 0 on the first site"), Fixture.Place(TEXT("Monk"), 0, FVector(0., 0., 0.)) != 0);
	TestTrue(TEXT("a Spearman of slot 0 on the second"), Fixture.Place(TEXT("Spearman"), 0, FVector(4000., 0., 0.)) != 0);
	TestTrue(TEXT("a hero of slot 1 on the third"), Fixture.Place(TEXT("Hero"), 1, FVector(8000., 0., 0.)) != 0);
	TestTrue(TEXT("a Peasant of slot 0 on the fourth"), Fixture.Place(TEXT("Peasant"), 0, FVector(12000., 0., 0.)) != 0);
	TestTrue(TEXT("a Monk of slot 0 on the fifth"), Fixture.Place(TEXT("Monk"), 0, FVector(16000., 0., 0.)) != 0);
	const uint32 Blocker = Fixture.Place(TEXT("Spearman"), 1, FVector(16000., 0., 0.));
	TestTrue(TEXT("with a Spearman of slot 1"), Blocker != 0);
	// A Monk of slot 1 beside a Peasant of slot 0 on the fourth site: the Peasant freezes nothing.
	TestTrue(TEXT("a Monk of slot 1 on the fourth"), Fixture.Place(TEXT("Monk"), 1, FVector(12000., 0., 0.)) != 0);

	Fixture.Tick(Hold1 / 2);
	TestEqual(TEXT("half the hold: the Monk's site is not taken yet"), Fixture.State(MonkSite).Owner, static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("half the hold: slot 0 fills its count"), Fixture.State(MonkSite).Taker, 0);
	TestTrue(TEXT("half the hold: the count has gone up"), Fixture.State(MonkSite).Held > 0);
	Fixture.Tick(Hold1);
	TestEqual(TEXT("past the hold: the Monk's site is slot 0's"), Fixture.State(MonkSite).Owner, 0);
	TestEqual(TEXT("slot 0 owns one site (the Monk's)"), Fixture.Sites->CountOwned(0), 1);
	TestEqual(TEXT("a Spearman alone takes nothing"), Fixture.State(SpearSite).Owner, static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("a Spearman alone fills no count"), Fixture.State(SpearSite).Held, 0);
	TestEqual(TEXT("a hero alone takes the site (slot 1)"), Fixture.State(HeroSite).Owner, 1);
	TestEqual(TEXT("a Peasant of slot 0 freezes nothing: slot 1's Monk takes the fourth"), Fixture.State(PeasantSite).Owner, 1);
	TestEqual(TEXT("a Spearman of another camp freezes the count: the fifth is nobody's"), Fixture.State(FrozenSite).Owner, static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("frozen at zero"), Fixture.State(FrozenSite).Held, 0);
	Fixture.Remove(Blocker);
	Fixture.Tick(Hold1 + 3);
	TestEqual(TEXT("the blocker gone, the Monk takes the fifth"), Fixture.State(FrozenSite).Owner, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSiteSubsystemKeepTest, "HeroesOfLiberty.Sites.SubsystemKeepAndRetake", UE::HeroesOfLiberty::Tests::SiteSubsystem::Flags)

// A site taken stays its taker's after he leaves (D698); another camp's Monk then takes it from him; the first loses it.
bool FHOLSiteSubsystemKeepTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SiteSubsystem;
	FScopedVariable Enabled(TEXT("hol.Site.Enabled"), TEXT("1"));
	FScopedVariable Hold(TEXT("hol.Site.HoldSeconds"), TEXT("1"));
	FFixture Fixture(*this);
	if (!Fixture.IsReady())
	{
		return false;
	}
	Fixture.Sites->AddSite(FVector2D(Here), 1500.f);
	const uint32 First = Fixture.Place(TEXT("Monk"), 0, Here);
	Fixture.Tick(HoldSteps() + 3);
	TestEqual(TEXT("slot 0 took it"), Fixture.State(0).Owner, 0);
	Fixture.Remove(First);
	Fixture.Tick(3 * HoldSteps());
	TestEqual(TEXT("the Monk gone for three holds, the site is still slot 0's"), Fixture.State(0).Owner, 0);
	TestEqual(TEXT("no count left"), Fixture.State(0).Held, 0);
	TestTrue(TEXT("a Monk of slot 1 arrives"), Fixture.Place(TEXT("Monk"), 1, Here) != 0);
	Fixture.Tick(HoldSteps() / 2);
	TestEqual(TEXT("half the hold: still slot 0's"), Fixture.State(0).Owner, 0);
	Fixture.Tick(HoldSteps());
	TestEqual(TEXT("past the hold: slot 1's"), Fixture.State(0).Owner, 1);
	TestEqual(TEXT("slot 0 owns none"), Fixture.Sites->CountOwned(0), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSiteSubsystemSameTwiceTest, "HeroesOfLiberty.Sites.SubsystemSameTwice", UE::HeroesOfLiberty::Tests::SiteSubsystem::Flags)

// Two identical simulations give the same states, step by step.
bool FHOLSiteSubsystemSameTwiceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SiteSubsystem;
	FScopedVariable Enabled(TEXT("hol.Site.Enabled"), TEXT("1"));
	FScopedVariable Hold(TEXT("hol.Site.HoldSeconds"), TEXT("1"));
	TArray<FHOLSiteState> Histories[2];
	for (TArray<FHOLSiteState>& History : Histories)
	{
		FFixture Fixture(*this);
		if (!Fixture.IsReady())
		{
			return false;
		}
		Fixture.Sites->AddSite(FVector2D(Here), 1500.f);
		Fixture.Place(TEXT("Monk"), 0, Here);
		for (int32 Index = 0; Index < 3 * HoldSteps(); ++Index)
		{
			Fixture.Tick(1);
			History.Add(Fixture.State(0));
		}
	}
	TestEqual(TEXT("both ran the same number of steps"), Histories[0].Num(), Histories[1].Num());
	bool bSame = Histories[0].Num() == Histories[1].Num();
	for (int32 Index = 0; bSame && Index < Histories[0].Num(); ++Index)
	{
		bSame = Histories[0][Index] == Histories[1][Index];
	}
	TestTrue(TEXT("the two histories are identical"), bSame);
	return true;
}

#endif
