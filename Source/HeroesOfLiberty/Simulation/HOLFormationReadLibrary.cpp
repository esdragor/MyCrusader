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
		{ TEXT("BlockTurnWindow"), HOLFormation::BlockTurnWindow },
		{ TEXT("BlockLagDistance"), HOLFormation::BlockLagDistance },
		{ TEXT("BlockSoftHoldDistance"), HOLFormation::BlockSoftHoldDistance },
		{ TEXT("BlockBlendMargin"), HOLFormation::BlockBlendMargin },
		{ TEXT("BlockJoinDistance"), HOLFormation::BlockJoinDistance },
		{ TEXT("SettleMinGapInSpacings"), HOLFormation::SettleMinGapInSpacings },
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

// ---- GALADRIEL-1117q (NOT COMPILED) ----

void UHOLFormationReadLibrary::PlaceGroupClamped(const TArray<FVector2D>& Positions, const TArray<int32>& KindOfUnit, const bool bPeasantsOnly,
	const FVector2D& Target, const FVector2D& Forward, const double ColumnSpacing, const double RowSpacing, const double MaxPlaceX,
	int32& OutProjected, TArray<FVector2D>& OutPlaces, TArray<int32>& OutPlaceOfUnit)
{
	int32 Projected = 0;
	HOLFormation::PlaceGroup(Positions, KindOfUnit, bPeasantsOnly, Target, Forward, ColumnSpacing, RowSpacing,
		[&Projected, MaxPlaceX](const FVector2D& Place) { ++Projected; return FVector2D(FMath::Min(Place.X, MaxPlaceX), Place.Y); },
		OutPlaces, OutPlaceOfUnit);
	OutProjected = Projected;
}

void UHOLFormationReadLibrary::PlaceGroupAvoidingDisc(const TArray<FVector2D>& Positions, const TArray<int32>& KindOfUnit, const bool bPeasantsOnly,
	const FVector2D& Target, const FVector2D& Forward, const double ColumnSpacing, const double RowSpacing, const bool bWalledIn,
	const FVector2D& DiscCentre, const double Clearance, int32& OutProjected, TArray<FVector2D>& OutPlaces, TArray<int32>& OutPlaceOfUnit)
{
	int32 Projected = 0;
	HOLFormation::PlaceGroup(Positions, KindOfUnit, bPeasantsOnly, Target, Forward, ColumnSpacing, RowSpacing,
		[bWalledIn, DiscCentre, Clearance](const FVector2D& Point) { return !bWalledIn && FVector2D::Distance(Point, DiscCentre) >= Clearance; },
		[&Projected, bWalledIn](const FVector2D& Place) { ++Projected; return bWalledIn ? FVector2D::ZeroVector : Place; },
		OutPlaces, OutPlaceOfUnit);
	OutProjected = Projected;
}

int32 UHOLFormationReadLibrary::SettleBlockedPlacesWalled(TArray<FVector2D>& InOutPlaces, const TArray<bool>& IsBlocked, const FVector2D& Anchor,
	const double Spacing, TArray<bool>& OutSettled)
{
	return HOLFormation::SettleBlockedPlaces(InOutPlaces, IsBlocked, Anchor, Spacing, [](const FVector2D&) { return false; }, OutSettled);
}

void UHOLFormationReadLibrary::AssignPlacesByKind(const TArray<FVector2D>& Positions, const TArray<int32>& KindOfUnit, const TArray<FVector2D>& Places,
	const FVector2D& Forward, TArray<int32>& OutPlaceOfUnit)
{
	HOLFormation::AssignPlacesByKind(Positions, KindOfUnit, Places, Forward, OutPlaceOfUnit);
}

double UHOLFormationReadLibrary::GetLineLength(const TArray<FVector2D>& Points) { return HOLFormation::GetLineLength(Points); }
FVector2D UHOLFormationReadLibrary::GetPointOnLine(const TArray<FVector2D>& Points, const double Distance) { return HOLFormation::GetPointOnLine(Points, Distance); }

FVector2D UHOLFormationReadLibrary::GetDirectionOnLine(const TArray<FVector2D>& Points, const double Distance, const double Window, const FVector2D& Fallback)
{
	return HOLFormation::GetDirectionOnLine(Points, Distance, Window, Fallback);
}

FVector2D UHOLFormationReadLibrary::BlockOffsetToWorld(const FVector2D& Offset, const FVector2D& Forward) { return HOLFormation::BlockOffsetToWorld(Offset, Forward); }
FVector2D UHOLFormationReadLibrary::WorldToBlockOffset(const FVector2D& Delta, const FVector2D& Forward) { return HOLFormation::WorldToBlockOffset(Delta, Forward); }

FVector2D UHOLFormationReadLibrary::GetSlotOnPath(const TArray<FVector2D>& Points, const double Distance, const FVector2D& Offset, const FVector2D& Fallback)
{
	return HOLFormation::GetSlotOnPath(Points, Distance, Offset, Fallback);
}

double UHOLFormationReadLibrary::GetBlockSoftPace(const double MaxGap) { return HOLFormation::GetBlockSoftPace(MaxGap); }
double UHOLFormationReadLibrary::GetBlockBlendDistance(const double BlockSize, const double PathLength) { return HOLFormation::GetBlockBlendDistance(BlockSize, PathLength); }
double UHOLFormationReadLibrary::GetBlockBlend(const double Remaining, const double BlendDistance) { return HOLFormation::GetBlockBlend(Remaining, BlendDistance); }

int32 UHOLFormationReadLibrary::FindPackets(const TArray<FVector2D>& Positions, const double LinkDistance, TArray<int32>& OutPacketOfUnit,
	TArray<int32>& OutPacketSizes)
{
	return HOLFormation::FindPackets(Positions, LinkDistance, OutPacketOfUnit, OutPacketSizes);
}

int32 UHOLFormationReadLibrary::FindBlockCentres(const TArray<FVector>& Locations, const double LinkDistance, TArray<FVector>& OutCentres)
{
	return HOLFormation::FindBlockCentres(Locations, LinkDistance, OutCentres);
}

int32 UHOLFormationReadLibrary::FindStragglers(const TArray<FVector2D>& Positions, const double LinkDistance, TArray<bool>& OutIsStraggler)
{
	return HOLFormation::FindStragglers(Positions, LinkDistance, OutIsStraggler);
}

int32 UHOLFormationReadLibrary::ReserveRearSlots(const TArray<FVector2D>& Slots, const TArray<FVector2D>& Positions, const TArray<int32>& KindOfUnit,
	const TArray<bool>& IsStraggler, TArray<int32>& OutSlotOfUnit, TArray<bool>& OutQueued)
{
	return HOLFormation::ReserveRearSlots(Slots, Positions, KindOfUnit, IsStraggler, OutSlotOfUnit, OutQueued);
}
