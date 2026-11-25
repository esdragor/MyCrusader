#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "HOLCombatSoundSubsystem.generated.h"

/** AR6-c: a typed combat sound to play (the unit's type, its owner's faction key, the sound's slot of the art asset, where). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FHOLCombatSoundSignature, FName, TypeName, FName, Faction, FName, Slot, FVector, Location);

/**
 * AR6-c (ArtistesInProgress.md): the combat sound events of the Mass simulation, one per blow (UHOLCombatProcessor, at the
 * strike's moment, melee and ranged) and one per body laid (HOLCorpse::Lay, the death), handed to the script's
 * HOLAudioPlayer.PlayCombat through OnCombatSound. C++ because they are raised per entity and per step inside the simulation.
 * Never on a dedicated server (no sound there): the subsystem does not exist, the event is dropped at no cost.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLCombatSoundSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static const FName AttackSlot;
	static const FName DeathSlot;

	/** Bound by the script's game screen to HOLAudioPlayer.PlayCombat. */
	UPROPERTY(BlueprintAssignable, Category = "HeroesOfLiberty|Audio")
	FHOLCombatSoundSignature OnCombatSound;

	/** One sound event in World (nothing on a dedicated server or a world without the subsystem). */
	static void Raise(const UWorld* World, FName TypeName, FName Faction, FName Slot, const FVector& Location);
	/** The same with the owner's slot: its faction key read from the launch (HOLFactionRules::GetSlotFactionKey). */
	static void RaiseForSlot(const UWorld* World, FName TypeName, int32 OwnerSlot, FName Slot, const FVector& Location);

	/** The events raised for Slot since the world began (tests, trials). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Audio")
	int32 GetEventCount(FName Slot) const;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	TMap<FName, int32> EventCounts;
};
