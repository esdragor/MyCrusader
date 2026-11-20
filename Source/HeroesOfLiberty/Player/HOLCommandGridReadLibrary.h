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
	/** FHOLBuildable::bAlwaysOpen per buildable type (a Nomad's first forum); a missing entry is false. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	TArray<bool> BuildableAlwaysOpen;
	/** The technologies the selected building researches, in the table's order (FHOLGridContext::Researches). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	TArray<FName> Researches;
	/** The technologies the owner has researched (FHOLGridContext::Researched, D856). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	TArray<FName> Researched;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bDeployables = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bDeployedAny = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bRageUnit = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bPrimaryForum = false;
	/** FHOLGridContext::HeroCooldownSeconds[3], indexed by the hero ability number (GetHeroAbilityAura / CloseRanks / Banner); a missing entry is 0. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	TArray<int32> HeroCooldownSeconds;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 SpecializationTier = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 SpecializationPendingOption = -1;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 SpecializationRemainingMs = 0;
	/** FHOLGridContext::SpecializationFaction (D868): the Forum owner's EHOLFaction index, -1 unknown. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 SpecializationFaction = -1;
};

/** The facts of one technology asked (HOLTechnology::FFacts without bCanPay, which CheckResearch reads from the stock); the defaults are FFacts'. */
USTRUCT(BlueprintType)
struct FHOLTechFactsView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 HeroTier = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bRequiresDone = true;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bBuildingFinished = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bResearched = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bQueued = false;
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
	/** EHOLHeroAbility as its number (GetHeroAbilityAura / CloseRanks / Banner); the enum is not a UENUM. Default: the Aura's. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 Ability = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 CooldownSeconds = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	bool bPack = false;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 Tier = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 Option = -1;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 RemainingMs = 0;
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
	/** HOLTechnology::EBlock as its number (GetBlockNone / TierTooLow / ...); the enum is not a UENUM. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|CommandGrid")
	int32 Block = 0;
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
	static bool IsBuildGrid(const FHOLGridContextView& Context);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsAgeReached(const FHOLGridContextView& Context, int32 Age);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLGridCommandView GetAgeBuilding(const FHOLGridContextView& Context, int32 Age, int32 Cell);

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
	/** Table reads for the script (D839, a binding): the row's IsBuildable(), and its BuildGridSlot (INDEX_NONE for a type not in the table). No rule in them. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsTypeBuildable(FName TypeName);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetBuildGridSlotOfType(FName TypeName);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool HasMissingResource(const FHOLResources& Stock, const FHOLResources& Cost);
	/** The first resource of Cost the stock does not cover; Gold when none is missing (read HasMissingResource first). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static EHOLResource GetMissingResource(const FHOLResources& Stock, const FHOLResources& Cost);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsPopulationBlocked(int32 PopulationUsed, int32 PopulationCap, int32 Places);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLGridCellFaceView GetCellFace(bool bHasIcon);


	// ---- MIG-C-CommandGrid, 2nd lot (GALADRIEL-1117n): the orders' cells, the hero, the ages, the cursor, the research. NOT COMPILED when written. ----
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetAttackMoveCell();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetStopCell();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetHoldCell();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetDeployCell();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetResearchFirstCell();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetNumAges();

	/** The EHOLHeroAbility values as numbers, for FHOLGridCommandView::Ability and the HeroCooldownSeconds index. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetHeroAbilityAura();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetHeroAbilityCloseRanks();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetHeroAbilityBanner();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetHeroAbilityCell(int32 Ability);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetRageCell();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetSpecializationCell(int32 Option);
	/** HOLCommandGrid::GetReportName of a command (None for a command that is not a specialization cell). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FName GetReportName(const FHOLGridCommandView& Command);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static float GetAgePanelOpacity(const FHOLGridContextView& Context, int32 Age);

	/** HOLCommandGrid::GetMouseCursor as EMouseCursor's number; compare with GetCursorCrosshairs / GetCursorDefault. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetMouseCursor(bool bAttackMoveArmed, bool bPlacing);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetCursorCrosshairs();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetCursorDefault();

	/** UHOLTechnologySettings::GetBuildingTechnologies(Building): the building's research row, in the table's order. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static TArray<FName> GetBuildingResearches(FName Building);

	/** The Requires column of a technology of the table (D856: what chains level II to level I); None for a first level or a technology not in the table. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FName GetTechnologyRequires(FName Technology);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsTechnologyInTable(FName Technology);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static bool IsTechnologyImplemented(FName Technology);
	/** The Food of the technology's cost (-1 when it is not in the table). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetTechnologyFoodCost(FName Technology);
	/** HOLCommandGrid::CheckResearch of a technology of the table (an empty feedback when it is not in it: read IsTechnologyInTable first). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static FHOLFeedbackView CheckResearch(FName Technology, const FHOLTechFactsView& Facts, const FHOLResources& Stock, int32 QueueLength, int32 MaxQueue);
	/** HOLTechnology::EBlock as numbers, for FHOLFeedbackView::Block. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetBlockNone();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetBlockNotImplemented();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetBlockAlreadyResearched();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetBlockAlreadyQueued();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetBlockTierTooLow();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetBlockRequiresMissing();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|CommandGrid")
	static int32 GetBlockNoBuilding();
};
