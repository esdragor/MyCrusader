// Port to script of the PURE Forum.* tests of HOLVictoryTests.cpp (D830, D836, D839), run against UHOLForumRulesReadLibrary
// (namespace HOLForumRulesRead, CELEBORN-1117s), the read library of the rule that STAYS in C++ (HOLForumRules, FHOLForumState is replicated).
// Same vectors as the C++ tests Forum.Rules, Forum.Hold.Rules, Forum.Block, Forum.Hold.Display (rings and lines in ASCII) and Forum.Look.
// NOT ported (here): the em-dash / accent lines of Block.Text (compared as bytes), GetRingColor, GetZoneOutlinePoint, GetPlayersTable
// (FHOLMatchConfig, Galadriel's lot), and the tests that build a world or Mass (Forum.Fall, Hold.Chain, Hold.Presence, Builders: D838).
// Phase: 0 Standing, 1 Ruin, 2 Rebuilding. Hold: 0 None, 1 Holding, 2 Contested, 3 Receding. Look: 0 Standing, 1 Rubble, 2 Site.
// The C++ tests are removed by the main session after red then green.

bool ForumNear(float A, float B, float Tolerance)
{
	float Delta = A - B;
	if (Delta < 0.0f)
	{
		Delta = -Delta;
	}
	return Delta <= Tolerance;
}

// HeroesOfLiberty.Victory.Forum.Rules: the lives and the phases.
void Test_ForumRulesLives(FUnitTest& T)
{
	T.AssertEquals(2, HOLForumRulesRead::GetLivesByTier(0, 0), "tier 0, no fall: 2 lives (D349)");
	T.AssertEquals(1, HOLForumRulesRead::GetLivesByTier(0, 1), "tier 0, one fall: 1 life");
	T.AssertEquals(0, HOLForumRulesRead::GetLivesByTier(0, 2), "tier 0, two falls: 0 life");
	T.AssertEquals(0, HOLForumRulesRead::GetLivesByTier(0, 3), "never below 0");
	T.AssertEquals(3, HOLForumRulesRead::GetLivesByTier(1, 0), "tier 1, no fall: 3 lives");
	T.AssertEquals(2, HOLForumRulesRead::GetBaseLives(), "the base lives");
}

// The game's values (DefaultGame.ini): 2 lives, ruin ~30 s, rebuilding alone ~120 s; D312 calibrated so that one Peasant does as much as the rebuilding alone.
void Test_ForumRulesGameValues(FUnitTest& T)
{
	FHOLForumTuningRead Game = HOLForumRulesRead::GetGameTuning();
	float StepSeconds = Game.StepSeconds;
	T.AssertEquals(0, Game.Tier, "the game's tier: 0");
	T.AssertTrue(ForumNear(Game.RuinSteps * StepSeconds, 30.0f, 0.1f), "the game's ruin: 30 s");
	T.AssertTrue(ForumNear(Game.RebuildSteps * StepSeconds, 120.0f, 0.1f), "the game's rebuilding alone: 120 s");
	T.AssertTrue(ForumNear(Game.BuilderExponent, 0.63f, 0.001f), "the game's builder exponent (D312)");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetStepsToRebuild(Game, 0) * StepSeconds, 120.0f, 0.1f), "0 Peasant: 120 s");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetStepsToRebuild(Game, 1) * StepSeconds, 60.0f, 0.1f), "1 Peasant: 60 s");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetStepsToRebuild(Game, 5) * StepSeconds, 32.0f, 0.5f), "5 Peasants: ~32 s");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetStepsToRebuild(Game, 10) * StepSeconds, 23.0f, 0.5f), "10 Peasants: ~23 s");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetStepsToRebuild(Game, 40) * StepSeconds, 11.0f, 0.5f), "40 Peasants: ~11 s");
	// The hold's values (Forum.Hold.Rules): 60 s held, 2 s given back a second, a zone of 12 m from the edge.
	T.AssertTrue(ForumNear(Game.HoldSteps * StepSeconds, 60.0f, 0.1f), "the game's hold: 60 s");
	T.AssertEquals(2, Game.HoldDecaySteps, "the game's decay: 2 s a second");
	T.AssertTrue(ForumNear(Game.HoldRadius, 1200.0f, 0.1f), "the game's zone: 12 m");
}

