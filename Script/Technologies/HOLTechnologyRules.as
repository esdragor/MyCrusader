// The availability of a technology (T2, GDD 11, D27, D180): pure rules, integers and booleans only (D468 determinism), no
// world access. A technology has three conditions (GDD 11): the hero (the tier his level has opened, and his level when the
// row asks more than the tier: T-Q1, both columns are in Tools/Data/Technologies.csv), the building (finished, owned),
// the cost (the stock covers it). A level II also asks for its level I (Requires). The caller (T3/T4) reads the row of the
// table, the player's tier (AHOLGameState::GetTier, from the hero level, D04) and level, and tells whether the building
// stands and the stock pays.
// Since MIG-CPP-Technology (D836 point 2) only the reasons live here: the rule is the C++ one (HOLTechnologyCheck::Check, the
// function the server applies), which the AI and the tests read; the script copy of the availability is gone.
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
	// Already in a production queue of its owner (the C++ rule refuses it right after the researched one).
	const int AlreadyQueued = 7;

	// No code applies its effect yet (the table's Implemented column is false: T-Q11 = B, D601): refused before every other reason,
	// as the C++ rule does (HOLTechnologyCheck::Check). The AI never starts such a research.
	const int NotImplemented = 8;
}
