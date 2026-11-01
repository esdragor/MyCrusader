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

	bool FindEngine(const FName TypeName, EEngine& OutEngine)
	{
		static const FName Ram(TEXT("Ram"));
		static const FName Mangonel(TEXT("Mangonel"));
		static const FName Trebuchet(TEXT("Trebuchet"));
		static const FName Cannon(TEXT("Cannon"));
		if (TypeName == Ram) { OutEngine = EEngine::Ram; return true; }
		if (TypeName == Mangonel) { OutEngine = EEngine::Mangonel; return true; }
		if (TypeName == Trebuchet) { OutEngine = EEngine::Trebuchet; return true; }
		if (TypeName == Cannon) { OutEngine = EEngine::Cannon; return true; }
		return false;
	}

	const TCHAR* DescribeBlock(const EBlock Block)
	{
		switch (Block)
		{
		case EBlock::NotInPrototype: return TEXT("not in the prototype");
		case EBlock::TierTooLow: return TEXT("tier too low");
		case EBlock::NoWorkshop: return TEXT("no workshop");
		case EBlock::CannotPay: return TEXT("cannot pay");
		default: return TEXT("none");
		}
	}
}
