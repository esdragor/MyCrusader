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

// D893 step 3 (server refusal): the production gate reads the applied tier 1 option of the seat's FHOLSpecializationView (AHOLPlayerState::GetSpecialization).
FHOLSpecializationView SpecUnlockViewWithTier1(int Option)
{
	FHOLSpecializationView View;
	View.Chosen.Add(-1);
	View.Chosen.Add(Option);
	View.Chosen.Add(-1);
	View.Chosen.Add(-1);
	return View;
}

void Test_SpecUnlockServerRefusesLaneUnitOfAnotherLane(FUnitTest& T)
{
	const EHOLFaction Templars = EHOLFaction::Templars;
	T.AssertTrue(HOLSpecUnlock::CanProduce(n"TempleSergeant", SpecUnlockViewWithTier1(0), Templars), "Iron chosen: the Sergent du Temple is produced");
	T.AssertTrue(HOLSpecUnlock::CanProduce(n"InfirmaryBrother", SpecUnlockViewWithTier1(1), Templars), "Faith chosen: the Frere infirmier is produced");
	T.AssertTrue(HOLSpecUnlock::CanProduce(n"PaviseCrossbowman", SpecUnlockViewWithTier1(2), Templars), "Stone chosen: the Arbaletrier a pavois is produced");
	T.AssertFalse(HOLSpecUnlock::CanProduce(n"TempleSergeant", SpecUnlockViewWithTier1(1), Templars), "another lane chosen: the Sergent du Temple is refused");
	T.AssertFalse(HOLSpecUnlock::CanProduce(n"InfirmaryBrother", SpecUnlockViewWithTier1(2), Templars), "another lane chosen: the Frere infirmier is refused");
	T.AssertFalse(HOLSpecUnlock::CanProduce(n"PaviseCrossbowman", SpecUnlockViewWithTier1(0), Templars), "another lane chosen: the Arbaletrier a pavois is refused");
}

void Test_SpecUnlockServerRefusesLaneUnitWithoutChoiceOrOfOtherFaction(FUnitTest& T)
{
	FHOLSpecializationView Empty;
	T.AssertFalse(HOLSpecUnlock::CanProduce(n"TempleSergeant", Empty, EHOLFaction::Templars), "no read at all: locked");
	T.AssertFalse(HOLSpecUnlock::CanProduce(n"TempleSergeant", SpecUnlockViewWithTier1(-1), EHOLFaction::Templars), "no lane chosen yet: locked");
	T.AssertFalse(HOLSpecUnlock::CanProduce(n"TempleSergeant", SpecUnlockViewWithTier1(0), EHOLFaction::DawnOrder), "another faction never produces a lane unit");
}

void Test_SpecUnlockServerLeavesOrdinaryUnitsAlone(FUnitTest& T)
{
	FHOLSpecializationView Empty;
	T.AssertTrue(HOLSpecUnlock::CanProduce(n"Halberdier", Empty, EHOLFaction::DawnOrder), "a common unit is produced without any choice");
	T.AssertTrue(HOLSpecUnlock::CanProduce(n"Monk", SpecUnlockViewWithTier1(2), EHOLFaction::Templars), "the Monk is produced whatever the lane");
}