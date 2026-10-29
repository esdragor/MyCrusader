// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHeroAbilityRules.h"

int32 HOLHeroAbility::GetUnlockLevel(const EHOLHeroAbility Ability)
{
	return Ability == EHOLHeroAbility::Banner ? 4 : 1;
}

bool HOLHeroAbility::IsUnlocked(const EHOLHeroAbility Ability, const int32 Level)
{
	return Level >= GetUnlockLevel(Ability);
}

int32 HOLHeroAbility::GetCooldownMs(const EHOLHeroAbility Ability)
{
	switch (Ability)
	{
	case EHOLHeroAbility::Aura: return 0;
	case EHOLHeroAbility::CloseRanks: return 60000;
	case EHOLHeroAbility::Banner: return 120000;
	}
	return 0;
}

bool HOLHeroAbility::CanCast(const EHOLHeroAbility Ability, const int32 Level, const bool bHeroAlive, const int32 CooldownMs)
{
	return Ability != EHOLHeroAbility::Aura && bHeroAlive && IsUnlocked(Ability, Level) && CooldownMs <= 0;
}

bool HOLHeroAbility::CanPlantBanner(const float Distance)
{
	return Distance <= BannerPlantRange;
}

bool HOLHeroAbility::IsWithinAura(const float Distance)
{
	return Distance <= AuraRadius;
}

void HOLHeroAbility::Tick(int32& RemainingMs, const int32 StepMs)
{
	RemainingMs = FMath::Max(RemainingMs - StepMs, 0);
}

int32 HOLHeroAbility::GetDamageTakenPercent(const bool bInAura, const bool bCloseRanks)
{
	const int32 Reduction = (bInAura ? AuraReductionPercent : 0) + (bCloseRanks ? CloseRanksReductionPercent : 0);
	return 100 - FMath::Min(Reduction, MoraleReductionCapPercent);
}

float HOLHeroAbility::GetReducedDamage(const float Damage, const int32 DamageTakenPercent)
{
	return Damage * static_cast<float>(DamageTakenPercent) / 100.f;
}
