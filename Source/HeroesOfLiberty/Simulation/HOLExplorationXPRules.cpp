// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLExplorationXPRules.h"

#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarExplorationPreset(
		TEXT("hol.HeroXP.ExplorationPreset"),
		0,
		TEXT("B2 exploration XP preset (Retours/Q-exploration-xp.md): 0 = C tranches + places (default), 1 = A tranches only, 2 = B places only. Read when the fog starts."),
		ECVF_Default);

	int32 MarkCell(const HOLExplorationXP::FSettings& Settings, HOLExplorationXP::FState& State, const TConstArrayView<FIntPoint> Places, const FIntPoint& Cell)
	{
		if (Cell.X < 0 || Cell.Y < 0 || Cell.X >= State.GridCells || Cell.Y >= State.GridCells)
		{
			return 0;
		}
		const int32 Side = FMath::Max(1, Settings.CellsPerTranche);
		const int32 Tranche = (Cell.Y / Side) * State.TranchesPerSide + Cell.X / Side;
		int32 XP = 0;
		if (State.TrancheDone.IsValidIndex(Tranche) && !State.TrancheDone[Tranche])
		{
			State.TrancheDone[Tranche] = true;
			XP += Settings.XPPerTranche;
		}
		for (int32 Index = 0; Index < Places.Num() && Index < State.PlaceDone.Num(); ++Index)
		{
			if (!State.PlaceDone[Index] && Places[Index] == Cell)
			{
				State.PlaceDone[Index] = true;
				XP += Settings.XPPerPlace;
			}
		}
		return XP;
	}
}

HOLExplorationXP::FSettings HOLExplorationXP::Preset(const int32 Index)
{
	FSettings Settings;
	Settings.CellsPerTranche = 16;
	if (Index == 1)
	{
		Settings.XPPerTranche = 6;
		Settings.XPPerPlace = 0;
	}
	else if (Index == 2)
	{
		Settings.XPPerTranche = 0;
		Settings.XPPerPlace = 40;
	}
	else
	{
		Settings.XPPerTranche = 3;
		Settings.XPPerPlace = 25;
	}
	return Settings;
}

int32 HOLExplorationXP::GetPresetIndex()
{
	return FMath::Clamp(CVarExplorationPreset.GetValueOnGameThread(), 0, 2);
}

HOLExplorationXP::FState HOLExplorationXP::NewState(const FSettings& Settings, const int32 GridCells, const int32 NumPlaces)
{
	FState State;
	const int32 Side = FMath::Max(1, Settings.CellsPerTranche);
	State.GridCells = FMath::Max(0, GridCells);
	State.TranchesPerSide = (State.GridCells + Side - 1) / Side;
	State.TrancheDone.Init(false, State.TranchesPerSide * State.TranchesPerSide);
	State.PlaceDone.Init(false, FMath::Max(0, NumPlaces));
	return State;
}

void HOLExplorationXP::MarkExploredAtStart(FState& State, const FSettings& Settings, const TConstArrayView<FIntPoint> Places, const FIntPoint& Cell)
{
	MarkCell(Settings, State, Places, Cell);
}

int32 HOLExplorationXP::OnCellExplored(const FSettings& Settings, FState& State, const TConstArrayView<FIntPoint> Places, const FIntPoint& Cell, const bool bHeroDead)
{
	return MarkCell(Settings, State, Places, Cell);
}
