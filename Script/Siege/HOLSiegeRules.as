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

	// S3: the engine a unit type is, by the name of its row (Ram, Mangonel, Trebuchet, Cannon); false for any other type. The C++
	// HOLSiege::FindEngine, which the production reads (HOLProductionSubsystem.cpp).
	bool FindEngine(FName TypeName, EHOLSiegeEngine& OutEngine)
	{
		if (TypeName == n"Ram")
		{
			OutEngine = EHOLSiegeEngine::Ram;
			return true;
		}
		if (TypeName == n"Mangonel")
		{
			OutEngine = EHOLSiegeEngine::Mangonel;
			return true;
		}
		if (TypeName == n"Trebuchet")
		{
			OutEngine = EHOLSiegeEngine::Trebuchet;
			return true;
		}
		if (TypeName == n"Cannon")
		{
			OutEngine = EHOLSiegeEngine::Cannon;
			return true;
		}
		return false;
	}

	// The words of a reason in the log of the production ("tier too low"), as the C++ HOLSiege::DescribeBlock.
	FString DescribeBlock(int Reason)
	{
		if (Reason == NotInPrototype)
			return "not in the prototype";
		if (Reason == TierTooLow)
			return "tier too low";
		if (Reason == NoWorkshop)
			return "no workshop";
		if (Reason == CannotPay)
			return "cannot pay";
		return "none";
	}

	// S5: the technology statistic that raises this type's health (Ram: RamHealth, Tools/Data/Technologies.csv ReinforcedRam),
	// NAME_None for any other type. The C++ HOLSiege::GetHealthStat.
	FName HealthStatOf(FName TypeName)
	{
		return TypeName == n"Ram" ? n"RamHealth" : NAME_None;
	}

	// S5: the technology statistic that raises this type's damage (Mangonel: MangonelDamage, HeavyMunitions), NAME_None for any
	// other type. The C++ HOLSiege::GetDamageStat.
	FName DamageStatOf(FName TypeName)
	{
		return TypeName == n"Mangonel" ? n"MangonelDamage" : NAME_None;
	}

	// S5: a maximum health with the percent its player's technologies add, rounded once to nearest (400 + 10 % = 440); a negative
	// percent is no loss. HOLCombat::GetMaxHealth at level 1 (the level's share is not in the script).
	int HealthWithTechnology(int Health, int HealthPercent)
	{
		const float Raised = float(Health) * (1.f + 0.01f * float(Math::Max(HealthPercent, 0)));
		return Raised >= 0.f ? int(Raised + 0.5f) : -int(0.5f - Raised);
	}

	// T6 / S5: the attack of one hit with the percent its player's technologies add (Heavy Munitions: 45 x 1.2 = 54), on the attack
	// alone, not the bonuses; a negative percent is no loss. HOLCombat::GetDamage's attack at level 1.
	float AttackWithTechnology(int Attack, int AttackPercent)
	{
		return float(Attack) * (1.f + 0.01f * float(Math::Max(AttackPercent, 0)));
	}

	// S4 (SQ1 = B, D614): whether a unit of this attack on units takes a unit on by itself; the Ram (attack 0 on units) does not, it
	// takes buildings only. The C++ combat sets FSeeker::bCanAttackUnits from it (HOLCombatProcessor.cpp: Constants.Attack > 0).
	bool TakesUnitsOn(int Attack)
	{
		return Attack > 0;
	}

	// S4 (D614): whether Other lies within an area of Radius cm around Centre (centre to centre, the circle's edge in): the units the
	// Mangonel's blow reaches around its target. The C++ HOLCombat::IsInArea.
	bool IsInArea(FVector2D Centre, FVector2D Other, float Radius)
	{
		const double DX = Other.X - Centre.X;
		const double DY = Other.Y - Centre.Y;
		return DX * DX + DY * DY <= double(Radius) * double(Radius);
	}
}
