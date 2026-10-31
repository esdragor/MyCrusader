// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLDuelSubsystem.h"

#include "Engine/World.h"
#include "Match/HOLMatchConfig.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationSettings.h"

UHOLDuelSubsystem* UHOLDuelSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLDuelSubsystem*> Books = World->GetSubsystemArrayCopy<UHOLDuelSubsystem>();
	return Books.Num() > 0 ? Books[0] : nullptr;
}

bool UHOLDuelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLHeroReturnSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLDuelSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UHOLDuelSubsystem::CanHitNow(const int32 AttackerHeroSlot, const int32 VictimHeroSlot)
{
	return !bDuelOn || CanHit(AttackerHeroSlot, VictimHeroSlot);
}

bool UHOLDuelSubsystem::BeginDuelNow(const int32 Challenger, const int32 Target)
{
	const UWorld* World = GetWorld();
	// The server (or a solo match) alone.
	if (!World || World->GetNetMode() == NM_Client)
	{
		return false;
	}
	const bool bBegan = BeginDuel(Challenger, Target);
	bDuelOn = AnyDuelOn();
	return bBegan;
}

bool UHOLDuelSubsystem::SetPostureNow(const int32 Slot, const int32 Posture)
{
	const UWorld* World = GetWorld();
	return World && World->GetNetMode() != NM_Client && bDuelOn && SetPosture(Slot, Posture);
}

int32 UHOLDuelSubsystem::GetDamagePercentNow(const int32 Slot)
{
	return bDuelOn ? DamagePercent(Slot) : 100;
}

bool UHOLDuelSubsystem::TickNow(const int32 Step)
{
	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return false;
	}
	const UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	TArray<int32> Health;
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		Health.Add(Heroes ? static_cast<int32>(Heroes->GetSheets(Slot).Public.Health) : 1);
	}
	const int32 StepMs = FMath::RoundToInt(GetDefault<UHOLSimulationSettings>()->GetStepSeconds() * 1000.f);
	NumEnded += AdvanceDuels(StepMs, Health);
	bDuelOn = AnyDuelOn();
	return true;
}
