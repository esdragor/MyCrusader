// Port of the part of HOLProductionTests.cpp (HeroesOfLiberty.Production.Advance) that uses only UHOLProductionReadLibrary (L17-12 PARTIAL, ELROND-1117d, D830/D839).
// Same vectors as the C++ test (an item started at step 10, 4 build steps), names adapted.
// NOT ported (the library does not expose FHOLProductionItem, FHOLProductionCost, TFunctionRef): HeroesOfLiberty.Production.Queue (all of it: TryEnqueue, Cancel),
// the Advance calls and the checks on the queue of HeroesOfLiberty.Production.Advance, and the data tables of HeroesOfLiberty.Production.Tables: they stay in C++.

// D03: the population cap is what the buildings give, 175 at most.
void Test_Production_PopulationCap(FUnitTest& T)
{
	T.AssertEquals(10, HOLProductionRead::GetPopulationCap(10), "population cap: what the buildings give");
	T.AssertEquals(175, HOLProductionRead::GetPopulationCap(400), "population cap: 175 at most (D03)");
}

// Progress of the head item: half done two steps later, capped at 100 % (a later start at step 15 is capped far after it).
void Test_Production_Progress(FUnitTest& T)
{
	T.AssertEquals(0.5f, HOLProductionRead::GetProgress(10, 12.0, 4), "half done two steps later");
	T.AssertEquals(1.0f, HOLProductionRead::GetProgress(15, 100.0, 4), "progress is capped");
}

// D829: done but no place, the item stays at 100 % waiting for a place; not waiting before 100 %.
void Test_Production_WaitingForRoom(FUnitTest& T)
{
	T.AssertFalse(HOLProductionRead::IsWaitingForRoom(10, 13.0, 4), "not waiting for room before 100 %");
	T.AssertTrue(HOLProductionRead::IsWaitingForRoom(10, 14.0, 4), "it waits at 100 % for a place");
}