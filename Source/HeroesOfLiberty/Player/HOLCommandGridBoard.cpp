// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLCommandGridBoard.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

// The read library's View -> rule conversion (external linkage, HOLCommandGridReadLibrary.cpp): the script's answer is a FHOLGridCommandView.
namespace HOLCommandGridReadLibraryPrivate
{
	FHOLGridCommand ToRule(const FHOLGridCommandView& View);
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