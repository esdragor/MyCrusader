// The availability of a technology (T2, GDD 11, D27, D180): pure rules, integers and booleans only (D468 determinism), no
// world access. A technology has three conditions (GDD 11): the hero (the tier his level has opened, and his level when the
// row asks more than the tier: T-Q1, both columns are in Tools/Data/Technologies.csv), the building (finished, owned),
// the cost (the stock covers it). A level II also asks for its level I (Requires). The caller (T3/T4) reads the row of the
// table, the player's tier (AHOLGameState::GetTier, from the hero level, D04) and level, and tells whether the building
// stands and the stock pays.
namespace HOLTechnologyRules
{
	// What stops a technology; Available when nothing does. The first reason in this order is the one given to the player.
	const int Available = 0;
	// Already researched by this player (a technology is researched once).
	const int AlreadyResearched = 1;
	// The hero has not opened the tier yet.
	const int TierTooLow = 2;
	// The tier is open but the row asks a higher hero level (MinHeroLevel).
	const int HeroLevelTooLow = 3;
	// The technology it follows (level I of a line) is not researched yet.
	const int RequiresMissing = 4;
	// No finished building of the type that researches it.
	const int NoBuilding = 5;
	// The stock does not cover the cost.
	const int CannotPay = 6;

	// Why a technology cannot be started now, or Available. HeroTier is the tier the hero's level has opened (HOLHeroXP::GetTier
	// in C++), HeroLevel his level (0 without a hero); MinTier and MinHeroLevel the row's (MinHeroLevel 0: the tier alone).
	// bRequiresDone: true when the row asks for no other technology or when that one is researched.
	int Check(int HeroTier, int HeroLevel, int MinTier, int MinHeroLevel, bool bRequiresDone, bool bBuildingFinished, bool bCanPay, bool bResearched)
	{
		if (bResearched)
			return AlreadyResearched;
		if (HeroTier < MinTier)
			return TierTooLow;
		if (HeroLevel < MinHeroLevel)
			return HeroLevelTooLow;
		if (!bRequiresDone)
			return RequiresMissing;
		if (!bBuildingFinished)
			return NoBuilding;
		if (!bCanPay)
			return CannotPay;
		return Available;
	}

	// True when the technology can be started now.
	bool IsAvailable(int HeroTier, int HeroLevel, int MinTier, int MinHeroLevel, bool bRequiresDone, bool bBuildingFinished, bool bCanPay, bool bResearched)
	{
		return Check(HeroTier, HeroLevel, MinTier, MinHeroLevel, bRequiresDone, bBuildingFinished, bCanPay, bResearched) == Available;
	}
}
