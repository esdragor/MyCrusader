// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLHeroSheet.generated.h"

/** Where a slot's hero stands (J8-Q5). None: the slot never had one. ReadyToReturn comes with J8a.4 (his return). */
UENUM()
enum class EHOLHeroState : uint8
{
	None,
	Alive,
	Dead,
	ReadyToReturn,
};

/**
 * The public part of a slot's hero sheet (J8-Q5, J8a3-Q1 = A): what every machine sees of him, carried by AHOLGameState
 * (one per slot, like the forums). His body travels with the troops (8-bit health share); this sheet gives the exact HP.
 * 4 bytes in memory; the true sheet lives on the server (UHOLHeroSubsystem, J8a.3 loop 3), the carriers get a copy.
 */
USTRUCT()
struct FHOLHeroSheet
{
	GENERATED_BODY()

	/** 1 from his birth on (levels come later); 0 without a hero. */
	UPROPERTY()
	uint8 Level = 0;

	/** Exact HP as shown (HOLCombat::GetShownHealth: the float health rounded up, 1 at least alive); 0 dead. */
	UPROPERTY()
	uint16 Health = 0;

	UPROPERTY()
	EHOLHeroState State = EHOLHeroState::None;

	bool operator==(const FHOLHeroSheet& Other) const
	{
		return Level == Other.Level && Health == Other.Health && State == Other.State;
	}
	bool operator!=(const FHOLHeroSheet& Other) const { return !(*this == Other); }
};

/**
 * The private part of a slot's hero sheet (J8-Q5): only its owner sees it (AHOLPlayerState, COND_OwnerOnly, like the
 * stock, D270). Steps are simulation steps (J8a3-Q2 = A: reproducible, D468; they follow the match's speed).
 */
USTRUCT()
struct FHOLHeroPrivateSheet
{
	GENERATED_BODY()

	UPROPERTY()
	int32 XP = 0;

	/** Step at which each ability is ready again, one per ability (none before the abilities). */
	UPROPERTY()
	TArray<int32> CooldownEndSteps;

	/** Step he fell at (Combat.DeathStep); -1 while alive or without a hero. Kept after his body is gone. */
	UPROPERTY()
	int32 DeathStep = -1;

	bool operator==(const FHOLHeroPrivateSheet& Other) const
	{
		return XP == Other.XP && DeathStep == Other.DeathStep && CooldownEndSteps == Other.CooldownEndSteps;
	}
	bool operator!=(const FHOLHeroPrivateSheet& Other) const { return !(*this == Other); }
};

/** What the server knows of a slot's hero at a step, read from his entity (J8a.3 loop 3 reads it). */
struct FHOLHeroFacts
{
	/** His entity exists (alive or dying); false before his birth and once his body is destroyed. */
	bool bPresent = false;
	/** The simulation's float health. */
	float Health = 0.f;
	/** Combat.DeathStep: the step he fell at, -1 alive. */
	int64 DeathStep = -1;
};

/** Both parts of a sheet, as the rule makes them. */
struct FHOLHeroSheets
{
	FHOLHeroSheet Public;
	FHOLHeroPrivateSheet Private;
};

namespace HOLHeroSheet
{
	/** His level for now (levels come later). */
	constexpr uint8 StartLevel = 1;

	/**
	 * The sheet from what the server knows (pure). Present and not dying: Alive, StartLevel, the shown HP, death step -1.
	 * Present and dying (a death step, or no health left): Dead, 0 HP, its death step. Absent: Dead stays Dead with its
	 * death step (the body destroyed after the fall); otherwise None (no hero yet). XP and cooldowns carry over from
	 * Previous. ReadyToReturn is never set before J8a.4.
	 */
	HEROESOFLIBERTY_API FHOLHeroSheets Make(const FHOLHeroFacts& Facts, const FHOLHeroSheets& Previous);
}
