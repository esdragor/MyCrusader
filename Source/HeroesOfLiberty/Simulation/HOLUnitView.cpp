// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitView.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Simulation/HOLHeroSheet.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace HOLUnitView
{
	void Gather(const UWorld& World, TArray<FHOLUnitView>& OutUnits, const bool bIncludeDying)
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
		if (!bIncludeDying)
		{
			OutUnits.RemoveAll([](const FHOLUnitView& Unit) { return Unit.Activity == EHOLUnitActivity::Dying; });
		}
	}

	bool Find(const UWorld& World, const uint32 UnitId, FHOLUnitView& OutUnit, const bool bIncludeDying)
	{
		bool bFound = false;
		if (World.GetNetMode() == NM_Client)
		{
			const UHOLUnitReceptionSubsystem* Reception = World.GetSubsystem<UHOLUnitReceptionSubsystem>();
			bFound = Reception && Reception->GetDisplayedUnit(UnitId, OutUnit);
		}
		else
		{
			const UHOLUnitSubsystem* Units = World.GetSubsystem<UHOLUnitSubsystem>();
			bFound = Units && Units->GetUnitView(UnitId, OutUnit);
		}
		return bFound && (bIncludeDying || OutUnit.Activity != EHOLUnitActivity::Dying);
	}

	void SetReceivedHealth(FHOLUnitView& View, const bool bFullHealth, const uint16 WireHealth, const int32 MaxHealth, const bool bHero,
		const FHOLHeroSheet* HeroSheet)
	{
		View.bFullHealth = bFullHealth;
		View.MaxHealth = MaxHealth;
		View.Health = GetReceivedHealth(bFullHealth, WireHealth, MaxHealth, bHero);
		View.HealthShare = bHero ? GetReceivedHealthShare(bFullHealth, WireHealth) : -1.f;
		View.bHealthExact = !bHero;
		// His sheet alive: its exact HP while the body received lives (a fall reaches the body and the sheet at their own
		// pace: the body's 0 is kept), the share still filling the bar. Unhurt by the body: his maximum, exact as well.
		if (bHero && HeroSheet && HeroSheet->State == EHOLHeroState::Alive && HeroSheet->Health > 0 && View.Health > 0)
		{
			if (!bFullHealth)
			{
				View.Health = FMath::Min(static_cast<int32>(HeroSheet->Health), MaxHealth);
			}
			View.bHealthExact = true;
		}
	}
}

namespace
{
	/**
	 * Scripted checks of J4 (D289): where this machine has the units (a client: where it draws them); and of J5 (D299,
	 * D303): what each one does and carries, its amount as "?" when this machine does not know it.
	 */
	FAutoConsoleCommand UnitsWhereCommand(
		TEXT("hol.Units.Where"),
		TEXT("hol.Units.Where [Slot]: logs the units known on this machine (of one slot, or all), where they are, what they do and carry"),
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
				HOLUnitView::Gather(*World, Units, true);
				Units.Sort([](const FHOLUnitView& A, const FHOLUnitView& B) { return A.UnitId < B.UnitId; });
				TArray<FString> Lines;
				for (const FHOLUnitView& Unit : Units)
				{
					if (Slot == INDEX_NONE || Unit.OwnerPlayer == Slot)
					{
						FString Line = FString::Printf(TEXT("#%u slot %d at (%.0f, %.0f) %s"), Unit.UnitId, Unit.OwnerPlayer, Unit.Location.X,
							Unit.Location.Y, *StaticEnum<EHOLUnitActivity>()->GetNameStringByValue(static_cast<int64>(Unit.Activity)));
						if (Unit.bCarrying || (Unit.bLoadAmountKnown && Unit.LoadAmount > 0))
						{
							Line += FString::Printf(TEXT(", carries %s %s"), *StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Unit.LoadResource)),
								Unit.bLoadAmountKnown ? *FString::FromInt(Unit.LoadAmount) : TEXT("?"));
						}
						if (Unit.Activity == EHOLUnitActivity::Gathering)
						{
							// Last, so the activity and the load read as before (quick_check).
							Line += FString::Printf(TEXT(", gathers %s"), *StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Unit.GatherResource)));
						}
						// J7b.4: the health of a wounded unit (exact, J7-Q5 revised), the target of an attacking one. A client's
						// hero (J8a.3): the share received too, whatever his health points come from (his sheet, or the share).
						if (!Unit.bFullHealth)
						{
							Line += FString::Printf(TEXT(", hp %d/%d"), Unit.Health, Unit.MaxHealth);
							if (Unit.HealthShare >= 0.f)
							{
								Line += FString::Printf(TEXT(" share %d/%d"), FMath::RoundToInt(Unit.HealthShare * HOLUnitView::MaxWireHealthShare),
									HOLUnitView::MaxWireHealthShare);
							}
						}
						if (Unit.Activity == EHOLUnitActivity::Attacking)
						{
							Line += FString::Printf(TEXT(", target #%u"), Unit.TargetUnitId);
						}
						Lines.Add(MoveTemp(Line));
					}
				}
				UE_LOG(LogHOLSimulation, Display, TEXT("Units where: %d units: %s"), Lines.Num(), *FString::Join(Lines, TEXT("; ")));
			}
		}));
}
