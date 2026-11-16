// Copyright Epic Games, Inc. All Rights Reserved.

#include "Siege/HOLSiegeCheckLibrary.h"

#include "Siege/HOLSiegeRules.h"

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

	/** The script's block number (HOLSiege::Available = 0... CannotPay = 4 in HOLSiegeRules.as), -1 for none of them. */
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
