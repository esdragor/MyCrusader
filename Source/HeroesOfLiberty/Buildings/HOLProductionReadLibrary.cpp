// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLProductionReadLibrary.h"

#include "Buildings/HOLProductionRules.h"

namespace
{
	TArray<FHOLProductionItem> ToItems(const FHOLProductionQueueState& State)
	{
		TArray<FHOLProductionItem> Items;
		for (int32 Index = 0; Index < State.Types.Num(); ++Index)
		{
			FHOLProductionItem& Item = Items.AddDefaulted_GetRef();
			Item.TypeName = State.Types[Index];
			Item.StartStep = State.StartSteps.IsValidIndex(Index) ? State.StartSteps[Index] : FHOLProductionItem::NotStarted;
		}
		return Items;
	}

	void FromItems(const TArray<FHOLProductionItem>& Items, FHOLProductionQueueState& State)
	{
		State.Types.Reset();
		State.StartSteps.Reset();
		for (const FHOLProductionItem& Item : Items)
		{
			State.Types.Add(Item.TypeName);
			State.StartSteps.Add(Item.StartStep);
		}
	}

	FHOLProductionCost MakeCost(const FHOLResources& Resources, const int32 BuildSteps, const int32 Population)
	{
		FHOLProductionCost Cost;
		Cost.Resources = Resources;
		Cost.BuildSteps = BuildSteps;
		Cost.Population = Population;
		return Cost;
	}
}

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

int32 UHOLProductionReadLibrary::TryEnqueue(FHOLProductionQueueState& Queue, const FName TypeName, const FHOLResources& CostResources, const int32 BuildSteps, const int32 Population, const int32 MaxQueue, FHOLResources& Stock)
{
	TArray<FHOLProductionItem> Items = ToItems(Queue);
	const EHOLProductionRefusal Refusal = HOLProduction::TryEnqueue(Items, TypeName, MakeCost(CostResources, BuildSteps, Population), MaxQueue, Stock);
	FromItems(Items, Queue);
	return static_cast<int32>(Refusal);
}

bool UHOLProductionReadLibrary::Cancel(FHOLProductionQueueState& Queue, const int32 Index, const FHOLResources& CostResources, const int32 BuildSteps, const int32 Population, FHOLResources& Stock)
{
	TArray<FHOLProductionItem> Items = ToItems(Queue);
	const bool bCancelled = HOLProduction::Cancel(Items, Index, MakeCost(CostResources, BuildSteps, Population), Stock);
	FromItems(Items, Queue);
	return bCancelled;
}

void UHOLProductionReadLibrary::Advance(FHOLProductionQueueState& Queue, const int64 Step, const FHOLResources& CostResources, const int32 BuildSteps, const int32 Population, int32& FreePopulation, TArray<FName>& OutDone)
{
	TArray<FHOLProductionItem> Items = ToItems(Queue);
	const FHOLProductionCost Cost = MakeCost(CostResources, BuildSteps, Population);
	HOLProduction::Advance(Items, Step, [&Cost](FName) { return Cost; }, FreePopulation, OutDone);
	FromItems(Items, Queue);
}