// The chain: fall, ruin, rebuilding, standing, second fall (the defeat), and the cheat's Skip.
void Test_ForumRulesPhases(FUnitTest& T)
{
	FHOLForumTuningRead Tuning;
	Tuning.RuinSteps = 3;
	Tuning.RebuildSteps = 4;
	FHOLForumStateRead State;
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetRebuildProgress(State, Tuning), 1.0f, 0.0001f), "standing: progress 1");
	T.AssertFalse(HOLForumRulesRead::Advance(State, Tuning, 5), "standing: nothing to advance");
	T.AssertEquals(1, HOLForumRulesRead::Fall(State, Tuning), "first fall: 1 life left");
	T.AssertEquals(1, State.Phase, "first fall: ruin");
	T.AssertEquals(-1, HOLForumRulesRead::Fall(State, Tuning), "a fall in ruin refused (unbreakable)");
	T.AssertEquals(1, State.Falls, "still one fall");
	T.AssertFalse(HOLForumRulesRead::Advance(State, Tuning, 40), "ruin step 1");
	T.AssertFalse(HOLForumRulesRead::Advance(State, Tuning, 40), "ruin step 2");
	T.AssertTrue(State.RebuildWork == 0, "the Peasants build nothing in the ruin");
	bool bChanged = HOLForumRulesRead::Advance(State, Tuning, 0);
	T.AssertTrue(bChanged, "ruin step 3: the phase changes");
	T.AssertEquals(2, State.Phase, "ruin step 3: rebuilding");
	T.AssertEquals(-1, HOLForumRulesRead::Fall(State, Tuning), "a fall in rebuilding refused (unbreakable)");
	T.AssertFalse(HOLForumRulesRead::Advance(State, Tuning, 0), "rebuilding step 1");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetRebuildProgress(State, Tuning), 0.25f, 0.001f), "rebuilding: a quarter done");
	// One Peasant doubles the rate: the 3 steps of work left take 2 steps (1.5).
	T.AssertFalse(HOLForumRulesRead::Advance(State, Tuning, 1), "rebuilding step 2, one Peasant");
	bChanged = HOLForumRulesRead::Advance(State, Tuning, 1);
	T.AssertTrue(bChanged, "rebuilding step 3, one Peasant: the phase changes");
	T.AssertEquals(0, State.Phase, "rebuilding step 3, one Peasant: standing");
	T.AssertEquals(1, HOLForumRulesRead::GetLives(State, Tuning), "rebuilt: lives kept");
	T.AssertEquals(0, HOLForumRulesRead::Fall(State, Tuning), "second fall: no life left, the defeat");

	FHOLForumStateRead Skipped;
	T.AssertFalse(HOLForumRulesRead::Skip(Skipped), "skip: nothing when standing");
	HOLForumRulesRead::Fall(Skipped, Tuning);
	bool bSkip1 = HOLForumRulesRead::Skip(Skipped);
	T.AssertTrue(bSkip1, "skip: ruin -> rebuilding (changed)");
	T.AssertEquals(2, Skipped.Phase, "skip: ruin -> rebuilding");
	bool bSkip2 = HOLForumRulesRead::Skip(Skipped);
	T.AssertTrue(bSkip2, "skip: rebuilding -> standing (changed)");
	T.AssertEquals(0, Skipped.Phase, "skip: rebuilding -> standing");
}

