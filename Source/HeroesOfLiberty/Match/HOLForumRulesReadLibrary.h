// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLForumRulesReadLibrary.generated.h"

/**
 * FHOLForumState (Match/HOLForumRules.h, USTRUCT but not BlueprintType, replicated) as plain script fields: the phase and the hold are
 * their enum's value (EHOLForumPhase: 0 Standing, 1 Ruin, 2 Rebuilding; EHOLForumHold: 0 None, 1 Holding, 2 Contested, 3 Receding).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLForumStateRead
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 Falls = 0;

	/** EHOLForumPhase as an int: 0 Standing, 1 Ruin, 2 Rebuilding. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 Phase = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 RuinStepsLeft = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int64 RebuildWork = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 HoldSteps = 0;

	/** EHOLForumHold as an int: 0 None, 1 Holding, 2 Contested, 3 Receding. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 Hold = 0;

	/** The attacker's slot whose colour fills the ring; -1 (INDEX_NONE) if unknown. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 HolderSlot = -1;
};

/** FHOLForumTuning (plain C++ struct there) as script fields; the defaults are the struct's (a bare FHOLForumTuning). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLForumTuningRead
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 Tier = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 RuinSteps = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 RebuildSteps = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	float BuilderExponent = 0.63f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	float StepSeconds = 0.1f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 HoldSteps = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 HoldDecaySteps = 2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	float HoldRadius = 1200.f;
};

/** HOLForumRules::FStepResult as script fields. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLForumStepResultRead
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	bool bPhaseChanged = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	bool bRazed = false;

	/** Lives left after a raze (0: defeated); -1 (INDEX_NONE) when nothing razed. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 Lives = -1;
};

/** FHOLForumBlock (the selection block of a main forum) as script fields; Text is the FText's source string, lines joined as the rule built them. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLForumBlockRead
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	bool bShown = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	bool bHideHealth = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	FString Shown;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	FString Text;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Forum")
	int32 Lives = 0;
};

/**
 * The main forum's lives rule (HOLForumRules, Match/HOLForumRules.h, pure C++, same module as this library) for Angelscript (namespace
 * HOLForumRulesRead there): the rule stays in C++ (D836, FHOLForumState is replicated) and the script reads it here, so that the pure tests
 * of HOLVictoryTests.cpp (Forum.Rules, Forum.Hold.Rules, Forum.Hold.Display, Forum.Block, Forum.Look) can be ported (D830, D839).
 * Wrappers only: no rule here. The state and the tuning go in and out as the plain structs above (a copy of the rule's, written back).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLForumRulesReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLForumRules::BaseLives. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int32 GetBaseLives();

	/** FHOLForumTuning::FromSettings: the game's values (DefaultGame.ini and the step's length). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static FHOLForumTuningRead GetGameTuning();

	/** HOLForumRules::GetLives(Tier, Falls). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int32 GetLivesByTier(int32 Tier, int32 Falls);

	/** HOLForumRules::GetLives(State, Tuning). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int32 GetLives(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetRebuildTotalWork. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int64 GetRebuildTotalWork(const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetRebuildWorkPerStep. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int32 GetRebuildWorkPerStep(int32 Builders, float Exponent);

	/** HOLForumRules::GetStepsToRebuild. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int32 GetStepsToRebuild(const FHOLForumTuningRead& Tuning, int32 Builders);

	/** HOLForumRules::GetRebuildProgress. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static float GetRebuildProgress(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetRebuildPercent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int32 GetRebuildPercent(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetRuinSecondsLeft. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int32 GetRuinSecondsLeft(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::Fall: the lives left (0 defeated), -1 if the forum was not standing. State is updated. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Forum")
	static int32 Fall(UPARAM(ref) FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::Advance: true when the phase changed. State is updated. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Forum")
	static bool Advance(UPARAM(ref) FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning, int32 Builders);

	/** HOLForumRules::Step with FHOLForumPresence {bAttackers, bDefenders, AttackerSlot}. State is updated. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Forum")
	static FHOLForumStepResultRead Step(UPARAM(ref) FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning, int32 Builders,
		bool bAttackers, bool bDefenders, int32 AttackerSlot);

	/** HOLForumRules::Raze: the lives left (0 defeated), -1 if the forum was standing. State is updated. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Forum")
	static int32 Raze(UPARAM(ref) FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::Skip: false when standing. State is updated. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Forum")
	static bool Skip(UPARAM(ref) FHOLForumStateRead& State);

	/** HOLForumRules::GetHoldSeconds. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int32 GetHoldSeconds(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetHoldProgress. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static float GetHoldProgress(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::ShowsHold. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static bool ShowsHold(const FHOLForumStateRead& State);

	/** HOLForumRules::DescribeHold. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static FString DescribeHold(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetRing(...).bShown. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static bool IsRingShown(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetRing(...).Fill. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static float GetRingFill(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetRing(...).HolderSlot. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int32 GetRingHolderSlot(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetRing(...).Shown: "none", "fill 0.50 holder slot 1", "fill 0.50 grey", "fill 0.25 faded slot 1". */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static FString GetRingShown(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetSelectionBlock; bIsMainForum false passes the null state (a secondary centre: nothing shown). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static FHOLForumBlockRead GetSelectionBlock(bool bIsMainForum, const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning,
		bool bOwn, int32 Builders);

	/** HOLForumRules::GetLook(...): Look as int (0 Standing, 1 Rubble, 2 Site). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static int32 GetLook(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::GetLook(...).Progress. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static float GetLookProgress(const FHOLForumStateRead& State, const FHOLForumTuningRead& Tuning);

	/** HOLForumRules::BuildersWait. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Forum")
	static bool BuildersWait(const FHOLForumStateRead& State);
};
