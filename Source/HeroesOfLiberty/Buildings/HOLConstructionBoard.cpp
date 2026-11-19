// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLConstructionBoard.h"

#include "Engine/World.h"

UHOLConstructionBoard* UHOLConstructionBoard::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLConstructionBoard*> Boards = World->GetSubsystemArrayCopy<UHOLConstructionBoard>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

int32 UHOLConstructionBoard::AskPlace(const UWorld* World, const bool bBuildable, const bool bFactionOpen, const int32 BuildingAge, const int32 Tier,
	const int32 Refusal, const FHOLResources& Stock, const FHOLResources& Cost)
{
	UHOLConstructionBoard* Board = Find(World);
	return Board ? Board->PlaceAnswer(bBuildable, bFactionOpen, BuildingAge, Tier, Refusal, Stock, Cost) : -1;
}

FHOLResources UHOLConstructionBoard::AskStockAfterPlace(const UWorld* World, const FHOLResources& Stock, const FHOLResources& Cost)
{
	UHOLConstructionBoard* Board = Find(World);
	return Board ? Board->StockAfterPlaceAnswer(Stock, Cost) : Stock;
}

int32 UHOLConstructionBoard::AskCancel(const UWorld* World, const bool bHasBuilding, const int32 OwnerPlayer, const int32 AskerPlayer, const int32 State)
{
	UHOLConstructionBoard* Board = Find(World);
	return Board ? Board->CancelAnswer(bHasBuilding, OwnerPlayer, AskerPlayer, State) : -1;
}

FHOLResources UHOLConstructionBoard::AskCancelRefund(const UWorld* World, const bool bHasType, const FHOLResources& Cost, const int32 State, const int64 Work,
	const int64 TotalWork)
{
	UHOLConstructionBoard* Board = Find(World);
	return Board ? Board->CancelRefundAnswer(bHasType, Cost, State, Work, TotalWork) : FHOLResources();
}

int32 UHOLConstructionBoard::AskQueueRefund(const UWorld* World, const bool bHasWorld, const bool bAuthority, const int32 QueueLength)
{
	UHOLConstructionBoard* Board = Find(World);
	return Board ? Board->QueueRefundAnswer(bHasWorld, bAuthority, QueueLength) : -1;
}

bool UHOLConstructionBoard::AskQueueTotal(const UWorld* World, const TArray<FHOLResources>& ItemCosts, FHOLResources& OutRefund)
{
	UHOLConstructionBoard* Board = Find(World);
	if (!Board)
	{
		return false;
	}
	OutRefund = Board->QueueTotalAnswer(ItemCosts);
	return true;
}

int32 UHOLConstructionBoard::AskDestroy(const UWorld* World, const bool bHasWorld, const bool bAuthority, const bool bValid)
{
	UHOLConstructionBoard* Board = Find(World);
	return Board ? Board->DestroyAnswer(bHasWorld, bAuthority, bValid) : -1;
}

int32 UHOLConstructionBoard::AskRubble(const UWorld* World, const bool bFoundation, const bool bHasType)
{
	UHOLConstructionBoard* Board = Find(World);
	return Board ? Board->RubbleAnswer(bFoundation, bHasType) : -1;
}

bool UHOLConstructionBoard::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLMatchFlowSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLConstructionBoard::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

int32 UHOLConstructionBoard::PlaceAnswer_Implementation(const bool, const bool, const int32, const int32, const int32, const FHOLResources, const FHOLResources)
{
	return -1;
}

FHOLResources UHOLConstructionBoard::StockAfterPlaceAnswer_Implementation(const FHOLResources Stock, const FHOLResources) { return Stock; }

int32 UHOLConstructionBoard::CancelAnswer_Implementation(const bool, const int32, const int32, const int32) { return -1; }

FHOLResources UHOLConstructionBoard::CancelRefundAnswer_Implementation(const bool, const FHOLResources, const int32, const int64, const int64) { return FHOLResources(); }

int32 UHOLConstructionBoard::QueueRefundAnswer_Implementation(const bool, const bool, const int32) { return -1; }

FHOLResources UHOLConstructionBoard::QueueTotalAnswer_Implementation(const TArray<FHOLResources>&) { return FHOLResources(); }

int32 UHOLConstructionBoard::DestroyAnswer_Implementation(const bool, const bool, const bool) { return -1; }

int32 UHOLConstructionBoard::RubbleAnswer_Implementation(const bool, const bool) { return -1; }
