// SC3 follow-up (HALDIR-1121sd, D879): the last script uses of MainCentre classified "any centre" by f85b3c0 are the console commands that fall back on "the own forum"
// when nothing is selected (HOLControllerCommandsC2b.as hol.Prod.Rally / hol.Prod.Queue: EnsureOwnSelection(Controller, n"MainCentre"), and hol.Prod.RallySite: FindOwnBuildingByType).
// A player whose only centre is a secondary centre (main centre lost or not yet built) must get that centre, not nothing. The rule is HOLCentreKind::OwnedCentreType:
// the main centre first, else the secondary centre, else the main centre (the historical default, so the "no own main centre" line stays the same).
// Values asserted are computed by the code under test (HOLCentreKind::IsCentre decides what the answer must be).
// Red until the rule is coded (commit after this one): the stub answers the main centre whatever is owned.

// The answer is always a centre (IsCentre), whatever the player owns.
void Test_OwnedCentreTypeIsAlwaysACentre(FUnitTest& T)
{
	for (int Mask = 0; Mask < 4; Mask++)
	{
		FName Type = HOLCentreKind::OwnedCentreType((Mask & 1) != 0, (Mask & 2) != 0);
		bool bCentre = HOLCentreKind::IsCentre(Type);
		T.AssertTrue(bCentre, f"mask {Mask}: the fallback type {Type} is a centre");
	}
}

// Effect: with only a secondary centre the fallback is that secondary centre (the selection finds it); with a main centre it stays the main centre.
void Test_OwnedCentreTypeFollowsWhatIsOwned(FUnitTest& T)
{
	FName OnlySecondary = HOLCentreKind::OwnedCentreType(false, true);
	FName Both = HOLCentreKind::OwnedCentreType(true, true);
	FName OnlyMain = HOLCentreKind::OwnedCentreType(true, false);
	FName Nothing = HOLCentreKind::OwnedCentreType(false, false);
	T.AssertEquals(n"SecondaryCentre", OnlySecondary, "only a secondary centre owned: the fallback is the secondary centre");
	T.AssertEquals(n"MainCentre", Both, "main and secondary owned: the main centre first");
	T.AssertEquals(n"MainCentre", OnlyMain, "only the main centre owned: the main centre");
	T.AssertEquals(n"MainCentre", Nothing, "no centre owned: the historical default, the main centre");
}
