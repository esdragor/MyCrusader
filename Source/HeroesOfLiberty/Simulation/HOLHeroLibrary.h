// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Simulation/HOLHeroAbilityRules.h"
#include "Simulation/HOLHeroSheet.h"
#include "HOLHeroLibrary.generated.h"

/**
 * The hero's return for Blueprint and Angelscript (J8a.4, J8a4-Q6 = A): the rule and the decision live in script
 * (Script/Hero, namespace HOLHero:: there, the library's name without U and Library); C++ keeps the network, the gold and
 * the birth. Slots and steps in int32 (script has no uint8 nor int64). Reading works everywhere (a client reads its
 * copies); acting is for the authority (server, solo) and answers false on a client.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLHeroLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The slot's hero state: the true sheet on the server, the game state's public copy on a client. None out of range. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static EHOLHeroState GetHeroState(const UObject* WorldContextObject, int32 Slot);

	/** The slot's hero level (0 without a hero), as GetHeroState reads it. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static int32 GetHeroLevel(const UObject* WorldContextObject, int32 Slot);

	/** The slot's hero exact HP (0 dead or without a hero), as GetHeroState reads it (J8a.5: the hero's block). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static int32 GetHeroHealth(const UObject* WorldContextObject, int32 Slot);

	/**
	 * A hero's HP when whole: the Hero type's MaxHealth (450, J8a-Q1) at the level of the slot's public sheet (J8b-Q3: 486
	 * at level 2; level 1 for Slot -1 or no level yet); 0 without the unit types (J8a.5: the hero's block).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static int32 GetHeroMaxHealth(const UObject* WorldContextObject, int32 Slot = -1);

	/**
	 * The step the slot's hero fell at (-1 alive or without a hero): the true sheet on the server; on a client, the local
	 * player's own private copy, -1 for any other slot (private, D270).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static int32 GetHeroDeathStep(const UObject* WorldContextObject, int32 Slot);

	/**
	 * J8b.4: the slot's hero XP where it is known (J8-Q5: to its owner alone): the true sheet on the server or in solo; on a
	 * client, the local player's own private copy, -1 for any other slot.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static int32 GetHeroXP(const UObject* WorldContextObject, int32 Slot);

	/** J8b.4: the total XP to reach Level (HOLHeroXP::GetLevelXP: 0, 200, 600, 1 300, 2 200, 3 300; clamped to 1..6). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero")
	static int32 GetLevelXP(int32 Level);

	/** J8b.4: the prototype's last level (HOLHeroXP::MaxLevel, 6). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero")
	static int32 GetMaxLevel();

	/** Slots of a match (FHOLMatchConfig::MaxSlots): the slots run 0 to this less one. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero")
	static int32 GetNumSlots();

	/** The slot is out of the match (AHOLGameState::IsSlotDefeated); false without a game state. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static bool IsSlotDefeated(const UObject* WorldContextObject, int32 Slot);

	/**
	 * The slot's gold (J8a.5 loop 1b, the return's quote): its stock on the server; on a client, the local player's own,
	 * -1 for any other slot (its stock replicates to its owner only); -1 without stock (an AI slot, no player state).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static int32 GetSlotGold(const UObject* WorldContextObject, int32 Slot);

	/** Simulation steps per second (UHOLSimulationSettings::SimulationRateHz, D217). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero")
	static int32 GetSimulationRateHz();

	/**
	 * The return times' percent (hol.Hero.ReturnTimePercent, J8a.4 loop 5): the delay and the price's drop at this percent of
	 * the rules' for the network test; 100 (the default) the real rules; 1 at least.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero")
	static int32 GetReturnTimePercent();

	/**
	 * The simulation's step (UHOLSimulationSubsystem::GetStepIndex, the clock of the death steps) on the server; on a
	 * client, the server's step estimated by the reception clock, -1 before its first sample.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static int32 GetServerStep(const UObject* WorldContextObject);

	/**
	 * J8c.3: the step the slot's Ability is ready again (private sheet, J8-Q5: the owner's slot only on a client; compare
	 * with GetServerStep); 0 never cast; -1 unknown (another's slot on a client, out of range).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static int32 GetHeroAbilityReadyStep(const UObject* WorldContextObject, int32 Slot, EHOLHeroAbility Ability);

	/** J8c.3: the level the hero's Ability opens at (D52: 1, the banner 4). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero")
	static int32 GetHeroAbilityUnlockLevel(EHOLHeroAbility Ability);

	/** J8c.4: the whole cool-down of the hero's Ability, in seconds (the cry 60, the banner 120; the aura 0). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero")
	static int32 GetHeroAbilityCooldownSeconds(EHOLHeroAbility Ability);

	/** J8c.3: the slot's hero's cries so far (public sheet, wraps at 256): one more, the cry is shown. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static int32 GetHeroCries(const UObject* WorldContextObject, int32 Slot);

	/** J8c.3: the step the slot's last banner falls at (public sheet; -1 never planted), and where it stands (cm). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static int32 GetHeroBannerEndStep(const UObject* WorldContextObject, int32 Slot);
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static FIntPoint GetHeroBannerLocation(const UObject* WorldContextObject, int32 Slot);

	/** The slot's main forum is in the world and not down (ruin, rebuilding). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static bool IsMainForumStanding(const UObject* WorldContextObject, int32 Slot);

	/** The slot's main forum's place (cm, rounded), standing or down; (0, 0) without one. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static FIntPoint GetMainForumPosition(const UObject* WorldContextObject, int32 Slot);

	/** The slot's other forums' places (cm), in the order RequestHeroReturn's PlaceIndex names them: none before D384. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static TArray<FIntPoint> GetOtherForumPositions(const UObject* WorldContextObject, int32 Slot);

	/** Server: the slot's dead hero ready to return (UHOLHeroSubsystem::SetReturnReady). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static bool SetHeroReturnReady(const UObject* WorldContextObject, int32 Slot);

	/**
	 * Server: Amount gold taken from the slot's stock if it has that much (true), nothing taken otherwise (false). False
	 * for a negative amount and for a slot without stock (no AHOLPlayerState: an AI slot before IA0.1).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static bool TryPayGold(const UObject* WorldContextObject, int32 Slot, int32 Amount);

	/** Server: the slot's dead hero born again by a forum (UHOLHeroSubsystem::RequestReturn; PlaceIndex -1 the main forum). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static bool RequestHeroReturn(const UObject* WorldContextObject, int32 Slot, int32 PlaceIndex);

	/**
	 * Server: the slot's paid return in one block (UHOLHeroSubsystem::BuyReturn): Price gold taken only if the slot has it
	 * and the hero is born (PlaceIndex as RequestHeroReturn's); refused, nothing is taken.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Hero", meta = (WorldContext = "WorldContextObject"))
	static bool BuyHeroReturn(const UObject* WorldContextObject, int32 Slot, int32 PlaceIndex, int32 Price);
};
