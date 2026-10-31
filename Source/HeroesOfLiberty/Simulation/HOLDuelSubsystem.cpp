// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLDuelSubsystem.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "Match/HOLMatchConfig.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"

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
	// Where the two heroes stand now (the circle is centred between them); (0, 0) for a hero not on the map.
	const auto LocationOf = [World](const int32 Slot)
	{
		const UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
		UMassEntitySubsystem* Entities = World->GetSubsystem<UMassEntitySubsystem>();
		const uint32 HeroId = Units ? Units->FindHeroUnit(Slot) : 0;
		const FMassEntityHandle Entity = HeroId != 0 ? Units->FindEntity(HeroId) : FMassEntityHandle();
		FMassEntityManager* Manager = Entities ? &Entities->GetMutableEntityManager() : nullptr;
		const FTransformFragment* Transform = Manager && Entity.IsSet() && Manager->IsEntityValid(Entity) ? Manager->GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr;
		return Transform ? Transform->GetTransform().GetLocation() : FVector::ZeroVector;
	};
	const FVector A = LocationOf(Challenger);
	const FVector B = LocationOf(Target);
	const bool bBegan = BeginDuel(Challenger, Target, FMath::RoundToInt32(A.X), FMath::RoundToInt32(A.Y), FMath::RoundToInt32(B.X), FMath::RoundToInt32(B.Y));
	bDuelOn = AnyDuelOn();
	return bBegan;
}

bool UHOLDuelSubsystem::SetPostureNow(const int32 Slot, const int32 Posture)
{
	const UWorld* World = GetWorld();
	return World && World->GetNetMode() != NM_Client && bDuelOn && SetPosture(Slot, Posture);
}

int32 UHOLDuelSubsystem::GetAdversaryNow(const int32 Slot)
{
	return bDuelOn ? Adversary(Slot) : -1;
}

FVector UHOLDuelSubsystem::GetArenaNow(const int32 Slot)
{
	return bDuelOn ? ArenaOf(Slot) : FVector(0.0, 0.0, -1.0);
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

namespace
{
	// The development door of the duel (D4's command will replace it): a duel accepted at once, the conditions of the challenge not checked.
	FAutoConsoleCommandWithWorldAndArgs CommandBegin(TEXT("hol.Duel.Begin"),
		TEXT("hol.Duel.Begin <Challenger> <Target>: a duel of the two slots' heroes, accepted at once, server or solo (development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLDuelSubsystem* Duels = World ? UHOLDuelSubsystem::Find(World) : nullptr;
			if (!Duels || Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("hol.Duel.Begin <Challenger> <Target>: on the server or in solo, two slots"));
				return;
			}
			const int32 Challenger = FCString::Atoi(*Args[0]);
			const int32 Target = FCString::Atoi(*Args[1]);
			const bool bBegan = Duels->BeginDuelNow(Challenger, Target);
			UE_LOG(LogHOLMatch, Display, TEXT("hol.Duel.Begin: slot %d against slot %d %s"), Challenger, Target, bBegan ? TEXT("began") : TEXT("refused"));
		}));
}
