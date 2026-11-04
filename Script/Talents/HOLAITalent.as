// The AI's talent choice (T6; GDD 9.6, D424), pure rules, tested in HOLAITalent_Test.as. The AI draws a main family at the start of the
// game among the families whose talents have an effect (HOLHero::TalentHasEffect, the game's own table: in the prototype only the
// Captain's, Boucliers levés and Pas de l'Aube), and spends each point of its hero on it as soon as the point is earned, the rows in
// order (HOLTalent): the main family stays (D424), so the seal and the key talent come. The profile (T6, second half) adds a secondary
// family: with the full game's eight points the maximum build of D103 is five on the main (its key talent) then three on the secondary
// (its seal); the prototype's five points never leave the main (confirmed by the user, 2026-11-04). The adaptation to the seals and keys scouted
// on the adversary (D424, the table of archetypes validated 2026-11-04, D662) is ChooseAdapted below: Easy ignores, Normal gives its next
// point to the family that answers, Hard also keeps one point in reserve (D105). It is behind a switch, DISABLED by default (AdaptEnabled),
// and the AI has no scouting yet: ThinkTalent hands it nothing scouted (HOLAITalent::ScoutedBuilds).
// What the AI drew at the start of its game: a main family and another one as the secondary (FamilyNone for either: not drawn).
struct FHOLAIProfile
{
	int Main = -1;
	int Secondary = -1;
}

// What the AI has scouted of an adversary's talents (the public sheet, D203): his faction (EHOLFaction as an int) and the families holding
// a seal and a key talent (one bit each).
struct FHOLAIScoutedBuild
{
	int Faction = 0;
	int Seals = 0;
	int Keys = 0;
}

// The answer's family and the new memory of the answers given (one bit per scouted adversary and family).
struct FHOLAITalentChoice
{
	int Family = -1;
	int AnsweredMask = 0;
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

	// ---- The adaptation (D424, D662) ----

	// The switch of the adaptation: false until the user has tried it (the console variable hol.AI.TalentAdapt will carry it, one C++ line
	// to add with the next build lot; until then this constant is the setting).
	const bool AdaptEnabled = false;

	// What the AI has scouted of the adversaries' talents: NOTHING yet, the AI has no scouting of the enemy heroes (their public sheets are
	// readable, HOLHero::GetHeroTalentSeals, but the AI may not read what it has not seen). The day it has, this is where it is read.
	TArray<FHOLAIScoutedBuild> ScoutedBuilds(int PlayerIndex)
	{
		TArray<FHOLAIScoutedBuild> Seen;
		return Seen;
	}

	// From this hero level the Hard AI keeps a point in reserve while it has scouted no seal (D424: "after level 5").
	const int HoldFromLevel = 6;

	// The families of the Templars (4); the others have 3: a seal bit above the family count of a faction is no family.
	const int MaxFamilies = 4;

	// The family of the Order of the Dawn that answers a family of an adversary's faction, by ARCHETYPE (the table validated on 2026-11-04):
	// army in mass -> the Captain (1); hero and duel -> the Champion (2); hold, economy and siege -> the Guardian (0). The classification of the
	// 17 families is read from their names (GDD 13.2 to 13.7). FamilyNone for a faction or family that does not exist.
	int AnswerFamily(int Faction, int Family)
	{
		// Per faction (EHOLFaction order: Dawn, Legions, Dragon, Shadow, Fire, Templars), per family column: 0 hold, 1 army, 2 hero.
		TArray<int> Archetypes;
		if (Faction == 0)
		{
			Archetypes.Add(0); Archetypes.Add(1); Archetypes.Add(2);			// Guardian, Captain, Champion
		}
		else if (Faction == 1)
		{
			Archetypes.Add(0); Archetypes.Add(1); Archetypes.Add(2);			// Moisson, Charnier, Effroi
		}
		else if (Faction == 2)
		{
			Archetypes.Add(0); Archetypes.Add(1); Archetypes.Add(0);			// Le Ciel, La Lignee, La Mue
		}
		else if (Faction == 3)
		{
			Archetypes.Add(2); Archetypes.Add(0); Archetypes.Add(0);			// La Lame, Le Miroir, La Toile
		}
		else if (Faction == 4)
		{
			Archetypes.Add(1); Archetypes.Add(0); Archetypes.Add(2);			// Le Marteau, L'Enclume, La Flamme
		}
		else if (Faction == 5)
		{
			Archetypes.Add(0); Archetypes.Add(1); Archetypes.Add(2); Archetypes.Add(0);	// Croisade, Chevalerie, Tresor, Citadelles
		}
		if (Family < 0 || Family >= Archetypes.Num())
			return FamilyNone;
		// Hold -> Guardian 0, army -> Captain 1, hero -> Champion 2: the archetype number is the family's.
		return Archetypes[Family];
	}

