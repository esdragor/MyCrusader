// The challenge command's rules, alone (DuelInProgress.md D4): the place measured from the facts and the view a sheet carries.

const int Facts_All = 1 + 2 + 8 + 16;

// Near, in sight, enemies, both alive, far from every forum: the conditions hold; each one lacking breaks them.
void Test_DuelCommandConditions(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	TArray<int> Forums;
	Forums.Add(100000);
	Forums.Add(100000);
	T.AssertTrue(HOLDuel::ConditionsHold(S, Facts_All, 0, 0, 200, 0, Forums), "everything holds");
	T.AssertFalse(HOLDuel::ConditionsHold(S, Facts_All - 2, 0, 0, 200, 0, Forums), "not in sight of each other");
	T.AssertFalse(HOLDuel::ConditionsHold(S, Facts_All - 1,0, 0, 200, 0, Forums), "not enemies");
	T.AssertFalse(HOLDuel::ConditionsHold(S, Facts_All - 16, 0, 0, 200, 0, Forums), "the target is down");
	T.AssertFalse(HOLDuel::ConditionsHold(S, Facts_All + HOLDuelFact::Blocked, 0, 0, 200, 0, Forums), "an effect forbids duels");
	T.AssertFalse(HOLDuel::ConditionsHold(S, Facts_All, 0, 0, S.ArenaRadiusCm * 2 + 1, 0, Forums), "too far: they do not fit in one arena");
}

// Inside the zone of any listed forum, edge included, either hero: no duel (D403). No forum listed: no zone.
void Test_DuelCommandForumZone(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	TArray<int> Forums;
	Forums.Add(10000);
	Forums.Add(0);
	Forums.Add(-50000);
	Forums.Add(0);
	T.AssertTrue(HOLDuel::InForumZone(S, 10000 + S.ForumZoneRadiusCm, 0, Forums), "the edge of the first forum's zone");
	T.AssertFalse(HOLDuel::InForumZone(S, 10000 + S.ForumZoneRadiusCm + 1, 0, Forums), "one centimetre beyond");
	T.AssertTrue(HOLDuel::InForumZone(S, -50000, S.ForumZoneRadiusCm, Forums), "the second forum's zone too (any player's)");
	T.AssertFalse(HOLDuel::InForumZone(S, 0, 0, TArray<int>()), "no forum, no zone");
	T.AssertFalse(HOLDuel::ConditionsHold(S, Facts_All, 10000, 0, 10200, 0, Forums), "both inside the zone");
	T.AssertFalse(HOLDuel::ConditionsHold(S, Facts_All, 10000, 0, 10000 + S.ArenaRadiusCm + S.ForumZoneRadiusCm, 0, Forums), "one hero inside is enough");
}

// A challenge is shown to its challenger (code 1) and its target (code 2) with the time left, a third slot sees nothing, and a duel
// on shows code 3 with its circle once accepted.
void Test_DuelCommandView(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuelBook Book = HOLDuel::NewBook(8);
	TArray<int> Forums;
	FString Reason = HOLDuel::BookStartChallenge(S, Book, 0, 1, Facts_All, 0, 0, 300, 0, Forums);
	T.AssertEquals("", Reason, "the challenge is made");
	FHOLDuelView Sent = HOLDuel::BookViewOf(Book, 0);
	FHOLDuelView Incoming = HOLDuel::BookViewOf(Book, 1);
	T.AssertEquals(1, Sent.Code, "the challenger's view: sent");
	T.AssertEquals(1, Sent.Opponent, "... against slot 1");
	T.AssertEquals(2, Incoming.Code, "the target's view: incoming");
	T.AssertEquals(0, Incoming.Opponent, "... from slot 0");
	T.AssertEquals(S.ResponseMs, Incoming.LeftMs, "the whole window left");
	T.AssertEquals(0, HOLDuel::BookViewOf(Book, 2).Code, "a third slot sees nothing");
	T.AssertTrue(HOLDuel::Answer(S, Book, 1, true), "the answer is taken");
	HOLDuel::BookSetArena(Book, 0, 1, HOLDuel::MakeArena(S, 0, 0, 300, 0));
	FHOLDuelView On = HOLDuel::BookViewOf(Book, 1);
	T.AssertEquals(3, On.Code, "accepted: a duel on");
	T.AssertEquals(S.MaxDurationMs, On.LeftMs, "its whole time left");
	T.AssertEquals(150, On.CenterX, "the circle's centre");
	T.AssertEquals(S.ArenaRadiusCm, On.RadiusCm, "the circle's radius");
}

// A challenge refused for a reason of CanChallenge leaves the book alone (the cooldown not spent).
void Test_DuelCommandRefusedReason(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuelBook Book = HOLDuel::NewBook(8);
	TArray<int> Forums;
	Forums.Add(0);
	Forums.Add(0);
	T.AssertEquals("forum_zone", HOLDuel::BookStartChallenge(S, Book, 0, 1, Facts_All, 0, 0, 300, 0, Forums), "under the forum");
	T.AssertEquals(0, Book.Duels.Num(), "no duel in the book");
	T.AssertEquals(0, Book.CooldownLeftMs[0], "no cooldown spent");
}
