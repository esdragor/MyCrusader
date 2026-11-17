// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Match/HOLResources.h"
#include "HOLCommandGridReadLibrary.generated.h"

/** EHOLGridCommand (Player/HOLCommandGrid.h) as a UENUM, same order. */
UENUM(BlueprintType)
enum class EHOLGridCommandKind : uint8
{
	None,
	Produce,
	Build,
	CancelConstruction,
	AttackMove,
	Stop,
	Hold,
	HeroAbility,
	NecroticRage,
	Research,
	Trade,
	Deploy,
	Specialize,
	CancelSpecialization,
};

/** EHOLCommandFeedback as a UENUM, same order. */
UENUM(BlueprintType)
enum class EHOLCommandFeedbackKind : uint8
{
	None,
	NotEnoughResources,
	QueueFull,
	PopulationFull,
	PlacementRefused,
	NoBuilding,
	ResearchRefused,
};

/** EHOLGridKeyTarget as a UENUM, same order. */
UENUM(BlueprintType)
enum class EHOLGridKeyTargetKind : uint8
{
	None,
	Grid,
	Orders,
	Age,
	Build,
};

/** EHOLAgeEvent as a UENUM, same order. */
UENUM(BlueprintType)
enum class EHOLAgeEventKind : uint8
{
	Placed,
	PlacedKept,
	Cancelled,
	Escape,
	SelectionChanged,
};

/** The part of FHOLGridContext that the command grid tests set: the selection as the machine knows it (the buildable types as three parallel arrays). */
USTRUCT(BlueprintType)
struct FHOLGridContextView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bOwnBuilding = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bBuildingGroup = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bBuildingFinished = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bForumDown = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bMarket = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	TArray<FName> Produces;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bBuilders = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bOwnUnits = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bSoldiers = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bOnlyKings = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bHero = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 HeroLevel = 0;
	/** The buildable types, parallel arrays: type name, cell (0 to 11), age (0 to 3). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	TArray<FName> BuildableTypes;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	TArray<int32> BuildableCells;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	TArray<int32> BuildableAges;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 ReachedAge = 0;
};

/** FHOLGridCommand's fields the tests read. */
USTRUCT(BlueprintType)
struct FHOLGridCommandView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	EHOLGridCommandKind Command = EHOLGridCommandKind::None;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	FName TypeName;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 Age = -1;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bGreyed = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	EHOLResource Resource = EHOLResource::Food;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bBuy = false;
};

/** FHOLGridKeyResult as a USTRUCT. */
USTRUCT(BlueprintType)
struct FHOLGridKeyResultView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	EHOLGridKeyTargetKind Target = EHOLGridKeyTargetKind::None;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 ChosenAge = -1;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	FName TypeName;
};

/** FHOLCommandFeedback as a USTRUCT (the technology Block is not exposed: ResearchRefused is read through Kind only). */
USTRUCT(BlueprintType)
struct FHOLFeedbackView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	EHOLCommandFeedbackKind Kind = EHOLCommandFeedbackKind::None;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	EHOLResource Resource = EHOLResource::Food;
	/** EHOLPlacementRefusal as its number (the enum is not a UENUM); see GetPlacementRefusalBuilding. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 Refusal = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	EHOLBuildingCategory Category = EHOLBuildingCategory::Economic;
};

/** FHOLTradeGain as a USTRUCT. */
USTRUCT(BlueprintType)
struct FHOLTradeGainView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 Amount = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	EHOLResource Logo = EHOLResource::Gold;
};

/** FHOLGridCellFace as a USTRUCT. */
USTRUCT(BlueprintType)
struct FHOLGridCellFaceView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bIcon = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bName = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bCost = false;
};

/** FHOLCycleBuilding as a USTRUCT (the key is an int32 here: the uint32 key's bit pattern). */
USTRUCT(BlueprintType)
struct FHOLCycleBuildingView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 Rank = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 Key = 0;
};

/** FHOLQueueChoice as a USTRUCT (int32 key). */
USTRUCT(BlueprintType)
struct FHOLQueueChoiceView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 QueueLength = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 MaxQueue = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 Key = 0;
};

/** FHOLGroupMember as a USTRUCT. */
USTRUCT(BlueprintType)
struct FHOLGroupMemberView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	FName TypeName;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bAlone = false;
};

/** FHOLSubgroup as a USTRUCT. */
USTRUCT(BlueprintType)
struct FHOLSubgroupView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bAll = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	FName TypeName;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	TArray<int32> Members;
};

