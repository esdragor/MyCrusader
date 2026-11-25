// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// SAURON-1125a (Dawn A7, D919): the Knight's challenge is a pure provocation (8 m, 8 s, no slowdown). The choice of target of an enemy
// inside the zone, while the challenge lasts: the Knight comes first, even beyond the enemy's search radius and over a nearer ally it is
// already fighting; once the challenge is over (the flag no longer set by the processor), the normal choice is back.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLKnightChallengeTargetsTheKnightTest, "HeroesOfLiberty.Combat.KnightChallengeTargetsTheKnight", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLKnightChallengeTargetsTheKnightTest::RunTest(const FString& Parameters)
{
	// An enemy unit at the origin, a search radius of 400 cm.
	HOLCombat::FSeeker Seeker;
	Seeker.Radius = 60.f;
	Seeker.AttackRange = 50.f;
	Seeker.AggressionRadius = 400.f;

	// [0] a nearer ally of the Knight, the seeker's current target, in reach; [1] the Knight, 600 cm away (beyond the search radius).
	TArray<HOLCombat::FCandidate> Candidates;
	HOLCombat::FCandidate& Ally = Candidates.AddDefaulted_GetRef();
	Ally.Id = 1;
	Ally.Position = FVector2D(150.0, 0.0);
	Ally.bIsCurrentTarget = true;
	HOLCombat::FCandidate& Knight = Candidates.AddDefaulted_GetRef();
	Knight.Id = 2;
	Knight.Position = FVector2D(0.0, 600.0);

	// Before the challenge: the nearer ally, kept as the current target (the Knight is outside the search radius).
	const int32 Before = HOLCombat::ChooseTarget(Seeker, Candidates);
	TestEqual(TEXT("Without a challenge the seeker keeps its nearer ally"), Before, 0);

	// During the challenge: the Knight, 600 cm away, wins over the ally in reach.
	Candidates[1].bChallengesMe = true;
	const int32 During = HOLCombat::ChooseTarget(Seeker, Candidates);
	TestEqual(TEXT("During the challenge the seeker takes the Knight, not the nearer ally"), During, 1);

	// Two challenged candidates (two Knights): the nearer one, by the usual distance rank.
	HOLCombat::FCandidate& Second = Candidates.AddDefaulted_GetRef();
	Second.Id = 3;
	Second.Position = FVector2D(0.0, 300.0);
	Second.bChallengesMe = true;
	const int32 Nearer = HOLCombat::ChooseTarget(Seeker, Candidates);
	TestEqual(TEXT("Between two challengers, the nearer"), Nearer, 2);
	Candidates.Pop();

	// A challenger that is not a unit (a building) is never forced on the seeker.
	Candidates[1].bIsBuilding = true;
	TestEqual(TEXT("A building is not a challenger"), HOLCombat::ChooseTarget(Seeker, Candidates), 0);
	Candidates[1].bIsBuilding = false;

	// A seeker that cannot attack units (the Ram) is not provoked.
	HOLCombat::FSeeker Ram = Seeker;
	Ram.bCanAttackUnits = false;
	TestEqual(TEXT("A Ram is not provoked"), HOLCombat::ChooseTarget(Ram, Candidates), static_cast<int32>(INDEX_NONE));

	// After the challenge: the flag is off again, the normal choice returns.
	Candidates[1].bChallengesMe = false;
	const int32 After = HOLCombat::ChooseTarget(Seeker, Candidates);
	TestEqual(TEXT("After the challenge the seeker is back to its ally"), After, 0);
	return true;
}

