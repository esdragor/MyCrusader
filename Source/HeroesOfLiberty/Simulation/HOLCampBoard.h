// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLCampRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLCampBoard.generated.h"

// The neutral camps' rule where the server thinks (bascule Camp, D830/D836 point 3: the script is the source). The structures and
// enumerations below are the bridge's own minimal copies of HOLCampRules' types (the C++ ones are not reflected and stay as they are:
// the bridge converts). The script subclass converts them again to its own (Script/Neutrals/HOLCampRules.as).

/** NQ1, as HOLCampRules::EAggro (same order). */
UENUM(BlueprintType)
enum class EHOLCampBoardAggro : uint8
{
	OnSight,
	OnAttack,
};

/** NQ2, as HOLCampRules::ECredit (same order). */
UENUM(BlueprintType)
enum class EHOLCampBoardCredit : uint8
{
	LastBlow,
	MostDamage,
	Shared,
};

/** As HOLCampRules::EState (same order). */
UENUM(BlueprintType)
enum class EHOLCampBoardState : uint8
{
	Guarding,
	Fighting,
	Returning,
	Cleared,
};

USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLCampBoardSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	EHOLCampBoardAggro Aggro = EHOLCampBoardAggro::OnSight;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	EHOLCampBoardCredit Credit = EHOLCampBoardCredit::Shared;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	float AggroRadius = 1500.f;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	float LeashRadius = 3000.f;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	float ArriveRadius = 400.f;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	int32 RewardXP = 100;
};

USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLCampBoardMember
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	int32 Id = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	FVector2D Location = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLCampBoardIntruder
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	int32 Id = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	int32 Owner = INDEX_NONE;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	FVector2D Location = FVector2D::ZeroVector;
};

/** The answer of one thought: the new state too (the C++ rule takes it by reference, the event cannot). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLCampBoardStep
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	EHOLCampBoardState State = EHOLCampBoardState::Guarding;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	bool bChanged = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	bool bJustCleared = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	TArray<int32> ReturnIds;
};

USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLCampBoardHit
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	int32 Owner = INDEX_NONE;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	int32 Damage = 0;
};

USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLCampBoardShare
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	int32 Owner = INDEX_NONE;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Camp")
	int32 XP = 0;
};

/**
 * The camps' rule for the server, as UHOLGarrisonBoard: UHOLCampSubsystem (C++, the units and the world) gives what a camp sees and
 * asks the world's board for the thought (Step) and the XP shares (Rewards); the script subclass (Script/Neutrals/HOLCampBoardScript.as)
 * answers by HOLCamp::Step / HOLCamp::Rewards of Script/Neutrals/HOLCampRules.as. Abstract: only a script subclass is born. Without
 * it the camps are inert (there is no C++ rule any more, D836 point 3): the default of each event keeps the state, gives no reward and
 * logs an error. Server and solo only.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLCampBoard : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's camp board (its script subclass), or null (no script compiled, or no world). */
	static UHOLCampBoard* Find(const UWorld* World);

	/** What the server applies: the board's thought when the script board exists, else the state is kept, nobody is sent back and an error is logged (the state goes in and out). */
	static HOLCampRules::FStep StepForCamp(const UWorld* World, HOLCampRules::EState& InOutState, const HOLCampRules::FSettings& Settings,
		const FVector2D& Post, const TArray<HOLCampRules::FMember>& Members, const TArray<HOLCampRules::FIntruder>& Intruders, bool bAttacked);

	/** What the server applies to a cleared camp: the board's shares when the script board exists, else no share and an error is logged. */
	static TArray<HOLCampRules::FShare> RewardsForCamp(const UWorld* World, const HOLCampRules::FSettings& Settings, const TArray<HOLCampRules::FHit>& Hits);

	/** The board's thought (the script's HOLCamp::Step). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Camp")
	FHOLCampBoardStep StepNow(EHOLCampBoardState State, const FHOLCampBoardSettings& Settings, FVector2D Post, const TArray<FHOLCampBoardMember>& Members,
		const TArray<FHOLCampBoardIntruder>& Intruders, bool bAttacked);

	/** The board's XP shares (the script's HOLCamp::Rewards). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Camp")
	TArray<FHOLCampBoardShare> RewardsNow(const FHOLCampBoardSettings& Settings, const TArray<FHOLCampBoardHit>& Hits);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** Overridden by the script board; the default is inert (state kept, no reward) and logs an error. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Camp")
	FHOLCampBoardStep CampStep(EHOLCampBoardState State, const FHOLCampBoardSettings& Settings, FVector2D Post, const TArray<FHOLCampBoardMember>& Members,
		const TArray<FHOLCampBoardIntruder>& Intruders, bool bAttacked);
	virtual FHOLCampBoardStep CampStep_Implementation(EHOLCampBoardState State, const FHOLCampBoardSettings& Settings, FVector2D Post,
		const TArray<FHOLCampBoardMember>& Members, const TArray<FHOLCampBoardIntruder>& Intruders, bool bAttacked);

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Camp")
	TArray<FHOLCampBoardShare> CampRewards(const FHOLCampBoardSettings& Settings, const TArray<FHOLCampBoardHit>& Hits);
	virtual TArray<FHOLCampBoardShare> CampRewards_Implementation(const FHOLCampBoardSettings& Settings, const TArray<FHOLCampBoardHit>& Hits);
};
