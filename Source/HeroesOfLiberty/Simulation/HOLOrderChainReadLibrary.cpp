// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLOrderChainReadLibrary.h"

#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLOrderChainRules.h"

namespace
{
	/** A building reference for the template of HOLOrderChain::EndBuildingTarget: an id, 0 for none (Reset, ==). */
	struct FBuildingIdRef
	{
		int32 Id = 0;
		void Reset() { Id = 0; }
		bool operator==(const FBuildingIdRef& Other) const { return Id == Other.Id; }
	};

	HOLOrderChain::FStep ToRule(const FHOLOrderChainStep& Step)
	{
		HOLOrderChain::FStep Out;
		Out.Kind = static_cast<HOLOrderChain::EStepKind>(Step.Kind);
		Out.Point = Step.Point;
		Out.TargetId = static_cast<uint32>(Step.TargetId);
		Out.SourceIndex = Step.SourceIndex;
		return Out;
	}

	FHOLOrderChainStep FromRule(const HOLOrderChain::FStep& Step)
	{
		FHOLOrderChainStep Out;
		Out.Kind = static_cast<EHOLOrderChainStepKind>(Step.Kind);
		Out.Point = Step.Point;
		Out.TargetId = static_cast<int32>(Step.TargetId);
		Out.SourceIndex = Step.SourceIndex;
		return Out;
	}
}

int32 UHOLOrderChainReadLibrary::GetMaxSteps()
{
	return HOLOrderChain::MaxSteps;
}

FHOLOrderChainStep UHOLOrderChainReadLibrary::MakeMove(const FVector Point, const bool bAttackMove)
{
	return FromRule(HOLOrderChain::FStep::MakeMove(Point, bAttackMove));
}

FHOLOrderChainStep UHOLOrderChainReadLibrary::MakeAttack(const int32 UnitId)
{
	return FromRule(HOLOrderChain::FStep::MakeAttack(static_cast<uint32>(UnitId)));
}

FHOLOrderChainStep UHOLOrderChainReadLibrary::MakeBuild(const int32 SiteId)
{
	return FromRule(HOLOrderChain::FStep::MakeBuild(static_cast<uint32>(SiteId)));
}

FHOLOrderChainStep UHOLOrderChainReadLibrary::MakeAttackBuilding(const int32 BuildingId)
{
	return FromRule(HOLOrderChain::FStep::MakeAttackBuilding(static_cast<uint32>(BuildingId)));
}

FHOLOrderChainStep UHOLOrderChainReadLibrary::MakeDeposit(const int32 BuildingId)
{
	return FromRule(HOLOrderChain::FStep::MakeDeposit(static_cast<uint32>(BuildingId)));
}

FHOLOrderChainStep UHOLOrderChainReadLibrary::MakeGather(const int32 SourceIndex)
{
	return FromRule(HOLOrderChain::FStep::MakeGather(SourceIndex));
}

FHOLOrderChainStep UHOLOrderChainReadLibrary::MakeRally(const FVector Point)
{
	return FromRule(HOLOrderChain::FStep::MakeRally(Point));
}

TArray<FHOLOrderChain> UHOLOrderChainReadLibrary::MakeChains(const TArray<FHOLOrderChainUnitSteps>& Units)
{
	TArray<HOLOrderChain::FUnitSteps> RuleUnits;
	RuleUnits.Reserve(Units.Num());
	for (const FHOLOrderChainUnitSteps& Unit : Units)
	{
		HOLOrderChain::FUnitSteps& RuleUnit = RuleUnits.AddDefaulted_GetRef();
		RuleUnit.UnitId = static_cast<uint32>(Unit.UnitId);
		for (const FHOLOrderChainStep& Step : Unit.Steps)
		{
			RuleUnit.Steps.Add(ToRule(Step));
		}
	}
	TArray<HOLOrderChain::FChain> RuleChains;
	HOLOrderChain::MakeChains(RuleUnits, RuleChains);

	TArray<FHOLOrderChain> Out;
	Out.Reserve(RuleChains.Num());
	for (const HOLOrderChain::FChain& RuleChain : RuleChains)
	{
		FHOLOrderChain& Chain = Out.AddDefaulted_GetRef();
		for (const uint32 UnitId : RuleChain.UnitIds)
		{
			Chain.UnitIds.Add(static_cast<int32>(UnitId));
		}
		for (const HOLOrderChain::FStep& Step : RuleChain.Steps)
		{
			Chain.Steps.Add(FromRule(Step));
		}
	}
	return Out;
}

FString UHOLOrderChainReadLibrary::KindName(const EHOLOrderChainStepKind Kind)
{
	return FString(HOLOrderChain::KindName(static_cast<HOLOrderChain::EStepKind>(Kind)));
}

bool UHOLOrderChainReadLibrary::IsCurrentOrderOver(const bool bWalkingOrder, const bool bOrderedTarget, const bool bOrderedBuilding,
	const bool bAttackMoveOn, const bool bDroppingOff)
{
	return HOLOrderChain::IsCurrentOrderOver(bWalkingOrder, bOrderedTarget, bOrderedBuilding, bAttackMoveOn, bDroppingOff);
}

bool UHOLOrderChainReadLibrary::MaySeek(const bool bWalkingOrder, const bool bAttackMove, const bool bOrderedTarget, const bool bOrderedBuilding,
	const bool bWalkStalled)
{
	return HOLCombat::MaySeek(bWalkingOrder, bAttackMove, bOrderedTarget, bOrderedBuilding, bWalkStalled);
}

void UHOLOrderChainReadLibrary::EndBuildingTarget(int32& TargetId, int32& OrderedId)
{
	FBuildingIdRef Target;
	Target.Id = TargetId;
	FBuildingIdRef Ordered;
	Ordered.Id = OrderedId;
	HOLOrderChain::EndBuildingTarget(Target, Ordered);
	TargetId = Target.Id;
	OrderedId = Ordered.Id;
}
