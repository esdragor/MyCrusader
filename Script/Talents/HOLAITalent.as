// The AI's talent choice (T6; GDD 9.6, D424), pure rules, tested in HOLAITalent_Test.as. The AI draws a main family at the start of the
// game among the families whose talents have an effect (HOLHero::TalentHasEffect, the game's own table: in the prototype only the
// Captain's, Boucliers levés and Pas de l'Aube), and spends each point of its hero on it as soon as the point is earned, the rows in
// order (HOLTalent): the main family stays (D424), so the seal and the key talent come. What is not here yet: the secondary family and
// the adaptation to what the AI has scouted of the enemy's seals and keys, with a point kept in reserve to answer them (D424; Easy
// none, Hard good), which need the scouting the AI does not have yet; the Easy level then takes the same rule.
namespace HOLAITalent
{
	// No family drawn, or nothing to take.
	const int FamilyNone = -1;

	// The families having an effect in some row of the prototype (rows 2 to the one the maximum level opens), in family order.
	TArray<int> EffectFamilies()
	{
		TArray<int> Families;
		const int Rows = HOLTalent::PointsEarned(HOLHero::GetMaxLevel());
		for (int Family = 0; Family < HOLTalent::FamilyCount; Family++)
		{
			for (int Index = 0; Index < Rows; Index++)
			{
				if (HOLHero::TalentHasEffect(HOLTalent::FirstRow + Index, Family))
				{
					Families.Add(Family);
					break;
				}
			}
		}
		return Families;
	}

	// The main family for a roll (any integer): one of EffectFamilies, FamilyNone when none has an effect.
	int DrawMain(int Roll)
	{
		const TArray<int> Families = EffectFamilies();
		if (Families.Num() == 0)
			return FamilyNone;
		const int Count = Families.Num();
		return Families[((Roll % Count) + Count) % Count];
	}

	// The family to take now for a hero of Level who took Picks: the main family while a point is in reserve and a row is left,
	// FamilyNone otherwise (or without a main family).
	int Choose(int Level, const TArray<int>& Picks, int Main)
	{
		if (!HOLTalent::CanPick(Level, Picks, Main))
			return FamilyNone;
		return Main;
	}
}
