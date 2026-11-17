// Copyright Epic Games, Inc. All Rights Reserved.

#include "HOLTableImportReadLibrary.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Match/HOLMarketRules.h"
#include "Match/HOLMarketTypes.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Technologies/HOLTechnologyTypes.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace
{
	UScriptStruct* FindRowStruct(const FString& Key)
	{
		if (Key == TEXT("Market"))
		{
			return FHOLMarketRow::StaticStruct();
		}
		if (Key == TEXT("Technology"))
		{
			return FHOLTechnologyRow::StaticStruct();
		}
		if (Key == TEXT("BuildingType"))
		{
			return FHOLBuildingTypeRow::StaticStruct();
		}
		return nullptr;
	}

	/** Tools/Data/File imported into a transient table of RowStruct; Problems filled when it fails. */
	UDataTable* Import(UScriptStruct* RowStruct, const FString& File, TArray<FString>& Problems)
	{
		if (RowStruct == nullptr || File.IsEmpty() || File.Contains(TEXT("/")) || File.Contains(TEXT("\\")) || File.Contains(TEXT("..")) || !File.EndsWith(TEXT(".csv")))
		{
			Problems.Add(FString::Printf(TEXT("unknown table or file: %s"), *File));
			return nullptr;
		}
		const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Data"), File);
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, *Path))
		{
			Problems.Add(FString::Printf(TEXT("%s is not there"), *Path));
			return nullptr;
		}
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = RowStruct;
		Problems.Append(Table->CreateTableFromCSVString(Text));
		return Problems.IsEmpty() ? Table : nullptr;
	}
}

bool UHOLTableImportReadLibrary::ImportTable(const FString& Key, const FString& File, TArray<FString>& OutRowNames, TArray<FString>& OutColumns, TArray<FString>& OutValues, TArray<FString>& OutProblems)
{
	OutRowNames.Reset();
	OutColumns.Reset();
	OutValues.Reset();
	OutProblems.Reset();
	UScriptStruct* RowStruct = FindRowStruct(Key);
	const UDataTable* Table = Import(RowStruct, File, OutProblems);
	if (Table == nullptr)
	{
		return false;
	}
	for (TFieldIterator<FProperty> It(RowStruct); It; ++It)
	{
		OutColumns.Add(It->GetName());
	}
	for (const FName RowName : Table->GetRowNames())
	{
		const uint8* RowData = Table->FindRowUnchecked(RowName);
		if (RowData == nullptr)
		{
			continue;
		}
		OutRowNames.Add(RowName.ToString());
		for (TFieldIterator<FProperty> It(RowStruct); It; ++It)
		{
			FString Text;
			It->ExportText_Direct(Text, It->ContainerPtrToValuePtr<void>(RowData), nullptr, nullptr, PPF_None);
			OutValues.Add(Text);
		}
	}
	return true;
}

bool UHOLTableImportReadLibrary::MarketSettingsFromRow(const FString& RowName, TArray<int32>& OutBasePrices, int32& OutStep, int32& OutFeePercent, int32& OutMinPrice, int32& OutMaxPrice, int32& OutRecoverPoints, int32& OutRecoverEverySeconds)
{
	TArray<FString> Problems;
	const UDataTable* Table = Import(FHOLMarketRow::StaticStruct(), TEXT("Market.csv"), Problems);
	const FHOLMarketRow* Row = Table != nullptr ? Table->FindRow<FHOLMarketRow>(FName(*RowName), TEXT("market rates"), false) : nullptr;
	if (Row == nullptr)
	{
		return false;
	}
	const HOLMarket::FSettings Settings = HOLMarket::SettingsFromRow(*Row);
	OutBasePrices.Reset();
	for (const int32 Price : Settings.BasePrices)
	{
		OutBasePrices.Add(Price);
	}
	OutStep = Settings.Step;
	OutFeePercent = Settings.FeePercent;
	OutMinPrice = Settings.MinPrice;
	OutMaxPrice = Settings.MaxPrice;
	OutRecoverPoints = Settings.RecoverPoints;
	OutRecoverEverySeconds = Settings.RecoverEverySeconds;
	return true;
}
