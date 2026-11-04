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

// The second half of the profile (D424): a main and a secondary family, never the same, whatever the roll (a negative one too); the main is
// still the one DrawMain draws among the families with effects.
void Test_AITalentProfileHasTwoDifferentFamilies(FUnitTest& T)
{
	for (int Roll = -7; Roll < 25; Roll++)
	{
		const FHOLAIProfile Profile = HOLAITalent::DrawProfile(Roll);
		T.AssertEquals(HOLAITalent::DrawMain(Roll), Profile.Main, "the main family, roll " + Roll);
		T.AssertTrue(Profile.Secondary >= 0 && Profile.Secondary < HOLTalent::FamilyCount, "the secondary family exists, roll " + Roll);
		T.AssertTrue(Profile.Secondary != Profile.Main, "and is another one, roll " + Roll);
	}
	// Both other families come up as the secondary across the rolls.
	bool bSeen0 = false;
	bool bSeen2 = false;
	for (int Roll = 0; Roll < 8; Roll++)
	{
		const int Secondary = HOLAITalent::DrawProfile(Roll).Secondary;
		bSeen0 = bSeen0 || Secondary == 0;
		bSeen2 = bSeen2 || Secondary == 2;
	}
	T.AssertTrue(bSeen0 && bSeen2, "the Guardian and the Champion both come up as the secondary with the Captain as the main");
}

// The profile spends its eight points as the maximum build of D103: five on the main family (its key talent), then the three others on the
// secondary (its seal); the server's rule agreeing each time. In the prototype (five points) only the main is ever reached.
void Test_AITalentProfileSpendsFiveThenThree(FUnitTest& T)
{
	const FHOLAIProfile Profile = HOLAITalent::DrawProfile(0);
	TArray<int> Picks;
	for (int Level = 2; Level <= 9; Level++)
	{
		const int Family = HOLAITalent::ChooseFor(Level, Picks, Profile);
		const int Expected = Picks.Num() < HOLTalent::KeyThreshold ? Profile.Main : Profile.Secondary;
		T.AssertEquals(Expected, Family, "level " + Level + ": the profile's family");
		T.AssertTrue(HOLTalent::Pick(Level, Picks, Family), "taken at level " + Level);
	}
	T.AssertEquals(8, Picks.Num(), "eight talents");
	T.AssertTrue(HOLTalent::HasKey(Picks, Profile.Main), "the main family's key talent");
	T.AssertEquals(3, HOLTalent::CountOf(Picks, Profile.Secondary), "three on the secondary");
	T.AssertTrue(HOLTalent::HasSeal(Picks, Profile.Secondary), "its seal");
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::ChooseFor(9, Picks, Profile), "nothing left to take");
}

// In the prototype's five points the profile does what Choose did: the main family only, so the key talent comes.
void Test_AITalentProfileInThePrototypeIsTheMainAlone(FUnitTest& T)
{
	const FHOLAIProfile Profile = HOLAITalent::DrawProfile(3);
	TArray<int> Picks;
	for (int Level = 2; Level <= HOLHero::GetMaxLevel(); Level++)
	{
		T.AssertEquals(HOLAITalent::Choose(Level, Picks, Profile.Main), HOLAITalent::ChooseFor(Level, Picks, Profile), "level " + Level + ": same as the main-only rule");
		T.AssertTrue(HOLTalent::Pick(Level, Picks, HOLAITalent::ChooseFor(Level, Picks, Profile)), "taken");
	}
	T.AssertEquals(HOLTalent::PointsEarned(HOLHero::GetMaxLevel()), HOLTalent::CountOf(Picks, Profile.Main), "all on the main family");
	T.AssertTrue(HOLTalent::HasKey(Picks, Profile.Main), "its key talent");
}

// No point or no profile: nothing; a profile without a secondary spends everything on the main.
void Test_AITalentProfileEdges(FUnitTest& T)
{
	TArray<int> Picks;
	FHOLAIProfile None;
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::ChooseFor(9, Picks, None), "an empty profile takes nothing");
	FHOLAIProfile Alone;
	Alone.Main = 1;
	for (int Level = 2; Level <= 9; Level++)
	{
		const int Family = HOLAITalent::ChooseFor(Level, Picks, Alone);
		T.AssertEquals(1, Family, "no secondary: the main, level " + Level);
		HOLTalent::Pick(Level, Picks, Family);
	}
	TArray<int> Spent;
	Spent.Add(1);
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::ChooseFor(2, Spent, Alone), "level 2 with its point spent: nothing");
}

