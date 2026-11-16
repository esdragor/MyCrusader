// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLRelayRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLRelayBoard.generated.h"

/** One neutral relay's state for the script (the USTRUCT twin of FHOLRelayState, which stays a plain C++ struct): owner slot, taker slot, steps held. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLRelayBoardState
{
	GENERATED_BODY()

	/** The owner's slot, -1 for nobody. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Relay")
	int32 Owner = -1;

	/** The slot filling the count, -1 while nobody does. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Relay")
	int32 Taker = -1;

	/** Steps held toward the capture. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Relay")
	int32 Held = 0;
};

/** The territory XP of one step for the script: the whole XP to give now and the fractions carried to the next step. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLRelayXPResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Relay")
	int32 Award = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Relay")
	int32 Carry = 0;
};

/**
 * The relays' rules where the server applies a step (bascule Relay, D830/D836 point 3: the script is the source). The thinnest bridge, as
 * UHOLGarrisonBoard: UHOLRelaySubsystem::OnStepEnded counts the units of each zone (Mass / by-step: C++) and asks the world's board, once per relay
 * and per step, for the relay after the step (RelayStep) and, once per slot, for the territory XP of the step (RelayTerritoryXP). The script
 * subclass (Script/Relays/HOLRelayBoardScript.as) answers by HOLRelay::StepWith and HOLRelay::TerritoryXPAt of Script/Relays/HOLRelayRules.as.
 * Abstract: only a script subclass is born. Without it (no script compiled) the server keeps the C++ rules (HOLRelay::Step, HOLRelay::TerritoryXP),
 * and the default of each event is that same C++ rule, so nothing changes until the script board exists. NOT bridged: HOLRelay::MakeView and
 * FillPercent, which fill the REPLICATED FHOLRelayView (network exception: they stay C++). Server and solo only.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLRelayBoard : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's relay board (its script subclass), or null (no script compiled, or no world). */
	static UHOLRelayBoard* Find(const UWorld* World);

	/** The relay after one step: the board's answer when the script board exists, else HOLRelay::Step (the server's call, once per relay and per step). */
	static FHOLRelayState StepForRelay(const UWorld* World, const FHOLRelayState& State, const TArray<int32>& UnitsBySlot, const TArray<int32>& TeamBySlot,
		int32 HoldSteps, int32 DecaySteps);

	/** The territory XP of one step: the board's answer when the script board exists, else HOLRelay::TerritoryXP. Carry is updated in place. */
	static int32 TerritoryXPFor(const UWorld* World, int32& Carry, int32 Relays, int32 Centres, int32 StepsPerSecond);

	/** The board's answer for a relay (the native event below, i.e. the script rule in its script subclass). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Relay")
	FHOLRelayBoardState StepNow(const FHOLRelayBoardState& State, const TArray<int32>& UnitsBySlot, const TArray<int32>& TeamBySlot, int32 HoldSteps,
		int32 DecaySteps);

	/** The board's answer for the territory XP. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Relay")
	FHOLRelayXPResult TerritoryXPNow(int32 Carry, int32 Relays, int32 Centres, int32 StepsPerSecond);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** Overridden by the script board; the default is the C++ rule HOLRelay::Step (same answer). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Relay")
	FHOLRelayBoardState RelayStep(const FHOLRelayBoardState& State, const TArray<int32>& UnitsBySlot, const TArray<int32>& TeamBySlot, int32 HoldSteps,
		int32 DecaySteps);
	virtual FHOLRelayBoardState RelayStep_Implementation(const FHOLRelayBoardState& State, const TArray<int32>& UnitsBySlot, const TArray<int32>& TeamBySlot,
		int32 HoldSteps, int32 DecaySteps);

	/** Overridden by the script board; the default is the C++ rule HOLRelay::TerritoryXP (same answer). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Relay")
	FHOLRelayXPResult RelayTerritoryXP(int32 Carry, int32 Relays, int32 Centres, int32 StepsPerSecond);
	virtual FHOLRelayXPResult RelayTerritoryXP_Implementation(int32 Carry, int32 Relays, int32 Centres, int32 StepsPerSecond);
};
