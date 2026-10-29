// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLHeroSheet.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLHeroSubsystem.generated.h"

class AHOLPlayerState;
enum class EHOLHeroAbility : uint8;
enum class EHOLXPSource : uint8;
struct FHOLCombatFragment;
struct FHOLUnitStateFragment;
struct FHOLUnitTypeConstants;

/** J8a.4: a simulation step's end, the hero sheets already written (Step: UHOLSimulationSubsystem::GetStepIndex). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHOLOnHeroStep, int32, Step);

/**
 * The true hero sheets (J8-Q5, J8a3-Q1 = A), where the simulation runs (solo, server; it does nothing on a client): at
 * the end of every simulation step, each slot's sheet is made from his entity (HOLHeroSheet::Make: present, health, death
 * step) and its copies written, only when they change: the public part in AHOLGameState (every machine), the private part
 * in the slot's AHOLPlayerState (its owner; an AI slot has none, its private part stays here). It only reads the
 * simulation.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLHeroSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The slot's true sheet (solo, server); the default (None) out of range or on a client. */
	const FHOLHeroSheets& GetSheets(int32 Slot) const;

	/** Follows the simulation's steps (OnWorldBeginPlay; a test world calls it itself). Refused on a client: false. */
	bool BindSteps();

	/** Makes every slot's sheet from its hero now and writes the copies that changed (the end of each step). */
	void UpdateSheets();

	/**
	 * J8b.2: XP earned by the slot's side (a kill, a building finished; HOLHeroXP), added to his true sheet; the copies and
	 * his body's level follow at the step's end (UpdateSheets). Nothing out of range, on a client, or for XP <= 0.
	 */
	void AddXP(int32 Slot, int32 XP, EHOLXPSource Source);

	/** Development (hol.Hero.SetXP): the slot's XP set, its copies written at once. False out of range or on a client. */
	bool SetXP(int32 Slot, int32 XP);

	/** R7.10: the slot's copies written again now (a player back with a new player state). False out of range or on a client. */
	bool RewriteCopies(int32 Slot);

	/**
	 * J8a.4: the script's return rule (Script/Hero) follows the steps here: broadcast at the end of each simulation step,
	 * right after UpdateSheets, where the simulation runs (solo, server; never on a client).
	 */
	UPROPERTY(BlueprintAssignable, Category = "HeroesOfLiberty|Hero")
	FHOLOnHeroStep OnHeroStep;

	/**
	 * J8a.4: the slot's dead hero may come back (the delay over): his true sheet ReadyToReturn, kept until he is born
	 * again (HOLHeroSheet::Make), the copies written at once. False out of range, on a client, or when he is not dead.
	 */
	bool SetReturnReady(int32 Slot);

	/**
	 * J8a.4: the slot's dead hero born again at full health by its forum: PlaceIndex -1 the main forum (it must stand),
	 * another index one of the slot's other forums (none before D384). Born at the next step's start, through the
	 * starting hero's path (RequestUnits). False: on a client, out of range, the slot defeated, its hero not dead (alive,
	 * none, or a return already asked), or no such place.
	 */
	bool RequestReturn(int32 Slot, int32 PlaceIndex);

	/**
	 * J8a.4 loop 4: the slot's paid return in one block: Price gold taken from its stock only if it has that much and the
	 * hero is born (RequestReturn), nothing taken otherwise. False: on a client, Price negative, no stock (an AI slot, no
	 * player state), not enough gold, or RequestReturn refused (alive, a return already asked, no such place, defeated).
	 */
	bool BuyReturn(int32 Slot, int32 PlaceIndex, int32 Price);

	/**
	 * J8c.2: the slot's hero cries Serrez les rangs (J8c-Q2): his cool-down starts, and at the next combat step his units
	 * within HOLHeroAbility::AuraRadius of him are under the cry (ConsumeCloseRanks). False: on a client, out of range, the
	 * hero not alive, or the cry not ready (HOLHeroAbility::CanCast).
	 */
	bool CastCloseRanks(int32 Slot);

	/** J8c.2, the combat processor: whether the slot's hero cried since its last step (then forgotten). */
	bool ConsumeCloseRanks(int32 Slot);

	/**
	 * J8c.2: the slot's hero plants his banner at Location (J8c-Q3): it gives his aura there for HOLHeroAbility::
	 * BannerDurationMs, even once he is dead; one at a time, his cool-down starts. False: on a client, out of range, the
	 * hero not alive, below level 4 or the banner not ready (HOLHeroAbility::CanCast), or Location farther than
	 * HOLHeroAbility::BannerPlantRange from him.
	 */
	bool PlantBanner(int32 Slot, const FVector& Location);

	/** J8c.2: where the slot's banner stands; false when it has none standing. */
	bool GetBanner(int32 Slot, FVector& OutLocation) const;

	/** J8c.2: milliseconds before the slot's hero can cast Ability again (0: ready; the aura: 0). */
	int32 GetCooldownMs(int32 Slot, EHOLHeroAbility Ability) const;

	/** J8c.2 (hol.Hero.ResetAbilities, tests): the slot's cool-downs over, its banner and a pending cry gone. False out of range or on a client. */
	bool ResetAbilities(int32 Slot);

	/** The slot's AHOLPlayerState in the game state's PlayerArray (as HOLAISummary::ReadStock); nullptr for an AI slot. */
	static AHOLPlayerState* FindPlayerState(const UWorld& World, int32 Slot);

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	void OnStepEnded(int64 StepIndex);
	/** The slot's copies (game state, player state), each written only when it changed. */
	void WriteCopies(int32 Slot) const;
	/** J8b.2: his body brought to the level of his XP (attack, max health; the HP gained added), alive only. */
	void ApplyLevel(int32 Slot, FHOLUnitStateFragment* State, FHOLCombatFragment* Combat, const FHOLUnitTypeConstants* Constants) const;
	void UnbindSteps();

	/** One per slot (FHOLMatchConfig::MaxSlots). */
	TArray<FHOLHeroSheets> Sheets;
	FDelegateHandle StepEndedHandle;
	/** J8a.4: one bit per slot whose return was asked, until his new body is seen (UpdateSheets). */
	uint8 PendingReturns = 0;

	/** J8c.2: a slot's abilities where the simulation runs (cool-downs, banner), counted down at each step's end. */
	struct FAbilities
	{
		int32 CloseRanksCooldownMs = 0;
		int32 BannerCooldownMs = 0;
		/** Milliseconds its banner still stands at BannerLocation; 0: none. */
		int32 BannerMs = 0;
		FVector BannerLocation = FVector::ZeroVector;
		/** Cried since the combat processor's last step (ConsumeCloseRanks). */
		bool bCloseRanksPending = false;
	};
	/** One per slot, as Sheets. */
	TArray<FAbilities> Abilities;
	/** J8c.2: the cool-downs and the banners counted down by a step (OnStepEnded). */
	void TickAbilities();
};
