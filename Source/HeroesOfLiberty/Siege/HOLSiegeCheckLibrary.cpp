// Copyright Epic Games, Inc. All Rights Reserved.

#include "Siege/HOLSiegeCheckLibrary.h"

#include "Siege/HOLSiegeRules.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLUnitTypes.h"

namespace
{
	/** The script's engine number (EHOLSiegeEngine, the same order as HOLSiege::EEngine), false out of range. */
	bool ToEngine(const int32 Engine, HOLSiege::EEngine& OutEngine)
	{
		if (Engine < 0 || Engine > static_cast<int32>(HOLSiege::EEngine::Cannon))
		{
			return false;
		}
		OutEngine = static_cast<HOLSiege::EEngine>(Engine);
		return true;
	}

	/** The script's block number (EHOLSiegeBlock: Available = 0... CannotPay = 4 of EHOLSiegeBlock), -1 for none of them. */
	int32 ToNumber(const HOLSiege::EBlock Block)
	{
		switch (Block)
		{
		case HOLSiege::EBlock::None: return 0;
		case HOLSiege::EBlock::NotInPrototype: return 1;
		case HOLSiege::EBlock::TierTooLow: return 2;
		case HOLSiege::EBlock::NoWorkshop: return 3;
		case HOLSiege::EBlock::CannotPay: return 4;
		}
		return -1;
	}
}

int32 UHOLSiegeCheckLibrary::Check(const int32 Engine, const int32 HeroTier, const bool bWorkshopFinished, const bool bCanPay)
{
	HOLSiege::EEngine Which;
	return ToEngine(Engine, Which) ? ToNumber(HOLSiege::Check(Which, HeroTier, bWorkshopFinished, bCanPay)) : -1;
}

bool UHOLSiegeCheckLibrary::FindEngine(const FName TypeName, int32& OutEngine)
{
	HOLSiege::EEngine Which;
	const bool bFound = HOLSiege::FindEngine(TypeName, Which);
	OutEngine = bFound ? static_cast<int32>(Which) : -1;
	return bFound;
}

FString UHOLSiegeCheckLibrary::DescribeBlock(const int32 Block)
{
	if (Block < 0 || Block > static_cast<int32>(HOLSiege::EBlock::CannotPay))
	{
		return FString();
	}
	return HOLSiege::DescribeBlock(static_cast<HOLSiege::EBlock>(Block));
}

int32 UHOLSiegeCheckLibrary::GetTier(const int32 Engine)
{
	HOLSiege::EEngine Which;
	return ToEngine(Engine, Which) ? HOLSiege::GetTier(Which) : 0;
}

bool UHOLSiegeCheckLibrary::InPrototype(const int32 Engine)
{
	HOLSiege::EEngine Which;
	return ToEngine(Engine, Which) && HOLSiege::InPrototype(Which);
}

FName UHOLSiegeCheckLibrary::GetHealthStat(const FName TypeName)
{
	return HOLSiege::GetHealthStat(TypeName);
}

FName UHOLSiegeCheckLibrary::GetDamageStat(const FName TypeName)
{
	return HOLSiege::GetDamageStat(TypeName);
}

int32 UHOLSiegeCheckLibrary::GetEngineCount()
{
	return static_cast<int32>(HOLSiege::EEngine::Cannon) + 1;
}

int32 UHOLSiegeCheckLibrary::HealthWithTechnology(const int32 Health, const int32 HealthPercent)
{
	FHOLUnitTypeConstants Type;
	Type.MaxHealth = Health;
	return HOLCombat::GetMaxHealth(Type, 1, HealthPercent);
}

float UHOLSiegeCheckLibrary::AttackWithTechnology(const int32 Attack, const int32 AttackPercent)
{
	FHOLUnitTypeConstants Attacker;
	Attacker.Attack = Attack;
	FHOLUnitTypeConstants Defender;
	Defender.MeleeArmor = 0;
	Defender.RangedArmor = 0;
	Defender.SiegeArmor = 0;
	Defender.HeroArmor = 0;
	return HOLCombat::GetDamage(Attacker, Defender, 1, 0, AttackPercent, 0);
}

bool UHOLSiegeCheckLibrary::TakesUnitsOn(const int32 Attack)
{
	return Attack > 0;
}

bool UHOLSiegeCheckLibrary::IsInArea(const FVector2D Centre, const FVector2D Other, const float Radius)
{
	return HOLCombat::IsInArea(Centre, Other, Radius);
}
