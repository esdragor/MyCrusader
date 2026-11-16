// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLMinimapReadLibrary.h"

#include "HAL/IConsoleManager.h"
#include "HUD/HOLMinimapView.h"
#include "Resources/HOLSourceSubsystem.h"

float UHOLMinimapReadLibrary::AlertDiameter(const float Age, const float Period, const float Size)
{
	return UHOLMinimapView::AlertDiameter(Age, Period, Size);
}

int64 UHOLMinimapReadLibrary::GetSourcesSignature(const TArray<FVector>& Locations, const TArray<int32>& Remaining)
{
	TArray<FHOLSource> Sources;
	Sources.SetNum(Locations.Num());
	for (int32 Index = 0; Index < Locations.Num(); ++Index)
	{
		Sources[Index].TypeIndex = 0;
		Sources[Index].Location = Locations[Index];
		Sources[Index].Remaining = Remaining.IsValidIndex(Index) ? Remaining[Index] : 0;
	}
	return static_cast<int64>(HOLMinimapTerrain::GetSourcesSignature(Sources));
}

float UHOLMinimapReadLibrary::GetConsoleFloat(const FString& Name)
{
	const IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(*Name);
	return Variable ? Variable->GetFloat() : -1.f;
}