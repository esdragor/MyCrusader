// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLMovementRulesLibrary.generated.h"

/**
 * MIG-C-Mouvement (CELEBORN-1117k): the script's read of HOLAvoidRules.h and HOLSanityRules.h, ENVELOPES ONLY (no rule here: a copy in, a call, a copy out).
 * The rules stay C++ (D836, D838: the avoidance runs per entity and per step in the Mass processors; the sanity audit reads the same traces); the tests of
 * them are script (Script/Tests/HOLAvoidRules_Test.as, HOLSanityRules_Test.as). Script name: HOLMovementRules::<Function>.
 * Flat values only: a lambda of the C++ (IsClear) becomes a table of bools indexed by turn, an enum an int32.
 */

/** HOLAvoid::FFootprintBox. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLFootprintBoxView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Movement") FVector2D Centre = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") FVector2D HalfExtent = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") double Cos = 1.;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") double Sin = 0.;
};

/** HOLAvoid::FCrossingChoice (Role: 0 None, 1 KeepCourse, 2 PassBehind). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLCrossingChoiceView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Movement") int32 Role = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") double Side = 0.;
};

/** HOLAvoid::FLoopPatience. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLLoopPatienceView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Movement") FVector2D Heading = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") double Turn = 0.;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") double WaitSeconds = 0.;
};

/** HOLSanity::FTrace (the fields the rules read); Flanking and PlaceWalk may stay empty. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLSanityTraceView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Movement") float Radius = 60.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") TArray<FVector2D> Places;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") TArray<uint8> Ordered;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") TArray<uint8> Flanking;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") TArray<uint8> PlaceWalk;
};

/** HOLSanity::FSettings; the defaults ARE HOLSanity::FSettings' (one source). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLSanitySettingsView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Movement") float MinMove = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float SpinTurn = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float SpinNetRadii = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float SpinMinPathRadii = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float ShakePerSecond = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float ShakeNetShare = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float JitterZigzagPerSecond = 0.f;

	FHOLSanitySettingsView();
};

/** HOLSanity::FResult. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLSanityResultView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Movement") bool bSpin = false;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") bool bShake = false;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float Path = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float Net = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float Turn = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float SignedTurn = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float Zigzag = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") int32 Reversals = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") float OrderedShare = 0.f;
};

/** HOLSanity::FStall (its counters; the state is the caller's). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLStallView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Movement") double Seconds = 0.;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") double Longest = 0.;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") bool bTold = false;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") int32 Runs = 0;
};

/** HOLSanity::FNoStrike (its counters; the state is the caller's). Starts at LastHits -1 like the rule. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLNoStrikeView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Movement") double Seconds = 0.;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") double Longest = 0.;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") int32 LastHits = -1;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") bool bTold = false;
	UPROPERTY(BlueprintReadWrite, Category = "Movement") int32 Runs = 0;
};

/** Envelopes of HOLAvoidRules.h and HOLSanityRules.h for the script tests. Doubles stay doubles (the rules are double). */
UCLASS()
class HEROESOFLIBERTY_API UHOLMovementRulesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ---- HOLAvoid ----
	UFUNCTION(BlueprintPure, Category = "Movement") static int32 EaseBackSteps();
	UFUNCTION(BlueprintPure, Category = "Movement") static double ChooseDetourSide(const FVector2D& Ahead, const FVector2D& ToBlocker, double ContactDistance);
	UFUNCTION(BlueprintPure, Category = "Movement") static FHOLCrossingChoiceView ChooseCrossingRole(const FVector2D& MyPosition, const FVector2D& MyVelocity, const FVector2D& OtherPosition, const FVector2D& OtherVelocity);
	/** HOLAvoid::NextDetourTurn: ClearByTurn[Turn] is IsClear(Turn); a turn past the table is not clear. */
	UFUNCTION(BlueprintCallable, Category = "Movement") static int32 NextDetourTurn(int32 RememberedTurn, int32 MaxTurns, UPARAM(ref) int32& ClearSteps, const TArray<bool>& ClearByTurn);
	UFUNCTION(BlueprintPure, Category = "Movement") static int32 TurnWithoutClearWay(int32 RememberedTurn, int32 MaxTurns, bool bEnemyAhead);
	UFUNCTION(BlueprintPure, Category = "Movement") static double TimeToContact(const FVector2D& RelativePosition, const FVector2D& RelativeVelocity, double ContactDistance, bool bPressedCounts);
	/** The largest double: HOLAvoid::TimeToContact's "never". */
	UFUNCTION(BlueprintPure, Category = "Movement") static double TimeNever();
	UFUNCTION(BlueprintPure, Category = "Movement") static bool IsHeadOn(const FVector2D& Ahead, const FVector2D& ToOther, const FVector2D& OtherVelocity, double MaxDistance, double MinClosing);
	UFUNCTION(BlueprintPure, Category = "Movement") static bool IsPlaceInBody(const FVector2D& Place, const FVector2D& BodyPosition, double Radius);
	UFUNCTION(BlueprintPure, Category = "Movement") static bool IsSlotBlockedSeen(int32 bSlotBlocked, int32 DetourOrderSerial, int32 DetourPhase, int32 OrderSerial, int32 OrderPhase);
	UFUNCTION(BlueprintPure, Category = "Movement") static double GetFootprintDistance(const FHOLFootprintBoxView& Box, double Keep, const FVector2D& Point);
	UFUNCTION(BlueprintPure, Category = "Movement") static bool LeadsIntoFootprint(const FHOLFootprintBoxView& Box, double Keep, const FVector2D& Position, const FVector2D& Way, double Distance);
	UFUNCTION(BlueprintCallable, Category = "Movement") static bool StepLoopPatience(UPARAM(ref) FHOLLoopPatienceView& State, const FVector2D& Asked, double DeltaTime, double PatienceDegrees, double WaitSeconds, double ForgetSeconds);
	UFUNCTION(BlueprintPure, Category = "Movement") static bool RemembersMemberDetour(int32 MaxSteps);
	UFUNCTION(BlueprintCallable, Category = "Movement") static bool IsMemberDetourOver(UPARAM(ref) int32& MemberSteps, int32 MaxSteps);

	// ---- HOLSanity ----
	/** HOLSanity::Analyse. */
	UFUNCTION(BlueprintPure, Category = "Movement") static FHOLSanityResultView Analyse(const FHOLSanityTraceView& Trace, float StepSeconds, const FHOLSanitySettingsView& Settings);
	/** The loop of the C++ test's JudgeTrack: each sample Added to a HOLSanity::FUnitTrack (default FSanityRules) then Judged; the first fault, or empty. Motion: 0 Ordered, 1 Idle, 2 Busy. */
	UFUNCTION(BlueprintPure, Category = "Movement") static FString JudgeTrack(int32 Motion, const TArray<double>& Times, const TArray<FVector2D>& Places);
	UFUNCTION(BlueprintPure, Category = "Movement") static double StallLimitSeconds(double BaseSeconds, bool bFinalLegWalk, double FinalLegSeconds, double MarginSeconds);
	UFUNCTION(BlueprintCallable, Category = "Movement") static bool StepStall(UPARAM(ref) FHOLStallView& State, bool bStalledWithRoom, double StepSeconds, double LimitSeconds);
	UFUNCTION(BlueprintCallable, Category = "Movement") static bool StepNoStrike(UPARAM(ref) FHOLNoStrikeView& State, bool bInReachOfLivingTarget, int32 Hits, double StepSeconds, double LimitSeconds);
	UFUNCTION(BlueprintPure, Category = "Movement") static TArray<FString> FindColourClashes(const TArray<int32>& Neutrals, const TArray<int32>& PlayerSlots, const TArray<uint8>& SlotColorIndices);
};