// HeroesOfLiberty.Victory.Forum.Hold.Rules: holding a forum down, the presence given to the rule.
void Test_ForumHoldRules(FUnitTest& T)
{
	FHOLForumTuningRead Tuning;
	Tuning.Tier = 1;
	Tuning.RuinSteps = 3;
	Tuning.RebuildSteps = 4;
	Tuning.HoldSteps = 10;
	Tuning.HoldDecaySteps = 2;

	FHOLForumStateRead State;
	// Held = attackers of slot 1 (true, false, 1); Contested = attackers and defenders (true, true, 1); Nobody (false, false, -1); DefendersOnly (false, true, -1).
	FHOLForumStepResultRead Standing = HOLForumRulesRead::Step(State, Tuning, 0, true, false, 1);
	T.AssertFalse(Standing.bRazed, "standing: nothing razed");
	T.AssertEquals(0, State.HoldSteps, "standing: no hold");
	T.AssertEquals(0, State.Hold, "standing: hold none");
	T.AssertEquals(-1, HOLForumRulesRead::Raze(State, Tuning), "standing: nothing to raze");
	T.AssertEquals(2, HOLForumRulesRead::Fall(State, Tuning), "first fall: 2 lives (tier 1)");

	// The count runs from the fall, the ruin included (D352); the ruin counts down as ever.
	HOLForumRulesRead::Step(State, Tuning, 0, true, false, 1);
	T.AssertEquals(1, State.HoldSteps, "held in ruin: 1 step");
	T.AssertEquals(1, State.Hold, "held in ruin: holding");
	T.AssertEquals(1, State.HolderSlot, "held by slot 1");
	T.AssertEquals(2, State.RuinStepsLeft, "the ruin counts down while held");
	HOLForumRulesRead::Step(State, Tuning, 0, true, false, 1);
	FHOLForumStepResultRead Over = HOLForumRulesRead::Step(State, Tuning, 0, true, false, 1);
	T.AssertTrue(Over.bPhaseChanged, "ruin over while held: the phase changes");
	T.AssertEquals(2, State.Phase, "ruin over while held: rebuilding");
	T.AssertEquals(3, State.HoldSteps, "3 steps held");

	// The rebuilding frozen while attackers stand there (D348), contested or not.
	HOLForumRulesRead::Step(State, Tuning, 5, true, false, 1);
	T.AssertEquals(4, State.HoldSteps, "held: 4 steps");
	T.AssertTrue(State.RebuildWork == 0, "held: the rebuilding frozen, Peasants or not");
	HOLForumRulesRead::Step(State, Tuning, 5, true, true, 1);
	HOLForumRulesRead::Step(State, Tuning, 5, true, true, 1);
	T.AssertEquals(4, State.HoldSteps, "contested: the count frozen");
	T.AssertEquals(2, State.Hold, "contested");
	T.AssertEquals(1, State.HolderSlot, "contested: the holder kept");
	T.AssertTrue(State.RebuildWork == 0, "contested: the rebuilding frozen");

	// No attacker: the count recedes 2 steps a step, never below 0 (V3-Q3); the rebuilding goes on.
	HOLForumRulesRead::Step(State, Tuning, 0, false, false, -1);
	T.AssertEquals(2, State.HoldSteps, "nobody: 2 steps given back");
	T.AssertEquals(3, State.Hold, "nobody: receding");
	T.AssertEquals(1, State.HolderSlot, "receding: the holder's colour kept");
	T.AssertTrue(State.RebuildWork > 0, "nobody: the rebuilding goes on");
	HOLForumRulesRead::Step(State, Tuning, 0, false, true, -1);
	T.AssertEquals(0, State.HoldSteps, "defenders alone: back to 0");
	T.AssertEquals(0, State.Hold, "back to 0: hold none");
	T.AssertEquals(-1, State.HolderSlot, "back to 0: no holder");
	HOLForumRulesRead::Step(State, Tuning, 0, false, false, -1);
	T.AssertEquals(0, State.HoldSteps, "never below 0");
	T.AssertEquals(2, State.Phase, "still rebuilding");
	int64 WorkBefore = State.RebuildWork;

	// Held 10 steps without contest: the foundations razed at the 10th, a new complete fall (V3-Q4).
	bool bRazedEarly = false;
	for (int Index = 0; Index < 9; ++Index)
	{
		FHOLForumStepResultRead Early = HOLForumRulesRead::Step(State, Tuning, 3, true, false, 1);
		if (Early.bRazed)
		{
			bRazedEarly = true;
		}
	}
	T.AssertFalse(bRazedEarly, "not razed before the 10th step");
	T.AssertEquals(9, State.HoldSteps, "9 steps held");
	T.AssertTrue(State.RebuildWork == WorkBefore, "held: the rebuilding kept where it was");
	FHOLForumStepResultRead Razed = HOLForumRulesRead::Step(State, Tuning, 3, true, false, 1);
	T.AssertTrue(Razed.bRazed, "10th step: razed");
	T.AssertTrue(Razed.bPhaseChanged, "10th step: the phase changed");
	T.AssertEquals(1, Razed.Lives, "razed: a life less");
	T.AssertEquals(2, State.Falls, "razed: a fall more");
	T.AssertEquals(1, State.Phase, "razed: in ruin again");
	T.AssertEquals(3, State.RuinStepsLeft, "razed: the whole ruin again");
	T.AssertTrue(State.RebuildWork == 0, "razed: the rebuilding from nothing");
	T.AssertEquals(0, State.HoldSteps, "razed: the count at 0");
	T.AssertEquals(0, State.Hold, "razed: hold none");
	T.AssertEquals(-1, State.HolderSlot, "razed: no holder");

	// The last life: razed again, the defeat (0 lives).
	for (int Index = 0; Index < 9; ++Index)
	{
		HOLForumRulesRead::Step(State, Tuning, 0, true, false, 1);
	}
	FHOLForumStepResultRead Last = HOLForumRulesRead::Step(State, Tuning, 0, true, false, 1);
	T.AssertTrue(Last.bRazed, "razed on the last life");
	T.AssertEquals(0, Last.Lives, "razed on the last life: 0 lives, the defeat");
	T.AssertEquals(3, State.Falls, "razed on the last life: the third fall");
}

