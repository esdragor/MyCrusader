// D893 step 3, server read (ARWEN-1125md): the specialization board answers whether a seat may produce a lane unit, from the seat's OWN state (the state
// the choice writes: HOLSpecialization::Start then Advance), not from a constant. The first effect of a Templar lane: Iron unlocks the Sergent du Temple, and only it.

void Test_SpecBoardSeatMayProduceFollowsTheChosenLane(FUnitTest& T)
{
	UHOLSpecializationBoardScript Board = NewObject(GetTransientPackage(), UHOLSpecializationBoardScript);
	Board.SetSeatFaction(0, int(EHOLFaction::Templars));
	Board.SetSeatFaction(1, int(EHOLFaction::Templars));
	Board.SetSeatFaction(2, int(EHOLFaction::DawnOrder));

	bool BeforeChoice = Board.SeatMayProduce(0, n"TempleSergeant");
	T.AssertFalse(BeforeChoice, "no lane chosen yet: the Sergent du Temple is locked");

	// Seat 0 chooses Iron (option 0) at tier 1 and the delay runs out.
	FHOLSpecializationState& State = Board.StateOf(0);
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(State, 1, 0, 1), "seat 0 starts Iron");
	HOLSpecialization::Advance(State, HOLSpecialization::DelayMs(1), true);

	bool IronSergeant = Board.SeatMayProduce(0, n"TempleSergeant");
	bool IronBrother = Board.SeatMayProduce(0, n"InfirmaryBrother");
	bool IronCrossbow = Board.SeatMayProduce(0, n"PaviseCrossbowman");
	bool IronCommon = Board.SeatMayProduce(0, n"Halberdier");
	T.AssertTrue(IronSergeant, "Iron chosen: the Sergent du Temple is produced");
	T.AssertFalse(IronBrother, "Iron chosen: the Frere infirmier stays locked");
	T.AssertFalse(IronCrossbow, "Iron chosen: the Arbaletrier a pavois stays locked");
	T.AssertTrue(IronCommon, "a common unit is always produced");

	// The other seats are untouched by seat 0's choice.
	bool OtherTemplar = Board.SeatMayProduce(1, n"TempleSergeant");
	bool DawnSeat = Board.SeatMayProduce(2, n"TempleSergeant");
	T.AssertFalse(OtherTemplar, "seat 1 chose nothing: still locked");
	T.AssertFalse(DawnSeat, "a Dawn seat never produces a lane unit");
}