// ---- The adaptation to the seals and keys scouted on the adversary (D424; the user's answers of 2026-11-04: Q-talents-contres Q1 = A, Q2 = A) ----
// Factions as EHOLFaction: 0 Dawn, 1 Legions, 2 Dragon, 3 Shadow, 4 Fire, 5 Templars. Difficulty as EHOLAIDifficulty: 0 Easy, 1 Normal, 2 Hard.

FHOLAIScoutedBuild AITalentScouted(int Faction, int Seals, int Keys = 0)
{
	FHOLAIScoutedBuild Build;
	Build.Faction = Faction;
	Build.Seals = Seals;
	Build.Keys = Keys;
	return Build;
}

TArray<int> AITalentSpent(int Count, int Family = 1)
{
	TArray<int> Picks;
	for (int Index = 0; Index < Count; Index++)
		Picks.Add(Family);
	return Picks;
}

FHOLAIProfile AITalentProfile(int Main, int Secondary)
{
	FHOLAIProfile Profile;
	Profile.Main = Main;
	Profile.Secondary = Secondary;
	return Profile;
}

// The table of the archetypes: army (answered by the Captain, 1), hero and duel (the Champion, 2), hold, economy and siege (the Guardian, 0), family by family.
void Test_AITalentArchetypesTable(FUnitTest& T)
{
	for (int Faction = 0; Faction < 2; Faction++)
	{
		T.AssertEquals(0, HOLAITalent::AnswerFamily(Faction, 0), f"faction {Faction}, first family: the Guardian answers");
		T.AssertEquals(1, HOLAITalent::AnswerFamily(Faction, 1), f"faction {Faction}, second family: the Captain answers");
		T.AssertEquals(2, HOLAITalent::AnswerFamily(Faction, 2), f"faction {Faction}, third family: the Champion answers");
	}
	T.AssertEquals(0, HOLAITalent::AnswerFamily(2, 0), "Dragon, Le Ciel");
	T.AssertEquals(1, HOLAITalent::AnswerFamily(2, 1), "Dragon, La Lignee");
	T.AssertEquals(0, HOLAITalent::AnswerFamily(2, 2), "Dragon, La Mue");
	T.AssertEquals(2, HOLAITalent::AnswerFamily(3, 0), "Shadow, La Lame");
	T.AssertEquals(0, HOLAITalent::AnswerFamily(3, 1), "Shadow, Le Miroir");
	T.AssertEquals(0, HOLAITalent::AnswerFamily(3, 2), "Shadow, La Toile");
	T.AssertEquals(1, HOLAITalent::AnswerFamily(4, 0), "Fire, Le Marteau");
	T.AssertEquals(0, HOLAITalent::AnswerFamily(4, 1), "Fire, L'Enclume");
	T.AssertEquals(2, HOLAITalent::AnswerFamily(4, 2), "Fire, La Flamme");
	T.AssertEquals(0, HOLAITalent::AnswerFamily(5, 0), "Templars, Croisade");
	T.AssertEquals(1, HOLAITalent::AnswerFamily(5, 1), "Templars, Chevalerie");
	T.AssertEquals(2, HOLAITalent::AnswerFamily(5, 2), "Templars, Tresor");
	T.AssertEquals(0, HOLAITalent::AnswerFamily(5, 3), "Templars, Citadelles");
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::AnswerFamily(9, 0), "no such faction");
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::AnswerFamily(0, 7), "no such family");
}

// Disabled (the default) or Easy: the profile's choice, whatever has been scouted.
void Test_AITalentAdaptationOffOrEasyChangesNothing(FUnitTest& T)
{
	const FHOLAIProfile Profile = AITalentProfile(1, 0);
	const TArray<int> Picks = AITalentSpent(1);
	TArray<FHOLAIScoutedBuild> Seen;
	Seen.Add(AITalentScouted(0, 4));
	const FHOLAITalentChoice Off = HOLAITalent::ChooseAdapted(false, 2, 3, Picks, Profile, Seen, 0);
	T.AssertEquals(HOLAITalent::ChooseFor(3, Picks, Profile), Off.Family, "disabled: the profile's choice");
	T.AssertEquals(0, Off.AnsweredMask, "disabled: nothing answered");
	const FHOLAITalentChoice Easy = HOLAITalent::ChooseAdapted(true, 0, 3, Picks, Profile, Seen, 0);
	T.AssertEquals(HOLAITalent::ChooseFor(3, Picks, Profile), Easy.Family, "Easy ignores what it scouted");
	T.AssertEquals(0, Easy.AnsweredMask, "Easy answers nothing");
}

