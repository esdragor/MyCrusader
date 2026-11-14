// Garrison, pure rule (F7, FortificationsInProgress.md; D693 garrison, D374 never in an ally's building). Who may enter a building and how
// many: a unit enters only a building of its OWN slot (an ally's and an enemy's are refused), only if its kind may garrison, and only up to the
// free places (places of the building type minus its occupants), in the order asked. No world access, tested in
// Script\Tests\HOLGarrisonRules_Test.as. Not here: the order, the hidden state of an occupant, the ejection, the network, the interface.
// The values are the defaults of Retours\Q-garrison.md (Q1 A, Q2 A, NOT answered yet), one setting each; the places move to a BuildingTypes.csv
// column once the user answers. The tower names are the ones FortificationsInProgress.md plans (TowerWood F8, TowerStone F10,
// TowerReinforced F12): no such row exists yet.

const FConsoleVariable CVar_GarrisonSlotsWatchTower("hol.Garrison.SlotsWatchTower", 3);
const FConsoleVariable CVar_GarrisonSlotsStoneTower("hol.Garrison.SlotsStoneTower", 5);
const FConsoleVariable CVar_GarrisonSlotsReinforcedTower("hol.Garrison.SlotsReinforcedTower", 8);
const FConsoleVariable CVar_GarrisonOccupantsProtected("hol.Garrison.OccupantsProtected", 1);
const FConsoleVariable CVar_GarrisonMeleeInfantry("hol.Garrison.MeleeInfantry", 1);
const FConsoleVariable CVar_GarrisonHero("hol.Garrison.Hero", 1);
const FConsoleVariable CVar_GarrisonCavalry("hol.Garrison.Cavalry", 0);

namespace HOLGarrison
{
	// Unit kinds as the garrison sees them.
	const int KindMeleeInfantry = 0;
	const int KindRangedInfantry = 1;
	const int KindCavalry = 2;
	const int KindSiege = 3;
	const int KindHero = 4;

	// Places of a building type (0 = no garrison).
	int SlotsOf(FName BuildingType)
	{
		if (BuildingType == n"TowerWood")
		{
			return Math::Max(0, CVar_GarrisonSlotsWatchTower.GetInt());
		}
		if (BuildingType == n"TowerStone")
		{
			return Math::Max(0, CVar_GarrisonSlotsStoneTower.GetInt());
		}
		if (BuildingType == n"TowerReinforced")
		{
			return Math::Max(0, CVar_GarrisonSlotsReinforcedTower.GetInt());
		}
		return 0;
	}

	// Q1: are the occupants out of reach while the building stands (A, B) or hurt with it (C)? Read by the later combat step, not here.
	bool OccupantsProtected()
	{
		return CVar_GarrisonOccupantsProtected.GetInt() != 0;
	}

	// May a unit of this kind garrison, given the three settings of Q2? Siege never does; ranged foot always do.
	bool KindMayEnter(int Kind, bool bMeleeInfantry, bool bHero, bool bCavalry)
	{
		if (Kind == KindRangedInfantry)
		{
			return true;
		}
		if (Kind == KindMeleeInfantry)
		{
			return bMeleeInfantry;
		}
		if (Kind == KindHero)
		{
			return bHero;
		}
		if (Kind == KindCavalry)
		{
			return bCavalry;
		}
		return false;
	}

	bool KindMayEnterByDefault(int Kind)
	{
		return KindMayEnter(Kind, CVar_GarrisonMeleeInfantry.GetInt() != 0, CVar_GarrisonHero.GetInt() != 0, CVar_GarrisonCavalry.GetInt() != 0);
	}

	// Only the building's own slot: an ally's (D374) and an enemy's building are refused.
	bool SlotMayEnter(int UnitSlot, int BuildingSlot)
	{
		return BuildingSlot >= 0 && UnitSlot == BuildingSlot;
	}

	int FreeSlots(int Slots, int Occupants)
	{
		return Math::Max(0, Slots - Math::Max(0, Occupants));
	}

	// Indexes (into UnitSlots / UnitKinds, in the order asked) of the candidates that enter a building of BuildingSlot with Slots places and
	// Occupants already inside; a refused candidate takes no place.
	TArray<int> Admitted(const TArray<int>& UnitSlots, const TArray<int>& UnitKinds, int BuildingSlot, int Slots, int Occupants)
	{
		TArray<int> In;
		int Free = FreeSlots(Slots, Occupants);
		for (int I = 0; I < UnitSlots.Num() && In.Num() < Free; ++I)
		{
			if (UnitKinds.IsValidIndex(I) && SlotMayEnter(UnitSlots[I], BuildingSlot) && KindMayEnterByDefault(UnitKinds[I]))
			{
				In.Add(I);
			}
		}
		return In;
	}

	// The same for a building type, with the default settings.
	TArray<int> AdmittedInto(FName BuildingType, const TArray<int>& UnitSlots, const TArray<int>& UnitKinds, int BuildingSlot, int Occupants)
	{
		return Admitted(UnitSlots, UnitKinds, BuildingSlot, SlotsOf(BuildingType), Occupants);
	}
}
