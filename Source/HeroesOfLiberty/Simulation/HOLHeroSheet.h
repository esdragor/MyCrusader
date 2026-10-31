// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLHeroSheet.generated.h"

/**
 * Where a slot's hero stands (J8-Q5). None: the slot never had one. ReadyToReturn (J8a.4): dead, his return may be bought
 * (UHOLHeroSubsystem::SetReturnReady). BlueprintType: the script's return rule reads it (UHOLHeroLibrary::GetHeroState).
 */
UENUM(BlueprintType)
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

	/** 1 to 6 from his XP (J8b.2, HOLHeroXP::GetLevel); 0 without a hero. */
	UPROPERTY()
	uint8 Level = 0;

	/** Exact HP as shown (HOLCombat::GetShownHealth: the float health rounded up, 1 at least alive); 0 dead. */
	UPROPERTY()
	uint16 Health = 0;

	UPROPERTY()
	EHOLHeroState State = EHOLHeroState::None;

	/** J8c.3: his cries of Serrez les rangs so far (wraps at 256): one more, every machine shows the cry (J8c-Q4). */
	UPROPERTY()
	uint8 Cries = 0;

	/** J8c.3: where his last banner was planted (cm), and the step it falls at (J8c-Q3); -1: never planted. */
	UPROPERTY()
	FIntPoint BannerLocation = FIntPoint::ZeroValue;
	UPROPERTY()
	int32 BannerEndStep = -1;

	/** T3: the families holding a seal (3 talents) and a key talent (5), one bit per family (D103, shown to the scouting adversary, D203). */
	UPROPERTY()
	uint8 TalentSeals = 0;
	UPROPERTY()
	uint8 TalentKeys = 0;

	bool operator==(const FHOLHeroSheet& Other) const
	{
		return Level == Other.Level && Health == Other.Health && State == Other.State && Cries == Other.Cries
			&& BannerLocation == Other.BannerLocation && BannerEndStep == Other.BannerEndStep
			&& TalentSeals == Other.TalentSeals && TalentKeys == Other.TalentKeys;
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

	/** Step at which each ability is ready again, indexed by EHOLHeroAbility (J8c.3; empty before his first cast). */
	UPROPERTY()
	TArray<int32> CooldownEndSteps;

	/** Step he fell at (Combat.DeathStep); -1 while alive or without a hero. Kept after his body is gone. */
	UPROPERTY()
	int32 DeathStep = -1;

	/** T3: the family of each talent taken, the row 2 first (the row of the n-th pick is 2 + n); the points not spent stay in reserve (D105). */
	UPROPERTY()
	TArray<uint8> TalentPicks;

	bool operator==(const FHOLHeroPrivateSheet& Other) const
	{
		return XP == Other.XP && DeathStep == Other.DeathStep && CooldownEndSteps == Other.CooldownEndSteps && TalentPicks == Other.TalentPicks;
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

/**
 * T3: the talent tree's rules where the simulation runs (the server checks a pick). The same rules as Script/Talents/HOLTalentRules.as
 * (the screen reads those); a script test checks both agree (HOLHero::CanPickTalent). One point a level from 2 to 9, rows in order,
 * one family of FamilyCount per pick (GDD 9.6, D103-D105).
 */
namespace HOLHeroTalent
{
	constexpr int32 FirstRow = 2;
	constexpr int32 LastRow = 9;
	constexpr int32 FamilyCount = 3;
	constexpr int32 SealThreshold = 3;
	constexpr int32 KeyThreshold = 5;

	/** The Dawn Order's families, the column of each row (D106): Oaths of the Guardian, the Captain, the Champion. */
	constexpr int32 FamilyGuardian = 0;
	constexpr int32 FamilyCaptain = 1;
	constexpr int32 FamilyChampion = 2;

	/** T4: Boucliers levees (row 2, Captain, D111): +1 armour for the units in the Dawn Aura. */
	inline bool GrantsAuraArmor(const TArray<uint8>& Picks)
	{
		return Picks.Num() > 0 && Picks[0] == FamilyCaptain;
	}

	/** D581: Pas de l'Aube (row 4, Captain, D111): the formation under Serrez les rangs advances at reduced speed, its damage reduction kept. */
	inline bool GrantsDawnStep(const TArray<uint8>& Picks)
	{
		return Picks.Num() > 2 && Picks[2] == FamilyCaptain;
	}

	/** A talent of Family can be taken by a hero of Level who took NumPicks already: a family of the row, a row left, a point in reserve. */
	inline bool CanPick(const int32 Level, const int32 NumPicks, const int32 Family)
	{
		const int32 Earned = FMath::Clamp(Level - FirstRow + 1, 0, LastRow - FirstRow + 1);
		return Family >= 0 && Family < FamilyCount && NumPicks < LastRow - FirstRow + 1 && Earned > NumPicks;
	}

	inline int32 CountOf(const TArray<uint8>& Picks, const int32 Family)
	{
		int32 Count = 0;
		for (const uint8 Pick : Picks)
		{
			Count += Pick == Family ? 1 : 0;
		}
		return Count;
	}

	/** One bit per family having at least Threshold talents. */
	inline uint8 FamiliesAtLeast(const TArray<uint8>& Picks, const int32 Threshold)
	{
		uint8 Mask = 0;
		for (int32 Family = 0; Family < FamilyCount; ++Family)
		{
			Mask |= CountOf(Picks, Family) >= Threshold ? static_cast<uint8>(1 << Family) : 0;
		}
		return Mask;
	}
}

namespace HOLHeroSheet
{
	/** His level at birth, 0 XP (J8b.2: then HOLHeroXP::GetLevel of his XP). */
	constexpr uint8 StartLevel = 1;

	/**
	 * The sheet from what the server knows (pure). Present and not dying: Alive, the level of his XP (J8b.2), the shown HP, death step -1.
	 * Present and dying (a death step, or no health left): Dead, 0 HP, its death step. Absent: Dead stays Dead with its
	 * death step and its level follows the XP (the body destroyed after the fall; D06); otherwise None (no hero yet). XP and cooldowns carry over from
	 * Previous. ReadyToReturn (set by UHOLHeroSubsystem::SetReturnReady, J8a.4) is kept while dead, body there or not.
	 */
	HEROESOFLIBERTY_API FHOLHeroSheets Make(const FHOLHeroFacts& Facts, const FHOLHeroSheets& Previous);
}
