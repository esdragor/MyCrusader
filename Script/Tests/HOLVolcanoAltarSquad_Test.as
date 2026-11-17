// MIG-C-Volcano (CELEBORN-1117j): HeroesOfLiberty.Volcano.AltarSend and .AltarHoldSquad of HOLVolcanoTests.cpp, in script, through the five functions
// UHOLVolcanoLibrary already exposes (HOLVolcano::DecideAltarSquad reads AltarSend / AltarHoldSquad with the simulation's default settings).
// Level: 0 Easy, 1 Normal, 2 Hard. The AI's own use of the squad is HOLAIAltar_Test.as (not redone here).

FHOLAIAltar VolcanoSquadAltar(float Strength, float TraceDistance)
{
	FHOLAIAltar Altar;
	Altar.SiteIndex = 0;
	Altar.Radius = 1200.f;
	Altar.bAdversary = Strength > 0.f;
	Altar.AdversaryStrength = Strength;
	Altar.TraceDistance = TraceDistance;
	return Altar;
}

int VolcanoSquadOf(const FHOLAIAltar& Altar, int Free, int Sent, int Level, bool&out bDiversion)
{
	int Squad = 0;
	HOLVolcano::DecideAltarSquad(Altar, Free, Sent, Level, Squad, bDiversion);
	return Squad;
}

// 10 free soldiers, the reserve (40 %) keeps 4 at home: 6 can go. A small adversary meets the minimum, a middle one 1.5 times (rounded up), a big army
// is not met head on in Hard (a diversion), the others send what they can spare; the soldiers already there stay.
void Test_VolcanoAltarSendSizesTheSquad(FUnitTest& T)
{
	bool bDiversion = false;
	T.AssertEquals(2, VolcanoSquadOf(VolcanoSquadAltar(1.f, 9000.f), 10, 0, 1, bDiversion), "a small adversary (1): the minimum of the settings (2)");
	T.AssertFalse(bDiversion, "a small adversary: no diversion");
	T.AssertEquals(5, VolcanoSquadOf(VolcanoSquadAltar(3.f, 9000.f), 10, 0, 1, bDiversion), "an adversary of 3: 1.5 times, rounded up, 5");

	T.AssertEquals(0, VolcanoSquadOf(VolcanoSquadAltar(8.f, 9000.f), 10, 0, 2, bDiversion), "a big army (8, needs 12 of the 6 spared), Hard: no massive send");
	T.AssertTrue(bDiversion, "Hard: a diversion instead (its camp is bare)");
	T.AssertEquals(6, VolcanoSquadOf(VolcanoSquadAltar(8.f, 9000.f), 10, 0, 1, bDiversion), "Normal sends all it can spare (6)");
	T.AssertFalse(bDiversion, "Normal: no diversion");
	T.AssertEquals(4, VolcanoSquadOf(VolcanoSquadAltar(8.f, 9000.f), 10, 4, 2, bDiversion), "Hard with 4 already there: they stay, no recall");
}

// Ten reflections with an adversary whose strength wobbles between 3 and 4: the squad never shrinks and settles on the bigger need (4 x 1.5 = 6).
void Test_VolcanoAltarSendNeverOscillates(FUnitTest& T)
{
	bool bDiversion = false;
	int Previous = 0;
	bool bNeverShrinks = true;
	TArray<int> Squads;
	for (int Reflection = 0; Reflection < 10; Reflection++)
	{
		const float Strength = (Reflection % 2 == 0) ? 3.f : 4.f;
		const int Squad = VolcanoSquadOf(VolcanoSquadAltar(Strength, 9000.f), 10, Previous, 1, bDiversion);
		if (Squad < Previous)
			bNeverShrinks = false;
		Previous = Squad;
		Squads.Add(Squad);
	}
	T.AssertTrue(bNeverShrinks, "ten reflections: the squad never shrinks");
	T.AssertEquals(Squads[4], Squads[9], "the same size over the last 6");
	T.AssertEquals(6, Squads[9], "the size of the bigger need (4 x 1.5 = 6)");
}

// The free altar the AI holds itself (D640): one free soldier in 4 (one in 8 in Easy) when no adversary stands in the zone and the nearest trace is
// farther than 40 m; never a recall.
void Test_VolcanoAltarHoldSquadFollowsTheShare(FUnitTest& T)
{
	bool bDiversion = false;
	const FHOLAIAltar Far = VolcanoSquadAltar(0.f, 9000.f);
	T.AssertEquals(2, VolcanoSquadOf(Far, 8, 0, 1, bDiversion), "Normal: one soldier in 4 of 8");
	T.AssertEquals(2, VolcanoSquadOf(Far, 8, 0, 2, bDiversion), "Hard: the same quarter");
	T.AssertEquals(1, VolcanoSquadOf(Far, 8, 0, 0, bDiversion), "Easy: one in 8");
	T.AssertEquals(2, VolcanoSquadOf(Far, 5, 0, 1, bDiversion), "5 free soldiers: the quarter rounded up, 2");
	T.AssertEquals(0, VolcanoSquadOf(Far, 0, 0, 1, bDiversion), "no free soldier: none");
	T.AssertEquals(0, VolcanoSquadOf(VolcanoSquadAltar(0.f, 4000.f), 8, 0, 1, bDiversion), "a trace at exactly 40 m is too near");
	T.AssertEquals(3, VolcanoSquadOf(Far, 8, 3, 1, bDiversion), "3 already there: they stay");
	T.AssertEquals(3, VolcanoSquadOf(VolcanoSquadAltar(0.f, 100.f), 8, 3, 1, bDiversion), "even if the trace were near: no recall");
}
