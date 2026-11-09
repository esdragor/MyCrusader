// The morale display rule (M6, HOLMoraleBlock.as; MoraleInProgress.md D707): what a unit shows of its effects. Every state is built on the real
// chain: HOLMorale::Apply of the rows of the real table (HOLMoraleTable::Effect), HOLMorale::Tick, GrantImmunity, RemoveMalus; no number by hand.

HOLMorale::FHOLMoraleState MoraleBlockPose(const HOLMorale::FHOLMoraleState& Before, int Id)
{
	HOLMorale::FHOLMoraleState State = Before;
	const HOLMorale::FHOLMoraleEffect Effect = HOLMoraleTable::Effect(Id);
	HOLMorale::Apply(State, Effect);
	return State;
}

// A unit with no effect shows nothing: no pastille, no line.
void Test_MoraleBlockNothingWhenNoEffect(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	const HOLMoraleBlock::FHOLMoraleBadge Badge = HOLMoraleBlock::Badge(State, false);
	T.AssertFalse(Badge.bBonus, "no bonus pastille without an effect");
	T.AssertFalse(Badge.bMalus, "no malus pastille without an effect");
	T.AssertFalse(Badge.bImmune, "no immunity pastille without an immunity");
	T.AssertEquals(0, HOLMoraleBlock::Lines(State, true, false).Num(), "no line without an effect");
}

// A bonus alone shows the green pastille only; a bonus and a malus together show both.
void Test_MoraleBlockBonusAndMalusShowBoth(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState Bonus = MoraleBlockPose(HOLMorale::FHOLMoraleState(), HOLMoraleTable::CloseRanks);
	const HOLMoraleBlock::FHOLMoraleBadge One = HOLMoraleBlock::Badge(Bonus, false);
	T.AssertTrue(One.bBonus, "the cry gives the green pastille");
	T.AssertFalse(One.bMalus, "and no red one");
	HOLMorale::FHOLMoraleState Both = MoraleBlockPose(Bonus, HOLMoraleTable::Demoralised);
	const HOLMoraleBlock::FHOLMoraleBadge Two = HOLMoraleBlock::Badge(Both, false);
	T.AssertTrue(Two.bBonus, "a bonus and a malus together: the green pastille stays");
	T.AssertTrue(Two.bMalus, "and the red one comes");
	HOLMorale::FHOLMoraleState MalusOnly = MoraleBlockPose(HOLMorale::FHOLMoraleState(), HOLMoraleTable::Hesitation);
	const HOLMoraleBlock::FHOLMoraleBadge Three = HOLMoraleBlock::Badge(MalusOnly, false);
	T.AssertFalse(Three.bBonus, "a malus alone: no green pastille");
	T.AssertTrue(Three.bMalus, "the red pastille");
}

// The list is sorted bonus first, then malus, whatever the order they were posed in.
void Test_MoraleBlockListIsBonusThenMalus(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State = MoraleBlockPose(HOLMorale::FHOLMoraleState(), HOLMoraleTable::Demoralised);
	State = MoraleBlockPose(State, HOLMoraleTable::Triumph);
	State = MoraleBlockPose(State, HOLMoraleTable::Hesitation);
	const TArray<HOLMoraleBlock::FHOLMoraleLine> Lines = HOLMoraleBlock::Lines(State, true, false);
	T.AssertEquals(3, Lines.Num(), "three effects, three lines");
	if (Lines.Num() < 3)
		return;
	T.AssertEquals(HOLMoraleTable::Triumph, Lines[0].Id, "the bonus comes first although it was posed second");
	T.AssertFalse(Lines[0].bMalus, "the first line is a bonus");
	T.AssertEquals(HOLMoraleTable::Demoralised, Lines[1].Id, "then the malus in the order they were posed");
	T.AssertEquals(HOLMoraleTable::Hesitation, Lines[2].Id, "then the next malus");
	T.AssertTrue(Lines[1].bMalus && Lines[2].bMalus, "the last two lines are malus");
}

// Our own unit shows the time left of a temporary effect (rounded up to the second, following the real tick); an aura shows none.
void Test_MoraleBlockOwnUnitShowsSecondsLeft(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State = MoraleBlockPose(HOLMorale::FHOLMoraleState(), HOLMoraleTable::Triumph);
	State = MoraleBlockPose(State, HOLMoraleTable::DawnAura);
	const int FullMs = HOLMoraleTable::Effect(HOLMoraleTable::Triumph).DurationMs;
	TArray<HOLMoraleBlock::FHOLMoraleLine> Lines = HOLMoraleBlock::Lines(State, true, false);
	T.AssertEquals(2, Lines.Num(), "the triumph and the aura");
	if (Lines.Num() < 2)
		return;
	T.AssertEquals(HOLMoraleTable::Triumph, Lines[0].Id, "temporary effect first (both are bonus, posed order)");
	T.AssertEquals((FullMs + 999) / 1000, Lines[0].SecondsLeft, "the full time to start with");
	T.AssertTrue(Lines[1].bAura, "the aura line says it is an aura");
	T.AssertEquals(-1, Lines[1].SecondsLeft, "an aura has no time to show");
	HOLMorale::Tick(State, 12000);
	Lines = HOLMoraleBlock::Lines(State, true, false);
	T.AssertEquals((FullMs - 12000 + 999) / 1000, Lines[0].SecondsLeft, "12 s later, 12 s less");
	HOLMorale::Tick(State, 500);
	Lines = HOLMoraleBlock::Lines(State, true, false);
	T.AssertEquals((FullMs - 12500 + 999) / 1000, Lines[0].SecondsLeft, "a started second still counts");
}

