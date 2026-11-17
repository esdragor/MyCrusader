// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Simulation/HOLVolcanoRules.h"
#include "HOLVolcanoRulesLibrary.generated.h"

/**
 * MIG-C-Volcano (CELEBORN-1117j): the script's read of HOLVolcanoRules.h, ENVELOPES ONLY (no rule here, a call and a copy each). The rules stay C++ (the volcano
 * subsystem steps them); the tests of them are script (Script/Tests/HOLVolcanoRules_Test.as). Script name: HOLVolcanoRules::<Function>.
 */

/** The numbers of FHOLVolcanoSettings the tests read or change; the defaults ARE FHOLVolcanoSettings' (one source). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLVolcanoTuning
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float WindowOpensAfter = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float WindowLength = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float WarningMin = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float WarningMax = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float LavaSpeedFactor = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float LavaHalfWidth = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float AltarAdvance = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float AltarHoldSeconds = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float AltarDecayRate = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float FreshDepositAlongMin = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float FreshDepositAlongMax = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float FreshDepositGoldShare = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float FreshDepositBigShare = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") FName FreshDepositGoldSmallType;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") FName FreshDepositGoldBigType;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") FName FreshDepositStoneSmallType;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") FName FreshDepositStoneBigType;

	FHOLVolcanoTuning();
	FHOLVolcanoSettings ToSettings() const;
};

/** HOLVolcano::FCycle. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLVolcanoCycleView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float WarningAt = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float EruptAt = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float EruptEndAt = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float CooledAt = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float ReArmAt = 0.f;
};

/** One flow: its polyline (HOLVolcano::FFlow). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLVolcanoFlowLine
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Volcano") TArray<FVector> Points;
};

/** A site as the map gives it (HOLVolcano::FSite). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLVolcanoSiteLayout
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Volcano") TArray<FHOLVolcanoFlowLine> Flows;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") bool bAltarPresent = false;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") FVector AltarLocation = FVector::ZeroVector;
};

/** HOLVolcano::FSiteState. Phase: EHOLVolcanoPhase as an integer (0 Dormant, 1 Armed, 2 Warning, 3 Erupting, 4 Cooling, 5 Rest). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLVolcanoStateView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Volcano") int32 Phase = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") int32 CycleIndex = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") int32 FlowIndex = INDEX_NONE;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float WarningAt = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float EruptAt = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float SecondsToErupt = -1.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float FrontDistance = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") FVector FrontPoint = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") bool bAltarLit = false;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") bool bWoken = false;
};

/** HOLVolcano::FObstacleBox. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLVolcanoBoxView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Volcano") FVector2D Centre = FVector2D::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float HalfLength = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float HalfWidth = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float YawDegrees = 0.f;
};

/** HOLVolcano::FFreshDeposit. Kind: 0 Gold, 1 Stone. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLVolcanoFreshView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Volcano") bool bPresent = false;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") int32 Kind = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") bool bBig = false;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") float Along = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") FVector Location = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "Volcano") FName TypeName;
};

/** Envelopes of HOLVolcanoRules.h for the script tests. A flow is its Points; no world needed. Altar holds: 0 Idle, 1 Holding, 2 Contested. */
UCLASS()
class HEROESOFLIBERTY_API UHOLVolcanoRulesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The Man-at-arms' speed read from the unit type table (the real chain of the lava's speed); 0 if the table has none. */
	UFUNCTION(BlueprintPure, Category = "Volcano") static float ManAtArmsSpeed();
	UFUNCTION(BlueprintPure, Category = "Volcano") static FHOLVolcanoCycleView MakeCycle(int32 Seed, int32 Index, float PreviousReArmAt, const FHOLVolcanoTuning& Tuning, float EruptionLength);
	UFUNCTION(BlueprintPure, Category = "Volcano") static int32 GetPhase(bool bCatastrophe, int32 Seed, float Now, const FHOLVolcanoTuning& Tuning, float EruptionLength);
	UFUNCTION(BlueprintPure, Category = "Volcano") static float FlowLength(const TArray<FVector>& Points);
	UFUNCTION(BlueprintPure, Category = "Volcano") static float EruptionDuration(const TArray<FVector>& Points, float Speed);
	UFUNCTION(BlueprintPure, Category = "Volcano") static float FrontDistance(const TArray<FVector>& Points, float Now, float EruptAt, float Speed);
	UFUNCTION(BlueprintPure, Category = "Volcano") static bool IsInLava(const TArray<FVector>& Points, float FrontDistance, float HalfWidth, const FVector& Point, float Radius);
	UFUNCTION(BlueprintPure, Category = "Volcano") static bool IsInCorridor(const TArray<FVector>& Points, float HalfWidth, const FVector& Point, float Radius);
	UFUNCTION(BlueprintPure, Category = "Volcano") static FVector EvacuationPoint(const TArray<FVector>& Points, float HalfWidth, const FVector& Point, float Radius, float Beyond);
	UFUNCTION(BlueprintPure, Category = "Volcano") static float EvacuationReach(float HalfWidth, float Radius);
	UFUNCTION(BlueprintPure, Category = "Volcano") static bool LavaDestroys(bool bIsHero, bool bHeroInDuel);
	UFUNCTION(BlueprintPure, Category = "Volcano") static FVector PointAt(const TArray<FVector>& Points, float Distance);
	UFUNCTION(BlueprintPure, Category = "Volcano") static float DistanceToFlow(const TArray<FVector>& Points, const FVector& Point);
	UFUNCTION(BlueprintPure, Category = "Volcano") static TArray<FHOLVolcanoBoxView> GetObstacleBoxes(const TArray<FVector>& Points, float HalfWidth);
	UFUNCTION(BlueprintPure, Category = "Volcano") static int32 DrawFlow(int32 Seed, int32 Index, int32 FlowCount);
	UFUNCTION(BlueprintPure, Category = "Volcano") static int32 SiteSeed(int32 MatchSeed, int32 SiteIndex);
	UFUNCTION(BlueprintCallable, Category = "Volcano") static int32 StepAltar(UPARAM(ref) float& HeldSeconds, int32 CampsInZone, float DeltaSeconds, const FHOLVolcanoTuning& Tuning);
	UFUNCTION(BlueprintCallable, Category = "Volcano") static int32 StepAltarOwned(UPARAM(ref) float& HeldSeconds, UPARAM(ref) int32& Holder, int32 CampsInZone, int32 SoleCamp, float DeltaSeconds, const FHOLVolcanoTuning& Tuning);
	UFUNCTION(BlueprintPure, Category = "Volcano") static FHOLVolcanoCycleView AdvanceForAltar(const FHOLVolcanoCycleView& Cycle, float HeldAt, const FHOLVolcanoTuning& Tuning);
	UFUNCTION(BlueprintPure, Category = "Volcano") static FHOLVolcanoFreshView FreshDeposit(int32 SiteSeedValue, int32 CycleIndex, const TArray<FVector>& Points, const FHOLVolcanoTuning& Tuning);
	/** WakeCycle INDEX_NONE: no altar held; else the cycle woken and the instant the altar was held (HOLVolcano::FWakes with one entry). */
	UFUNCTION(BlueprintPure, Category = "Volcano") static FHOLVolcanoStateView SiteStateAt(bool bCatastrophe, int32 MatchSeed, int32 SiteIndex, const FHOLVolcanoSiteLayout& Site, float Now, const FHOLVolcanoTuning& Tuning, float ManAtArmsSpeed, int32 WakeCycle, float WakeAt);
	UFUNCTION(BlueprintPure, Category = "Volcano") static bool FreshDepositDue(int32 Phase, int32 FlowIndex);
};
