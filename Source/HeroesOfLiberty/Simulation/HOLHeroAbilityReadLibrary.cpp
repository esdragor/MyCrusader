// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHeroAbilityReadLibrary.h"

int32 UHOLHeroAbilityReadLibrary::GetUnlockLevel(const EHOLHeroAbility Ability) { return HOLHeroAbility::GetUnlockLevel(Ability); }

bool UHOLHeroAbilityReadLibrary::IsUnlocked(const EHOLHeroAbility Ability, const int32 Level) { return HOLHeroAbility::IsUnlocked(Ability, Level); }

int32 UHOLHeroAbilityReadLibrary::GetCooldownMs(const EHOLHeroAbility Ability) { return HOLHeroAbility::GetCooldownMs(Ability); }

bool UHOLHeroAbilityReadLibrary::CanCast(const EHOLHeroAbility Ability, const int32 Level, const bool bHeroAlive, const int32 CooldownMs)
{
	return HOLHeroAbility::CanCast(Ability, Level, bHeroAlive, CooldownMs);
}

bool UHOLHeroAbilityReadLibrary::CanPlantBanner(const float Distance) { return HOLHeroAbility::CanPlantBanner(Distance); }

bool UHOLHeroAbilityReadLibrary::IsWithinAura(const float Distance) { return HOLHeroAbility::IsWithinAura(Distance); }

int32 UHOLHeroAbilityReadLibrary::Tick(int32 RemainingMs, const int32 StepMs)
{
	HOLHeroAbility::Tick(RemainingMs, StepMs);
	return RemainingMs;
}

int32 UHOLHeroAbilityReadLibrary::GetDamageTakenPercent(const bool bInAura, const bool bCloseRanks) { return HOLHeroAbility::GetDamageTakenPercent(bInAura, bCloseRanks); }

int32 UHOLHeroAbilityReadLibrary::GetCloseRanksSpeedPercent(const bool bUnderCry, const bool bDawnStep, const int32 BasePercent)
{
	return HOLHeroAbility::GetCloseRanksSpeedPercent(bUnderCry, bDawnStep, BasePercent);
}

float UHOLHeroAbilityReadLibrary::GetReducedDamage(const float Damage, const int32 DamageTakenPercent) { return HOLHeroAbility::GetReducedDamage(Damage, DamageTakenPercent); }

int32 UHOLHeroAbilityReadLibrary::GetHealRatePercent(const bool bInAura, const bool bTalent) { return HOLHeroAbility::GetHealRatePercent(bInAura, bTalent); }

int32 UHOLHeroAbilityReadLibrary::GetCloseRanksDurationMs() { return HOLHeroAbility::CloseRanksDurationMs; }

int32 UHOLHeroAbilityReadLibrary::GetBannerDurationMs() { return HOLHeroAbility::BannerDurationMs; }

int32 UHOLHeroAbilityReadLibrary::GetMoraleReductionCapPercent() { return HOLHeroAbility::MoraleReductionCapPercent; }