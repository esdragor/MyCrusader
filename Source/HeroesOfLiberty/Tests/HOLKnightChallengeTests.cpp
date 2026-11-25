// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatRules.h"

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

#endif // WITH_DEV_AUTOMATION_TESTS
