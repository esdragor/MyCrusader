// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLCommandGridBoard.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

// The read library's View -> rule conversion (external linkage, HOLCommandGridReadLibrary.cpp): the script's answer is a FHOLGridCommandView.
namespace HOLCommandGridReadLibraryPrivate
{
	FHOLGridCommand ToRule(const FHOLGridCommandView& View);
	FHOLFeedbackView FromRule(const FHOLCommandFeedback& Feedback);
	FHOLGridCommandView FromRule(const FHOLGridCommand& Command);
	FHOLCommandFeedback ToRule(const FHOLFeedbackView& View);
}

namespace HOLCommandGridBoardPrivate
{
	/** FHOLGridKeyResultView -> rule (the read library has only the other way; same enum order). */
	FHOLGridKeyResult ToKeyResult(const FHOLGridKeyResultView& View)
	{
		FHOLGridKeyResult Out;
		Out.Target = static_cast<EHOLGridKeyTarget>(static_cast<uint8>(View.Target));
		Out.ChosenAge = View.ChosenAge;
		Out.TypeName = View.TypeName;
		return Out;
	}
}

using namespace HOLCommandGridBoardPrivate;

UHOLCommandGridBoard* UHOLCommandGridBoard::FindAny()
{
	// Game thread only: the script runs there; a read from another thread keeps the C++ body.
	if (!GEngine || !IsInGameThread())
	{
		return nullptr;
	}
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		const UWorld* World = Context.World();
		if (!World)
		{
			continue;
		}
		const TArray<UHOLCommandGridBoard*> Boards = World->GetSubsystemArrayCopy<UHOLCommandGridBoard>();
		if (Boards.Num() > 0)
		{
			return Boards[0];
		}
	}
	return nullptr;
}

FHOLGridContextView UHOLCommandGridBoard::MakeContextView(const FHOLGridContext& Context)
{
	FHOLGridContextView Out;
	Out.bOwnBuilding = Context.bOwnBuilding;
	Out.bBuildingGroup = Context.bBuildingGroup;
	Out.bBuildingFinished = Context.bBuildingFinished;
	Out.bForumDown = Context.bForumDown;
	Out.bMarket = Context.bMarket;
	Out.Produces = Context.Produces;
	Out.Researches = Context.Researches;
	Out.Researched = Context.Researched;
	Out.bBuilders = Context.bBuilders;
	Out.bOwnUnits = Context.bOwnUnits;
	Out.bSoldiers = Context.bSoldiers;
	Out.bOnlyKings = Context.bOnlyKings;
	Out.bDeployables = Context.bDeployables;
	Out.bDeployedAny = Context.bDeployedAny;
	Out.bHero = Context.bHero;
	Out.bRageUnit = Context.bRageUnit;
	Out.HeroLevel = Context.HeroLevel;
	Out.ReachedAge = Context.ReachedAge;
	Out.bPrimaryForum = Context.bPrimaryForum;
	Out.SpecializationTier = Context.SpecializationTier;
	Out.SpecializationPendingOption = Context.SpecializationPendingOption;
	Out.SpecializationRemainingMs = Context.SpecializationRemainingMs;
	Out.HeroCooldownSeconds.Append(Context.HeroCooldownSeconds, 3);
	const int32 Count = Context.Buildable.Num();
	Out.BuildableTypes.Reserve(Count);
	Out.BuildableCells.Reserve(Count);
	Out.BuildableAges.Reserve(Count);
	Out.BuildableAlwaysOpen.Reserve(Count);
	for (const FHOLBuildable& Buildable : Context.Buildable)
	{
		Out.BuildableTypes.Add(Buildable.TypeName);
		Out.BuildableCells.Add(Buildable.Cell);
		Out.BuildableAges.Add(Buildable.Age);
		Out.BuildableAlwaysOpen.Add(Buildable.bAlwaysOpen);
	}
	return Out;
}

bool UHOLCommandGridBoard::AskGetCommand(const FHOLGridContext& Context, const int32 Cell, FHOLGridCommand& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board)
	{
		return false;
	}
	Out = HOLCommandGridReadLibraryPrivate::ToRule(Board->GetCommandAnswer(MakeContextView(Context), Cell));
	return true;
}

