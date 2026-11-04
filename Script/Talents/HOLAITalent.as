// The AI's talent choice (T6; GDD 9.6, D424), pure rules, tested in HOLAITalent_Test.as. The AI draws a main family at the start of the
// game among the families whose talents have an effect (HOLHero::TalentHasEffect, the game's own table: in the prototype only the
// Captain's, Boucliers levés and Pas de l'Aube), and spends each point of its hero on it as soon as the point is earned, the rows in
// order (HOLTalent): the main family stays (D424), so the seal and the key talent come. The profile (T6, second half) adds a secondary
// family: with the full game's eight points the maximum build of D103 is five on the main (its key talent) then three on the secondary
// (its seal); the prototype's five points never leave the main. PROVISIONAL (Retours\Q-talents-ia.md): which families the secondary is
// drawn among and when it takes over. What is not here yet: the adaptation to what the AI has scouted of the enemy's seals and keys, with
// a point kept in reserve to answer them (D424; Easy none, Hard good), which needs the scouting the AI does not have yet (Q-talents-ia.md).
// What the AI drew at the start of its game: a main family and another one as the secondary (FamilyNone for either: not drawn).
struct FHOLAIProfile
{
	int Main = -1;
	int Secondary = -1;
}

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

	// The profile for a roll (any integer): the main as DrawMain draws it, the secondary one of the OTHER families, drawn from the roll too
	// (the roll's quotient by the number of mains, so the two draws do not move together). PROVISIONAL, see the top of the file.
	FHOLAIProfile DrawProfile(int Roll)
	{
		FHOLAIProfile Profile;
		Profile.Main = DrawMain(Roll);
		if (Profile.Main == FamilyNone)
			return Profile;
		const int Others = HOLTalent::FamilyCount - 1;
		const int Spread = Math::Max(1, EffectFamilies().Num());
		const int Pick = (((Math::IntegerDivisionTrunc(Roll, Spread)) % Others) + Others) % Others;
		Profile.Secondary = (Profile.Main + 1 + Pick) % HOLTalent::FamilyCount;
		return Profile;
	}

	// The family to take now with a profile: the main until it holds its key talent's worth of talents (five), then the secondary; FamilyNone
	// when nothing can be taken. Without a secondary the main takes everything.
	int ChooseFor(int Level, const TArray<int>& Picks, const FHOLAIProfile& Profile)
	{
		if (Profile.Secondary >= 0 && HOLTalent::CountOf(Picks, Profile.Main) >= HOLTalent::KeyThreshold)
			return Choose(Level, Picks, Profile.Secondary);
		return Choose(Level, Picks, Profile.Main);
	}
}
