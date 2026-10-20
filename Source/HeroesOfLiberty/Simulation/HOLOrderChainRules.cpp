// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLOrderChainRules.h"

namespace HOLOrderChain
{
	void MakeChains(const TConstArrayView<FUnitSteps> Units, TArray<FChain>& OutChains)
	{
		OutChains.Reset();
		for (const FUnitSteps& Unit : Units)
		{
			const int32 NumSteps = FMath::Min(Unit.Steps.Num(), MaxSteps);
			if (NumSteps == 0)
			{
				continue;
			}
			FChain* Found = OutChains.FindByPredicate([&Unit, NumSteps](const FChain& Chain)
			{
				if (Chain.Steps.Num() != NumSteps)
				{
					return false;
				}
				for (int32 Index = 0; Index < NumSteps; ++Index)
				{
					if (Chain.Steps[Index] != Unit.Steps[Index])
					{
						return false;
					}
				}
				return true;
			});
			if (Found == nullptr)
			{
				Found = &OutChains.AddDefaulted_GetRef();
				Found->Steps.Append(Unit.Steps.GetData(), NumSteps);
			}
			Found->UnitIds.Add(Unit.UnitId);
		}
	}

	const TCHAR* KindName(const EStepKind Kind)
	{
		switch (Kind)
		{
		case EStepKind::Move: return TEXT("Move");
		case EStepKind::Attack: return TEXT("Attack");
		case EStepKind::AttackMove: return TEXT("AttackMove");
		case EStepKind::Build: return TEXT("Build");
		case EStepKind::Gather: return TEXT("Gather");
		case EStepKind::Rally: return TEXT("Rally");
		}
		return TEXT("?");
	}
}