// What the HUD shows (V3.3): whole seconds held, the ring's fill; and a rebuilt forum's count at 0.
void Test_ForumHoldProgress(FUnitTest& T)
{
	FHOLForumTuningRead Tuning;
	Tuning.Tier = 1;
	Tuning.RuinSteps = 3;
	Tuning.RebuildSteps = 4;
	Tuning.HoldSteps = 50;
	FHOLForumStateRead Half;
	HOLForumRulesRead::Fall(Half, Tuning);
	Half.HoldSteps = 25;
	T.AssertEquals(2, HOLForumRulesRead::GetHoldSeconds(Half, Tuning), "25 steps of 0.1 s: 2 s");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetHoldProgress(Half, Tuning), 0.5f, 0.001f), "25 of 50: half the ring");

	// Standing again: the count at 0.
	FHOLForumStateRead Rebuilt;
	HOLForumRulesRead::Fall(Rebuilt, Tuning);
	HOLForumRulesRead::Step(Rebuilt, Tuning, 0, true, false, 1);
	T.AssertEquals(1, Rebuilt.HoldSteps, "held a step in ruin");
	HOLForumRulesRead::Skip(Rebuilt);
	HOLForumRulesRead::Skip(Rebuilt);
	T.AssertEquals(0, Rebuilt.Phase, "rebuilt: standing");
	T.AssertEquals(0, Rebuilt.HoldSteps, "rebuilt: the count at 0");
	T.AssertEquals(0, Rebuilt.Hold, "rebuilt: hold none");
}

