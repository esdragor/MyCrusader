// Copyright Epic Games, Inc. All Rights Reserved.

#include "Technologies/HOLTechnologyCheckLibrary.h"

#include "Technologies/HOLTechnologyRules.h"

int32 UHOLTechnologyCheckLibrary::Check(const bool bImplemented, const int32 HeroTier, const int32 HeroLevel, const int32 MinTier, const int32 MinHeroLevel,
	const bool bRequiresDone, const bool bBuildingFinished, const bool bCanPay, const bool bResearched, const bool bQueued)
{
	FHOLTechnologyRow Row;
	Row.Implemented = bImplemented;
	Row.MinTier = MinTier;
	Row.MinHeroLevel = MinHeroLevel;
	HOLTechnology::FFacts Facts;
	Facts.HeroTier = HeroTier;
	Facts.HeroLevel = HeroLevel;
	Facts.bRequiresDone = bRequiresDone;
	Facts.bBuildingFinished = bBuildingFinished;
	Facts.bCanPay = bCanPay;
	Facts.bResearched = bResearched;
	Facts.bQueued = bQueued;
	// The numbers of Script/Technologies/HOLTechnologyRules.as (the script has no enum of the simulation).
	switch (HOLTechnology::Check(Row, Facts))
	{
	case HOLTechnology::EBlock::None: return 0;
	case HOLTechnology::EBlock::AlreadyResearched: return 1;
	case HOLTechnology::EBlock::TierTooLow: return 2;
	case HOLTechnology::EBlock::HeroLevelTooLow: return 3;
	case HOLTechnology::EBlock::RequiresMissing: return 4;
	case HOLTechnology::EBlock::NoBuilding: return 5;
	case HOLTechnology::EBlock::CannotPay: return 6;
	case HOLTechnology::EBlock::AlreadyQueued: return 7;
	case HOLTechnology::EBlock::NotImplemented: return 8;
	}
	return -1;
}