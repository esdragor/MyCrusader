// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLRageVisual.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace
{
	// Default 4 = A+C (Q-legions-warrior-rage-visual, the user's answer of 2026-11-11; ring B not kept).
	TAutoConsoleVariable<int32> CVarRageVisual(TEXT("hol.Rage.Visual"), 4,
		TEXT("How the Damned Warrior's necrotic rage is drawn (L4): 0 nothing, 1 body tint, 2 ring on the ground, 3 state on the rage cell, 4 tint + cell state (default)."),
		ECVF_Default);
}

int32 HOLRageVisual::GetMode()
{
	return CVarRageVisual.GetValueOnAnyThread();
}

bool HOLRageVisual::GetRageState(const UWorld* World, const uint32 UnitId, int32& OutRageMs, int32& OutCooldownMs)
{
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	const FMassEntityHandle Entity = Units && UnitId != 0 ? Units->FindEntity(UnitId) : FMassEntityHandle();
	const FHOLCombatFragment* Combat = EntitySubsystem && Entity.IsSet() ? EntitySubsystem->GetEntityManager().GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
	OutRageMs = Combat ? Combat->RageMs : 0;
	OutCooldownMs = Combat ? Combat->RageCooldownMs : 0;
	return Combat != nullptr;
}