	// The first seal or key scouted and not answered yet: its bit in the memory of the answers, -1 when none; Family and the scouted
	// entry's faction through the out parameters.
	int FirstUnanswered(const TArray<FHOLAIScoutedBuild>& Seen, int AnsweredMask, int& OutAnswer)
	{
		for (int Index = 0; Index < Seen.Num(); Index++)
		{
			for (int Family = 0; Family < MaxFamilies; Family++)
			{
				const int Bit = (Index * MaxFamilies) + Family;
				const bool bScouted = (((Seen[Index].Seals | Seen[Index].Keys) >> Family) & 1) != 0;
				const int Answer = AnswerFamily(Seen[Index].Faction, Family);
				if (bScouted && Answer != FamilyNone && ((AnsweredMask >> Bit) & 1) == 0)
				{
					OutAnswer = Answer;
					return Bit;
				}
			}
		}
		return -1;
	}

	// Whether the adversaries scouted show any seal or key at all.
	bool AnySealScouted(const TArray<FHOLAIScoutedBuild>& Seen)
	{
		for (int Index = 0; Index < Seen.Num(); Index++)
		{
			if ((Seen[Index].Seals | Seen[Index].Keys) != 0)
				return true;
		}
		return false;
	}

	// The family to take now, adapted to what has been scouted (bEnabled: the switch; Difficulty as EHOLAIDifficulty: 0 Easy, 1 Normal, 2 Hard):
	// - disabled or Easy: the profile's choice (ChooseFor), nothing answered;
	// - Normal: the next point goes to the family that answers the first seal or key scouted and not answered yet, once (the answer is
	//   remembered in AnsweredMask); with nothing to answer, the profile's choice;
	// - Hard: as Normal, and from level 6 it keeps ONE point in reserve (D105) while it has scouted no seal: it spends one only when it has two.
	FHOLAITalentChoice ChooseAdapted(bool bEnabled, int Difficulty, int Level, const TArray<int>& Picks, const FHOLAIProfile& Profile,
		const TArray<FHOLAIScoutedBuild>& Seen, int AnsweredMask)
	{
		FHOLAITalentChoice Choice;
		Choice.AnsweredMask = AnsweredMask;
		if (!bEnabled || Difficulty < 1)
		{
			Choice.Family = ChooseFor(Level, Picks, Profile);
			return Choice;
		}
		int Answer = FamilyNone;
		const int Bit = FirstUnanswered(Seen, AnsweredMask, Answer);
		if (Bit >= 0 && HOLTalent::CanPick(Level, Picks, Answer))
		{
			Choice.Family = Answer;
			Choice.AnsweredMask = AnsweredMask | (1 << Bit);
			return Choice;
		}
		if (Difficulty >= 2 && Level >= HoldFromLevel && !AnySealScouted(Seen) && HOLTalent::PointsAvailable(Level, Picks) <= 1)
		{
			Choice.Family = FamilyNone;
			return Choice;
		}
		Choice.Family = ChooseFor(Level, Picks, Profile);
		return Choice;
	}
}
