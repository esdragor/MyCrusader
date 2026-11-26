// A Templars seat can queue the Infirmary Brother at the Chapel (ARWEN-1125e, FAIL of the acceptance GLORFINDEL-1125a point 2; D839).
// The chain is the real one: the `Faction` column of the real UnitTypes.csv row (`Templars` for InfirmaryBrother), read by the server's own
// HOLFactionRules::GetTable().Allows through HOLAIOrder::IsAIUnitOpenToFaction (the same Allows as HOLFactionRules::IsUnitOpenTo, HOLProductionSubsystem.cpp:473),
// then the verdict of the queue (HOLProductionBoard::QueueVerdict, the gate order of ApplyOrders). Red while Factions.csv / BuiltInRoles has
// no `Templars` row: HasFaction(Templars) is false, ResolveKey reads it as Dawn and Allows("Templars", Dawn) is false, the order is refused (UnitFaction).

FHOLResources TBStock()
{
	FHOLResources Stock;
	Stock.Food = 1000;
	Stock.Wood = 1000;
	Stock.Stone = 1000;
	Stock.Gold = 1000;
	return Stock;
}

int TBVerdictForSeat(FName UnitType, FName Seat)
{
	const bool bOpen = HOLAIOrder::IsAIUnitOpenToFaction(UnitType, Seat);
	FHOLResources Cost;
	Cost.Food = 100;
	Cost.Gold = 100;
	return int(HOLProductionBoard::QueueVerdict(true, false, true, false, false, false, bOpen, 0, 5, TBStock(), Cost));
}

void Test_TemplarsSeatQueuesTheInfirmaryBrother(FUnitTest& T)
{
	// 0 = Enqueue (accepted), 8 = UnitFaction (refused: the faction does not field the unit).
	const int Templars = TBVerdictForSeat(n"InfirmaryBrother", n"Templars");
	T.AssertEquals(0, Templars, "a Templars seat: the order for an Infirmary Brother is accepted (queued)");
	const int Sergeant = TBVerdictForSeat(n"TempleSergeant", n"Templars");
	T.AssertEquals(0, Sergeant, "a Templars seat: the Temple Sergeant (same column) is accepted too");
}

// D920 (Q-chapel-faction = A, ARWEN-1126m): the Chapel is built by the Dawn (it forms the Monk) AND by the Templars (they form the Infirmary Brother).
// Read from the compiled BuildingTypes table (the `Faction` column that FTable::Allows splits on ';'): red while the row says `Dawn` only, so a
// Templars seat's builders never get the Chapel cell and the Infirmary Brother stays unreachable.
bool TBChapelColumnHas(FName Key)
{
	FHOLBuildingTypeRow Chapel;
	const bool bFound = HOLConstructionRead::FindBuildingType(n"Chapel", Chapel);
	if (!bFound)
	{
		return false;
	}
	const FString Column = ";" + Chapel.Faction.ToString() + ";";
	return Column.Contains(";" + Key.ToString() + ";");
}

void Test_ChapelIsBuiltByDawnAndTemplarsOnly(FUnitTest& T)
{
	T.AssertTrue(TBChapelColumnHas(n"Templars"), "Chapel Faction column holds Templars (D920): a Templars seat builds the Chapel");
	T.AssertTrue(TBChapelColumnHas(n"Dawn"), "Chapel Faction column keeps Dawn (it forms the Monk)");
	T.AssertFalse(TBChapelColumnHas(n"Legions"), "Chapel Faction column does not hold Legions");
}

void Test_TemplarsBrotherStaysClosedToOtherSeats(FUnitTest& T)
{
	// Guard of the fix: giving the Templars rows must not open their units to the others (column `Templars` only).
	const int Dawn = TBVerdictForSeat(n"InfirmaryBrother", n"Dawn");
	T.AssertEquals(8, Dawn, "a Dawn seat: the Infirmary Brother (column Templars) is refused by faction");
	const int Legions = TBVerdictForSeat(n"InfirmaryBrother", n"Legions");
	T.AssertEquals(8, Legions, "a Legions seat: the Infirmary Brother (column Templars) is refused by faction");
}
