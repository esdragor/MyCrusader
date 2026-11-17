// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLFormationReadLibrary.h"

#include "Simulation/HOLFormationRules.h"

namespace HOLFormationReadLibraryPrivate
{
	HOLFormation::FPlaceHold ToHold(const FHOLPlaceHoldView& View)
	{
		HOLFormation::FPlaceHold Hold;
		Hold.LastLocation = View.LastLocation;
		Hold.RestSteps = View.RestSteps;
		Hold.FailedReturns = View.FailedReturns;
		Hold.bHeld = View.bHeld;
		Hold.bReturning = View.bReturning;
		return Hold;
	}

	void FromHold(const HOLFormation::FPlaceHold& Hold, FHOLPlaceHoldView& View)
	{
		View.LastLocation = Hold.LastLocation;
		View.RestSteps = Hold.RestSteps;
		View.FailedReturns = Hold.FailedReturns;
		View.bHeld = Hold.bHeld;
		View.bReturning = Hold.bReturning;
	}
}

double UHOLFormationReadLibrary::GetConstant(const FName Name)
{
	static const TMap<FName, double> Constants = {
		{ TEXT("OtherMeleeRowPriority"), HOLFormation::OtherMeleeRowPriority },
		{ TEXT("ArcherRowPriority"), HOLFormation::ArcherRowPriority },
		{ TEXT("OtherRangedRowPriority"), HOLFormation::OtherRangedRowPriority },
		{ TEXT("SiegeRowPriority"), HOLFormation::SiegeRowPriority },
		{ TEXT("PeasantRowPriority"), HOLFormation::PeasantRowPriority },
		{ TEXT("ReturnRestSteps"), HOLFormation::ReturnRestSteps },
		{ TEXT("MaxReturnTries"), HOLFormation::MaxReturnTries },
		{ TEXT("BlockBreakDistance"), HOLFormation::BlockBreakDistance },
		{ TEXT("BlockMemberStuckSteps"), HOLFormation::BlockMemberStuckSteps },
		{ TEXT("FileSlideMaxAngle"), HOLFormation::FileSlideMaxAngle },
	};
	const double* Found = Constants.Find(Name);
	return Found ? *Found : 0.;
}

void UHOLFormationReadLibrary::MakePeasantRows(const int32 Count, TArray<int32>& OutWidths) { HOLFormation::MakePeasantRows(Count, OutWidths); }
void UHOLFormationReadLibrary::MakeSquareRows(const int32 Count, TArray<int32>& OutWidths) { HOLFormation::MakeSquareRows(Count, OutWidths); }
void UHOLFormationReadLibrary::MakeMixedRows(const int32 Count, TArray<int32>& OutWidths) { HOLFormation::MakeMixedRows(Count, OutWidths); }
void UHOLFormationReadLibrary::MakeRowsOfWidth(const int32 Count, const int32 Width, TArray<int32>& OutWidths) { HOLFormation::MakeRowsOfWidth(Count, Width, OutWidths); }

void UHOLFormationReadLibrary::MakeGroupRows(const TArray<int32>& CountOfKind, const bool bPeasantsOnly, TArray<int32>& OutWidths)
{
	HOLFormation::MakeGroupRows(CountOfKind, bPeasantsOnly, OutWidths);
}

void UHOLFormationReadLibrary::PlaceKinds(TArray<int32>& InOutRowWidths, const TArray<int32>& CountOfKind, TArray<int32>& OutKindOfPlace)
{
	HOLFormation::PlaceKinds(InOutRowWidths, CountOfKind, OutKindOfPlace);
}

int32 UHOLFormationReadLibrary::GetRowPriority(const FName TypeName, const int32 CarryCapacity, const bool bRanged, const bool bSiege)
{
	return HOLFormation::GetRowPriority(TypeName, CarryCapacity, bRanged, bSiege);
}

int32 UHOLFormationReadLibrary::GetRowClass(const int32 RowPriority)
{
	return static_cast<int32>(HOLFormation::GetRowClass(RowPriority));
}

void UHOLFormationReadLibrary::MakePlaces(const TArray<int32>& RowWidths, const FVector2D& Target, const FVector2D& Forward,
	const double ColumnSpacing, const double RowSpacing, TArray<FVector2D>& OutPlaces)
{
	HOLFormation::MakePlaces(RowWidths, Target, Forward, ColumnSpacing, RowSpacing, OutPlaces);
}

