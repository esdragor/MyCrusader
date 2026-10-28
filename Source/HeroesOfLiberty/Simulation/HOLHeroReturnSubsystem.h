// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLHeroReturnSubsystem.generated.h"

/** What a quote of the hero's return knows of him (J8a.5 loop 1b). */
UENUM(BlueprintType)
enum class EHOLHeroReturnQuoteState : uint8
{
	/** Not known here: no hero, another player's death step on a client, or no server step yet. Never a price. */
	Unknown,
	Alive,
	/** Dead, an order sent now would come before the ready step. */
	Waiting,
	/** Dead, an order sent now would be taken at or after the ready step: priced. */
	Ready,
};

/**
 * The hero's return quoted for an order sent at step Step (J8a.5 loop 1b): the queue takes it at the next step
 * (TakeStep = Step + 1), so every field is at that step, the price that the purchase would pay (HOLHeroReturnRule.Buy).
 * Filled in script (Script/Hero/HOLHeroReturnQuote.as), read by the hero's block of the HUD on every machine.
 */
USTRUCT(BlueprintType)
struct FHOLHeroReturnQuote
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Hero")
	EHOLHeroReturnQuoteState State = EHOLHeroReturnQuoteState::Unknown;

	/** The step that takes an order sent now (Step + 1), -1 when unknown. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Hero")
	int32 TakeStep = -1;

	/** Steps from TakeStep to the ready step (0 once Ready), -1 alive or unknown. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Hero")
	int32 StepsToReady = -1;

	/** StepsToReady in whole seconds, rounded up (-1 alive or unknown). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Hero")
	int32 SecondsToReady = -1;

	/** Gold an order sent now would pay (the price at TakeStep), -1 unless Ready. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Hero")
	int32 Price = -1;

	/** The slot's gold, -1 when unknown (another player's on a client) or without stock (an AI slot). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Hero")
	int32 Gold = -1;

	/** A forum to come back to (HOLHeroReturn::ReturnPlace answers one). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Hero")
	bool bHasPlace = false;

	/** An order sent now would be bought: Ready, the slot in the match, gold known and enough, a place. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Hero")
	bool bCanBuy = false;
};

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

	/**
	 * J8a.5 loop 1b: the slot's hero's return quoted for an order sent at step Step (FHOLHeroReturnQuote), on every machine
	 * (a client quotes what it knows). The HUD calls the script's HOLHeroReturnQuote::Read directly; this door is for C++
	 * (the automation tests on the real chain). Unknown without a script.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Hero")
	FHOLHeroReturnQuote QuoteNow(int32 Slot, int32 Step) { return Quote(Slot, Step); }

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

	/** The quote of QuoteNow, overridden in script (HOLHeroReturnQuote::Read). The C++ base knows nothing (Unknown). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Hero")
	FHOLHeroReturnQuote Quote(int32 Slot, int32 Step);
	virtual FHOLHeroReturnQuote Quote_Implementation(int32 Slot, int32 Step) { return FHOLHeroReturnQuote(); }

private:
	int32 NumDecisions = 0;
};
