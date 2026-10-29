// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHeroXPRules.h"

#include "Match/HOLResources.h"

namespace
{
	/** Total XP of each level, 1 to MaxLevel (J8b-Q2). */
	constexpr int32 LevelXP[HOLHeroXP::MaxLevel] = {0, 200, 600, 1300, 2200, 3300};

	/** Stats gained a level, in percent of level 1's (J8b-Q3). */
	constexpr int32 StatPercentPerLevel = 8;

	int32 ClampLevel(const int32 Level)
	{
		return FMath::Clamp(Level, 1, HOLHeroXP::MaxLevel);
	}
}

int32 HOLHeroXP::GetWeightPercent(const EHOLXPSource Source)
{
	switch (Source)
	{
	case EHOLXPSource::Combat: return 100;
	case EHOLXPSource::Command: return 75;
	case EHOLXPSource::Development: return 50;
	case EHOLXPSource::Exploration: return 25;
	}
	return 0;
}

int32 HOLHeroXP::GetValue(const FHOLResources& Cost)
{
	return Cost.Food + Cost.Wood + Cost.Stone + Cost.Gold;
}

int32 HOLHeroXP::GetXP(const int32 Value, const EHOLXPSource Source)
{
	return FMath::Max(Value, 0) * GetWeightPercent(Source) / 100;
}

int32 HOLHeroXP::GetHeroKillXP(const int32 VictimLevel, const int32 KillerLevel, const EHOLXPSource Source)
{
	// One division at the end: 300 x 1 / 6 x 75 % = 37 (not 50 x 75 % rounded twice).
	return HeroKillXP * ClampLevel(VictimLevel) * GetWeightPercent(Source) / (ClampLevel(KillerLevel) * 100);
}

bool HOLHeroXP::CountsWhileDead(const EHOLXPSource Source)
{
	return Source == EHOLXPSource::Development || Source == EHOLXPSource::Exploration;
}

TOptional<EHOLXPSource> HOLHeroXP::GetKillSource(const bool bKilledByHero, const bool bHeroAlive, const float DistanceToHero)
{
	if (!bHeroAlive)
	{
		return {};
	}
	if (bKilledByHero)
	{
		return EHOLXPSource::Combat;
	}
	if (DistanceToHero <= CommandRadius)
	{
		return EHOLXPSource::Command;
	}
	return {};
}

int32 HOLHeroXP::GetLevelXP(const int32 Level)
{
	return LevelXP[ClampLevel(Level) - 1];
}

int32 HOLHeroXP::GetLevel(const int32 XP)
{
	int32 Level = 1;
	while (Level < MaxLevel && XP >= LevelXP[Level])
	{
		++Level;
	}
	return Level;
}

int32 HOLHeroXP::GetStatPercent(const int32 Level)
{
	return 100 + StatPercentPerLevel * (ClampLevel(Level) - 1);
}

float HOLHeroXP::GetLevelStat(const float Level1Stat, const int32 Level)
{
	return Level1Stat * static_cast<float>(GetStatPercent(Level)) / 100.f;
}

float HOLHeroXP::GetHealthAfterLevelUp(const float Health, const float Level1MaxHealth, const int32 OldLevel, const int32 NewLevel)
{
	if (Health <= 0.f)
	{
		return Health;
	}
	return Health + GetLevelStat(Level1MaxHealth, NewLevel) - GetLevelStat(Level1MaxHealth, OldLevel);
}
