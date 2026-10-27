// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLAIBrain.generated.h"

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

	/** The AI slots of the match (bot player states), by player index; empty without a game state. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI")
	TArray<int32> GetAISlots() const;

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

private:
	/** Tail of each step: the AI slots whose turn it is think, spread over the interval by slot. */
	void OnStepEnded(int64 StepIndex);

	FDelegateHandle StepEndedHandle;
	int32 NumThoughts = 0;
};
