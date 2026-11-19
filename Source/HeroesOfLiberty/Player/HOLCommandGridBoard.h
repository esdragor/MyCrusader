// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Player/HOLCommandGrid.h"
#include "Player/HOLCommandGridReadLibrary.h"
#include "HOLCommandGridBoard.generated.h"

/**
 * The command grid's rules where the game runs (MIG-CPP, lot G1 bridge; D839: a binding to expose). The cells' content is the script's
 * (Script/Player/HOLCommandGridBoard.as calls namespace HOLCommandGridRules of Script/Player/HOLCommandGridRules.as, tested against the C++ by the
 * agreement tests); this world subsystem asks its script subclass. Abstract: only a script subclass is born. HOLCommandGrid::GetCommand and GetOrder
 * (and every other bridged rule of HOLCommandGrid) ask it (Ask*): the rule is the script's alone, no C++ twin remains; with no script board in any
 * game world they answer a neutral default and log one line per function and world (CommandGrid: <Function> refused, no script grid board).
 * Same pattern as UHOLPlayerOrdersBoard and UHOLConstructionBoard. Game thread only (the script runs there; another thread keeps the C++ body).
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLCommandGridBoard : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The board of the first game or PIE world that has one (the grid's functions have no world argument), or null. */
	static UHOLCommandGridBoard* FindAny();

	/** True and Out filled when a script board answered; false: the caller keeps its C++ body. */
	static bool AskGetCommand(const FHOLGridContext& Context, int32 Cell, FHOLGridCommand& Out);
	static bool AskGetOrder(const FHOLGridContext& Context, int32 Cell, FHOLGridCommand& Out);
	// Family 1 (ages and keys): same contract, Out filled when a script board answered.
	static bool AskIsAgeReached(const FHOLGridContext& Context, int32 Age, bool& Out);
	static bool AskIsAgeOpen(const FHOLGridContext& Context, int32 Age, bool& Out);
	static bool AskGetAgeBuilding(const FHOLGridContext& Context, int32 Age, int32 Cell, FHOLGridCommand& Out);
	static bool AskPressKey(const FHOLGridContext& Context, int32 ChosenAge, int32 Cell, FHOLGridKeyResult& Out);
	static bool AskClickAge(const FHOLGridContext& Context, int32 ChosenAge, int32 Age, int32 Cell, FHOLGridKeyResult& Out);
	static bool AskGetChosenAgeAfter(int32 ChosenAge, EHOLAgeEvent Event, int32& Out);
	// Family 2 (checks and Describe). CheckResearch is NOT bridged (no script rule: its table rows are not exposed).
	static bool AskCheckProduction(const FHOLResources& Stock, const FHOLResources& Cost, int32 QueueLength, int32 MaxQueue, int32 PopulationUsed,
		int32 PopulationCap, int32 Places, FHOLCommandFeedback& Out);
	static bool AskCheckBuild(const FHOLResources& Stock, const FHOLResources& Cost, FHOLCommandFeedback& Out);
	static bool AskDescribe(const FHOLCommandFeedback& Feedback, FString& Out);
	// Family 3 (groups). Ask* return false (the caller keeps its C++ body) with no board, or when MergeCommands' input is not NumCells wide per building.
	static bool AskNextInCycle(TConstArrayView<FHOLCycleBuilding> Buildings, uint32 LastKey, int32& Out);
	static bool AskPickEvenly(TConstArrayView<FHOLQueueChoice> Choices, uint32 LastKey, int32& Out);
	static bool AskSplitSubgroups(TConstArrayView<FHOLGroupMember> Members, TArray<FHOLSubgroup>& Out);
	static bool AskMergeCommands(TConstArrayView<TArray<FHOLGridCommand>> PerBuilding, TConstArrayView<int32> Priority, TArray<FHOLGridCommand>& Out);
	// Family 4 (small rules, D.1): the row-reading ones pass the fields read (IsBuildable / BuildGridSlot, Category), never the row.
	static bool AskIsAloneInGroup(FName TypeName, bool& Out);
	static bool AskIsRepeatPress(TConstArrayView<uint32> Selected, TConstArrayView<uint32> LastSelected, bool bSameKey, bool& Out);
	static bool AskGetCommandPriority(TConstArrayView<FHOLGroupMember> Members, TArray<int32>& Out);
	static bool AskIsSelectedByCategory(EHOLBuildingCategory TypeCategory, EHOLBuildingCategory Category, bool& Out);
	static bool AskBlocks(const FHOLCommandFeedback& Feedback, bool& Out);
	static bool AskGetCycleRank(bool bBuildable, int32 BuildGridSlot, int32& Out);

	/** FHOLGridContext as the view the script reads (the inverse of the read library's ToRule). */
	static FHOLGridContextView MakeContextView(const FHOLGridContext& Context);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** The script subclass overrides these (HOLCommandGridRules::GetCommand / GetOrder); the default is an empty command (the board existing is the answer). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	FHOLGridCommandView GetCommandAnswer(const FHOLGridContextView& Context, int32 Cell);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	FHOLGridCommandView GetOrderAnswer(const FHOLGridContextView& Context, int32 Cell);
	virtual FHOLGridCommandView GetCommandAnswer_Implementation(const FHOLGridContextView& Context, int32 Cell);
	virtual FHOLGridCommandView GetOrderAnswer_Implementation(const FHOLGridContextView& Context, int32 Cell);

	// Family 1 (ages and keys; the rules of HOLCommandGridRules / HOLCommandGridKeys). Default = no answer (empty / false / INDEX_NONE).
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	bool IsAgeReachedAnswer(const FHOLGridContextView& Context, int32 Age);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	bool IsAgeOpenAnswer(const FHOLGridContextView& Context, int32 Age);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	FHOLGridCommandView GetAgeBuildingAnswer(const FHOLGridContextView& Context, int32 Age, int32 Cell);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	FHOLGridKeyResultView PressKeyAnswer(const FHOLGridContextView& Context, int32 ChosenAge, int32 Cell);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	FHOLGridKeyResultView ClickAgeAnswer(const FHOLGridContextView& Context, int32 ChosenAge, int32 Age, int32 Cell);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	int32 GetChosenAgeAfterAnswer(int32 ChosenAge, EHOLAgeEventKind Event);
	virtual bool IsAgeReachedAnswer_Implementation(const FHOLGridContextView& Context, int32 Age);
	virtual bool IsAgeOpenAnswer_Implementation(const FHOLGridContextView& Context, int32 Age);
	virtual FHOLGridCommandView GetAgeBuildingAnswer_Implementation(const FHOLGridContextView& Context, int32 Age, int32 Cell);
	virtual FHOLGridKeyResultView PressKeyAnswer_Implementation(const FHOLGridContextView& Context, int32 ChosenAge, int32 Cell);
	virtual FHOLGridKeyResultView ClickAgeAnswer_Implementation(const FHOLGridContextView& Context, int32 ChosenAge, int32 Age, int32 Cell);
	virtual int32 GetChosenAgeAfterAnswer_Implementation(int32 ChosenAge, EHOLAgeEventKind Event);

	// Family 2 (HOLCommandGridKeys::CheckProduction / CheckBuild / Describe). Default = no answer.
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	FHOLFeedbackView CheckProductionAnswer(const FHOLResources& Stock, const FHOLResources& Cost, int32 QueueLength, int32 MaxQueue, int32 PopulationUsed,
		int32 PopulationCap, int32 Places);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	FHOLFeedbackView CheckBuildAnswer(const FHOLResources& Stock, const FHOLResources& Cost);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	FString DescribeAnswer(const FHOLFeedbackView& Feedback);
	virtual FHOLFeedbackView CheckProductionAnswer_Implementation(const FHOLResources& Stock, const FHOLResources& Cost, int32 QueueLength, int32 MaxQueue,
		int32 PopulationUsed, int32 PopulationCap, int32 Places);
	virtual FHOLFeedbackView CheckBuildAnswer_Implementation(const FHOLResources& Stock, const FHOLResources& Cost);
	virtual FString DescribeAnswer_Implementation(const FHOLFeedbackView& Feedback);

	// Family 3 (HOLCommandGridGroups). Default = no answer. MergeCommands takes the buildings' cells flat (NumCells each, like the read library).
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	int32 NextInCycleAnswer(const TArray<FHOLCycleBuildingView>& Buildings, int32 LastKey);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	int32 PickEvenlyAnswer(const TArray<FHOLQueueChoiceView>& Choices, int32 LastKey);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	TArray<FHOLSubgroupView> SplitSubgroupsAnswer(const TArray<FHOLGroupMemberView>& Members);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	TArray<FHOLGridCommandView> MergeCommandsAnswer(const TArray<FHOLGridCommandView>& PerBuilding, const TArray<int32>& Priority);
	virtual int32 NextInCycleAnswer_Implementation(const TArray<FHOLCycleBuildingView>& Buildings, int32 LastKey);
	virtual int32 PickEvenlyAnswer_Implementation(const TArray<FHOLQueueChoiceView>& Choices, int32 LastKey);
	virtual TArray<FHOLSubgroupView> SplitSubgroupsAnswer_Implementation(const TArray<FHOLGroupMemberView>& Members);
	virtual TArray<FHOLGridCommandView> MergeCommandsAnswer_Implementation(const TArray<FHOLGridCommandView>& PerBuilding, const TArray<int32>& Priority);

	// Family 4 (HOLCommandGridGroups / HOLCommandGridKeys small rules). Default = no answer. Keys cross as int32 (the uint32 bits, like NextInCycle).
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	bool IsAloneInGroupAnswer(FName TypeName);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	bool IsRepeatPressAnswer(const TArray<int32>& Selected, const TArray<int32>& LastSelected, bool bSameKey);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	TArray<int32> GetCommandPriorityAnswer(const TArray<FHOLGroupMemberView>& Members);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	bool IsSelectedByCategoryAnswer(EHOLBuildingCategory TypeCategory, EHOLBuildingCategory Category);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	bool BlocksAnswer(const FHOLFeedbackView& Feedback);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|CommandGrid")
	int32 GetCycleRankAnswer(bool bBuildable, int32 BuildGridSlot);
	virtual bool IsAloneInGroupAnswer_Implementation(FName TypeName);
	virtual bool IsRepeatPressAnswer_Implementation(const TArray<int32>& Selected, const TArray<int32>& LastSelected, bool bSameKey);
	virtual TArray<int32> GetCommandPriorityAnswer_Implementation(const TArray<FHOLGroupMemberView>& Members);
	virtual bool IsSelectedByCategoryAnswer_Implementation(EHOLBuildingCategory TypeCategory, EHOLBuildingCategory Category);
	virtual bool BlocksAnswer_Implementation(const FHOLFeedbackView& Feedback);
	virtual int32 GetCycleRankAnswer_Implementation(bool bBuildable, int32 BuildGridSlot);
};
