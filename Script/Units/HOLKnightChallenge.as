// SAURON-1125b (Dawn A7, D919): the RULE of the Knight's challenge, in script (D839: one source). The numbers (8 s, 8 m, 45 s) are the row
// KnightChallenge of Tools/Data/UnitAbilities.csv, read through HOLDawnValues; this file holds only the two relations the cast needs. The simulation
// (C++, UHOLUnitSubsystem::CastKnightChallenge) only writes the result on the Knight's combat fragment.
namespace HOLKnightChallenge
{
	// The simulation step at which a challenge started at NowStep ends: the duration counted in whole steps.
	int64 EndStepOf(int64 NowStep, int DurationMs, int StepMs)
	{
		if (StepMs <= 0)
			return NowStep;
		return NowStep + int64(DurationMs / StepMs);
	}

	// The cast the Knight's button makes: the values of the table row (D839, one source), the end step by the rule above, then the C++ liaison
	// writes the Knight's combat fragment (it refuses another type, a dead Knight, another owner, a running cool-down).
	bool Cast(UHOLUnitSubsystem Units, int UnitId, int RequiredOwner)
	{
		const int StepMs = HOLUnitAbilityRead::GetSimulationStepMs(Units);
		if (StepMs <= 0)
			return false;
		const int64 EndStep = EndStepOf(HOLUnitAbilityRead::GetSimulationStep(Units), HOLDawnValues::KnightChallengeDurationMs(), StepMs);
		return HOLUnitAbilityRead::CastKnightChallenge(Units, UnitId, RequiredOwner, EndStep, HOLDawnValues::RadiusCm(), HOLDawnValues::CooldownMs());
	}

	// The Knight may cast: alive, and its cool-down over.
	bool CanCast(bool bAlive, int CooldownMs)
	{
		return bAlive && CooldownMs <= 0;
	}
}
