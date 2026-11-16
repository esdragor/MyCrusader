// The unit tier rule (Palier3InProgress.md G1, D711; GDD 11.1), pure script half: integers, booleans and a type name only (D468 determinism),
// no world access, tested alone in HOLUnitTierRules_Test.as. ONE rule says why a unit cannot be queued now; the siege engines' rule
// (HOLSiege::Check, SiegeInProgress.md S1) is one case of it and gives the same reason numbers, so the two can be put together without a
// number changed. NOT here (the C++ lot of G1): the CSV column MinTier of UnitTypes.csv (the table below is its mirror, to be read from the
// column once it exists and compared with it by a test), the C++ twin and its agreement test, the call sites (production, command grid,
// AI), and the building a unit comes out of (the caller says whether it is finished). Every row of the table is listed, the hero, the King
// and the test dummies at tier 0 as the column says (D836 point 6); only a type outside the table is refused with NotListed.
namespace HOLUnitTier
{
	// What stops a unit; Available when nothing does. The first reason in this order is the one given to the player. The numbers are
	// HOLSiege's (Available 0, TierTooLow 2, NoWorkshop 3 = NoBuilding, CannotPay 4); 1 is the siege's "not in the prototype" and here "not in the table".
	const int Available = 0;
	const int NotListed = 1;
	const int TierTooLow = 2;
	const int NoBuilding = 3;
	const int CannotPay = 4;

	// The tier of a type outside the table.
	const int UnknownTier = -1;

	// The tier a unit opens at (GDD 11.1, D711): the Peasant, the Spearman and the Archer at 0; the Man-at-arms, the Crossbowman, the Light
	// cavalry, the Monk and the Ram at 1; the Mangonel at 2; the Trebuchet, the Cannon and the Arquebusier at 3. The Legions' Zombie (0) and Damned Warrior (1) as the column MinTier
	// of Tools\Data\UnitTypes.csv gives them (MIG-J-UnitTier: the whole-table agreement test found them missing here).
	int MinTierOf(const FString& UnitType)
	{
		if (UnitType == "Peasant" || UnitType == "Spearman" || UnitType == "Archer" || UnitType == "Zombie")
			return 0;
		// D836 point 6: the hero, the King and the two test dummies are of tier 0, as the column MinTier (the one truth) gives them.
		if (UnitType == "Hero" || UnitType == "King" || UnitType == "LightInfantry" || UnitType == "LightInfantryNanite")
			return 0;
		if (UnitType == "ManAtArms" || UnitType == "Crossbowman" || UnitType == "LightCavalry" || UnitType == "Monk" || UnitType == "Ram" || UnitType == "DamnedWarrior")
			return 1;
		if (UnitType == "Mangonel")
			return 2;
		if (UnitType == "Trebuchet" || UnitType == "Cannon" || UnitType == "Arquebusier")
			return 3;
		return UnknownTier;
	}

	// True when Tools\Data\UnitTypes.csv holds a row for the type. The Trebuchet and the Cannon have none yet (G7, G8): the C++ rule, which reads the column,
	// answers NotListed for them on every tier and the production leaves them to HOLSiege::Check; Check below answers the same (MIG-CPP-UnitTier,
	// ELROND-1116q). Their GDD tier stays in MinTierOf (the AI reads it). Delete this function and its use in Check when their rows exist.
	bool HasRowInUnitTypes(const FString& UnitType)
	{
		return UnitType != "Trebuchet" && UnitType != "Cannon";
	}

	// Why a unit cannot be queued now, or Available. Tier: the player's tier (the hero's level or a forced one); bBuildingFinished: the
	// player owns a finished building that produces it; bCanPay: the stock covers the cost.
	int Check(const FString& UnitType, int Tier, bool bBuildingFinished, bool bCanPay)
	{
		const int MinTier = MinTierOf(UnitType);
		if (MinTier == UnknownTier || !HasRowInUnitTypes(UnitType))
			return NotListed;
		if (Tier < MinTier)
			return TierTooLow;
		if (!bBuildingFinished)
			return NoBuilding;
		if (!bCanPay)
			return CannotPay;
		return Available;
	}

	// True when the unit can be queued now.
	bool IsAvailable(const FString& UnitType, int Tier, bool bBuildingFinished, bool bCanPay)
	{
		return Check(UnitType, Tier, bBuildingFinished, bCanPay) == Available;
	}
}