// A visible enemy unit shows its effects but never a time.
void Test_MoraleBlockEnemyShowsNoTime(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State = MoraleBlockPose(HOLMorale::FHOLMoraleState(), HOLMoraleTable::Demoralised);
	const TArray<HOLMoraleBlock::FHOLMoraleLine> Lines = HOLMoraleBlock::Lines(State, false, false);
	T.AssertEquals(1, Lines.Num(), "the enemy's effect is listed");
	if (Lines.Num() < 1)
		return;
	T.AssertEquals(HOLMoraleTable::Demoralised, Lines[0].Id, "it is the right effect");
	T.AssertEquals(-1, Lines[0].SecondsLeft, "no time for an enemy unit");
	T.AssertTrue(HOLMoraleBlock::Badge(State, false).bMalus, "its pastille is there");
}

// A Peasant never shows anything, whatever its state holds.
void Test_MoraleBlockPeasantShowsNothing(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State = MoraleBlockPose(HOLMorale::FHOLMoraleState(), HOLMoraleTable::Triumph);
	State = MoraleBlockPose(State, HOLMoraleTable::Demoralised);
	HOLMorale::GrantImmunity(State, 5000);
	const HOLMoraleBlock::FHOLMoraleBadge Badge = HOLMoraleBlock::Badge(State, true);
	T.AssertFalse(Badge.bBonus || Badge.bMalus || Badge.bImmune, "a Peasant has no pastille");
	T.AssertEquals(0, HOLMoraleBlock::Lines(State, true, true).Num(), "a Peasant has no line");
	T.AssertEquals(-1, HOLMoraleBlock::ImmunitySeconds(State, true, true), "a Peasant shows no immunity time");
}

// The immunity shows the gold pastille (and its time on our own unit only); a malus cannot be posed during it, a bonus can.
void Test_MoraleBlockImmunityShowsGold(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	HOLMorale::GrantImmunity(State, 5000);
	const int Refused = HOLMorale::Apply(State, HOLMoraleTable::Effect(HOLMoraleTable::Demoralised));
	T.AssertEquals(HOLMorale::ApplyBlockedByImmunity, Refused, "the real chain refuses the malus during the immunity");
	HOLMorale::Apply(State, HOLMoraleTable::Effect(HOLMoraleTable::Triumph));
	const HOLMoraleBlock::FHOLMoraleBadge Badge = HOLMoraleBlock::Badge(State, false);
	T.AssertTrue(Badge.bImmune, "the gold pastille");
	T.AssertTrue(Badge.bBonus, "the bonus is posed and shown");
	T.AssertFalse(Badge.bMalus, "no red pastille: the malus was refused");
	T.AssertEquals(5, HOLMoraleBlock::ImmunitySeconds(State, true, false), "our unit shows 5 s of immunity");
	T.AssertEquals(-1, HOLMoraleBlock::ImmunitySeconds(State, false, false), "an enemy unit shows no time");
	HOLMorale::Tick(State, 5000);
	T.AssertFalse(HOLMoraleBlock::Badge(State, false).bImmune, "the gold pastille goes with the immunity");
}

// Lumiere sacree takes the malus and leaves the bonus: the red pastille goes, the green stays; an expired effect leaves the list.
void Test_MoraleBlockFollowsRemovalAndExpiry(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State = MoraleBlockPose(HOLMorale::FHOLMoraleState(), HOLMoraleTable::Triumph);
	State = MoraleBlockPose(State, HOLMoraleTable::Demoralised);
	HOLMorale::RemoveMalus(State);
	HOLMoraleBlock::FHOLMoraleBadge Badge = HOLMoraleBlock::Badge(State, false);
	T.AssertFalse(Badge.bMalus, "the red pastille goes with the malus");
	T.AssertTrue(Badge.bBonus, "the green one stays");
	T.AssertEquals(1, HOLMoraleBlock::Lines(State, true, false).Num(), "only the bonus is listed");
	HOLMorale::Tick(State, HOLMoraleTable::Effect(HOLMoraleTable::Triumph).DurationMs);
	T.AssertFalse(HOLMoraleBlock::Badge(State, false).bBonus, "an expired effect shows no pastille");
	T.AssertEquals(0, HOLMoraleBlock::Lines(State, true, false).Num(), "and no line");
}
