// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLStartRules.h"

TArray<FString> FHOLStartRules::GetMissing(const TConstArrayView<FString> Expected, const TConstArrayView<FString> Present)
{
	TArray<FString> Missing;
	for (const FString& Name : Expected)
	{
		const FString Wanted = Name.TrimStartAndEnd();
		const bool bHere = Present.ContainsByPredicate([&Wanted](const FString& Other)
		{
			return Other.TrimStartAndEnd().Equals(Wanted, ESearchCase::IgnoreCase);
		});
		if (!Wanted.IsEmpty() && !bHere)
		{
			Missing.Add(Wanted);
		}
	}
	return Missing;
}

bool FHOLStartRules::ShouldStart(const int32 NumMissing, const double ElapsedSeconds, const double MaxWaitSeconds)
{
	return NumMissing <= 0 || ElapsedSeconds >= MaxWaitSeconds;
}
