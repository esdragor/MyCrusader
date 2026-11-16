// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMarketSubsystem.h"

#include "Engine/World.h"

UHOLMarketSubsystem* UHOLMarketSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLMarketSubsystem*> Boards = World->GetSubsystemArrayCopy<UHOLMarketSubsystem>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

HOLMarket::FBook UHOLMarketSubsystem::Recover(const UWorld* World, const HOLMarket::FSettings& Settings, const HOLMarket::FBook& Book, const int32 Intervals)
{
	if (UHOLMarketSubsystem* Board = Find(World))
	{
		return FromBridge(Board->RecoverNow(ToBridge(Settings), ToBridge(Book), Intervals));
	}
	return HOLMarket::Recover(Settings, Book, Intervals);
}

int32 UHOLMarketSubsystem::Received(const UWorld* World, const int32 Sent, const int32 TaxPercent)
{
	if (UHOLMarketSubsystem* Board = Find(World))
	{
		return Board->ReceivedNow(Sent, TaxPercent);
	}
	return HOLTribute::Received(Sent, TaxPercent);
}

FHOLMarketBridgeBook UHOLMarketSubsystem::RecoverNow(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, const int32 Intervals)
{
	return RecoverPrices(Settings, Book, Intervals);
}

int32 UHOLMarketSubsystem::ReceivedNow(const int32 Sent, const int32 TaxPercent)
{
	return TributeReceived(Sent, TaxPercent);
}

FHOLMarketBridgeSettings UHOLMarketSubsystem::ToBridge(const HOLMarket::FSettings& Settings)
{
	FHOLMarketBridgeSettings Out;
	for (int32 Index = 0; Index < HOLMarket::BookSize; ++Index)
	{
		Out.BasePrices.Add(Settings.BasePrices[Index]);
	}
	Out.Step = Settings.Step;
	Out.FeePercent = Settings.FeePercent;
	Out.MinPrice = Settings.MinPrice;
	Out.MaxPrice = Settings.MaxPrice;
	Out.RecoverPoints = Settings.RecoverPoints;
	Out.RecoverEverySeconds = Settings.RecoverEverySeconds;
	return Out;
}

HOLMarket::FSettings UHOLMarketSubsystem::FromBridge(const FHOLMarketBridgeSettings& Settings)
{
	HOLMarket::FSettings Out;
	for (int32 Index = 0; Index < HOLMarket::BookSize; ++Index)
	{
		Out.BasePrices[Index] = Settings.BasePrices.IsValidIndex(Index) ? Settings.BasePrices[Index] : 0;
	}
	Out.Step = Settings.Step;
	Out.FeePercent = Settings.FeePercent;
	Out.MinPrice = Settings.MinPrice;
	Out.MaxPrice = Settings.MaxPrice;
	Out.RecoverPoints = Settings.RecoverPoints;
	Out.RecoverEverySeconds = Settings.RecoverEverySeconds;
	return Out;
}

FHOLMarketBridgeBook UHOLMarketSubsystem::ToBridge(const HOLMarket::FBook& Book)
{
	FHOLMarketBridgeBook Out;
	for (int32 Index = 0; Index < HOLMarket::BookSize; ++Index)
	{
		Out.Prices.Add(Book.Prices[Index]);
	}
	return Out;
}

HOLMarket::FBook UHOLMarketSubsystem::FromBridge(const FHOLMarketBridgeBook& Book)
{
	HOLMarket::FBook Out;
	for (int32 Index = 0; Index < HOLMarket::BookSize; ++Index)
	{
		Out.Prices[Index] = Book.Prices.IsValidIndex(Index) ? Book.Prices[Index] : 0;
	}
	return Out;
}

bool UHOLMarketSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLUnitTierSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLMarketSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

FHOLMarketBridgeBook UHOLMarketSubsystem::RecoverPrices_Implementation(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, const int32 Intervals)
{
	return ToBridge(HOLMarket::Recover(FromBridge(Settings), FromBridge(Book), Intervals));
}

int32 UHOLMarketSubsystem::TributeReceived_Implementation(const int32 Sent, const int32 TaxPercent)
{
	return HOLTribute::Received(Sent, TaxPercent);
}