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
	UE_LOG(LogTemp, Error, TEXT("Market: no script board in this world, the book is not recovered (Script/Economy/HOLMarketBoardScript.as did not compile?)"));
	return Book;
}

FHOLTributeBridgeResult UHOLMarketSubsystem::SettleTribute(const UWorld* World, const bool bSenderHasMarket, const int32 Sender, const int32 Receiver, const bool bReceiverInMatch, const FHOLResources& GiverStock, const FHOLResources& TakerStock, const int32 Resource, const int32 Sent, const int32 TaxPercent)
{
	if (UHOLMarketSubsystem* Board = Find(World))
	{
		return Board->SettleNow(bSenderHasMarket, Sender, Receiver, bReceiverInMatch, GiverStock, TakerStock, Resource, Sent, TaxPercent);
	}
	UE_LOG(LogTemp, Error, TEXT("Market: no script board in this world, the tribute is not settled (Script/Economy/HOLMarketBoardScript.as did not compile?)"));
	FHOLTributeBridgeResult Refused;
	Refused.Giver = GiverStock;
	Refused.Taker = TakerStock;
	return Refused;
}

HOLMarket::FBook UHOLMarketSubsystem::NewBook(const UWorld* World, const HOLMarket::FSettings& Settings)
{
	if (UHOLMarketSubsystem* Board = Find(World))
	{
		return FromBridge(Board->NewBookNow(ToBridge(Settings)));
	}
	UE_LOG(LogTemp, Error, TEXT("Market: no script board in this world, the book is not created (Script/Economy/HOLMarketBoardScript.as did not compile?)"));
	return HOLMarket::FBook();
}

namespace
{
	HOLMarket::FTrade RefusedForBoard(const HOLMarket::FBook& Book)
	{
		HOLMarket::FTrade Refused;
		Refused.Reason = FName(TEXT("board"));
		Refused.Book = Book;
		return Refused;
	}

	/** The board's answer as the rule's trade; a refused one keeps the book given. */
	HOLMarket::FTrade FromBoard(const FHOLMarketBridgeTrade& Done, const HOLMarket::FBook& Given)
	{
		HOLMarket::FTrade Trade;
		Trade.bOk = Done.bOk;
		Trade.Reason = Done.Reason.IsEmpty() ? FName() : FName(*Done.Reason);
		Trade.Gold = Done.Gold;
		Trade.Units = Done.Units;
		Trade.Book = Trade.bOk ? UHOLMarketSubsystem::FromBridge(Done.Book) : Given;
		return Trade;
	}
}

HOLMarket::FTrade UHOLMarketSubsystem::Quote(const UWorld* World, const HOLMarket::FSettings& Settings, const HOLMarket::FBook& Book, const int32 Resource, const int32 Lots, const bool bBuy)
{
	UHOLMarketSubsystem* Board = Find(World);
	if (!Board)
	{
		// The grid asks at every refresh: said once, not at every frame.
		static bool bSaid = false;
		if (!bSaid)
		{
			bSaid = true;
			UE_LOG(LogTemp, Error, TEXT("Market: no script board in this world, the quote is refused (Script/Economy/HOLMarketBoardScript.as did not compile?)"));
		}
		return RefusedForBoard(Book);
	}
	// What is given up is not counted: the quote is the price of the lot, the cell is greyed apart when the stock is short.
	return FromBoard(Board->TradeNow(ToBridge(Settings), ToBridge(Book), Resource, Lots, bBuy, MAX_int32), Book);
}

HOLMarket::FTrade UHOLMarketSubsystem::Apply(const UWorld* World, const HOLMarket::FSettings& Settings, const HOLMarket::FBook& Book, const FHOLResources& Stock, const EHOLResource Resource, const int32 Lots, const bool bBuy, FHOLResources& OutStock)
{
	OutStock = Stock;
	UHOLMarketSubsystem* Board = Find(World);
	if (!Board)
	{
		UE_LOG(LogTemp, Error, TEXT("Market: no script board in this world, the trade is refused (Script/Economy/HOLMarketBoardScript.as did not compile?)"));
		return RefusedForBoard(Book);
	}
	const HOLMarket::FTrade Trade = FromBoard(Board->TradeNow(ToBridge(Settings), ToBridge(Book), static_cast<int32>(Resource), Lots, bBuy, bBuy ? Stock.Gold : Stock[Resource]), Book);
	if (!Trade.bOk)
	{
		return Trade;
	}
	OutStock[Resource] += bBuy ? Trade.Units : -Trade.Units;
	OutStock.Gold += bBuy ? -Trade.Gold : Trade.Gold;
	return Trade;
}

FHOLMarketBridgeTrade UHOLMarketSubsystem::TradeNow(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, const int32 Resource, const int32 Lots, const bool bBuy, const int32 Stock)
{
	return TradeOnce(Settings, Book, Resource, Lots, bBuy, Stock);
}

FHOLMarketBridgeTrade UHOLMarketSubsystem::TradeOnce_Implementation(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, const int32 Resource, const int32 Lots, const bool bBuy, const int32 Stock)
{
	UE_LOG(LogTemp, Error, TEXT("Market: TradeOnce has no script implementation, the trade is refused"));
	FHOLMarketBridgeTrade Refused;
	Refused.Reason = TEXT("board");
	Refused.Book = Book;
	return Refused;
}

FHOLMarketBridgeBook UHOLMarketSubsystem::NewBookNow(const FHOLMarketBridgeSettings& Settings)
{
	return FreshBook(Settings);
}

FHOLMarketBridgeBook UHOLMarketSubsystem::RecoverNow(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, const int32 Intervals)
{
	return RecoverPrices(Settings, Book, Intervals);
}

FHOLTributeBridgeResult UHOLMarketSubsystem::SettleNow(const bool bSenderHasMarket, const int32 Sender, const int32 Receiver, const bool bReceiverInMatch, const FHOLResources& GiverStock, const FHOLResources& TakerStock, const int32 Resource, const int32 Sent, const int32 TaxPercent)
{
	return TributeSettle(bSenderHasMarket, Sender, Receiver, bReceiverInMatch, GiverStock, TakerStock, Resource, Sent, TaxPercent);
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
	UE_LOG(LogTemp, Error, TEXT("Market: RecoverPrices has no script implementation, the book is returned unchanged"));
	return Book;
}

FHOLMarketBridgeBook UHOLMarketSubsystem::FreshBook_Implementation(const FHOLMarketBridgeSettings& Settings)
{
	UE_LOG(LogTemp, Error, TEXT("Market: FreshBook has no script implementation, the book is empty"));
	return FHOLMarketBridgeBook();
}

FHOLTributeBridgeResult UHOLMarketSubsystem::TributeSettle_Implementation(const bool bSenderHasMarket, const int32 Sender, const int32 Receiver, const bool bReceiverInMatch, const FHOLResources& GiverStock, const FHOLResources& TakerStock, const int32 Resource, const int32 Sent, const int32 TaxPercent)
{
	UE_LOG(LogTemp, Error, TEXT("Market: TributeSettle has no script implementation, the tribute is refused and the stocks are returned unchanged"));
	FHOLTributeBridgeResult Refused;
	Refused.Giver = GiverStock;
	Refused.Taker = TakerStock;
	return Refused;
}