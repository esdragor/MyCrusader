// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitAbilityReadLibrary.h"

#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitAbilityRules.h"
#include "Simulation/HOLUnitSubsystem.h"

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

int32 UHOLUnitAbilityReadLibrary::GetRageCooldownEndMs(const int32 StartMs)
{
	return HOLUnitAbility::GetRageCooldownEndMs(StartMs);
}

int32 UHOLUnitAbilityReadLibrary::GetRageEffectEndMs(const int32 StartMs)
{
	return HOLUnitAbility::GetRageEffectEndMs(StartMs);
}

bool UHOLUnitAbilityReadLibrary::IsRageEffectEnded(const int32 StartMs, const int32 NowMs, const float Health)
{
	return HOLUnitAbility::IsRageEffectEnded(StartMs, NowMs, Health);
}
int64 UHOLUnitAbilityReadLibrary::GetSimulationStep(const UHOLUnitSubsystem* Units)
{
	const UWorld* World = Units ? Units->GetWorld() : nullptr;
	const UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	return Simulation ? Simulation->GetStepIndex() : 0;
}

int32 UHOLUnitAbilityReadLibrary::GetSimulationStepMs(const UHOLUnitSubsystem* Units)
{
	const UWorld* World = Units ? Units->GetWorld() : nullptr;
	const UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	return Simulation ? FMath::RoundToInt(Simulation->GetStepSeconds() * 1000.f) : 0;
}

bool UHOLUnitAbilityReadLibrary::CastKnightChallenge(UHOLUnitSubsystem* Units, const int32 UnitId, const int32 RequiredOwner, const int64 EndStep, const int32 RadiusCm, const int32 CooldownMs)
{
	return Units && Units->CastKnightChallenge(static_cast<uint32>(UnitId), RequiredOwner, EndStep, static_cast<float>(RadiusCm), CooldownMs);
}

