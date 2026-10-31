// The siege engines' pure rules (SiegeInProgress.md S1, GDD 6.1, D23, D77, D180): which engine opens at which tier, which are in the
// prototype, and why an engine cannot be queued now. Integers and booleans only (D468 determinism), no world access, tested alone
// in script (HOLSiegeRules_Test.as). The numbers that are the GDD's (the tiers, the prototype's two engines) are here; the stats of an
// engine (health, damage, cost) are not: they belong to the tables (S2) and wait for SQ2.

// The engines of GDD 6.1, in the order of their tier. The Cracheur de bile, the Bombarde and the other factions' variants (D59) are
// the same role as the Cannon and come with their factions, after the prototype.
enum EHOLSiegeEngine
{
	// Tier 1: buildings and gates; weak to melee.
	Ram,
	// Tier 2: groups and walls, long range; weak to the light cavalry and melee.
	Mangonel,
	// Tier 3: fortifications from afar; must be set up and taken down.
	Trebuchet,
	// Tier 3, with the powder (D38, D180): short range, heavy damage.
	Cannon,
}

namespace HOLSiege
{
	// What stops an engine; Available when nothing does. The first reason in this order is the one given to the player.
	const int Available = 0;
	// The engine is not one of the prototype's two (D77).
	const int NotInPrototype = 1;
	// The hero has not opened the engine's tier yet.
	const int TierTooLow = 2;
	// No finished siege workshop of the player's.
	const int NoWorkshop = 3;
	// The stock does not cover the cost.
	const int CannotPay = 4;

	// The number of engines, and the engine at an index (0 to EngineCount - 1, in the order of the enum).
	const int EngineCount = 4;

	EHOLSiegeEngine EngineAt(int Index)
	{
		switch (Index)
		{
			case 0: return EHOLSiegeEngine::Ram;
			case 1: return EHOLSiegeEngine::Mangonel;
			case 2: return EHOLSiegeEngine::Trebuchet;
		}
		return EHOLSiegeEngine::Cannon;
	}

	// The tier an engine opens at (GDD 6.1: Ram 1, Mangonel 2, Trebuchet 3, Cannon 3 with the powder, D180). The siege workshop is
	// itself a tier 1 building (GDD 11.1), so no engine opens before 1.
	int TierOf(EHOLSiegeEngine Engine)
	{
		switch (Engine)
		{
			case EHOLSiegeEngine::Ram: return 1;
			case EHOLSiegeEngine::Mangonel: return 2;
			case EHOLSiegeEngine::Trebuchet: return 3;
			case EHOLSiegeEngine::Cannon: return 3;
		}
		return 3;
	}

	// True for the prototype's siege: the Ram and the Mangonel (D77). The other two wait.
	bool InPrototype(EHOLSiegeEngine Engine)
	{
		return Engine == EHOLSiegeEngine::Ram || Engine == EHOLSiegeEngine::Mangonel;
	}

	// The prototype's engines, in the order of their tier.
	TArray<EHOLSiegeEngine> PrototypeEngines()
	{
		TArray<EHOLSiegeEngine> Result;
		for (int Index = 0; Index < EngineCount; Index++)
		{
			if (InPrototype(EngineAt(Index)))
				Result.Add(EngineAt(Index));
		}
		return Result;
	}

	// The prototype's engines a hero tier has opened (HOLHeroXP::GetTier in C++; 0 without a hero or below level 3).
	TArray<EHOLSiegeEngine> OpenedEngines(int HeroTier)
	{
		TArray<EHOLSiegeEngine> Result;
		TArray<EHOLSiegeEngine> Prototype = PrototypeEngines();
		for (int Index = 0; Index < Prototype.Num(); Index++)
		{
			if (TierOf(Prototype[Index]) <= HeroTier)
				Result.Add(Prototype[Index]);
		}
		return Result;
	}

	// Why an engine cannot be queued now, or Available. HeroTier is the tier the hero's level has opened; bWorkshopFinished: the
	// player owns a finished siege workshop; bCanPay: the stock covers the engine's cost.
	int Check(EHOLSiegeEngine Engine, int HeroTier, bool bWorkshopFinished, bool bCanPay)
	{
		if (!InPrototype(Engine))
			return NotInPrototype;
		if (HeroTier < TierOf(Engine))
			return TierTooLow;
		if (!bWorkshopFinished)
			return NoWorkshop;
		if (!bCanPay)
			return CannotPay;
		return Available;
	}

	// True when the engine can be queued now.
	bool IsAvailable(EHOLSiegeEngine Engine, int HeroTier, bool bWorkshopFinished, bool bCanPay)
	{
		return Check(Engine, HeroTier, bWorkshopFinished, bCanPay) == Available;
	}
}
