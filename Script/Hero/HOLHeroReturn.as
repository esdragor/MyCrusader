// The hero's return after death (J8a.4): pure rules, integers only (D468 determinism), no world access.
// After death the player waits (30 + 8 x level) seconds, then may pay to bring the hero back. The price starts at
// 150 gold and goes down in a straight line to 0 over 60 s, rounded up to a whole gold at each step. At 0 the hero
// comes back by itself (players, AI and absent players alike).
namespace HOLHeroReturn
{
	// Simulation steps per second: UHOLSimulationSettings::SimulationRateHz (C++, D217), read through the bridge.
	const int StepsPerSecond = HOLHero::GetSimulationRateHz();

	const int BaseDelaySeconds = 30;
	const int DelayPerLevelSeconds = 8;
	const int StartPrice = 150;
	const int PriceDropSeconds = 60;

	// Price before the delay ends: no return can be bought yet.
	const int PriceNotReady = -1;

	// Return place: the main forum, or none (other answers are an index in the list of the other forums).
	const int PlaceMainForum = -1;
	const int PlaceNone = -2;

	// The return times at this percent of the rules' (hol.Hero.ReturnTimePercent, read in C++): 100 the real rules, less
	// for the network test (J8a.4 loop 5). Only the times change: the price keeps its formula.
	int TimePercent()
	{
		return HOLHero::GetReturnTimePercent();
	}

	// Steps to wait after death before the return can be bought, at Percent of the rules' times.
	int DelayStepsAt(int Level, int Percent)
	{
		return (BaseDelaySeconds + DelayPerLevelSeconds * Level) * StepsPerSecond * Percent / 100;
	}

	// Steps to wait after death before the return can be bought.
	int DelaySteps(int Level)
	{
		return DelayStepsAt(Level, TimePercent());
	}

	// First step at which the return can be bought.
	int ReadyStep(int DeathStep, int Level)
	{
		return DeathStep + DelaySteps(Level);
	}

	// Steps the price takes to go from StartPrice to 0.
	int PriceDropSteps()
	{
		return PriceDropStepsAt(TimePercent());
	}

	// Steps the price takes to go from StartPrice to 0 at Percent of the rules' times (one at least).
	int PriceDropStepsAt(int Percent)
	{
		return Math::Max(1, PriceDropSeconds * StepsPerSecond * Percent / 100);
	}

	bool IsReady(int Step, int DeathStep, int Level)
	{
		return Step >= ReadyStep(DeathStep, Level);
	}

	// Gold to pay at this step, or PriceNotReady before the delay ends.
	int PriceAt(int Step, int DeathStep, int Level)
	{
		const int SinceReady = Step - ReadyStep(DeathStep, Level);
		if (SinceReady < 0)
			return PriceNotReady;
		const int DropSteps = PriceDropSteps();
		if (SinceReady >= DropSteps)
			return 0;
		// Straight line from StartPrice to 0, rounded up to a whole gold (1 just before the end).
		return (StartPrice * (DropSteps - SinceReady) + DropSteps - 1) / DropSteps;
	}

	// A paid return is allowed only once ready (the price is then whatever PriceAt says).
	bool CanBuyReturn(int Step, int DeathStep, int Level)
	{
		return IsReady(Step, DeathStep, Level);
	}

	// The hero comes back by itself once the price reaches 0.
	bool ShouldAutoReturn(int Step, int DeathStep, int Level)
	{
		return Step >= ReadyStep(DeathStep, Level) + PriceDropSteps();
	}

	// Where the hero comes back: the main forum if it stands; else the other forum of the player nearest to the main
	// forum (lowest index on a tie); else PlaceNone. Returns PlaceMainForum, PlaceNone or an index in OtherForums.
	int ReturnPlace(bool bMainForumStanding, FIntPoint MainForum, const TArray<FIntPoint>& OtherForums)
	{
		if (bMainForumStanding)
			return PlaceMainForum;
		int Best = PlaceNone;
		int64 BestDistanceSquared = 0;
		for (int Index = 0; Index < OtherForums.Num(); Index++)
		{
			const int64 DX = int64(OtherForums[Index].X) - int64(MainForum.X);
			const int64 DY = int64(OtherForums[Index].Y) - int64(MainForum.Y);
			const int64 DistanceSquared = DX * DX + DY * DY;
			if (Best == PlaceNone || DistanceSquared < BestDistanceSquared)
			{
				Best = Index;
				BestDistanceSquared = DistanceSquared;
			}
		}
		return Best;
	}
}
