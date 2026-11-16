// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLHUDLayoutReadLibrary.h"

#include "HUD/HOLHUDLayoutRules.h"

bool UHOLHUDLayoutReadLibrary::Overlaps(const FVector2D& AMin, const FVector2D& AMax, const FVector2D& BMin, const FVector2D& BMax, const float Tolerance)
{
	return HOLHUDLayout::Overlaps(FBox2D(AMin, AMax), FBox2D(BMin, BMax), Tolerance);
}

float UHOLHUDLayoutReadLibrary::GetLowWindowHeight() { return HOLHUDLayout::LowWindowHeight; }

bool UHOLHUDLayoutReadLibrary::IsLowWindow(const float HeightPx) { return HOLHUDLayout::IsLowWindow(HeightPx); }

bool UHOLHUDLayoutReadLibrary::CheatsStartOpen(const bool bSavedOpen, const float HeightPx)
{
	return HOLHUDLayout::CheatsStartOpen(bSavedOpen, HeightPx);
}

FHOLHUDFoldState UHOLHUDLayoutReadLibrary::ShouldFoldCheats(const float HeightPx, const bool bHeroBlockShown, const bool bCheatsOpen, const FHOLHUDFoldState& Previous)
{
	FHOLHUDFoldState Result = Previous;
	Result.bFold = HOLHUDLayout::ShouldFoldCheats(HeightPx, bHeroBlockShown, bCheatsOpen, Result.bWasLow, Result.bWasShown);
	return Result;
}
