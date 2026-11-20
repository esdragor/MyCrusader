// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLDeployReadLibrary.generated.h"

/** HOLDeploy::EPhase (Siege/HOLDeployRules.h) as a UENUM, same order. */
UENUM(BlueprintType)
enum class EHOLDeployPhase : uint8
{
	Packed,
	Deploying,
	Deployed,
	Packing,
};

/** HOLDeploy::EOrder as a UENUM, same order. */
UENUM(BlueprintType)
enum class EHOLDeployOrder : uint8
{
	Attack,
	Move,
};

/** HOLDeploy::EAction as a UENUM, same order. */
UENUM(BlueprintType)
enum class EHOLDeployAction : uint8
{
	Wait,
	March,
	Deploy,
	Pack,
	Fire,
	CancelDeploy,
	Refuse,
};

/** HOLDeploy::FProfile as a USTRUCT: the two delays (ms) and the range (cm). */
USTRUCT(BlueprintType)
struct FHOLDeployProfile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Deploy")
	int32 DeployMs = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Deploy")
	int32 PackMs = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Deploy")
	int32 MinRangeCm = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Deploy")
	int32 MaxRangeCm = 0;
};

/** HOLDeploy::FState as a USTRUCT: the phase and the time a change of state has lasted (ms). */
USTRUCT(BlueprintType)
struct FHOLDeployState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Deploy")
	EHOLDeployPhase Phase = EHOLDeployPhase::Packed;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Deploy")
	int32 ElapsedMs = 0;
};

/** HOLDeploy::FDecision as a USTRUCT: the action, why when refused, the phase right after. */
USTRUCT(BlueprintType)
struct FHOLDeployDecision
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Deploy")
	EHOLDeployAction Action = EHOLDeployAction::Wait;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Deploy")
	int32 Reason = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Deploy")
	EHOLDeployPhase NextPhase = EHOLDeployPhase::Packed;
};

/**
 * The deployable siege engine's rule that STAYS in C++ (HOLDeploy, Siege/HOLDeployRules.h, read by Mass) read by Angelscript (namespace
 * HOLDeployRead there): D836 keeps the rule in C++, so that its 9 tests, now in Script/Tests/HOLDeployRules_Test.as, run against the real rule (socle
 * step 1f, D830, D839). Wrappers only: no rule here; the C++ enums and structs are copied to their UENUM/USTRUCT twins and back.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLDeployReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLDeploy::TooClose, the reason of a refusal under the minimum range. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static int32 GetTooClose();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static EHOLDeployPhase PhaseAt(int32 Index);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static FHOLDeployProfile DefaultProfile();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static bool CanFire(const FHOLDeployState& State);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static bool CanMove(const FHOLDeployState& State);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static bool CanAct(const FHOLDeployState& State);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static bool CanAim(const FHOLDeployProfile& Profile, int32 DistanceCm);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static int32 Shortened(int32 BaseMs, int32 CounterweightPercent);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static int32 DeployDelayMs(const FHOLDeployProfile& Profile, int32 CounterweightPercent);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static int32 PackDelayMs(const FHOLDeployProfile& Profile, int32 CounterweightPercent);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static int32 ButtonLabel(EHOLDeployPhase Phase);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static int32 ProgressPermille(const FHOLDeployState& State, const FHOLDeployProfile& Profile, int32 CounterweightPercent);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static FHOLDeployState Advance(const FHOLDeployState& State, int32 DtMs, const FHOLDeployProfile& Profile, int32 CounterweightPercent);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static FHOLDeployDecision Decide(const FHOLDeployState& State, EHOLDeployOrder Order, int32 DistanceCm, const FHOLDeployProfile& Profile);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static FHOLDeployDecision Toggle(const FHOLDeployState& State);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Deploy")
	static FHOLDeployState Apply(const FHOLDeployState& State, const FHOLDeployDecision& Decision);
};