// SAURON-1125e (Dawn A7, D919): (a) a hero seeker is not provoked; (b) a Knight beyond the seeker's search radius but inside the challenge
// zone challenges it (the processor adds such a Knight as a candidate with this very predicate); a Knight outside the zone, a challenge
// over, or a hero seeker: not challenged.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLKnightChallengeReachTest, "HeroesOfLiberty.Combat.KnightChallengeReach", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLKnightChallengeReachTest::RunTest(const FString& Parameters)
{
	const FVector2D Seeker(0.0, 0.0);
	const FVector2D Knight(0.0, 700.0); // beyond a 400 cm search radius, inside the 800 cm zone
	TestTrue(TEXT("A Knight at 700 cm of a 800 cm zone, the challenge lasting, challenges a soldier"), HOLCombat::IsChallengedBy(Seeker, false, Knight, 160, 800.f, 100));
	TestFalse(TEXT("... not beyond the zone"), HOLCombat::IsChallengedBy(Seeker, false, FVector2D(0.0, 900.0), 160, 800.f, 100));
	TestFalse(TEXT("... not once the challenge is over"), HOLCombat::IsChallengedBy(Seeker, false, Knight, 160, 800.f, 160));
	TestFalse(TEXT("... never a hero"), HOLCombat::IsChallengedBy(Seeker, true, Knight, 160, 800.f, 100));

	// The choice itself: an enemy hero at 700 cm is not provoked (it keeps its nearer ally), a soldier is, whichever flag the processor sets.
	HOLCombat::FSeeker Hero;
	Hero.AggressionRadius = 400.f;
	Hero.bIsHero = true;
	TArray<HOLCombat::FCandidate> Candidates;
	HOLCombat::FCandidate& Ally = Candidates.AddDefaulted_GetRef();
	Ally.Id = 1;
	Ally.Position = FVector2D(150.0, 0.0);
	Ally.bIsCurrentTarget = true;
	HOLCombat::FCandidate& Challenger = Candidates.AddDefaulted_GetRef();
	Challenger.Id = 2;
	Challenger.Position = Knight;
	Challenger.bChallengesMe = true;
	TestEqual(TEXT("A hero seeker keeps its ally, the Knight does not provoke it"), HOLCombat::ChooseTarget(Hero, Candidates), 0);
	HOLCombat::FSeeker Soldier = Hero;
	Soldier.bIsHero = false;
	TestEqual(TEXT("A soldier takes the Knight"), HOLCombat::ChooseTarget(Soldier, Candidates), 1);
	return true;
}

// SAURON-1125b (Dawn A7, D919): the cast, the C++ liaison of the script rule (HOLKnightChallenge.as). CastKnightChallenge writes the end step, the radius and
// the cool-down on the Knight's combat fragment; it refuses another type, another owner, and a second cast during the cool-down (45 s in the table row).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLKnightChallengeCastTest, "HeroesOfLiberty.Combat.KnightChallengeCast", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLKnightChallengeCastTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		const auto Place = [Units](const FName Type, const int32 Owner, const FVector& Location)
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
			return Born;
		};
		const uint32 Knight = Place(FName(TEXT("VirtuousKnight")), 0, FVector(0.0, 0.0, 0.0));
		const uint32 Peasant = Place(FName(TEXT("Peasant")), 0, FVector(300.0, 0.0, 0.0));
		FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
		const FMassEntityHandle KnightEntity = Knight != 0 ? Units->FindEntity(Knight) : FMassEntityHandle();
		FHOLCombatFragment* Combat = KnightEntity.IsSet() ? EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(KnightEntity) : nullptr;
		if (TestTrue(TEXT("a Knight of slot 0 and a Peasant of slot 0"), Combat && Peasant != 0))
		{
			TestFalse(TEXT("no challenge before the cast"), Combat->ChallengeEndStep > 0);
			TestFalse(TEXT("another type does not challenge"), Units->CastKnightChallenge(Peasant, 0, 160, 800.f, 45000));
			TestFalse(TEXT("another owner is refused"), Units->CastKnightChallenge(Knight, 1, 160, 800.f, 45000));
			TestTrue(TEXT("the cast is accepted"), Units->CastKnightChallenge(Knight, 0, 160, 800.f, 45000));
			TestEqual(TEXT("the end step is written"), Combat->ChallengeEndStep, static_cast<int64>(160));
			TestEqual(TEXT("the radius is written"), Combat->ChallengeRadiusCm, 800.f);
			TestEqual(TEXT("the cool-down is written"), Combat->KnightChallengeCooldownMs, 45000);
			TestFalse(TEXT("a second cast during the cool-down is refused"), Units->CastKnightChallenge(Knight, 0, 320, 800.f, 45000));
			TestEqual(TEXT("... and changes nothing"), Combat->ChallengeEndStep, static_cast<int64>(160));
			Combat->KnightChallengeCooldownMs = 0;
			TestTrue(TEXT("the cast is accepted again once the cool-down is over"), Units->CastKnightChallenge(Knight, 0, 320, 800.f, 45000));
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