bool UHOLCommandGridBoard::AskGetOrder(const FHOLGridContext& Context, const int32 Cell, FHOLGridCommand& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board)
	{
		return false;
	}
	Out = HOLCommandGridReadLibraryPrivate::ToRule(Board->GetOrderAnswer(MakeContextView(Context), Cell));
	return true;
}

bool UHOLCommandGridBoard::AskIsAgeReached(const FHOLGridContext& Context, const int32 Age, bool& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = Board->IsAgeReachedAnswer(MakeContextView(Context), Age);
	return true;
}

bool UHOLCommandGridBoard::AskIsAgeOpen(const FHOLGridContext& Context, const int32 Age, bool& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = Board->IsAgeOpenAnswer(MakeContextView(Context), Age);
	return true;
}

bool UHOLCommandGridBoard::AskGetAgeBuilding(const FHOLGridContext& Context, const int32 Age, const int32 Cell, FHOLGridCommand& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = HOLCommandGridReadLibraryPrivate::ToRule(Board->GetAgeBuildingAnswer(MakeContextView(Context), Age, Cell));
	return true;
}

bool UHOLCommandGridBoard::AskPressKey(const FHOLGridContext& Context, const int32 ChosenAge, const int32 Cell, FHOLGridKeyResult& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = ToKeyResult(Board->PressKeyAnswer(MakeContextView(Context), ChosenAge, Cell));
	return true;
}

bool UHOLCommandGridBoard::AskClickAge(const FHOLGridContext& Context, const int32 ChosenAge, const int32 Age, const int32 Cell, FHOLGridKeyResult& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = ToKeyResult(Board->ClickAgeAnswer(MakeContextView(Context), ChosenAge, Age, Cell));
	return true;
}

bool UHOLCommandGridBoard::AskGetChosenAgeAfter(const int32 ChosenAge, const EHOLAgeEvent Event, int32& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = Board->GetChosenAgeAfterAnswer(ChosenAge, static_cast<EHOLAgeEventKind>(static_cast<uint8>(Event)));
	return true;
}

bool UHOLCommandGridBoard::AskCheckProduction(const FHOLResources& Stock, const FHOLResources& Cost, const int32 QueueLength, const int32 MaxQueue,
	const int32 PopulationUsed, const int32 PopulationCap, const int32 Places, FHOLCommandFeedback& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = HOLCommandGridReadLibraryPrivate::ToRule(Board->CheckProductionAnswer(Stock, Cost, QueueLength, MaxQueue, PopulationUsed, PopulationCap, Places));
	return true;
}

bool UHOLCommandGridBoard::AskCheckBuild(const FHOLResources& Stock, const FHOLResources& Cost, FHOLCommandFeedback& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = HOLCommandGridReadLibraryPrivate::ToRule(Board->CheckBuildAnswer(Stock, Cost));
	return true;
}

bool UHOLCommandGridBoard::AskDescribe(const FHOLCommandFeedback& Feedback, FString& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = Board->DescribeAnswer(HOLCommandGridReadLibraryPrivate::FromRule(Feedback));
	return true;
}

bool UHOLCommandGridBoard::AskNextInCycle(const TConstArrayView<FHOLCycleBuilding> Buildings, const uint32 LastKey, int32& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	TArray<FHOLCycleBuildingView> Views;
	Views.Reserve(Buildings.Num());
	for (const FHOLCycleBuilding& Building : Buildings)
	{
		FHOLCycleBuildingView& View = Views.AddDefaulted_GetRef();
		View.Rank = Building.Rank;
		View.Key = static_cast<int32>(Building.Key);
	}
	Out = Board->NextInCycleAnswer(Views, static_cast<int32>(LastKey));
	return true;
}

bool UHOLCommandGridBoard::AskPickEvenly(const TConstArrayView<FHOLQueueChoice> Choices, const uint32 LastKey, int32& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	TArray<FHOLQueueChoiceView> Views;
	Views.Reserve(Choices.Num());
	for (const FHOLQueueChoice& Choice : Choices)
	{
		FHOLQueueChoiceView& View = Views.AddDefaulted_GetRef();
		View.QueueLength = Choice.QueueLength;
		View.MaxQueue = Choice.MaxQueue;
		View.Key = static_cast<int32>(Choice.Key);
	}
	Out = Board->PickEvenlyAnswer(Views, static_cast<int32>(LastKey));
	return true;
}

