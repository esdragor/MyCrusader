// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLDuelSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
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

namespace
{
	/** Where the slot's hero stands (its entity's place); false (and the origin) for a hero not on the map. */
	bool HeroLocation(const UWorld* World, const int32 Slot, FVector& OutLocation)
	{
		OutLocation = FVector::ZeroVector;
		const UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
		UMassEntitySubsystem* Entities = World->GetSubsystem<UMassEntitySubsystem>();
		const uint32 HeroId = Units ? Units->FindHeroUnit(Slot) : 0;
		const FMassEntityHandle Entity = HeroId != 0 ? Units->FindEntity(HeroId) : FMassEntityHandle();
		FMassEntityManager* Manager = Entities ? &Entities->GetMutableEntityManager() : nullptr;
		const FTransformFragment* Transform = Manager && Entity.IsSet() && Manager->IsEntityValid(Entity) ? Manager->GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr;
		if (!Transform)
		{
			return false;
		}
		OutLocation = Transform->GetTransform().GetLocation();
		return true;
	}

	// The facts of Script/Duel/HOLDuelCommand.as (HOLDuelFact), one bit each.
	constexpr int32 FactEnemies = 1;
	constexpr int32 FactSeeEachOther = 2;
	constexpr int32 FactChallengerAlive = 8;
	constexpr int32 FactTargetAlive = 16;
}

void UHOLDuelSubsystem::MeasureFacts(const int32 Challenger, const int32 Target, int32& OutFacts, FVector& OutA, FVector& OutB, TArray<int32>& OutForumXY) const
{
	const UWorld* World = GetWorld();
	OutFacts = 0;
	OutForumXY.Reset();
	HeroLocation(World, Challenger, OutA);
	HeroLocation(World, Target, OutB);
	if (FHOLCampTable::FromWorld(World).AreHostile(Challenger, Target))
	{
		OutFacts |= FactEnemies;
	}
	// In sight of each other: each one's camp sees the other's place (without a fog, or before it started: everything is in sight).
	const UHOLVisionSubsystem* Vision = World->GetSubsystem<UHOLVisionSubsystem>();
	if (!Vision || !Vision->IsStarted() || (Vision->IsVisible(Challenger, OutB) && Vision->IsVisible(Target, OutA)))
	{
		OutFacts |= FactSeeEachOther;
	}
	const UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	if (Heroes && Heroes->GetSheets(Challenger).Public.State == EHOLHeroState::Alive)
	{
		OutFacts |= FactChallengerAlive;
	}
	if (Heroes && Heroes->GetSheets(Target).Public.State == EHOLHeroState::Alive)
	{
		OutFacts |= FactTargetAlive;
	}
	// D403: the main forum of any player, standing or in ruin (the secondary centres are not listed).
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		if (const AHOLBuilding* Forum = AHOLGameState::FindPrimaryForum(*World, Slot))
		{
			const FVector Place = Forum->GetActorLocation();
			OutForumXY.Add(FMath::RoundToInt32(Place.X));
			OutForumXY.Add(FMath::RoundToInt32(Place.Y));
		}
	}
}

FString UHOLDuelSubsystem::ChallengeNow(const int32 Challenger, const int32 Target)
{
	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return TEXT("client");
	}
	if (Challenger < 0 || Target < 0 || Challenger >= FHOLMatchConfig::MaxSlots || Target >= FHOLMatchConfig::MaxSlots || Challenger == Target)
	{
		return TEXT("not_enemy");
	}
	int32 Facts = 0;
	FVector A, B;
	TArray<int32> Forums;
	MeasureFacts(Challenger, Target, Facts, A, B, Forums);
	return StartChallenge(Challenger, Target, Facts, FMath::RoundToInt32(A.X), FMath::RoundToInt32(A.Y), FMath::RoundToInt32(B.X), FMath::RoundToInt32(B.Y), Forums);
}

bool UHOLDuelSubsystem::AnswerNow(const int32 Slot, const bool bAccept)
{
	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return false;
	}
	// The pending challenge addressed to the slot: its challenger gives the other end of the facts.
	const TArray<int32> Pairs = PendingPairs();
	for (int32 Index = 0; Index + 1 < Pairs.Num(); Index += 2)
	{
		if (Pairs[Index + 1] != Slot)
		{
			continue;
		}
		int32 Facts = 0;
		FVector A, B;
		TArray<int32> Forums;
		MeasureFacts(Pairs[Index], Slot, Facts, A, B, Forums);
		const bool bAnswered = AnswerChallenge(Slot, bAccept, Facts, FMath::RoundToInt32(A.X), FMath::RoundToInt32(A.Y), FMath::RoundToInt32(B.X), FMath::RoundToInt32(B.Y), Forums);
		bDuelOn = AnyDuelOn();
		return bAnswered;
	}
	return false;
}

FHOLDuelView UHOLDuelSubsystem::GetViewNow(const int32 Slot)
{
	FHOLDuelView View;
	const TArray<int32> Out = ViewOf(Slot);
	if (Out.Num() >= 6)
	{
		View.Code = Out[0];
		View.Opponent = Out[1];
		View.LeftMs = Out[2];
		View.CenterX = Out[3];
		View.CenterY = Out[4];
		View.RadiusCm = Out[5];
	}
	return View;
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
	FVector A, B;
	HeroLocation(World, Challenger, A);
	HeroLocation(World, Target, B);
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
	// D4 (DU-Q2): the conditions of each challenge waiting are read again; one that ceased to hold falls (or not, by the setting).
	const TArray<int32> Pairs = PendingPairs();
	for (int32 Index = 0; Index + 1 < Pairs.Num(); Index += 2)
	{
		int32 Facts = 0;
		FVector A, B;
		TArray<int32> Forums;
		MeasureFacts(Pairs[Index], Pairs[Index + 1], Facts, A, B, Forums);
		RecheckChallenge(Pairs[Index + 1], Facts, FMath::RoundToInt32(A.X), FMath::RoundToInt32(A.Y), FMath::RoundToInt32(B.X), FMath::RoundToInt32(B.Y), Forums);
	}
	const int32 StepMs = FMath::RoundToInt(GetDefault<UHOLSimulationSettings>()->GetStepSeconds() * 1000.f);
	NumEnded += AdvanceDuels(StepMs, Health);
	bDuelOn = AnyDuelOn();
	return true;
}

namespace
{
	// Development (scenarios): the answer of a slot given through the order queue, as that slot's player would, server or solo.
	FAutoConsoleCommandWithWorldAndArgs CommandForceAnswer(TEXT("hol.Duel.ForceAnswer"),
		TEXT("hol.Duel.ForceAnswer <Slot> <1|0>: the slot answers the challenge waiting for it, accepted (1) or refused (0), through the order queue (development only)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
			if (!Commands || Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("hol.Duel.ForceAnswer <Slot> <1|0>: on the server or in solo"));
				return;
			}
			const int32 Slot = FCString::Atoi(*Args[0]);
			const bool bAccept = FCString::Atoi(*Args[1]) != 0;
			UE_LOG(LogHOLMatch, Display, TEXT("hol.Duel.ForceAnswer: slot %d %s"), Slot, bAccept ? TEXT("accepts") : TEXT("refuses"));
			Commands->IssueDuelOrder(Slot, bAccept ? EHOLDuelOrderKind::Accept : EHOLDuelOrderKind::Refuse, INDEX_NONE);
		}));

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
