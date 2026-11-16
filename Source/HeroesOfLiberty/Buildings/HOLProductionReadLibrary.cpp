// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLProductionReadLibrary.h"

#include "Buildings/HOLProductionRules.h"

int32 UHOLProductionReadLibrary::GetPopulationCap(const int32 Provided)
{
	return HOLProduction::GetPopulationCap(Provided);
}

float UHOLProductionReadLibrary::GetProgress(const int64 StartStep, const double Step, const int32 BuildSteps)
{
	FHOLProductionItem Item;
	Item.StartStep = StartStep;
	return HOLProduction::GetProgress(Item, Step, BuildSteps);
}

bool UHOLProductionReadLibrary::IsWaitingForRoom(const int64 StartStep, const double Step, const int32 BuildSteps)
{
	FHOLProductionItem Item;
	Item.StartStep = StartStep;
	return HOLProduction::IsWaitingForRoom(Item, Step, BuildSteps);
}