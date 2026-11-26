#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "HOLCombatSoundSubsystem.generated.h"

/** AR6-c: a typed combat sound to play (the unit's type, its owner's faction key, the sound's slot of the art asset, where). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FHOLCombatSoundSignature, FName, TypeName, FName, Faction, FName, Slot, FVector, Location);

/** AR6-d: an order of the local player (Kind: Move, Attack, Gather, Build, Garrison, or Place for a foundation laid), the type that answers it, where. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHOLOrderSoundSignature, FName, Kind, FName, TypeName, FVector, Point);

/**
 * AR6-c / AR6-d (ArtistesInProgress.md): the typed sound events of the game, handed to the script's HOLAudioPlayer:
 * - combat (OnCombatSound): one per blow (UHOLCombatProcessor, melee and ranged; a tower's volley; on a client the blows its display
 *   replays, UHOLNetUnitDisplayProcessor, and the volleys a tower shows, AHOLBuilding::ShowArrowShot) and one per body laid (HOLCorpse::Lay);
 * - orders (OnOrderSound): one per order the local player gives (AHOLPlayerController::OrderAt, AttackMoveAt, OrderLineAt, a foundation laid),
 *   on the machine of that player only (input), its type the first commanded unit's (or the building laid).
 * C++ because the combat events are raised per entity and per step inside the simulation. Never on a dedicated server (no sound there):
 * the subsystem does not exist, the event is dropped at no cost.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLCombatSoundSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static const FName AttackSlot;
	static const FName DeathSlot;
	/** The art asset's slots of an order (OrderSound) and of a foundation laid (ConstructionSound); also the counters' keys. */
	static const FName OrderSlot;
	static const FName ConstructionSlot;
	/** The kinds of an order. */
	static const FName MoveKind;
	static const FName AttackKind;
	static const FName GatherKind;
	static const FName BuildKind;
	static const FName GarrisonKind;
	static const FName PlaceKind;

	/** Bound by the script's game screen to HOLAudioPlayer.PlayCombat. */
	UPROPERTY(BlueprintAssignable, Category = "HeroesOfLiberty|Audio")
	FHOLCombatSoundSignature OnCombatSound;

	/** Bound by the script's game screen to HOLAudioPlayer.PlayTyped (AR6-d). */
	UPROPERTY(BlueprintAssignable, Category = "HeroesOfLiberty|Audio")
	FHOLOrderSoundSignature OnOrderSound;

	/** One sound event in World (nothing on a dedicated server or a world without the subsystem). */
	static void Raise(const UWorld* World, FName TypeName, FName Faction, FName Slot, const FVector& Location);
	/** The same with the owner's slot: its faction key read from the launch (HOLFactionRules::GetSlotFactionKey). */
	static void RaiseForSlot(const UWorld* World, FName TypeName, int32 OwnerSlot, FName Slot, const FVector& Location);

	/** AR6-d: an order of Kind given to Units (its type: the first unit's, GetFirstUnitType), at Point. Nothing without units. */
	static void RaiseOrder(const UWorld* World, FName Kind, TConstArrayView<uint32> Units, const FVector& Point);
	/** AR6-d: a foundation of BuildingType laid at Point by the local player (Kind Place, the building's ConstructionSound). */
	static void RaisePlacement(const UWorld* World, FName BuildingType, const FVector& Point);
	/** The type of the first of Units this machine knows (HOLUnitView::Find: server, solo or client), NAME_None when none is known. */
	static FName GetFirstUnitType(const UWorld* World, TConstArrayView<uint32> Units);

	/** The events raised for Slot since the world began (tests, trials). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Audio")
	int32 GetEventCount(FName Slot) const;

	/** The events raised for TypeName in Slot since the world began (tests, trials). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Audio")
	int32 GetEventCountFor(FName TypeName, FName Slot) const;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void Count(FName TypeName, FName Slot);

	TMap<FName, int32> EventCounts;
	TMap<TPair<FName, FName>, int32> TypeEventCounts;
};
