// The unit tier rule (Palier3InProgress.md G1, D711; GDD 11.1), pure script half: integers, booleans and a type name only (D468 determinism),
// no world access, tested alone in HOLUnitTierRules_Test.as. ONE rule says why a unit cannot be queued now; the siege engines' rule
// (HOLSiege::Check, SiegeInProgress.md S1) is one case of it and gives the same reason numbers, so the two can be put together without a
// number changed. The tier of each type is the column MinTier of UnitTypes.csv itself, read through the library (no typed copy of the table,
// no C++ twin of the rule). NOT here: the call sites (production, command grid, AI), and the building a unit comes out of (the caller says whether it is finished). Every row of the table is listed, the hero, the King
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

	// The tier a unit opens at: the column MinTier of Tools\Data\UnitTypes.csv, read by the library (HOLUnitTier::GetMinTier, D836 point 6: the
	// column is the one truth, the script keeps no copy). UnknownTier for a type with no row: the Trebuchet and the Cannon (G7, G8: no row yet,
	// so NotListed, and the production leaves them to HOLSiege::Check) and any type outside the table. Pinned by Test_UnitTierColumnOfEveryRow.
	int MinTierOf(const FString& UnitType)
	{
		const int Column = HOLUnitTierRules::GetUnitMinTier(FName(UnitType));
		if (Column < 0)
			return UnknownTier;
		return Column;
	}

	// Why a unit cannot be queued now, or Available. Tier: the player's tier (the hero's level or a forced one); bBuildingFinished: the
	// player owns a finished building that produces it; bCanPay: the stock covers the cost.
	int Check(const FString& UnitType, int Tier, bool bBuildingFinished, bool bCanPay)
	{
		const int MinTier = MinTierOf(UnitType);
		if (MinTier == UnknownTier)
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
