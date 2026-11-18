// Copyright Epic Games, Inc. All Rights Reserved.

#include "Technologies/HOLTechnologyCheckLibrary.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Simulation/HOLUnitTypes.h"
#include "Technologies/HOLTechnologyRules.h"

int32 UHOLTechnologyCheckLibrary::Check(const bool bImplemented, const int32 HeroTier, const int32 HeroLevel, const int32 MinTier, const int32 MinHeroLevel,
	const bool bRequiresDone, const bool bBuildingFinished, const bool bCanPay, const bool bResearched, const bool bQueued)
{
	FHOLTechnologyRow Row;
	Row.Implemented = bImplemented;
	Row.MinTier = MinTier;
	Row.MinHeroLevel = MinHeroLevel;
	HOLTechnology::FFacts Facts;
	Facts.HeroTier = HeroTier;
	Facts.HeroLevel = HeroLevel;
	Facts.bRequiresDone = bRequiresDone;
	Facts.bBuildingFinished = bBuildingFinished;
	Facts.bCanPay = bCanPay;
	Facts.bResearched = bResearched;
	Facts.bQueued = bQueued;
	// The numbers of Script/Technologies/HOLTechnologyRules.as (the script has no enum of the simulation).
	switch (HOLTechnology::Check(Row, Facts))
	{
	case HOLTechnology::EBlock::None: return 0;
	case HOLTechnology::EBlock::AlreadyResearched: return 1;
	case HOLTechnology::EBlock::TierTooLow: return 2;
	case HOLTechnology::EBlock::HeroLevelTooLow: return 3;
	case HOLTechnology::EBlock::RequiresMissing: return 4;
	case HOLTechnology::EBlock::NoBuilding: return 5;
	case HOLTechnology::EBlock::CannotPay: return 6;
	case HOLTechnology::EBlock::AlreadyQueued: return 7;
	case HOLTechnology::EBlock::NotImplemented: return 8;
	}
	return -1;
}

FString UHOLTechnologyCheckLibrary::DescribeBlock(const int32 Reason)
{
	using HOLTechnology::EBlock;
	static const EBlock Blocks[] = { EBlock::None, EBlock::AlreadyResearched, EBlock::TierTooLow, EBlock::HeroLevelTooLow, EBlock::RequiresMissing,
		EBlock::NoBuilding, EBlock::CannotPay, EBlock::AlreadyQueued, EBlock::NotImplemented };
	return FString(HOLTechnology::DescribeBlock(Blocks[FMath::Clamp(Reason, 0, 8)]));
}

bool UHOLTechnologyCheckLibrary::IsPlayable(const FName Technology)
{
	const FHOLTechnologyRow* Row = UHOLTechnologySettings::FindTechnology(Technology);
	return Row && HOLTechnology::IsPlayable(*Row, [](const FName Building) { return UHOLBuildingSettings::FindType(Building) != nullptr; });
}

TArray<FName> UHOLTechnologyCheckLibrary::GetPlayableTechnologies()
{
	TArray<FName> Result;
	const UDataTable* Table = UHOLTechnologySettings::GetTable();
	if (Table)
	{
		for (const FName Name : Table->GetRowNames())
		{
			if (IsPlayable(Name))
			{
				Result.Add(Name);
			}
		}
	}
	return Result;
}

int32 UHOLTechnologyCheckLibrary::GetResearchQueueSize(const FName Technology)
{
	const FHOLTechnologyRow* Row = UHOLTechnologySettings::FindTechnology(Technology);
	const FHOLBuildingTypeRow* Building = Row ? UHOLBuildingSettings::FindType(Row->Building) : nullptr;
	return Building ? Building->MaxQueue : -1;
}

bool UHOLTechnologyCheckLibrary::GetResearchCost(const FName Technology, const float StepSeconds, FHOLResources& OutCost, int32& OutBuildSteps, int32& OutPopulation)
{
	const FHOLTechnologyRow* Row = UHOLTechnologySettings::FindTechnology(Technology);
	if (!Row)
	{
		return false;
	}
	const FHOLProductionCost Cost = HOLTechnology::MakeCost(*Row, StepSeconds);
	OutCost = Cost.Resources;
	OutBuildSteps = Cost.BuildSteps;
	OutPopulation = Cost.Population;
	return true;
}

int32 UHOLTechnologyCheckLibrary::GetBonusPercent(const TArray<FName>& Done, const FName Stat)
{
	return HOLTechnology::GetBonusPercent(Done, Stat, [](const FName Name) { return UHOLTechnologySettings::FindTechnology(Name); });
}

int32 UHOLTechnologyCheckLibrary::ScaleByPercent(const int32 Base, const int32 Percent)
{
	return HOLTechnology::ScaleByPercent(Base, Percent);
}

FName UHOLTechnologyCheckLibrary::GetAttackStat(const int32 AttackType)
{
	return HOLTechnology::GetAttackStat(static_cast<EHOLAttackType>(AttackType));
}

FName UHOLTechnologyCheckLibrary::GetArmorStat(const int32 ArmorCategoryMask)
{
	return HOLTechnology::GetArmorStat(ArmorCategoryMask);
}

bool UHOLTechnologyCheckLibrary::ArmorCoversAttack(const int32 AttackType)
{
	return HOLTechnology::ArmorCoversAttack(static_cast<EHOLAttackType>(AttackType));
}

bool UHOLTechnologyCheckLibrary::IsQueued(const FHOLProductionQueueState& Queue, const FName Technology)
{
	TArray<FHOLProductionItem> Items;
	for (const FName Type : Queue.Types)
	{
		Items.AddDefaulted_GetRef().TypeName = Type;
	}
	return HOLTechnology::IsQueued(Items, Technology);
}

bool UHOLTechnologyCheckLibrary::StateHas(const TArray<FName>& Done, const FName Technology)
{
	FHOLTechnologyState State;
	State.Done = Done;
	return State.Has(Technology);
}

bool UHOLTechnologyCheckLibrary::StateAdd(TArray<FName>& Done, const FName Technology)
{
	FHOLTechnologyState State;
	State.Done = Done;
	const bool bAdded = State.Add(Technology);
	Done = State.Done;
	return bAdded;
}
