// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLHeroSheet.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLHeroSubsystem.generated.h"

class AHOLPlayerState;

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
	void UnbindSteps();

	/** One per slot (FHOLMatchConfig::MaxSlots). */
	TArray<FHOLHeroSheets> Sheets;
	FDelegateHandle StepEndedHandle;
	/** J8a.4: one bit per slot whose return was asked, until his new body is seen (UpdateSheets). */
	uint8 PendingReturns = 0;
};
