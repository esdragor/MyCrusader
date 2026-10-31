// The AI's duel decisions, alone (DuelInProgress.md D9; GDD 10.3, D422, D423). Every expectation compares what the rules
// answer for two views, or reads the preset: no number of the rules is typed in.

FHOLAIDuelView DuelEven()
{
	FHOLAIDuelView V;
	V.MyLevel = 4;
	V.EnemyLevel = 4;
	return V;
}

// Equal heroes: even odds; a higher level, more health or a ready kit raise them; the two sides' odds add up to the whole.
void Test_AIDuelOddsFollowTheHeroes(FUnitTest& T)
{
	FHOLAIDuelView V = DuelEven();
	const int Even = HOLAIDuel::Odds(V);
	T.AssertEquals(50, Even, "equal heroes: even");
	FHOLAIDuelView Higher = V;
	Higher.MyLevel = V.MyLevel + 2;
	T.AssertTrue(HOLAIDuel::Odds(Higher) > Even, "a higher level raises the odds");
	FHOLAIDuelView Hurt = V;
	Hurt.MyHealthPercent = 40;
	T.AssertTrue(HOLAIDuel::Odds(Hurt) < Even, "less health lowers them");
	FHOLAIDuelView Tired = V;
	Tired.MyKitReadyPercent = 0;
	T.AssertTrue(HOLAIDuel::Odds(Tired) < Even, "a kit on recharge lowers them");
	FHOLAIDuelView Mirror = Higher;
	Mirror.MyLevel = Higher.EnemyLevel;
	Mirror.EnemyLevel = Higher.MyLevel;
	T.AssertEquals(100, HOLAIDuel::Odds(Higher) + HOLAIDuel::Odds(Mirror), "the two sides add up to 100");
	FHOLAIDuelView Dead = V;
	Dead.MyHealthPercent = 0;
	T.AssertEquals(0, HOLAIDuel::Odds(Dead), "no health, no chance");
}

// The estimate is the truth moved by the level's error; Easy errs more than Hard; always within 0..100.
void Test_AIDuelEstimateErrsByLevel(FUnitTest& T)
{
	FHOLAIDuelView V = DuelEven();
	FHOLAIDuelSettings Easy = HOLAIDuel::Preset(EHOLAIDifficulty::Easy);
	FHOLAIDuelSettings Hard = HOLAIDuel::Preset(EHOLAIDifficulty::Hard);
	T.AssertEquals(HOLAIDuel::Odds(V), HOLAIDuel::EstimatedOdds(Hard, V, 0), "no noise: the truth");
	T.AssertTrue(HOLAIDuel::EstimatedOdds(Easy, V, 100) > HOLAIDuel::EstimatedOdds(Hard, V, 100), "Easy overestimates further than Hard");
	T.AssertTrue(HOLAIDuel::EstimatedOdds(Easy, V, -100) < HOLAIDuel::EstimatedOdds(Hard, V, -100), "Easy underestimates further than Hard");
	FHOLAIDuelView Sure = V;
	Sure.EnemyHealthPercent = 0;
	T.AssertEquals(100, HOLAIDuel::EstimatedOdds(Easy, Sure, 100), "never above 100");
	FHOLAIDuelView Lost = V;
	Lost.MyHealthPercent = 0;
	T.AssertEquals(0, HOLAIDuel::EstimatedOdds(Easy, Lost, -100), "never below 0");
}

// The moment weighs the answer: the same odds are refused at peace and accepted when a refusal would fall in a battle.
void Test_AIDuelMomentWeighsTheAnswer(FUnitTest& T)
{
	FHOLAIDuelSettings S = HOLAIDuel::Preset(EHOLAIDifficulty::Normal);
	FHOLAIDuelView Peace = DuelEven();
	Peace.MyLevel = 3; // a little behind: the odds sit between the floor and the base
	FHOLAIDuelView Battle = Peace;
	Battle.bBattleNear = true;
	Battle.TroopsNearMe = 20;
	T.AssertTrue(HOLAIDuel::Pressure(S, Battle) > HOLAIDuel::Pressure(S, Peace), "a battle near lowers the bar");
	T.AssertEquals(0, HOLAIDuel::Pressure(S, Peace), "no pressure at peace");
	FHOLAIDuelDecision AtPeace = HOLAIDuel::Answer(S, Peace, 0);
	FHOLAIDuelDecision InBattle = HOLAIDuel::Answer(S, Battle, 0);
	T.AssertEquals(AtPeace.Odds, InBattle.Odds, "the odds are the same");
	T.AssertTrue(InBattle.Threshold < AtPeace.Threshold, "the bar is lower in a battle");
	T.AssertFalse(AtPeace.bYes, "a slightly worse duel is refused at peace");
	T.AssertTrue(InBattle.bYes, "the same duel is accepted when a refusal would fall in a battle");
	// The Dawn loses Honour on a refusal: it accepts a little more readily.
	FHOLAIDuelView Dawn = Peace;
	Dawn.bDawn = true;
	T.AssertTrue(HOLAIDuel::Pressure(S, Dawn) > HOLAIDuel::Pressure(S, Peace), "the Dawn's Honour lowers the bar");
	// Troops near: capped.
	FHOLAIDuelView Crowd = Battle;
	Crowd.TroopsNearMe = 1000;
	FHOLAIDuelView Crowd2 = Battle;
	Crowd2.TroopsNearMe = 2000;
	T.AssertEquals(HOLAIDuel::Pressure(S, Crowd), HOLAIDuel::Pressure(S, Crowd2), "the troops' share is capped");
}

