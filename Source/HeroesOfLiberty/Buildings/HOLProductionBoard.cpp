// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLProductionBoard.h"

#include "Engine/World.h"

UHOLProductionBoard* UHOLProductionBoard::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLProductionBoard*> Boards = World->GetSubsystemArrayCopy<UHOLProductionBoard>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

int32 UHOLProductionBoard::AskOrderGate(const UWorld* World, const bool bHasState, const bool bOwner, const bool bHasType, const bool bFinished,
	const bool bEarlyRally, const bool bForumDown, const bool bRally)
{
	UHOLProductionBoard* Board = Find(World);
	return Board ? Board->OrderGateAnswer(bHasState, bOwner, bHasType, bFinished, bEarlyRally, bForumDown, bRally) : -1;
}

int32 UHOLProductionBoard::AskCancel(const UWorld* World, const bool bValidIndex)
{
	UHOLProductionBoard* Board = Find(World);
	return Board ? Board->CancelAnswer(bValidIndex) : -1;
}

FHOLResources UHOLProductionBoard::AskStockAfterCancel(const UWorld* World, const FHOLResources& Stock, const FHOLResources& Cost)
{
	UHOLProductionBoard* Board = Find(World);
	if (!Board)
	{
		FHOLResources Back = Stock;
		Back += Cost;
		return Back;
	}
	return Board->StockAfterCancelAnswer(Stock, Cost);
}

bool UHOLProductionBoard::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLMatchFlowSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLProductionBoard::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

int32 UHOLProductionBoard::OrderGateAnswer_Implementation(const bool, const bool, const bool, const bool, const bool, const bool, const bool) { return -1; }

int32 UHOLProductionBoard::CancelAnswer_Implementation(const bool) { return -1; }

FHOLResources UHOLProductionBoard::StockAfterCancelAnswer_Implementation(const FHOLResources Stock, const FHOLResources) { return Stock; }