// Normal: the next point goes to the family that answers the seal scouted, ONCE; the answer is remembered and the profile resumes. A key talent counts as a seal.
void Test_AITalentNormalAnswersAScoutedSealOnce(FUnitTest& T)
{
	const FHOLAIProfile Profile = AITalentProfile(1, 0);
	TArray<int> Picks = AITalentSpent(1);
	TArray<FHOLAIScoutedBuild> Seen;
	Seen.Add(AITalentScouted(0, 4));
	const FHOLAITalentChoice First = HOLAITalent::ChooseAdapted(true, 1, 3, Picks, Profile, Seen, 0);
	T.AssertEquals(2, First.Family, "the Champion answers the Champion's seal");
	T.AssertTrue(First.AnsweredMask != 0, "the answer is remembered");
	T.AssertTrue(HOLTalent::Pick(3, Picks, First.Family), "taken");
	const FHOLAITalentChoice Second = HOLAITalent::ChooseAdapted(true, 1, 4, Picks, Profile, Seen, First.AnsweredMask);
	T.AssertEquals(1, Second.Family, "answered already: the profile resumes");
	T.AssertEquals(First.AnsweredMask, Second.AnsweredMask, "nothing new answered");
	Seen[0] = AITalentScouted(0, 4 | 1);
	T.AssertEquals(0, HOLAITalent::ChooseAdapted(true, 1, 4, Picks, Profile, Seen, First.AnsweredMask).Family, "a second seal, a second answer: the Guardian's hold archetype");
	TArray<FHOLAIScoutedBuild> KeyOnly;
	KeyOnly.Add(AITalentScouted(0, 0, 4));
	T.AssertEquals(2, HOLAITalent::ChooseAdapted(true, 1, 2, AITalentSpent(0), AITalentProfile(0, 1), KeyOnly, 0).Family, "a scouted key talent is answered like a seal (the Champion's)");
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::ChooseAdapted(true, 1, 2, Picks, Profile, Seen, 0).Family, "no point in reserve: nothing");
	T.AssertEquals(0, HOLAITalent::ChooseAdapted(true, 1, 2, Picks, Profile, Seen, 0).AnsweredMask, "an answer not taken is not remembered");
}

// Hard: as Normal, and from level 6 it keeps ONE point in reserve while no seal of the adversary is scouted (D105); with two points it spends one; a seal
// scouted releases it; below level 6 it spends as usual.
void Test_AITalentHardKeepsOnePointInReserve(FUnitTest& T)
{
	const FHOLAIProfile Profile = AITalentProfile(1, 0);
	const TArray<FHOLAIScoutedBuild> Nothing;
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::ChooseAdapted(true, 2, 6, AITalentSpent(4), Profile, Nothing, 0).Family, "level 6, one point, nothing scouted: kept in reserve");
	T.AssertEquals(1, HOLAITalent::ChooseAdapted(true, 1, 6, AITalentSpent(4), Profile, Nothing, 0).Family, "Normal spends it");
	T.AssertEquals(1, HOLAITalent::ChooseAdapted(true, 2, 5, AITalentSpent(3), Profile, Nothing, 0).Family, "level 5 (before 6): spent as usual");
	T.AssertEquals(1, HOLAITalent::ChooseAdapted(true, 2, 7, AITalentSpent(4), Profile, Nothing, 0).Family, "two points in reserve: one spent, the other kept");
	T.AssertEquals(HOLAITalent::FamilyNone, HOLAITalent::ChooseAdapted(true, 2, 7, AITalentSpent(5), Profile, Nothing, 0).Family, "then one point: kept again, never more than one");
	TArray<FHOLAIScoutedBuild> Seen;
	Seen.Add(AITalentScouted(1, 4));
	const FHOLAITalentChoice Released = HOLAITalent::ChooseAdapted(true, 2, 6, AITalentSpent(4), Profile, Seen, 0);
	T.AssertEquals(2, Released.Family, "a seal scouted: the point goes to its answer");
	T.AssertEquals(1, HOLAITalent::ChooseAdapted(true, 2, 6, AITalentSpent(4), Profile, Seen, Released.AnsweredMask).Family, "answered: no more reason to hold, it spends as usual");
}
