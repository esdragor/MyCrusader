// The numbers of the Dawn's Honor gauge and of the Knight's challenge (D859, AubeInProgress.md). ONE source (D839): a constant of a
// file is not visible from another file, so each value is a function of the namespace; Tools/Data/ValueSheet.csv points at each
// of them (locator on the `return`). Nothing here decides a rule: the gauge rules are HOLFactionResource, the zone and cool-down
// rules are HOLGroundAbility; they take these values as parameters. Tested in Script/Tests/HOLDawnValues_Test.as.
// STUB of the first commit of step 1a: every function returns 0 (the test is red); the values come with the next commit.
namespace HOLDawnValues
{
	// The Honor gauge: cap, value at the start of a match.
	int HonorCap()
	{
		return 0;
	}

	int HonorStart()
	{
		return 0;
	}

	// Honor gained by accepting a duel, by winning it; Honor lost by refusing one.
	int HonorDuelAccept()
	{
		return 0;
	}

	int HonorDuelWin()
	{
		return 0;
	}

	int HonorDuelRefuse()
	{
		return 0;
	}

	// Combat: Honor per kill made within HonorKillRadiusCm (cm) of a building of the faction; one Honor per HonorHealPerPoint hit points healed.
	int HonorKillGain()
	{
		return 0;
	}

	int HonorKillRadiusCm()
	{
		return 0;
	}

	int HonorHealPerPoint()
	{
		return 0;
	}

	// The Knight's challenge (provisional, no Knight unit type yet): duration (ms), zone radius (cm) centred on the Knight, cool-down (ms).
	int KnightChallengeDurationMs()
	{
		return 0;
	}

	int RadiusCm()
	{
		return 0;
	}

	int CooldownMs()
	{
		return 0;
	}
}
