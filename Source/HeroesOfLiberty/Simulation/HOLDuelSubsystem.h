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
/** D4: a slot's duel as its hero sheet shows it (the script's HOLDuel::BookViewOf): Code 0 none, 1 a challenge sent, 2 one received, 3 a duel on. */
struct FHOLDuelView
{
	int32 Code = 0;
	int32 Opponent = -1;
	int32 LeftMs = 0;
	int32 CenterX = 0;
	int32 CenterY = 0;
	int32 RadiusCm = 0;
};

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

	/**
	 * D4, the challenge command (server or solo; the order queue calls it at a step's end): Challenger's hero challenges Target's.
	 * The match measures the facts (enemies, alive, in sight of each other, the main forums' places) and the script's rules
	 * decide (HOLDuel::CanChallenge). Returns "" when the challenge is made (the target has the window to answer), else the
	 * reason it is refused (not_enemy, too_far, forum_zone...; "no_script" without the book, "client" on a client).
	 */
	FString ChallengeNow(int32 Challenger, int32 Target);

	/**
	 * D4, the answer command: the slot's pending challenge accepted or refused (D80: final), the conditions read now (DU-Q2).
	 * Whether a challenge addressed to the slot was waiting.
	 */
	bool AnswerNow(int32 Slot, bool bAccept);

	/** D4: the slot's duel as its sheet shows it (the hero subsystem copies it into the public sheet each step). */
	FHOLDuelView GetViewNow(int32 Slot);

	/** The end of simulation step Step (UHOLHeroSubsystem calls it, after the sheets): the duels advance with the heroes' health. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Duel")
	bool TickNow(int32 Step);

	/**
	 * D6: the slot's posture in its duel (EHOLDuelPosture: 0 offensive, 1 balanced, 2 defensive), changed at once (D118).
	 * Whether it was taken (false on a client, or when the slot is in no duel on).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Duel")
	bool SetPostureNow(int32 Slot, int32 Posture);

	/** D6, the combat processor: the percent of its damage the slot's hero deals in his duel (100 outside one, or without a script). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Duel")
	int32 GetDamagePercentNow(int32 Slot);

	/** D120: the slot's adversary in the duel that is on; -1 outside one (or without a script). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Duel")
	int32 GetAdversaryNow(int32 Slot);

	/** D120: the circle of the slot's duel, X and Y its centre and Z its radius (cm); Z -1 when the slot is in no duel on. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Duel")
	FVector GetArenaNow(int32 Slot);

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

	/** A duel of Challenger and Target begun and accepted, the two heroes at (AX, AY) and (BX, BY) cm (the circle's centre is the middle), overridden in script. The C++ base begins none. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	bool BeginDuel(int32 Challenger, int32 Target, int32 AX, int32 AY, int32 BX, int32 BY);
	virtual bool BeginDuel_Implementation(int32 Challenger, int32 Target, int32 AX, int32 AY, int32 BX, int32 BY) { return false; }

	/** D4: a challenge, overridden in script ("" made, else the reason). Facts: HOLDuelFact bits; AX..BY the heroes' places (cm); ForumXY the main forums' places (X, Y pairs). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	FString StartChallenge(int32 Challenger, int32 Target, int32 Facts, int32 AX, int32 AY, int32 BX, int32 BY, const TArray<int32>& ForumXY);
	virtual FString StartChallenge_Implementation(int32 Challenger, int32 Target, int32 Facts, int32 AX, int32 AY, int32 BX, int32 BY, const TArray<int32>& ForumXY) { return TEXT("no_script"); }

	/** D4: an answer, overridden in script (whether a challenge was waiting for the slot). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	bool AnswerChallenge(int32 Slot, bool bAccept, int32 Facts, int32 AX, int32 AY, int32 BX, int32 BY, const TArray<int32>& ForumXY);
	virtual bool AnswerChallenge_Implementation(int32 Slot, bool bAccept, int32 Facts, int32 AX, int32 AY, int32 BX, int32 BY, const TArray<int32>& ForumXY) { return false; }

	/** D4: the challenges waiting as (challenger, target) pairs, overridden in script. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	TArray<int32> PendingPairs();
	virtual TArray<int32> PendingPairs_Implementation() { return TArray<int32>(); }

	/** D4 (DU-Q2): the conditions of the challenge addressed to Target read again, overridden in script (whether the challenge fell). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	bool RecheckChallenge(int32 Target, int32 Facts, int32 AX, int32 AY, int32 BX, int32 BY, const TArray<int32>& ForumXY);
	virtual bool RecheckChallenge_Implementation(int32 Target, int32 Facts, int32 AX, int32 AY, int32 BX, int32 BY, const TArray<int32>& ForumXY) { return false; }

	/** D4: the slot's duel as the sheet shows it, overridden in script: code, opponent, time left, circle X, Y, radius. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	TArray<int32> ViewOf(int32 Slot);
	virtual TArray<int32> ViewOf_Implementation(int32 Slot) { return TArray<int32>(); }

	/** The slot's adversary in its duel on, overridden in script (-1 outside one). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	int32 Adversary(int32 Slot);
	virtual int32 Adversary_Implementation(int32 Slot) { return -1; }

	/** The circle of the slot's duel on (X, Y centre, Z radius), overridden in script; Z -1 outside one. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	FVector ArenaOf(int32 Slot);
	virtual FVector ArenaOf_Implementation(int32 Slot) { return FVector(0.0, 0.0, -1.0); }

	/** The slot's posture in its duel, overridden in script. The C++ base takes none. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	bool SetPosture(int32 Slot, int32 Posture);
	virtual bool SetPosture_Implementation(int32 Slot, int32 Posture) { return false; }

	/** The percent of its damage the slot's hero deals in his duel, overridden in script (100 outside one). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	int32 DamagePercent(int32 Slot);
	virtual int32 DamagePercent_Implementation(int32 Slot) { return 100; }

	/** DeltaMs go by, Health the health of every slot's hero (0 dead or none); returns the number of duels that ended. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Duel")
	int32 AdvanceDuels(int32 DeltaMs, const TArray<int32>& Health);
	virtual int32 AdvanceDuels_Implementation(int32 DeltaMs, const TArray<int32>& Health) { return 0; }

private:
	/** D4: what only the match can see of a challenge of Challenger at Target: the HOLDuelFact bits, the two heroes' places, the main forums' places. */
	void MeasureFacts(int32 Challenger, int32 Target, int32& OutFacts, FVector& OutA, FVector& OutB, TArray<int32>& OutForumXY) const;
	bool bDuelOn = false;
	int32 NumEnded = 0;
};
