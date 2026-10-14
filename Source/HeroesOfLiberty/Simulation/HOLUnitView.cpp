// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitView.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace HOLUnitView
{
	void Gather(const UWorld& World, TArray<FHOLUnitView>& OutUnits)
	{
		OutUnits.Reset();
		// A client simulates nothing: its units are the ones it displays (D271).
		if (World.GetNetMode() == NM_Client)
		{
			if (const UHOLUnitReceptionSubsystem* Reception = World.GetSubsystem<UHOLUnitReceptionSubsystem>())
			{
				Reception->GetDisplayedUnits(OutUnits);
			}
		}
		else if (const UHOLUnitSubsystem* Units = World.GetSubsystem<UHOLUnitSubsystem>())
		{
			Units->GetUnitViews(OutUnits);
		}
	}
}

namespace
{
	/** Scripted checks of J4 (D289): where this machine has the units (a client: where it draws them). */
	FAutoConsoleCommand UnitsWhereCommand(
		TEXT("hol.Units.Where"),
		TEXT("hol.Units.Where [Slot]: logs the units known on this machine (of one slot, or all) and where they are"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 Slot = Args.IsEmpty() ? INDEX_NONE : FCString::Atoi(*Args[0]);
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				const UWorld* World = Context.World();
				if (!World || !World->IsGameWorld())
				{
					continue;
				}
				TArray<FHOLUnitView> Units;
				HOLUnitView::Gather(*World, Units);
				Units.Sort([](const FHOLUnitView& A, const FHOLUnitView& B) { return A.UnitId < B.UnitId; });
				TArray<FString> Lines;
				for (const FHOLUnitView& Unit : Units)
				{
					if (Slot == INDEX_NONE || Unit.OwnerPlayer == Slot)
					{
						Lines.Add(FString::Printf(TEXT("#%u slot %d at (%.0f, %.0f)"), Unit.UnitId, Unit.OwnerPlayer, Unit.Location.X, Unit.Location.Y));
					}
				}
				UE_LOG(LogHOLSimulation, Display, TEXT("Units where: %d units: %s"), Lines.Num(), *FString::Join(Lines, TEXT("; ")));
			}
		}));
}
