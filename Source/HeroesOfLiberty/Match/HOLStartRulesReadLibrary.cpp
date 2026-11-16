// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLStartRulesReadLibrary.h"

#include "Match/HOLStartRules.h"

TArray<FString> UHOLStartRulesReadLibrary::GetMissing(const TArray<FString>& Expected, const TArray<FString>& Present)
{
	return FHOLStartRules::GetMissing(Expected, Present);
}

bool UHOLStartRulesReadLibrary::ShouldStart(const int32 NumMissing, const double ElapsedSeconds, const double MaxWaitSeconds)
{
	return FHOLStartRules::ShouldStart(NumMissing, ElapsedSeconds, MaxWaitSeconds);
}
