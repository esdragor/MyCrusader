// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLOrderChainReadLibrary.generated.h"

/** The kind of a step of a chain of queued orders (mirrors HOLOrderChain::EStepKind, same order: converted by value). */
UENUM(BlueprintType)
enum class EHOLOrderChainStepKind : uint8
{
	Move,
	Attack,
	AttackMove,
	Build,
	Gather,
	Rally,
	AttackBuilding,
	Deposit,
};

/** One step as the script sees it (HOLOrderChain::FStep): TargetId is the uint32 of the rule, kept as int32 (identifiers are small). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLOrderChainStep
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|OrderChain")
	EHOLOrderChainStepKind Kind = EHOLOrderChainStepKind::Move;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|OrderChain")
	FVector Point = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|OrderChain")
	int32 TargetId = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|OrderChain")
	int32 SourceIndex = INDEX_NONE;
};

/** One unit's steps, current action first (HOLOrderChain::FUnitSteps). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLOrderChainUnitSteps
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|OrderChain")
	int32 UnitId = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|OrderChain")
	TArray<FHOLOrderChainStep> Steps;
};

/** A chain: its units, its steps (HOLOrderChain::FChain). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLOrderChain
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|OrderChain")
	TArray<int32> UnitIds;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|OrderChain")
	TArray<FHOLOrderChainStep> Steps;
};

/**
 * The rules of the chains of queued orders (HOLOrderChain, Simulation/HOLOrderChainRules.h, and HOLCombat::MaySeek, Simulation/HOLCombatRules.h,
 * C++, used by Mass) for Angelscript (namespace HOLOrderChainRead there): D836 keeps the rules in C++ and the script reads them here, so that
 * their tests (HOLOrderChainTests.cpp) can be ported to script (socle step 1, S1g, D830, D839). Wrappers only: no rule here.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLOrderChainReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLOrderChain::MaxSteps. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static int32 GetMaxSteps();

	/** HOLOrderChain::FStep::MakeMove (bAttackMove: an attack-move). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static FHOLOrderChainStep MakeMove(FVector Point, bool bAttackMove);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static FHOLOrderChainStep MakeAttack(int32 UnitId);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static FHOLOrderChainStep MakeBuild(int32 SiteId);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static FHOLOrderChainStep MakeAttackBuilding(int32 BuildingId);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static FHOLOrderChainStep MakeDeposit(int32 BuildingId);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static FHOLOrderChainStep MakeGather(int32 SourceIndex);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static FHOLOrderChainStep MakeRally(FVector Point);

	/** HOLOrderChain::MakeChains: units with the same steps (the first MaxSteps) share one chain, chains in the order of their first unit. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static TArray<FHOLOrderChain> MakeChains(const TArray<FHOLOrderChainUnitSteps>& Units);

	/** HOLOrderChain::KindName. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static FString KindName(EHOLOrderChainStepKind Kind);

	/** HOLOrderChain::IsCurrentOrderOver: its next queued order (Shift) may start. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static bool IsCurrentOrderOver(bool bWalkingOrder, bool bOrderedTarget, bool bOrderedBuilding, bool bAttackMoveOn, bool bDroppingOff);

	/** HOLCombat::MaySeek: the unit takes on enemies by itself. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|OrderChain")
	static bool MaySeek(bool bWalkingOrder, bool bAttackMove, bool bOrderedTarget, bool bOrderedBuilding, bool bWalkStalled);
};