// A duel one is sure to lose is refused even under the greatest pressure; a sure win is accepted at peace.
void Test_AIDuelFloorAndSureWin(FUnitTest& T)
{
	FHOLAIDuelSettings S = HOLAIDuel::Preset(EHOLAIDifficulty::Hard);
	FHOLAIDuelView Doomed = DuelEven();
	Doomed.MyHealthPercent = 10;
	Doomed.bBattleNear = true;
	Doomed.bDawn = true;
	Doomed.TroopsNearMe = 1000;
	FHOLAIDuelDecision No = HOLAIDuel::Answer(S, Doomed, 0);
	T.AssertFalse(No.bYes, "refused");
	T.AssertTrue(No.Threshold >= S.MinAcceptPercent, "the bar never falls under the floor");
	FHOLAIDuelView Strong = DuelEven();
	Strong.MyLevel = 8;
	Strong.EnemyHealthPercent = 50;
	T.AssertTrue(HOLAIDuel::Answer(S, Strong, 0).bYes, "a sure win is accepted at peace");
}

// A human's reaction: inside the window of the duel (or the silence would be a refusal); Easy slower than Hard.
void Test_AIDuelReactsLikeAHuman(FUnitTest& T)
{
	FHOLDuelSettings Duel = HOLDuel::Preset();
	FHOLAIDuelSettings Easy = HOLAIDuel::Preset(EHOLAIDifficulty::Easy);
	FHOLAIDuelSettings Normal = HOLAIDuel::Preset(EHOLAIDifficulty::Normal);
	FHOLAIDuelSettings Hard = HOLAIDuel::Preset(EHOLAIDifficulty::Hard);
	T.AssertTrue(Hard.ReactionMs > 0, "never instant");
	T.AssertTrue(Easy.ReactionMs > Normal.ReactionMs && Normal.ReactionMs > Hard.ReactionMs, "the lower the level, the slower");
	T.AssertTrue(Easy.ReactionMs < Duel.ResponseMs, "even Easy answers inside the window");
	T.AssertEquals(Normal.ReactionMs, HOLAIDuel::Answer(Normal, DuelEven(), 0).ReactionMs, "the answer carries the reaction time");
}

// To challenge: only when allowed, only around a fight, only with the odds; and as a protection when the hero is in danger.
void Test_AIDuelChallenges(FUnitTest& T)
{
	FHOLAIDuelSettings S = HOLAIDuel::Preset(EHOLAIDifficulty::Normal);
	FHOLAIDuelView Strong = DuelEven();
	Strong.MyLevel = 9;
	Strong.EnemyHealthPercent = 50;
	Strong.bBattleNear = true;
	T.AssertTrue(HOLAIDuel::Challenge(S, Strong, true, 0).bYes, "a strong hero challenges in a battle");
	T.AssertFalse(HOLAIDuel::Challenge(S, Strong, false, 0).bYes, "not when the challenge is not allowed");
	FHOLAIDuelView Peaceful = Strong;
	Peaceful.bBattleNear = false;
	T.AssertFalse(HOLAIDuel::Challenge(S, Peaceful, true, 0).bYes, "not at peace: it challenges just before or during a fight");
	FHOLAIDuelView Even = DuelEven();
	Even.bBattleNear = true;
	T.AssertFalse(HOLAIDuel::Challenge(S, Even, true, 0).bYes, "an even duel is not sought");
	FHOLAIDuelView Danger = Even;
	Danger.bMyHeroInDanger = true;
	FHOLAIDuelDecision Protect = HOLAIDuel::Challenge(S, Danger, true, 0);
	T.AssertTrue(Protect.bYes, "in danger, an even duel is worth it as a protection");
	FHOLAIDuelView Hopeless = Danger;
	Hopeless.MyHealthPercent = 5;
	T.AssertFalse(HOLAIDuel::Challenge(S, Hopeless, true, 0).bYes, "but not a duel it would surely lose");
}

// The journal says the decision and the numbers behind it.
void Test_AIDuelJournalLine(FUnitTest& T)
{
	FHOLAIDuelSettings S = HOLAIDuel::Preset(EHOLAIDifficulty::Normal);
	FHOLAIDuelView V = DuelEven();
	V.bBattleNear = true;
	FHOLAIDuelDecision D = HOLAIDuel::Answer(S, V, 0);
	FString Line = HOLAIDuel::Describe("answer", D);
	T.AssertTrue(Line.Contains("answer"), "says what was asked");
	T.AssertTrue(Line.Contains(D.bYes ? "accept" : "refuse"), "says the decision");
	T.AssertTrue(Line.Contains("odds " + D.Odds), "says the odds");
	T.AssertTrue(Line.Contains("bar " + D.Threshold), "says the bar");
	T.AssertTrue(D.Reason.Len() > 0, "gives a reason");
}