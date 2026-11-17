// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSmallRulesReadLibrary.h"

#include "Match/HOLForumRules.h"
#include "Match/HOLResources.h"
#include "Simulation/HOLHealRules.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Technologies/HOLTechnologyRules.h"

namespace
{
	EHOLXPSource ToSource(const int32 Source)
	{
		return Source >= 0 && Source <= static_cast<int32>(EHOLXPSource::Exploration) ? static_cast<EHOLXPSource>(Source) : EHOLXPSource::Exploration;
	}

	HOLHeal::FCandidate MakeCandidate(const int32 UnitId, const float X, const float Y, const float Health, const float MaxHealth, const int32 Camp, const bool bUnhealable)
	{
		HOLHeal::FCandidate Candidate;
		Candidate.UnitId = static_cast<uint32>(UnitId);
		Candidate.Location = FVector2D(X, Y);
		Candidate.Health = Health;
		Candidate.MaxHealth = MaxHealth;
		Candidate.Camp = static_cast<uint8>(Camp);
		Candidate.bUnhealable = bUnhealable;
		return Candidate;
	}
}

int32 UHOLHeroXPReadLibrary::GetWeightPercent(const int32 Source) { return HOLHeroXP::GetWeightPercent(ToSource(Source)); }

int32 UHOLHeroXPReadLibrary::GetValue(const int32 Food, const int32 Wood, const int32 Stone, const int32 Gold)
{
	return HOLHeroXP::GetValue(FHOLResources(Food, Wood, Stone, Gold));
}

int32 UHOLHeroXPReadLibrary::GetXP(const int32 Value, const int32 Source) { return HOLHeroXP::GetXP(Value, ToSource(Source)); }

int32 UHOLHeroXPReadLibrary::GetHeroKillXP(const int32 VictimLevel, const int32 KillerLevel, const int32 Source)
{
	return HOLHeroXP::GetHeroKillXP(VictimLevel, KillerLevel, ToSource(Source));
}

bool UHOLHeroXPReadLibrary::CountsWhileDead(const int32 Source) { return HOLHeroXP::CountsWhileDead(ToSource(Source)); }

bool UHOLHeroXPReadLibrary::GetKillSource(const bool bKilledByHero, const bool bHeroAlive, const float DistanceToHero, int32& OutSource)
{
	const TOptional<EHOLXPSource> Source = HOLHeroXP::GetKillSource(bKilledByHero, bHeroAlive, DistanceToHero);
	if (!Source.IsSet())
	{
		return false;
	}
	OutSource = static_cast<int32>(Source.GetValue());
	return true;
}

int32 UHOLHeroXPReadLibrary::GetLevelXP(const int32 Level) { return HOLHeroXP::GetLevelXP(Level); }
int32 UHOLHeroXPReadLibrary::GetLevel(const int32 XP) { return HOLHeroXP::GetLevel(XP); }
int32 UHOLHeroXPReadLibrary::GetTier(const int32 Level) { return HOLHeroXP::GetTier(Level); }
int32 UHOLHeroXPReadLibrary::GetStatPercent(const int32 Level) { return HOLHeroXP::GetStatPercent(Level); }
float UHOLHeroXPReadLibrary::GetLevelStat(const float Level1Stat, const int32 Level) { return HOLHeroXP::GetLevelStat(Level1Stat, Level); }

float UHOLHeroXPReadLibrary::GetHealthAfterLevelUp(const float Health, const float Level1MaxHealth, const int32 OldLevel, const int32 NewLevel)
{
	return HOLHeroXP::GetHealthAfterLevelUp(Health, Level1MaxHealth, OldLevel, NewLevel);
}

int32 UHOLHeroXPReadLibrary::GetForumLives(const int32 Tier, const int32 Falls) { return HOLForumRules::GetLives(Tier, Falls); }

bool UHOLHealReadLibrary::IsHealable(const int32 HealerId, const int32 HealerCamp, const int32 UnitId, const float X, const float Y, const float Health, const float MaxHealth, const int32 Camp, const bool bUnhealable)
{
	return HOLHeal::IsHealable(static_cast<uint32>(HealerId), static_cast<uint8>(HealerCamp), MakeCandidate(UnitId, X, Y, Health, MaxHealth, Camp, bUnhealable));
}

int32 UHOLHealReadLibrary::ChooseTarget(const int32 HealerId, const int32 HealerCamp, const float HealerX, const float HealerY, const float Range, const int32 OrderedId, const int32 KeptId,
	const TArray<int32>& Ids, const TArray<float>& Xs, const TArray<float>& Ys, const TArray<float>& Healths, const TArray<float>& MaxHealths,
	const TArray<int32>& Camps, const TArray<int32>& Unhealables)
{
	TArray<HOLHeal::FCandidate> Candidates;
	for (int32 Index = 0; Index < Ids.Num(); ++Index)
	{
		Candidates.Add(MakeCandidate(Ids[Index], Xs.IsValidIndex(Index) ? Xs[Index] : 0.f, Ys.IsValidIndex(Index) ? Ys[Index] : 0.f,
			Healths.IsValidIndex(Index) ? Healths[Index] : 0.f, MaxHealths.IsValidIndex(Index) ? MaxHealths[Index] : 0.f,
			Camps.IsValidIndex(Index) ? Camps[Index] : 0, Unhealables.IsValidIndex(Index) && Unhealables[Index] != 0));
	}
	return HOLHeal::ChooseTarget(static_cast<uint32>(HealerId), static_cast<uint8>(HealerCamp), FVector2D(HealerX, HealerY), Range,
		static_cast<uint32>(OrderedId), static_cast<uint32>(KeptId), Candidates);
}

int32 UHOLHealReadLibrary::GetOrderWalk(const float Distance, const float Range, const bool bWalking, const float GoalDrift)
{
	return static_cast<int32>(HOLHeal::GetOrderWalk(Distance, Range, bWalking, GoalDrift));
}

float UHOLHealReadLibrary::GetOrderStopMargin() { return HOLHeal::OrderStopMargin; }
float UHOLHealReadLibrary::GetOrderRepathDistance() { return HOLHeal::OrderRepathDistance; }

float UHOLHealReadLibrary::GetHealedHealth(const float Health, const float MaxHealth, const float HealRate, const int32 StepMs)
{
	return HOLHeal::GetHealedHealth(Health, MaxHealth, HealRate, StepMs);
}

FString UHOLHealReadLibrary::GetCostDiscountStat(const FString& UnitType)
{
	const FName Stat = HOLTechnology::GetCostDiscountStat(FName(*UnitType));
	return Stat.IsNone() ? FString() : Stat.ToString();
}

void UHOLHealReadLibrary::GetDiscountedCost(const int32 Food, const int32 Wood, const int32 Stone, const int32 Gold, const int32 Percent, int32& OutFood, int32& OutWood, int32& OutStone, int32& OutGold)
{
	const FHOLResources Cost = HOLTechnology::GetDiscountedCost(FHOLResources(Food, Wood, Stone, Gold), Percent);
	OutFood = Cost.Food;
	OutWood = Cost.Wood;
	OutStone = Cost.Stone;
	OutGold = Cost.Gold;
}