bool UHOLCommandGridBoard::AskSplitSubgroups(const TConstArrayView<FHOLGroupMember> Members, TArray<FHOLSubgroup>& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	TArray<FHOLGroupMemberView> Views;
	Views.Reserve(Members.Num());
	for (const FHOLGroupMember& Member : Members)
	{
		FHOLGroupMemberView& View = Views.AddDefaulted_GetRef();
		View.TypeName = Member.TypeName;
		View.bAlone = Member.bAlone;
	}
	Out.Reset();
	for (const FHOLSubgroupView& View : Board->SplitSubgroupsAnswer(Views))
	{
		FHOLSubgroup& Subgroup = Out.AddDefaulted_GetRef();
		Subgroup.bAll = View.bAll;
		Subgroup.TypeName = View.TypeName;
		Subgroup.Members = View.Members;
	}
	return true;
}

bool UHOLCommandGridBoard::AskMergeCommands(const TConstArrayView<TArray<FHOLGridCommand>> PerBuilding, const TConstArrayView<int32> Priority, TArray<FHOLGridCommand>& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	TArray<FHOLGridCommandView> Flat;
	Flat.Reserve(PerBuilding.Num() * HOLCommandGrid::NumCells);
	// The flat form is NumCells wide per building. A building given fewer cells is padded with empty commands: the controller sizes the list
	// to the whole group and fills only the active subgroup's members (HOLPlayerController.cpp, GetGridCommands), so the others come EMPTY.
	// Refusing them here was harmless while the C++ body took over; once it was removed (L3a) a subgroup narrower than its group had an empty
	// grid (quick_check "build hud", 2026-11-19 18:02: Tab on the main centre of a group of 8, cells "none").
	const FHOLGridCommand NoCommand;
	for (const TArray<FHOLGridCommand>& Building : PerBuilding)
	{
		for (int32 Cell = 0; Cell < HOLCommandGrid::NumCells; ++Cell)
		{
			Flat.Add(HOLCommandGridReadLibraryPrivate::FromRule(Building.IsValidIndex(Cell) ? Building[Cell] : NoCommand));
		}
	}
	TArray<int32> PriorityCopy(Priority.GetData(), Priority.Num());
	Out.Reset();
	for (const FHOLGridCommandView& View : Board->MergeCommandsAnswer(Flat, PriorityCopy))
	{
		Out.Add(HOLCommandGridReadLibraryPrivate::ToRule(View));
	}
	return true;
}

bool UHOLCommandGridBoard::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLPlayerOrdersBoard).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLCommandGridBoard::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

FHOLGridCommandView UHOLCommandGridBoard::GetCommandAnswer_Implementation(const FHOLGridContextView&, const int32) { return FHOLGridCommandView(); }
FHOLGridCommandView UHOLCommandGridBoard::GetOrderAnswer_Implementation(const FHOLGridContextView&, const int32) { return FHOLGridCommandView(); }

bool UHOLCommandGridBoard::IsAgeReachedAnswer_Implementation(const FHOLGridContextView&, const int32) { return false; }
bool UHOLCommandGridBoard::IsAgeOpenAnswer_Implementation(const FHOLGridContextView&, const int32) { return false; }
FHOLGridCommandView UHOLCommandGridBoard::GetAgeBuildingAnswer_Implementation(const FHOLGridContextView&, const int32, const int32) { return FHOLGridCommandView(); }
FHOLGridKeyResultView UHOLCommandGridBoard::PressKeyAnswer_Implementation(const FHOLGridContextView&, const int32, const int32) { return FHOLGridKeyResultView(); }
FHOLGridKeyResultView UHOLCommandGridBoard::ClickAgeAnswer_Implementation(const FHOLGridContextView&, const int32, const int32, const int32) { return FHOLGridKeyResultView(); }
int32 UHOLCommandGridBoard::GetChosenAgeAfterAnswer_Implementation(const int32, const EHOLAgeEventKind) { return INDEX_NONE; }
FHOLFeedbackView UHOLCommandGridBoard::CheckProductionAnswer_Implementation(const FHOLResources&, const FHOLResources&, const int32, const int32, const int32, const int32, const int32) { return FHOLFeedbackView(); }
FHOLFeedbackView UHOLCommandGridBoard::CheckBuildAnswer_Implementation(const FHOLResources&, const FHOLResources&) { return FHOLFeedbackView(); }
FString UHOLCommandGridBoard::DescribeAnswer_Implementation(const FHOLFeedbackView&) { return FString(); }
int32 UHOLCommandGridBoard::NextInCycleAnswer_Implementation(const TArray<FHOLCycleBuildingView>&, const int32) { return INDEX_NONE; }
int32 UHOLCommandGridBoard::PickEvenlyAnswer_Implementation(const TArray<FHOLQueueChoiceView>&, const int32) { return INDEX_NONE; }
TArray<FHOLSubgroupView> UHOLCommandGridBoard::SplitSubgroupsAnswer_Implementation(const TArray<FHOLGroupMemberView>&) { return TArray<FHOLSubgroupView>(); }
TArray<FHOLGridCommandView> UHOLCommandGridBoard::MergeCommandsAnswer_Implementation(const TArray<FHOLGridCommandView>&, const TArray<int32>&) { return TArray<FHOLGridCommandView>(); }

