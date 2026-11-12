// Nomad option, pure rules (N1, HOLNomadRules.as; GDD 14.1 and 14.3, D207, D354, D402). With the option, a player starts without a
// main forum: his Peasants are dispersed in small groups and the price of a forum is added to his starting stock (D402). As long as no
// forum of his is complete, he loses when he has no Peasant left (D354); a forum destroyed while being built costs no life (its resources
// are lost); from the first complete forum on, the normal rules apply (lives by tier, D347); before it, the hero comes back beside the
// largest group of Peasants. The counts are the caller's input (the C++ match state); the provisional values are the rules' constants,
// read from them (a test does not retype a value).

void Test_NomadLastPeasantDeadWithoutForumIsDefeat(FUnitTest& T)
{
	FHOLNomadState State = HOLNomad::NewState(true);
	T.AssertTrue(!HOLNomad::IsDefeated(State, 5), "5 Peasants, no forum yet: still in the match");
	T.AssertTrue(!HOLNomad::IsDefeated(State, 1), "one Peasant left: still in the match");
	T.AssertTrue(HOLNomad::IsDefeated(State, 0), "the last Peasant died before any forum was complete: defeat (D354)");
}

void Test_NomadCompleteForumBringsTheNormalRules(FUnitTest& T)
{
	FHOLNomadState State = HOLNomad::NewState(true);
	HOLNomad::NoteForumComplete(State);
	T.AssertTrue(!HOLNomad::IsDefeated(State, 0), "a complete forum: no Peasant left is no defeat any more (D354)");
	T.AssertTrue(!HOLNomad::IsBeforeFirstForum(State), "the normal rules apply (lives by tier, D347)");
	HOLNomad::NoteForumComplete(State);
	T.AssertTrue(!HOLNomad::IsDefeated(State, 0), "it stays so: the first complete forum is noted once for all");
}

void Test_NomadOffNeverLosesByPeasants(FUnitTest& T)
{
	FHOLNomadState State = HOLNomad::NewState(false);
	T.AssertTrue(!HOLNomad::IsBeforeFirstForum(State), "standard start: the main forum is there from the first second");
	T.AssertTrue(!HOLNomad::IsDefeated(State, 0), "without the option, no Peasant left is no defeat (the forum with lives, D347)");
}

void Test_NomadForumFallingWhileBuiltCostsNoLife(FUnitTest& T)
{
	T.AssertTrue(!HOLNomad::FallCostsLife(false), "a forum destroyed while being built costs no life (D354, D402)");
	T.AssertTrue(HOLNomad::FallCostsLife(true), "a complete forum falling costs a life as usual (D347)");
	FHOLNomadState State = HOLNomad::NewState(true);
	T.AssertTrue(HOLNomad::IsBeforeFirstForum(State), "its fall leaves the player before his first forum");
	T.AssertTrue(HOLNomad::IsDefeated(State, 0), "so without Peasants he still loses");
}

void Test_NomadStartingStockAddsTheForumPrice(FUnitTest& T)
{
	T.AssertEquals(200 + 300, HOLNomad::StartingAmount(200, 300, true), "Nomad: the forum price is added to the stock (D402)");
	T.AssertEquals(200, HOLNomad::StartingAmount(200, 300, false), "standard: the stock as usual, the forum is given");
	T.AssertEquals(0 + 150, HOLNomad::StartingAmount(0, 150, true), "a resource absent from the standard stock but in the forum price");
}

void Test_NomadHeroComesBackBesideTheLargestGroup(FUnitTest& T)
{
	TArray<int> Groups;
	Groups.Add(1);
	Groups.Add(3);
	Groups.Add(2);
	T.AssertEquals(1, HOLNomad::LargestGroup(Groups), "the group of 3 Peasants (D354)");
	Groups[2] = 3;
	T.AssertEquals(1, HOLNomad::LargestGroup(Groups), "a tie: the first of them (same answer on every machine)");
	TArray<int> Empty;
	Empty.Add(0);
	Empty.Add(0);
	T.AssertEquals(HOLNomad::None, HOLNomad::LargestGroup(Empty), "no Peasant: no group to come back to");
	FHOLNomadState State = HOLNomad::NewState(true);
	T.AssertTrue(HOLNomad::HeroBackAtLargestGroup(State), "before the first forum, beside the largest group (D354)");
	HOLNomad::NoteForumComplete(State);
	T.AssertTrue(!HOLNomad::HeroBackAtLargestGroup(State), "afterwards, at the forum as usual");
}

void Test_NomadPeasantsDispersedInSmallGroups(FUnitTest& T)
{
	TArray<int> Sizes = HOLNomad::SplitIntoGroups(5);
	int Total = 0;
	for (int Index = 0; Index < Sizes.Num(); Index++)
	{
		T.AssertTrue(Sizes[Index] >= 1 && Sizes[Index] <= HOLNomad::GroupSize, "a small group (D207)");
		Total += Sizes[Index];
	}
	T.AssertEquals(5, Total, "every Peasant of the start in one group");
	T.AssertTrue(Sizes.Num() >= 2, "dispersed: more than one group (D207)");
	T.AssertEquals(0, HOLNomad::SplitIntoGroups(0).Num(), "no Peasant, no group");
	T.AssertTrue(!HOLNomad::FarEnough(HOLNomad::MinEnemyDistanceCm - 1), "too close to an enemy group (D207 vigilance)");
	T.AssertTrue(HOLNomad::FarEnough(HOLNomad::MinEnemyDistanceCm), "far enough");
}
