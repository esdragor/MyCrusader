// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLCommandGridBoard.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

// The read library's View -> rule conversion (external linkage, HOLCommandGridReadLibrary.cpp): the script's answer is a FHOLGridCommandView.
namespace HOLCommandGridReadLibraryPrivate
{
	FHOLGridCommand ToRule(const FHOLGridCommandView& View);
}

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
