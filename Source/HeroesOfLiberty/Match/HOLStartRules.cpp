// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLStartRules.h"

#include "Match/HOLMatchConfig.h"

const FName FHOLStartRules::KingTypeName(TEXT("King"));
const FName FHOLStartRules::PeasantTypeName(TEXT("Peasant"));
const FName FHOLStartRules::HeroTypeName(TEXT("Hero"));

bool FHOLStartRules::ShouldSpawnHero(const FHOLMatchConfig& Config, const int32 Slot, const bool bSetting)
{
	const bool bAsked = Config.StartingHero >= 0 ? Config.StartingHero > 0 : bSetting;
	return bAsked && Config.Slots.IsValidIndex(Slot) && Config.Slots[Slot].State != EHOLSlotState::Closed;
}

FVector FHOLStartRules::GetHeroLocation(const FVector& CentreLocation, const float HalfBody)
{
	// In the King's row, beyond the starting Peasants' row, to his side: clear of both whatever their number.
	return GetKingLocation(CentreLocation, HalfBody) + FVector(0.0, HeroSideOffset, 0.0);
}

int32 FHOLStartRules::GetNumStartingPeasants(const FHOLMatchConfig& Config, const int32 Setting)
{
	return Config.StartingPeasants >= 0 ? Config.StartingPeasants : FMath::Max(Setting, 0);
}

TArray<FVector> FHOLStartRules::GetStartingPeasantLocations(const FVector& CentreLocation, const float HalfBody, const int32 Num)
{
	TArray<FVector> Locations;
	Locations.Reserve(FMath::Max(Num, 0));
	// One row across, centred on the forum: in front of the King, who stands KingGap from the body's edge.
	const double X = CentreLocation.X - HalfBody - StartingPeasantGap;
	for (int32 Index = 0; Index < Num; ++Index)
	{
		const double Y = CentreLocation.Y + (Index - 0.5 * (Num - 1)) * StartingPeasantSpacing;
		Locations.Add(FVector(X, Y, CentreLocation.Z));
	}
	return Locations;
}

bool FHOLStartRules::ShouldSpawnKing(const FHOLMatchConfig& Config, const int32 Slot)
{
	return Config.bRegicide && Config.Slots.IsValidIndex(Slot) && Config.Slots[Slot].State != EHOLSlotState::Closed;
}

FVector FHOLStartRules::GetKingLocation(const FVector& CentreLocation, const float HalfBody)
{
	return FVector(CentreLocation.X - HalfBody - KingGap, CentreLocation.Y, CentreLocation.Z);
}

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
