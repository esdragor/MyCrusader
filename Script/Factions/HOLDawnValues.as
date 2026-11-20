// The numbers of the Dawn's Honor gauge and of the Knight's challenge (D859, AubeInProgress.md). ONE source (D839): a constant of a
// file is not visible from another file, so each value is a function of the namespace; Tools/Data/ValueSheet.csv points at each
// of them (locator on the `return`). Nothing here decides a rule: the gauge rules are HOLFactionResource, the zone and cool-down
// rules are HOLGroundAbility; they take these values as parameters. Tested in Script/Tests/HOLDawnValues_Test.as.
namespace HOLDawnValues
{
	// The Honor gauge: cap, value at the start of a match.
	int HonorCap()
	{
		return 100;
	}

	int HonorStart()
	{
		return 0;
	}

	// Honor gained by accepting a duel, by winning it; Honor lost by refusing one.
	int HonorDuelAccept()
	{
		return 10;
	}

	int HonorDuelWin()
	{
		return 30;
	}

	int HonorDuelRefuse()
	{
		return 5;
	}

	// Combat: Honor per kill made within HonorKillRadiusCm (cm) of a building of the faction; one Honor per HonorHealPerPoint hit points healed.
	int HonorKillGain()
	{
		return 1;
	}

	int HonorKillRadiusCm()
	{
		return 1200;
	}

	int HonorHealPerPoint()
	{
		return 50;
	}

	// The Knight's challenge (D875): the row KnightChallenge of Tools/Data/UnitAbilities.csv is the ONE source (D839); duration (ms), zone radius (cm)
	// centred on the Knight, cool-down (ms).
	int KnightChallengeDurationMs()
	{
		return HOLUnitAbilityTable::Row(HOLUnitAbilityTable::KnightChallenge).DurationMs;
	}

	int RadiusCm()
	{
		return HOLUnitAbilityTable::Row(HOLUnitAbilityTable::KnightChallenge).RadiusCm;
	}

	int CooldownMs()
	{
		return HOLUnitAbilityTable::Row(HOLUnitAbilityTable::KnightChallenge).CooldownMs;
	}
}
