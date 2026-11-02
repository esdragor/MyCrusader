// Copyright Epic Games, Inc. All Rights Reserved.

#include "Technologies/HOLTechnologyRules.h"

#include "Simulation/HOLUnitTypes.h"

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
	// T-Q11 = B (D601): no code applies its effect yet, whatever the rest says.
	if (!Row.Implemented)
	{
		return EBlock::NotImplemented;
	}
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

const TCHAR* HOLTechnology::DescribeBlock(const EBlock Block)
{
	switch (Block)
	{
	case EBlock::NotImplemented: return TEXT("soon");
	case EBlock::AlreadyResearched: return TEXT("already researched");
	case EBlock::AlreadyQueued: return TEXT("already queued");
	case EBlock::TierTooLow: return TEXT("tier too low");
	case EBlock::HeroLevelTooLow: return TEXT("hero level too low");
	case EBlock::RequiresMissing: return TEXT("requires missing");
	case EBlock::NoBuilding: return TEXT("no building");
	case EBlock::CannotPay: return TEXT("cannot pay");
	default: return TEXT("none");
	}
}

bool HOLTechnology::IsPlayable(const FHOLTechnologyRow& Row, TFunctionRef<bool(FName)> BuildingTypeExists)
{
	return Row.Implemented && BuildingTypeExists(Row.Building);
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

int32 HOLTechnology::ScaleByPercent(const int32 Base, const int32 Percent)
{
	return Base + FMath::RoundToInt(static_cast<double>(Base) * FMath::Max(Percent, 0) / 100.0);
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
FName HOLTechnology::GetCostDiscountStat(const FName UnitType)
{
	static const FName Monk(TEXT("Monk"));
	static const FName MonkCostDiscount(TEXT("MonkCostDiscount"));
	return UnitType == Monk ? MonkCostDiscount : NAME_None;
}

FHOLResources HOLTechnology::GetDiscountedCost(const FHOLResources& Cost, const int32 Percent)
{
	const int32 Kept = 100 - FMath::Clamp(Percent, 0, 90);
	const auto Scale = [Kept](const int32 Amount) { return FMath::RoundToInt(static_cast<float>(Amount * Kept) / 100.f); };
	return FHOLResources(Scale(Cost.Food), Scale(Cost.Wood), Scale(Cost.Stone), Scale(Cost.Gold));
}

FName HOLTechnology::GetAttackStat(const EHOLAttackType AttackType)
{
	switch (AttackType)
	{
	case EHOLAttackType::Melee: return MeleeAttackStat;
	case EHOLAttackType::Ranged: return RangedAttackStat;
	default: return NAME_None;
	}
}