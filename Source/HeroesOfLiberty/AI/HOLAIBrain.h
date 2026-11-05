// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLAIBrain.generated.h"

/** IA0.4 B5: one thought the step made a slot think (OnStepEnded), read by the tests of the cadence. */
struct FHOLAIStepThought
{
	/** The step index OnStepEnded gave (the simulation's, already advanced past the step just run). */
	int64 Step = 0;
	int32 Slot = INDEX_NONE;
};

/**
 * The AI slots' brain (IA0.4 B1): the C++ part is the cadence and the list of AI slots only; the rule lives in a script
 * subclass (Script/AI/HOLAIBrain.as) that overrides Think. Abstract: only a script subclass is born, in each game or PIE
 * world (Angelscript activates a script world subsystem by itself). Server and solo only: a client never thinks.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLAIBrainSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's brain (its script subclass), or null (no script brain compiled, or no world). */
	static UHOLAIBrainSubsystem* Find(const UWorld* World);

	/** Steps between two thoughts of one AI slot (hol.AI.ThinkSteps, at least 1). */
	static int32 GetThinkSteps();

	/** One thought of slot PlayerIndex now (the step calls it; tests too). Whether a script brain thought. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI")
	bool ThinkNow(int32 PlayerIndex);

	/** Thoughts a script brain ran so far, all slots together. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI")
	int32 GetNumThoughts() const { return NumThoughts; }

	/**
	 * IA0.4 B5: the thoughts the step ran (not the direct ThinkNow calls), oldest first; the last MaxStepThoughts at least
	 * are kept. For the tests of the cadence (which slot thought at which step).
	 */
	const TArray<FHOLAIStepThought>& GetStepThoughts() const { return StepThoughts; }

	/**
	 * The simulation's step index; 0 without a simulation. For the decision log. At the step's tail (where the brain
	 * thinks) it is already advanced: the index OnStepEnded gives, 1 after the first step.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI")
	int64 GetStepIndex() const;

	/** The AI slots of the match (bot player states), by player index; empty without a game state. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI")
	TArray<int32> GetAISlots() const;

	/**
	 * D717 (T2): the AI holds the seat of a player who left (HOLAIHold.as: the base economy and the defence, no far attack, no
	 * expansion, no duel). The game state sets it when the window of absence opens and clears it when the player is back; a held
	 * seat thinks as long as it is absent (GetThinkingSlots) and is read as a Normal level (UHOLAIToolsLibrary::GetSlotLevel).
	 * Released: the script forgets what it remembered of the seat (ForgetSlot).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI")
	void SetHeld(int32 PlayerIndex, bool bHeld);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI")
	bool IsHeld(int32 PlayerIndex) const { return HeldSlots.Contains(PlayerIndex); }

	/**
	 * D718 (T5): the window of absence of a seat is over and the AI keeps it (FHOLDisconnectRules::GetWindowEnd = AIHolds): the seat leaves the
	 * hold mode (IsHeld false: the script plays its full brain), keeps thinking while it is absent, at the Normal level; what the brain remembered
	 * of it is kept. SetHeld(false) (the player is back, or the seat defeated) clears both.
	 */
	void SetFullBrain(int32 PlayerIndex);
	bool IsFullBrain(int32 PlayerIndex) const { return FullBrainSlots.Contains(PlayerIndex); }

	/**
	 * IA0.4 B2b (IA0.4-Q3 A): the AI slots whose brain is on (FHOLMatchSlot::bAIBrain of the game state's launch
	 * configuration), by player index: the ones the step makes think. Every AI slot without an AHOLGameState.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI")
	TArray<int32> GetThinkingSlots() const;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/**
	 * Slot PlayerIndex thinks, overridden in script (UFUNCTION(BlueprintOverride)). Whether the brain thought: the C++
	 * base has no rule and answers false.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|AI")
	bool Think(int32 PlayerIndex);
	virtual bool Think_Implementation(int32 PlayerIndex) { return false; }

	/** D717 (T2): the script drops all it remembered of slot PlayerIndex (overridden in script); the C++ base remembers nothing. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|AI")
	void ForgetSlot(int32 PlayerIndex);
	virtual void ForgetSlot_Implementation(int32 PlayerIndex) {}

private:
	/** Tail of each step: the AI slots whose turn it is think, spread over the interval by slot. */
	void OnStepEnded(int64 StepIndex);

	FDelegateHandle StepEndedHandle;
	/** D717 (T2): the seats the AI holds for a player who left. */
	TSet<int32> HeldSlots;
	/** D718 (T5): the seats kept by the AI after the window, in the full brain. */
	TSet<int32> FullBrainSlots;
	int32 NumThoughts = 0;
	/** IA0.5 B1: the "awake" line written once per world, at the first step (the AI PlayerStates exist from StartPlay). */
	bool bAwakeLogged = false;

	/** Bounded record of the step's thoughts: trimmed back to MaxStepThoughts when it doubles. */
	static constexpr int32 MaxStepThoughts = 256;
	TArray<FHOLAIStepThought> StepThoughts;
};
