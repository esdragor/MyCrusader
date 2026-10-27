// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLHeroSheet.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLHeroSubsystem.generated.h"

class AHOLPlayerState;

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
};
