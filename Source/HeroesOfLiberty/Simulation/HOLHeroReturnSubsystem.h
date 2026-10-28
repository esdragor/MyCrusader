// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLHeroReturnSubsystem.generated.h"

/**
 * The hero's return decision (J8a.4 loop 3, J8a4-Q6 = A): the C++ part only gives the step; the rule and the decision
 * live in a script subclass (Script/Hero/HOLHeroReturnRule.as) that overrides Decide. Abstract: only a script subclass is
 * born, in each game or PIE world (as UHOLAIBrainSubsystem, IA0.4). UHOLHeroSubsystem calls DecideNow at the end of each
 * simulation step, right after its sheets are written (a fixed order no delegate binding can change), where the
 * simulation runs only (solo, server): a client never decides.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLHeroReturnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's return decision (its script subclass), or null (no script compiled, or no world). */
	static UHOLHeroReturnSubsystem* Find(const UWorld* World);

	/** The decision at the end of step Step (UHOLHeroSubsystem calls it). Whether a script decided; false on a client. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Hero")
	bool DecideNow(int32 Step);

	/**
	 * J8a.4 loop 4: the slot's player buys its hero's return at step Step (an order taken from the queue by
	 * UHOLHeroSubsystem, the same step as DecideNow's). Whether it was bought (gold taken, hero asked); false on a client.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Hero")
	bool BuyNow(int32 Slot, int32 Step);

	/** Steps a script decided so far. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero")
	int32 GetNumDecisions() const { return NumDecisions; }

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/**
	 * The dead heroes' return at the end of step Step, overridden in script (UFUNCTION(BlueprintOverride)): ready once the
	 * delay is over, back by themselves at price 0. Whether it decided: the C++ base has no rule and answers false.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Hero")
	bool Decide(int32 Step);
	virtual bool Decide_Implementation(int32 Step) { return false; }

	/**
	 * The slot's paid return at step Step, overridden in script: the rules (ready, price at Step, place) and then
	 * HOLHero::BuyHeroReturn, which takes the gold and asks the birth in one block. The C++ base buys nothing (false).
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Hero")
	bool Buy(int32 Slot, int32 Step);
	virtual bool Buy_Implementation(int32 Slot, int32 Step) { return false; }

private:
	int32 NumDecisions = 0;
};