// HeroesOfLiberty.Victory.Forum.Block: what the selection block shows of a main forum (the ASCII "Shown" line; Text only by its numbers).
void Test_ForumBlock(FUnitTest& T)
{
	FHOLForumTuningRead Tuning;
	Tuning.Tier = 0;
	Tuning.RuinSteps = 300;
	Tuning.RebuildSteps = 1200;
	Tuning.StepSeconds = 0.1f;

	FHOLForumStateRead State;
	// A secondary centre (no forum state): nothing changes.
	FHOLForumBlockRead Secondary = HOLForumRulesRead::GetSelectionBlock(false, State, Tuning, true, 3);
	T.AssertFalse(Secondary.bShown, "secondary centre: no forum block");
	T.AssertFalse(Secondary.bHideHealth, "secondary centre: health kept");
	T.AssertTrue(Secondary.Shown == "", "secondary centre: nothing described");

	// Standing: its health as before, its lives under it, to anyone.
	for (int Own = 0; Own < 2; ++Own)
	{
		FHOLForumBlockRead Standing = HOLForumRulesRead::GetSelectionBlock(true, State, Tuning, Own == 0, 0);
		T.AssertTrue(Standing.bShown, "standing: shown");
		T.AssertFalse(Standing.bHideHealth, "standing: health kept");
		T.AssertTrue(Standing.Shown == ", lives 2", "standing: described");
		T.AssertEquals(2, Standing.Lives, "standing: its lives");
		T.AssertTrue(Standing.Text == "", "standing: no line");
	}

	// In ruin, 25 s left (24.1 s rounds up): the seconds in place of the health, then its lives.
	T.AssertEquals(1, HOLForumRulesRead::Fall(State, Tuning), "fell: 1 life left");
	State.RuinStepsLeft = 250;
	T.AssertEquals(25, HOLForumRulesRead::GetRuinSecondsLeft(State, Tuning), "ruin: 25 s left");
	State.RuinStepsLeft = 241;
	T.AssertEquals(25, HOLForumRulesRead::GetRuinSecondsLeft(State, Tuning), "ruin: 24.1 s shows 25 s");
	for (int Own = 0; Own < 2; ++Own)
	{
		FHOLForumBlockRead Ruin = HOLForumRulesRead::GetSelectionBlock(true, State, Tuning, Own == 0, 2);
		T.AssertTrue(Ruin.bShown, "ruin: shown");
		T.AssertTrue(Ruin.bHideHealth, "ruin: health hidden");
		T.AssertTrue(Ruin.Shown == ", in ruin 25 s, lives 1", "ruin: described");
		T.AssertTrue(Ruin.Text.Contains("25 s"), "ruin: its line carries the seconds");
		T.AssertEquals(1, Ruin.Lives, "ruin: its lives");
	}

	// Rising again at 40 %: its builders to its owner alone (as a site's, D313).
	State.Phase = 2;
	State.RuinStepsLeft = 0;
	State.RebuildWork = HOLForumRulesRead::GetRebuildTotalWork(Tuning) * 40 / 100;
	T.AssertEquals(40, HOLForumRulesRead::GetRebuildPercent(State, Tuning), "rebuilding: 40 %");
	T.AssertEquals(0, HOLForumRulesRead::GetRuinSecondsLeft(State, Tuning), "rebuilding: no ruin left");
	FHOLForumBlockRead OwnRebuilding = HOLForumRulesRead::GetSelectionBlock(true, State, Tuning, true, 3);
	T.AssertTrue(OwnRebuilding.bHideHealth, "rebuilding: health hidden");
	T.AssertTrue(OwnRebuilding.Shown == ", rebuilding 40%, builders 3, lives 1", "rebuilding, own: described");
	T.AssertTrue(OwnRebuilding.Text.Contains("40 %") && OwnRebuilding.Text.Contains(": 3"), "rebuilding, own: its lines carry the percent and the builders");
	T.AssertEquals(1, OwnRebuilding.Lives, "rebuilding, own: its lives");
	FHOLForumBlockRead OtherRebuilding = HOLForumRulesRead::GetSelectionBlock(true, State, Tuning, false, 3);
	T.AssertTrue(OtherRebuilding.bHideHealth, "rebuilding, another's: health hidden");
	T.AssertTrue(OtherRebuilding.Shown == ", rebuilding 40%, lives 1", "rebuilding, another's: described");
	T.AssertFalse(OtherRebuilding.Text.Contains(": 3"), "rebuilding, another's: no builders");
	T.AssertEquals(1, OtherRebuilding.Lives, "rebuilding, another's: its lives");
}

