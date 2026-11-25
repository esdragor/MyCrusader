// SAURON-1125b (Dawn A7, D919): the RULE of the Knight's challenge, in script (D839: one source). The numbers (8 s, 8 m, 45 s) are the row
// KnightChallenge of Tools/Data/UnitAbilities.csv, read through HOLDawnValues; this file holds only the two relations the cast needs. The simulation
// (C++, UHOLUnitSubsystem::CastKnightChallenge) only writes the result on the Knight's combat fragment.
namespace HOLKnightChallenge
{
	// The simulation step at which a challenge started at NowStep ends (STUB, red: the rule is not written yet).
	int64 EndStepOf(int64 NowStep, int DurationMs, int StepMs)
	{
		return -1;
	}

	// The Knight may cast: alive, and its cool-down over.
	bool CanCast(bool bAlive, int CooldownMs)
	{
		return bAlive && CooldownMs <= 0;
	}
}
