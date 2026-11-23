// D893 step 2: the tier 1 commandery units are unlocked by the lane chosen, for the Templars only (HOLSpecUnlock::IsUnlocked, pure rule).
// Lanes (option index): 0 Iron = TempleSergeant, 1 Faith = InfirmaryBrother, 2 Stone = PaviseCrossbowman.

void Test_SpecUnlockLaneUnitNeedsItsLane(FUnitTest& T)
{
	const EHOLFaction Templars = EHOLFaction::Templars;
	const int None = HOLSpecialization::None;
	T.AssertTrue(HOLSpecUnlock::IsUnlocked(n"TempleSergeant", 0, Templars), "Iron unlocks the Sergent du Temple");
	T.AssertTrue(HOLSpecUnlock::IsUnlocked(n"InfirmaryBrother", 1, Templars), "Faith unlocks the Frere infirmier");
	T.AssertTrue(HOLSpecUnlock::IsUnlocked(n"PaviseCrossbowman", 2, Templars), "Stone unlocks the Arbaletrier a pavois");
	T.AssertFalse(HOLSpecUnlock::IsUnlocked(n"TempleSergeant", 1, Templars), "Faith does not unlock the Sergent du Temple");
	T.AssertFalse(HOLSpecUnlock::IsUnlocked(n"TempleSergeant", 2, Templars), "Stone does not unlock the Sergent du Temple");
	T.AssertFalse(HOLSpecUnlock::IsUnlocked(n"InfirmaryBrother", 0, Templars), "Iron does not unlock the Frere infirmier");
	T.AssertFalse(HOLSpecUnlock::IsUnlocked(n"PaviseCrossbowman", 1, Templars), "Faith does not unlock the Arbaletrier a pavois");
	T.AssertFalse(HOLSpecUnlock::IsUnlocked(n"TempleSergeant", None, Templars), "no lane chosen: locked");
	T.AssertFalse(HOLSpecUnlock::IsUnlocked(n"PaviseCrossbowman", None, Templars), "no lane chosen: locked");
}

void Test_SpecUnlockOtherFactionNeverSeesLaneUnits(FUnitTest& T)
{
	T.AssertFalse(HOLSpecUnlock::IsUnlocked(n"TempleSergeant", 0, EHOLFaction::DawnOrder), "Dawn with option 0 still has no Sergent du Temple");
	T.AssertFalse(HOLSpecUnlock::IsUnlocked(n"InfirmaryBrother", 1, EHOLFaction::DawnOrder), "Dawn with option 1 still has no Frere infirmier");
}

void Test_SpecUnlockOrdinaryUnitsAreUntouched(FUnitTest& T)
{
	T.AssertTrue(HOLSpecUnlock::IsUnlocked(n"Halberdier", HOLSpecialization::None, EHOLFaction::DawnOrder), "a common unit is always available");
	T.AssertTrue(HOLSpecUnlock::IsUnlocked(n"Monk", HOLSpecialization::None, EHOLFaction::Templars), "the common Monk is always available to the Templars");
	T.AssertEquals(HOLSpecUnlock::NotALaneUnit, HOLSpecUnlock::LaneOfUnit(n"Halberdier"), "not a lane unit");
	T.AssertEquals(0, HOLSpecUnlock::LaneOfUnit(n"TempleSergeant"), "Sergent du Temple: lane 0");
	T.AssertEquals(1, HOLSpecUnlock::LaneOfUnit(n"InfirmaryBrother"), "Frere infirmier: lane 1");
	T.AssertEquals(2, HOLSpecUnlock::LaneOfUnit(n"PaviseCrossbowman"), "Arbaletrier a pavois: lane 2");
}
