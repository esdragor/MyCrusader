// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLDuelSubsystem.generated.h"

/**
 * The hero duel's book where the simulation runs (DuelInProgress.md D5, GDD 10.2, D18; D483: the rules live in script).
 * The C++ part is the thinnest bridge: it gives the step, the heroes' health and the cached "a duel is on", and the
 * combat processor asks it, once a step, which (attacker, victim) pairs of heroes may touch each other. The rules (the
 * book, who may hit whom, when a duel ends) are in Script/Duel (HOLDuelRules.as, HOLDuelBook.as). Abstract: only a
 * script subclass is born, in each game or PIE world (as UHOLHeroReturnSubsystem). A client never decides.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLDuelSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's duel book (its script subclass), or null (no script compiled, or no world). */
	static UHOLDuelSubsystem* Find(const UWorld* World);

	/** Whether a duel is on (Active), cached at the end of every change: the combat processor reads it each step. */
	bool IsDuelOn() const { return bDuelOn; }

	/**
	 * Whether a blow of the hero AttackerHeroSlot lands on the hero VictimHeroSlot (-1: not a hero: a unit, a tower, a power,
	 * an event), the script's HOLDuel::BookCanHit. True without a script or while no duel is on.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Duel")
	bool CanHitNow(int32 AttackerHeroSlot, int32 VictimHeroSlot);

	/**
	 * The development door and the later command's end (D4): a challenge of Challenger at Target, accepted at once, the
	 * conditions of the challenge not checked. Whether the duel began (false on a client, or when one hero is in a duel already).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Duel")
	bool BeginDuelNow(int32 Challenger, int32 Target);

	/** The end of simulation step Step (UHOLHeroSubsystem calls it, after the sheets): the duels advance with the heroes' health. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Duel")
	bool TickNow(int32 Step);

	/** Duels ended so far (a test reads it). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Duel")
	int32 GetNumEnded() const { return NumEnded; }

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** The book has a duel on, overridden in script. The C++ base has none. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	bool AnyDuelOn();
	virtual bool AnyDuelOn_Implementation() { return false; }

	/** The book's HOLDuel::BookCanHit, overridden in script. The C++ base lets every blow land. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	bool CanHit(int32 AttackerHeroSlot, int32 VictimHeroSlot);
	virtual bool CanHit_Implementation(int32 AttackerHeroSlot, int32 VictimHeroSlot) { return true; }

	/** A duel of Challenger and Target begun and accepted, overridden in script. The C++ base begins none. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	bool BeginDuel(int32 Challenger, int32 Target);
	virtual bool BeginDuel_Implementation(int32 Challenger, int32 Target) { return false; }

	/** DeltaMs go by, Health the health of every slot's hero (0 dead or none); returns the number of duels that ended. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	int32 AdvanceDuels(int32 DeltaMs, const TArray<int32>& Health);
	virtual int32 AdvanceDuels_Implementation(int32 DeltaMs, const TArray<int32>& Health) { return 0; }

private:
	bool bDuelOn = false;
	int32 NumEnded = 0;
};
