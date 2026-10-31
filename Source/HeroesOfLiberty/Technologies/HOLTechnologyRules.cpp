// Copyright Epic Games, Inc. All Rights Reserved.

#include "Technologies/HOLTechnologyRules.h"

bool FHOLTechnologyState::Add(const FName Technology)
{
	if (Technology.IsNone() || Done.Contains(Technology))
	{
		return false;
	}
	Done.Add(Technology);
	return true;
}

HOLTechnology::EBlock HOLTechnology::Check(const FHOLTechnologyRow& Row, const FFacts& Facts)
{
	if (Facts.bResearched)
	{
		return EBlock::AlreadyResearched;
	}
	if (Facts.bQueued)
	{
		return EBlock::AlreadyQueued;
	}
	if (Facts.HeroTier < Row.MinTier)
	{
		return EBlock::TierTooLow;
	}
	if (Facts.HeroLevel < Row.MinHeroLevel)
	{
		return EBlock::HeroLevelTooLow;
	}
	if (!Facts.bRequiresDone)
	{
		return EBlock::RequiresMissing;
	}
	if (!Facts.bBuildingFinished)
	{
		return EBlock::NoBuilding;
	}
	if (!Facts.bCanPay)
	{
		return EBlock::CannotPay;
	}
	return EBlock::None;
}

bool HOLTechnology::IsPlayable(const FHOLTechnologyRow& Row, TFunctionRef<bool(FName)> BuildingTypeExists)
{
	return BuildingTypeExists(Row.Building);
}

FHOLProductionCost HOLTechnology::MakeCost(const FHOLTechnologyRow& Row, const float StepSeconds)
{
	FHOLProductionCost Cost;
	Cost.Resources = Row.Cost;
	Cost.BuildSteps = FMath::Max(1, FMath::RoundToInt(Row.ResearchTime / FMath::Max(StepSeconds, UE_KINDA_SMALL_NUMBER)));
	Cost.Population = 0;
	return Cost;
}

bool HOLTechnology::IsQueued(const TArray<FHOLProductionItem>& Queue, const FName Technology)
{
	return Queue.ContainsByPredicate([Technology](const FHOLProductionItem& Item) { return Item.TypeName == Technology; });
}

int32 HOLTechnology::GetBonusPercent(const TArray<FName>& Done, const FName Stat, TFunctionRef<const FHOLTechnologyRow*(FName)> Find)
{
	int32 Total = 0;
	for (const FName Name : Done)
	{
		const FHOLTechnologyRow* Row = Find(Name);
		if (!Row || !Row->Stats.Contains(Stat))
		{
			continue;
		}
		// The row's percent is its line's total: only what it adds to the technology it follows counts.
		int32 Gain = Row->Percent;
		if (!Row->Requires.IsNone() && Done.Contains(Row->Requires))
		{
			if (const FHOLTechnologyRow* Before = Find(Row->Requires))
			{
				Gain -= Before->Percent;
			}
		}
		Total += Gain;
	}
	return Total;
}

FName HOLTechnology::GetGatherStat(const EHOLResource Resource)
{
	switch (Resource)
	{
	case EHOLResource::Food: return TEXT("GatherFood");
	case EHOLResource::Wood: return TEXT("GatherWood");
	case EHOLResource::Stone: return TEXT("GatherStone");
	default: return TEXT("GatherGold");
	}
}