/**
 * The command grid's pure rules (HOLCommandGrid, Player/HOLCommandGrid.h, which STAY in C++: the HUD reads them, D836) read by Angelscript
 * (namespace HOLCommandGridRead there), so that the tests of HOLCommandGridTests.cpp can be ported to Script/Tests/HOLCommandGrid_Test.as
 * (MIG-C-CommandGrid, D830, D839). Wrappers only, no rule here: the C++ structs are copied to their USTRUCT twins and back.
 * NOT exposed yet (TYPES_A_EXPOSER): CheckResearch (HOLTechnology::FFacts / FHOLTechnologyRow), the hero cells and cooldowns of the context,
 * the specialization view, GetMouseCursor, the Specialize / report-name cells.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLCommandGridReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetNumCells();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetCancelCell();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetGridColumns();
	/** EHOLPlacementRefusal::Building as its number, for FHOLFeedbackView::Refusal. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetPlacementRefusalBuilding();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetCategoryRow(EHOLBuildingCategory Category);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetTradeCell(EHOLResource Resource, bool bBuy);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLTradeGainView GetTradeGain(EHOLResource Resource, bool bBuy, int32 Received);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FString GetTradeGainText(const FHOLTradeGainView& Gain);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLGridCommandView GetCommand(const FHOLGridContextView& Context, int32 Cell);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLGridCommandView GetOrder(const FHOLGridContextView& Context, int32 Cell);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsBuildGrid(const FHOLGridContextView& Context);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsAgeReached(const FHOLGridContextView& Context, int32 Age);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLGridCommandView GetAgeBuilding(const FHOLGridContextView& Context, int32 Age, int32 Cell);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLGridKeyResultView PressKey(const FHOLGridContextView& Context, int32 ChosenAge, int32 Cell);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLGridKeyResultView ClickAge(const FHOLGridContextView& Context, int32 ChosenAge, int32 Age, int32 Cell);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetChosenAgeAfter(int32 ChosenAge, EHOLAgeEventKind Event);

	/** The context with its Buildable arrays filled from the building type table (UHOLBuildingSettings::GetBuildableTypes). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLGridContextView WithTableBuildables(const FHOLGridContextView& Context);
	/** The Produces list of a type of the building type table (empty when the type is not in it). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static TArray<FName> GetProducesOf(FName TypeName);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsTypeInTable(FName TypeName);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static EHOLBuildingCategory GetTypeCategory(FName TypeName);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetCycleRankOfType(FName TypeName);
	/** HOLCommandGrid::IsSelectedByCategory for a type of the table. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsTypeSelectedByCategory(FName TypeName, EHOLBuildingCategory Category);
	/** HOLCommandGrid::IsSelectedByCategory for a row that has TypeCategory alone (a building not in the table). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsCategorySelectedByCategory(EHOLBuildingCategory TypeCategory, EHOLBuildingCategory Category);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool HasMissingResource(const FHOLResources& Stock, const FHOLResources& Cost);
	/** The first resource of Cost the stock does not cover; Gold when none is missing (read HasMissingResource first). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static EHOLResource GetMissingResource(const FHOLResources& Stock, const FHOLResources& Cost);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLFeedbackView CheckProduction(const FHOLResources& Stock, const FHOLResources& Cost, int32 QueueLength, int32 MaxQueue,
		int32 PopulationUsed, int32 PopulationCap, int32 Places);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLFeedbackView CheckBuild(const FHOLResources& Stock, const FHOLResources& Cost);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool Blocks(const FHOLFeedbackView& Feedback);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsPopulationBlocked(int32 PopulationUsed, int32 PopulationCap, int32 Places);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FString Describe(const FHOLFeedbackView& Feedback);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLGridCellFaceView GetCellFace(bool bHasIcon);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsRepeatPress(const TArray<int32>& Selected, const TArray<int32>& LastSelected, bool bSameKey);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 NextInCycle(const TArray<FHOLCycleBuildingView>& Buildings, int32 LastKey);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsAloneInGroup(FName TypeName);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static TArray<FHOLSubgroupView> SplitSubgroups(const TArray<FHOLGroupMemberView>& Members);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static TArray<int32> GetCommandPriority(const TArray<FHOLGroupMemberView>& Members);
	/** HOLCommandGrid::MergeCommands; PerBuilding is the buildings' cells end to end (GetNumCells each: building i, cell c at i * GetNumCells() + c). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static TArray<FHOLGridCommandView> MergeCommands(const TArray<FHOLGridCommandView>& PerBuilding, const TArray<int32>& Priority);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 PickEvenly(const TArray<FHOLQueueChoiceView>& Choices, int32 LastKey);
};
