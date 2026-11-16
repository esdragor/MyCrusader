// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitAbilityReadLibrary.h"

#include "Simulation/HOLUnitAbilityRules.h"

FName UHOLUnitAbilityReadLibrary::GetRageUnitType() { return HOLUnitAbility::RageUnitType; }
int32 UHOLUnitAbilityReadLibrary::GetRageDurationMs() { return HOLUnitAbility::RageDurationMs; }
int32 UHOLUnitAbilityReadLibrary::GetRageCooldownMs() { return HOLUnitAbility::RageCooldownMs; }
int32 UHOLUnitAbilityReadLibrary::GetRageHpCostPerHit() { return HOLUnitAbility::RageHpCostPerHit; }
int32 UHOLUnitAbilityReadLibrary::GetRageMinHp() { return HOLUnitAbility::RageMinHp; }
int32 UHOLUnitAbilityReadLibrary::GetRageStopHp() { return HOLUnitAbility::RageStopHp; }
int32 UHOLUnitAbilityReadLibrary::GetRageDamagePercent() { return HOLUnitAbility::RageDamagePercent; }
int32 UHOLUnitAbilityReadLibrary::GetRageRatePercent() { return HOLUnitAbility::RageRatePercent; }

bool UHOLUnitAbilityReadLibrary::CanCastRage(const bool bAlive, const float Health, const int32 CooldownMs)
{
	return HOLUnitAbility::CanCastRage(bAlive, Health, CooldownMs);
}

float UHOLUnitAbilityReadLibrary::GetRageHitCost(const float Health)
{
	return HOLUnitAbility::GetRageHitCost(Health);
}

int32 UHOLUnitAbilityReadLibrary::GetRageIntervalMs(const int32 IntervalMs)
{
	return HOLUnitAbility::GetRageIntervalMs(IntervalMs);
}
