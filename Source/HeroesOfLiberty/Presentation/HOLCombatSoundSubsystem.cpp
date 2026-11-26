#include "Presentation/HOLCombatSoundSubsystem.h"

#include "Engine/World.h"
#include "Match/HOLFactionRules.h"

const FName UHOLCombatSoundSubsystem::AttackSlot(TEXT("Attack"));
const FName UHOLCombatSoundSubsystem::DeathSlot(TEXT("Death"));

void UHOLCombatSoundSubsystem::Raise(const UWorld* World, const FName TypeName, const FName Faction, const FName Slot, const FVector& Location)
{
	UHOLCombatSoundSubsystem* Sounds = World ? World->GetSubsystem<UHOLCombatSoundSubsystem>() : nullptr;
	if (Sounds == nullptr)
	{
		return;
	}
	++Sounds->EventCounts.FindOrAdd(Slot);
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

int32 UHOLCombatSoundSubsystem::GetEventCount(const FName Slot) const
{
	const int32* Count = EventCounts.Find(Slot);
	return Count ? *Count : 0;
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
