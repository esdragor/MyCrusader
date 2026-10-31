// The AI's talent choice (T6; GDD 9.6, D424): a main family drawn among the families whose talents have an effect in the game, then
// the hero's points spent on it in order. The table of effects (HOLHero::TalentHasEffect), the rows and the server's refusal
// (HOLHero::CanPickTalent) are the game's own; the AI's rule is HOLAITalent (Script/Talents/HOLAITalent.as, pure).
// Families: 0 Guardian, 1 Captain, 2 Champion. In the prototype only the Captain has effects: Boucliers levés (row 2) and Pas de
// l'Aube (row 4).

// The families with an effect somewhere in the rows the prototype reaches, read from the game's table.
void Test_AITalentEffectFamilies(FUnitTest& T)
{
	const TArray<int> Families = HOLAITalent::EffectFamilies();
	T.AssertEquals(1, Families.Num(), "one family has effects in the prototype");
	if (Families.Num() == 1)
	{
		T.AssertEquals(1, Families[0], "the Captain");
	}
	T.AssertTrue(HOLHero::TalentHasEffect(2, 1), "Boucliers levés exists (row 2, Captain)");
	T.AssertTrue(HOLHero::TalentHasEffect(4, 1), "Pas de l'Aube exists (row 4, Captain)");
	T.AssertFalse(HOLHero::TalentHasEffect(2, 0), "no Guardian effect on row 2");
	T.AssertFalse(HOLHero::TalentHasEffect(3, 1), "no effect on row 3");
	T.AssertFalse(HOLHero::TalentHasEffect(1, 1), "no row 1");
	T.AssertFalse(HOLHero::TalentHasEffect(2, 5), "no such family");
}

// The profile is drawn among those families only, whatever the roll (a negative one too).
void Test_AITalentDrawMain(FUnitTest& T)
{
	for (int Roll = -3; Roll < 12; Roll++)
	{
		T.AssertEquals(1, HOLAITalent::DrawMain(Roll), "the main family is the Captain's, roll " + Roll);
	}
}

// No point, no talent; a point and no profile, no talent.
void Test_AITalentNothingToSpend(FUnitTest& T)
{
	TArray<int> Picks;
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::Choose(1, Picks, 1), "level 1: no point");
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::Choose(2, Picks, HOLAITalent::FamilyNone), "no main family: nothing taken");
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::Choose(2, Picks, 7), "a main family that does not exist: nothing taken");
	Picks.Add(1);
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::Choose(2, Picks, 1), "level 2, the point spent already");
}

// A hero who rises to the prototype's maximum level, the AI asking at each level (the server's rule agreeing each time): five Captain
// talents, Boucliers levés on row 2 and Pas de l'Aube on row 4, then the key talent; nothing more at the last level.
void Test_AITalentRisesToTheKey(FUnitTest& T)
{
	const int MaxLevel = HOLHero::GetMaxLevel();
	const int Main = HOLAITalent::DrawMain(0);
	TArray<int> Picks;
	for (int Level = 1; Level <= MaxLevel; Level++)
	{
		const int Family = HOLAITalent::Choose(Level, Picks, Main);
		if (Level == 1)
		{
			T.AssertEquals(HOLAITalent::FamilyNone, Family, "level 1: nothing");
			continue;
		}
		T.AssertEquals(Main, Family, "level " + Level + ": the main family");
		T.AssertTrue(HOLHero::CanPickTalent(Level, Picks.Num(), Family), "the server accepts it at level " + Level);
		T.AssertTrue(HOLTalent::Pick(Level, Picks, Family), "the rules take it");
		T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::Choose(Level, Picks, Main), "the point of level " + Level + " is spent");
	}
	T.AssertEquals(5, Picks.Num(), "five talents in the prototype");
	T.AssertTrue(HOLTalent::HasKey(Picks, Main), "the key talent of the main family");
	T.AssertEquals(1, HOLTalent::SealCount(Picks), "its seal");
	T.AssertTrue(HOLHero::TalentHasEffect(HOLTalent::FirstRow, Picks[0]), "row 2 is Boucliers levés");
	T.AssertTrue(HOLHero::TalentHasEffect(HOLTalent::FirstRow + 2, Picks[2]), "row 4 is Pas de l'Aube");
}

// Points kept in reserve (a hero who levelled up between two thoughts) are spent one per call, the rows in order.
void Test_AITalentCatchesUp(FUnitTest& T)
{
	TArray<int> Picks;
	const int Main = HOLAITalent::DrawMain(4);
	for (int Taken = 0; Taken < 5; Taken++)
	{
		const int Family = HOLAITalent::Choose(6, Picks, Main);
		T.AssertEquals(Main, Family, "pick " + Taken + " at level 6");
		T.AssertTrue(HOLTalent::Pick(6, Picks, Family), "taken");
	}
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::Choose(6, Picks, Main), "all five spent");
}