// Family 4 (D.1): same contract as the others, Out filled when a script board answered.
bool UHOLCommandGridBoard::AskIsAloneInGroup(const FName TypeName, bool& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = Board->IsAloneInGroupAnswer(TypeName);
	return true;
}

bool UHOLCommandGridBoard::AskIsRepeatPress(const TConstArrayView<uint32> Selected, const TConstArrayView<uint32> LastSelected, const bool bSameKey, bool& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	TArray<int32> SelectedKeys;
	SelectedKeys.Reserve(Selected.Num());
	for (const uint32 Key : Selected) { SelectedKeys.Add(static_cast<int32>(Key)); }
	TArray<int32> LastKeys;
	LastKeys.Reserve(LastSelected.Num());
	for (const uint32 Key : LastSelected) { LastKeys.Add(static_cast<int32>(Key)); }
	Out = Board->IsRepeatPressAnswer(SelectedKeys, LastKeys, bSameKey);
	return true;
}

bool UHOLCommandGridBoard::AskGetCommandPriority(const TConstArrayView<FHOLGroupMember> Members, TArray<int32>& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	TArray<FHOLGroupMemberView> Views;
	Views.Reserve(Members.Num());
	for (const FHOLGroupMember& Member : Members)
	{
		FHOLGroupMemberView& View = Views.AddDefaulted_GetRef();
		View.TypeName = Member.TypeName;
		View.bAlone = Member.bAlone;
	}
	Out = Board->GetCommandPriorityAnswer(Views);
	return true;
}

bool UHOLCommandGridBoard::AskIsSelectedByCategory(const EHOLBuildingCategory TypeCategory, const EHOLBuildingCategory Category, bool& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = Board->IsSelectedByCategoryAnswer(TypeCategory, Category);
	return true;
}

bool UHOLCommandGridBoard::AskBlocks(const FHOLCommandFeedback& Feedback, bool& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = Board->BlocksAnswer(HOLCommandGridReadLibraryPrivate::FromRule(Feedback));
	return true;
}

bool UHOLCommandGridBoard::AskGetCycleRank(const bool bBuildable, const int32 BuildGridSlot, int32& Out)
{
	UHOLCommandGridBoard* Board = FindAny();
	if (!Board) { return false; }
	Out = Board->GetCycleRankAnswer(bBuildable, BuildGridSlot);
	return true;
}

bool UHOLCommandGridBoard::IsAloneInGroupAnswer_Implementation(const FName) { return false; }
bool UHOLCommandGridBoard::IsRepeatPressAnswer_Implementation(const TArray<int32>&, const TArray<int32>&, const bool) { return false; }
TArray<int32> UHOLCommandGridBoard::GetCommandPriorityAnswer_Implementation(const TArray<FHOLGroupMemberView>&) { return TArray<int32>(); }
bool UHOLCommandGridBoard::IsSelectedByCategoryAnswer_Implementation(const EHOLBuildingCategory, const EHOLBuildingCategory) { return false; }
bool UHOLCommandGridBoard::BlocksAnswer_Implementation(const FHOLFeedbackView&) { return false; }
int32 UHOLCommandGridBoard::GetCycleRankAnswer_Implementation(const bool, const int32) { return INDEX_NONE; }