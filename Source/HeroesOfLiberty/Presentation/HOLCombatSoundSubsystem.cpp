#include "Presentation/HOLCombatSoundSubsystem.h"

#include "Engine/World.h"
#include "Match/HOLFactionRules.h"
#include "Simulation/HOLUnitView.h"

const FName UHOLCombatSoundSubsystem::AttackSlot(TEXT("Attack"));
const FName UHOLCombatSoundSubsystem::DeathSlot(TEXT("Death"));
const FName UHOLCombatSoundSubsystem::OrderSlot(TEXT("Order"));
const FName UHOLCombatSoundSubsystem::ConstructionSlot(TEXT("Construction"));
const FName UHOLCombatSoundSubsystem::MoveKind(TEXT("Move"));
const FName UHOLCombatSoundSubsystem::AttackKind(TEXT("Attack"));
const FName UHOLCombatSoundSubsystem::GatherKind(TEXT("Gather"));
const FName UHOLCombatSoundSubsystem::BuildKind(TEXT("Build"));
const FName UHOLCombatSoundSubsystem::GarrisonKind(TEXT("Garrison"));
const FName UHOLCombatSoundSubsystem::PlaceKind(TEXT("Place"));

void UHOLCombatSoundSubsystem::Count(const FName TypeName, const FName Slot)
{
	++EventCounts.FindOrAdd(Slot);
	++TypeEventCounts.FindOrAdd(TPair<FName, FName>(TypeName, Slot));
}

void UHOLCombatSoundSubsystem::Raise(const UWorld* World, const FName TypeName, const FName Faction, const FName Slot, const FVector& Location)
{
	UHOLCombatSoundSubsystem* Sounds = World ? World->GetSubsystem<UHOLCombatSoundSubsystem>() : nullptr;
	if (Sounds == nullptr)
	{
		return;
	}
	Sounds->Count(TypeName, Slot);
	Sounds->OnCombatSound.Broadcast(TypeName, Faction, Slot, Location);
}

void UHOLCombatSoundSubsystem::RaiseForSlot(const UWorld* World, const FName TypeName, const int32 OwnerSlot, const FName Slot, const FVector& Location)
{
	if (World == nullptr || World->GetSubsystem<UHOLCombatSoundSubsystem>() == nullptr)
	{
		return;
	}
	Raise(World, TypeName, HOLFactionRules::GetSlotFactionKey(World, OwnerSlot), Slot, Location);
}

void UHOLCombatSoundSubsystem::RaiseOrder(const UWorld* World, const FName Kind, const TConstArrayView<uint32> Units, const FVector& Point)
{
	UHOLCombatSoundSubsystem* Sounds = World ? World->GetSubsystem<UHOLCombatSoundSubsystem>() : nullptr;
	if (Sounds == nullptr || Units.IsEmpty())
	{
		return;
	}
	const FName TypeName = GetFirstUnitType(World, Units);
	Sounds->Count(TypeName, OrderSlot);
	Sounds->OnOrderSound.Broadcast(Kind, TypeName, Point);
}

void UHOLCombatSoundSubsystem::RaisePlacement(const UWorld* World, const FName BuildingType, const FVector& Point)
{
	UHOLCombatSoundSubsystem* Sounds = World ? World->GetSubsystem<UHOLCombatSoundSubsystem>() : nullptr;
	if (Sounds == nullptr || BuildingType.IsNone())
	{
		return;
	}
	Sounds->Count(BuildingType, ConstructionSlot);
	Sounds->OnOrderSound.Broadcast(PlaceKind, BuildingType, Point);
}

FName UHOLCombatSoundSubsystem::GetFirstUnitType(const UWorld* World, const TConstArrayView<uint32> Units)
{
	if (World == nullptr)
	{
		return NAME_None;
	}
	for (const uint32 UnitId : Units)
	{
		FHOLUnitView View;
		if (HOLUnitView::Find(*World, UnitId, View))
		{
			return View.TypeName;
		}
	}
	return NAME_None;
}

int32 UHOLCombatSoundSubsystem::GetEventCount(const FName Slot) const
{
	const int32* Found = EventCounts.Find(Slot);
	return Found ? *Found : 0;
}

int32 UHOLCombatSoundSubsystem::GetEventCountFor(const FName TypeName, const FName Slot) const
{
	const int32* Found = TypeEventCounts.Find(TPair<FName, FName>(TypeName, Slot));
	return Found ? *Found : 0;
}

bool UHOLCombatSoundSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->GetNetMode() != NM_DedicatedServer && !IsRunningDedicatedServer();
}

bool UHOLCombatSoundSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