// HeroesOfLiberty.Victory.Forum.Hold.Display (ASCII lines): the hold shown on the HUD's line, the block's description and the ring.
void Test_ForumHoldDisplay(FUnitTest& T)
{
	FHOLForumTuningRead Tuning;
	Tuning.Tier = 0;
	Tuning.RuinSteps = 300;
	Tuning.RebuildSteps = 1200;
	Tuning.StepSeconds = 0.1f;
	Tuning.HoldSteps = 600;

	// Standing: nothing of the hold.
	FHOLForumStateRead State;
	T.AssertFalse(HOLForumRulesRead::ShowsHold(State), "standing: no hold shown");
	T.AssertTrue(HOLForumRulesRead::GetRingShown(State, Tuning) == "none", "standing: no ring");
	T.AssertFalse(HOLForumRulesRead::IsRingShown(State, Tuning), "standing: ring hidden");

	// In ruin, no one there, count 0: nothing either.
	HOLForumRulesRead::Fall(State, Tuning);
	State.RuinStepsLeft = 250;
	T.AssertFalse(HOLForumRulesRead::ShowsHold(State), "ruin, no one: no hold shown");
	T.AssertTrue(HOLForumRulesRead::GetRingShown(State, Tuning) == "none", "ruin, no one: no ring");
	FHOLForumBlockRead Quiet = HOLForumRulesRead::GetSelectionBlock(true, State, Tuning, false, 0);
	T.AssertTrue(Quiet.Shown == ", in ruin 25 s, lives 1", "ruin, no one: block unchanged");
	T.AssertTrue(HOLForumRulesRead::DescribeHold(State, Tuning) == "", "ruin, no one: nothing on the HUD line");

	// Held by slot 1 for 35 s: the block's line, the HUD's, the ring at 0.58 in the holder's colour.
	State.Hold = 1;
	State.HoldSteps = 350;
	State.HolderSlot = 1;
	T.AssertTrue(HOLForumRulesRead::ShowsHold(State), "holding: shown");
	T.AssertTrue(HOLForumRulesRead::DescribeHold(State, Tuning) == "hold 35 s holding by slot 1", "holding: HUD line");
	FHOLForumBlockRead Block = HOLForumRulesRead::GetSelectionBlock(true, State, Tuning, false, 0);
	T.AssertTrue(Block.Shown == ", in ruin 25 s, hold 35 s holding by slot 1, lives 1", "holding: block described");
	T.AssertTrue(HOLForumRulesRead::IsRingShown(State, Tuning), "holding: ring shown");
	T.AssertTrue(HOLForumRulesRead::GetRingShown(State, Tuning) == "fill 0.58 holder slot 1", "holding: ring");
	T.AssertEquals(1, HOLForumRulesRead::GetRingHolderSlot(State, Tuning), "holding: holder");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetRingFill(State, Tuning), 350.0f / 600.0f, 0.001f), "holding: the ring's fill is the hold's share");

	// Contested: grey and frozen.
	State.Hold = 2;
	T.AssertTrue(HOLForumRulesRead::DescribeHold(State, Tuning) == "hold 35 s contested by slot 1", "contested: HUD line");
	T.AssertTrue(HOLForumRulesRead::GetRingShown(State, Tuning) == "fill 0.58 grey", "contested: ring");

	// Receding at 20 s, rising again at 40 %: the ring faded in the holder's colour.
	State.Phase = 2;
	State.RuinStepsLeft = 0;
	State.RebuildWork = HOLForumRulesRead::GetRebuildTotalWork(Tuning) * 40 / 100;
	State.Hold = 3;
	State.HoldSteps = 200;
	Block = HOLForumRulesRead::GetSelectionBlock(true, State, Tuning, true, 3);
	T.AssertTrue(Block.Shown == ", rebuilding 40%, builders 3, hold 20 s receding by slot 1, lives 1", "receding: block described");
	T.AssertTrue(HOLForumRulesRead::GetRingShown(State, Tuning) == "fill 0.33 faded slot 1", "receding: ring");

	// Contested at 0 (defenders came first): the ring shown, empty and grey; held with no holder known: grey too.
	State.Hold = 2;
	State.HoldSteps = 0;
	T.AssertTrue(HOLForumRulesRead::GetRingShown(State, Tuning) == "fill 0.00 grey", "contested at 0: ring");
	State.Hold = 1;
	State.HoldSteps = 300;
	State.HolderSlot = -1;
	T.AssertTrue(HOLForumRulesRead::GetRingShown(State, Tuning) == "fill 0.50 grey", "no holder: ring grey");
	T.AssertTrue(HOLForumRulesRead::DescribeHold(State, Tuning) == "hold 30 s holding", "no holder: HUD line");
}

// HeroesOfLiberty.Victory.Forum.Look: what a main forum shows (itself, rubble, a site at its progress).
void Test_ForumLook(FUnitTest& T)
{
	FHOLForumTuningRead Tuning;
	Tuning.RuinSteps = 30;
	Tuning.RebuildSteps = 100;
	FHOLForumStateRead State;
	T.AssertEquals(0, HOLForumRulesRead::GetLook(State, Tuning), "standing: itself");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetLookProgress(State, Tuning), 1.0f, 0.0001f), "standing: whole");
	T.AssertFalse(HOLForumRulesRead::BuildersWait(State), "standing: nobody waits");

	T.AssertEquals(1, HOLForumRulesRead::Fall(State, Tuning), "the fall");
	T.AssertEquals(1, HOLForumRulesRead::GetLook(State, Tuning), "ruin: rubble");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetLookProgress(State, Tuning), 0.0f, 0.0001f), "ruin: nothing risen");
	T.AssertTrue(HOLForumRulesRead::BuildersWait(State), "ruin: its Peasants wait");

	State.Phase = 2;
	State.RebuildWork = HOLForumRulesRead::GetRebuildTotalWork(Tuning) / 4;
	T.AssertEquals(2, HOLForumRulesRead::GetLook(State, Tuning), "rebuilding: a site");
	T.AssertTrue(ForumNear(HOLForumRulesRead::GetLookProgress(State, Tuning), 0.25f, 0.001f), "rebuilding: at its progress");
	T.AssertFalse(HOLForumRulesRead::BuildersWait(State), "rebuilding: its Peasants strike");
}
