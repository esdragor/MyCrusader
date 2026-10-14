// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLProductionRules.h"

int32 HOLProduction::GetPopulationCap(const int32 Provided)
{
	return FMath::Clamp(Provided, 0, MaxPopulation);
}

EHOLProductionRefusal HOLProduction::TryEnqueue(TArray<FHOLProductionItem>& Queue, const FName TypeName,
	const FHOLProductionCost& Cost, const int32 MaxQueue, FHOLResources& Stock)
{
	if (Queue.Num() >= MaxQueue)
	{
		return EHOLProductionRefusal::QueueFull;
	}
	if (!Stock.Covers(Cost.Resources))
	{
		return EHOLProductionRefusal::NotEnoughResources;
	}
	Stock -= Cost.Resources;
	FHOLProductionItem& Item = Queue.AddDefaulted_GetRef();
	Item.TypeName = TypeName;
	return EHOLProductionRefusal::None;
}

bool HOLProduction::Cancel(TArray<FHOLProductionItem>& Queue, const int32 Index, const FHOLProductionCost& Cost, FHOLResources& Stock)
{
	if (!Queue.IsValidIndex(Index))
	{
		return false;
	}
	Stock += Cost.Resources;
	Queue.RemoveAt(Index);
	return true;
}

void HOLProduction::Advance(TArray<FHOLProductionItem>& Queue, const int64 Step,
	TFunctionRef<FHOLProductionCost(FName)> GetCost, int32& FreePopulation, TArray<FName>& OutDone)
{
	// The head finishes, then the next one may start at the same step: a full queue never loses a step between units.
	while (Queue.Num() > 0)
	{
		FHOLProductionItem& Head = Queue[0];
		const FHOLProductionCost Cost = GetCost(Head.TypeName);
		if (!Head.IsStarted())
		{
			if (FreePopulation < Cost.Population)
			{
				return;
			}
			FreePopulation -= Cost.Population;
			Head.StartStep = Step;
		}
		if (Step - Head.StartStep < FMath::Max(Cost.BuildSteps, 1))
		{
			return;
		}
		OutDone.Add(Head.TypeName);
		Queue.RemoveAt(0);
	}
}

int32 HOLProduction::GetStartedPopulation(const TArray<FHOLProductionItem>& Queue, TFunctionRef<FHOLProductionCost(FName)> GetCost)
{
	int32 Population = 0;
	for (const FHOLProductionItem& Item : Queue)
	{
		if (Item.IsStarted())
		{
			Population += GetCost(Item.TypeName).Population;
		}
	}
	return Population;
}

float HOLProduction::GetProgress(const FHOLProductionItem& Item, const double Step, const int32 BuildSteps)
{
	if (!Item.IsStarted())
	{
		return 0.f;
	}
	return static_cast<float>(FMath::Clamp((Step - static_cast<double>(Item.StartStep)) / FMath::Max(BuildSteps, 1), 0.0, 1.0));
}