void UHOLFormationReadLibrary::PlaceGroup(const TArray<FVector2D>& Positions, const TArray<int32>& KindOfUnit, const bool bPeasantsOnly,
	const FVector2D& Target, const FVector2D& Forward, const double ColumnSpacing, const double RowSpacing, const int32 FrontWidth,
	TArray<FVector2D>& OutPlaces, TArray<int32>& OutPlaceOfUnit)
{
	HOLFormation::PlaceGroup(Positions, KindOfUnit, bPeasantsOnly, Target, Forward, ColumnSpacing, RowSpacing,
		[](const FVector2D& Place) { return Place; }, OutPlaces, OutPlaceOfUnit, FrontWidth);
}

void UHOLFormationReadLibrary::AssignPlaces(const TArray<FVector2D>& Positions, const TArray<FVector2D>& Places, const FVector2D& Forward,
	TArray<int32>& OutPlaceOfUnit)
{
	HOLFormation::AssignPlaces(Positions, Places, Forward, OutPlaceOfUnit);
}

int32 UHOLFormationReadLibrary::GetFrontWidth(const double Length, const double ColumnSpacing, const int32 Count)
{
	return HOLFormation::GetFrontWidth(Length, ColumnSpacing, Count);
}

FVector2D UHOLFormationReadLibrary::GetFrontFacing(const FVector2D& Arrow) { return HOLFormation::GetFrontFacing(Arrow); }

float UHOLFormationReadLibrary::GetMarchSpeed(const float OwnSpeed, const float Slowest) { return HOLFormation::GetMarchSpeed(OwnSpeed, Slowest); }

void UHOLFormationReadLibrary::EndWalkToPlace(FHOLPlaceHoldView& Hold, const FVector2D& Location, const FVector2D& Place, const double Radius)
{
	HOLFormation::FPlaceHold Native = HOLFormationReadLibraryPrivate::ToHold(Hold);
	HOLFormation::EndWalkToPlace(Native, Location, Place, Radius);
	HOLFormationReadLibraryPrivate::FromHold(Native, Hold);
}

bool UHOLFormationReadLibrary::WatchPlace(FHOLPlaceHoldView& Hold, const FVector2D& Location, const FVector2D& Place, const double Radius)
{
	HOLFormation::FPlaceHold Native = HOLFormationReadLibraryPrivate::ToHold(Hold);
	const bool bWalk = HOLFormation::WatchPlace(Native, Location, Place, Radius);
	HOLFormationReadLibraryPrivate::FromHold(Native, Hold);
	return bWalk;
}

bool UHOLFormationReadLibrary::ShouldYieldHeldPlace(const FVector2D& HeldPlace, const TArray<FVector2D>& NewPlacesByX, const double RadiusSum)
{
	return HOLFormation::ShouldYieldHeldPlace(HeldPlace, NewPlacesByX, RadiusSum);
}

bool UHOLFormationReadLibrary::ShouldBreakBlock(const bool bPlainMove, const double ClosestEnemy, const double ClosestFightingFriend)
{
	return HOLFormation::ShouldBreakBlock(bPlainMove, ClosestEnemy, ClosestFightingFriend);
}

bool UHOLFormationReadLibrary::IsFightingFriend(const bool bAttacking, const bool bApproaching, const bool bHasTarget)
{
	return HOLFormation::IsFightingFriend(bAttacking, bApproaching, bHasTarget);
}

bool UHOLFormationReadLibrary::IsMemberStuck(const int32 Steps) { return HOLFormation::IsMemberStuck(Steps); }

void UHOLFormationReadLibrary::MakeCorridor(const TArray<double>& FreeLeft, const TArray<double>& FreeRight, const double SampleStep,
	const double OffsetMinY, const double OffsetMaxY, const double MaxShift, const double MaxSlope, TArray<double>& OutShift,
	TArray<double>& OutSqueeze)
{
	HOLFormation::MakeCorridor(FreeLeft, FreeRight, SampleStep, OffsetMinY, OffsetMaxY, MaxShift, MaxSlope, OutShift, OutSqueeze);
}

void UHOLFormationReadLibrary::GetCorridorAt(const TArray<double>& Shift, const TArray<double>& Squeeze, const double SampleStep,
	const double Distance, double& OutShift, double& OutSqueeze)
{
	HOLFormation::GetCorridorAt(Shift, Squeeze, SampleStep, Distance, OutShift, OutSqueeze);
}
