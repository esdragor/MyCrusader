// Copyright Epic Games, Inc. All Rights Reserved.

#include "Siege/HOLSiegeRules.h"

namespace HOLSiege
{
	int32 GetTier(const EEngine Engine)
	{
		switch (Engine)
		{
		case EEngine::Ram: return 1;
		case EEngine::Mangonel: return 2;
		case EEngine::Trebuchet: return 3;
		case EEngine::Cannon: return 3;
		}
		return 3;
	}

	bool InPrototype(const EEngine Engine)
	{
		return Engine == EEngine::Ram || Engine == EEngine::Mangonel;
	}

	EBlock Check(const EEngine Engine, const int32 HeroTier, const bool bWorkshopFinished, const bool bCanPay)
	{
		if (!InPrototype(Engine))
		{
			return EBlock::NotInPrototype;
		}
		if (HeroTier < GetTier(Engine))
		{
			return EBlock::TierTooLow;
		}
		if (!bWorkshopFinished)
		{
			return EBlock::NoWorkshop;
		}
		if (!bCanPay)
		{
			return EBlock::CannotPay;
		}
		return EBlock::None;
	}
}
