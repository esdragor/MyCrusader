// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLVolcanoRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLVolcanoSubsystem.generated.h"

/**
 * The volcano sites of a match (GDD 12, V5): server side only (D288). Holds the sites (the map's recipe gives them, V4b; a test or a
 * scenario sets them) and, at the head of each simulation step, reads each site's state from the match's configuration and the clock
 * (HOLVolcano::SiteStateOfMatch) and destroys what its lava runs over while it advances: units (heroes too, but a hero in a duel),
 * buildings (a main forum falls into ruin as from any blow, D350). Deposits are spared (D379).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLVolcanoSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's volcano book, or null. */
	static UHOLVolcanoSubsystem* Find(const UWorld* World) { return World ? World->GetSubsystem<UHOLVolcanoSubsystem>() : nullptr; }

	void SetSites(const TArray<HOLVolcano::FSite>& InSites) { Sites = InSites; }
	const TArray<HOLVolcano::FSite>& GetSites() const { return Sites; }
	FHOLVolcanoSettings& GetMutableSettings() { return Settings; }
	const FHOLVolcanoSettings& GetSettings() const { return Settings; }

	/** Listens to the simulation's steps (done at the world's begin play; a test world calls it by hand). Server and solo only. */
	void BindSteps();

	/** The match clock (s) of a step: the step index over the step rate. */
	float GetClockOf(int64 StepIndex) const;

	/**
	 * Burns what Site's lava covers in State (nothing unless it is erupting); returns the units and buildings destroyed, added to the
	 * totals. The step calls it for every site; a test may call it with a state of its own.
	 */
	int32 Burn(const HOLVolcano::FSite& Site, const HOLVolcano::FSiteState& State);

	int32 GetNumUnitsBurnt() const { return NumUnitsBurnt; }
	int32 GetNumBuildingsBurnt() const { return NumBuildingsBurnt; }

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void OnStepStarted(int64 StepIndex);

	TArray<HOLVolcano::FSite> Sites;
	FHOLVolcanoSettings Settings;
	FDelegateHandle StepStartedHandle;
	int32 NumUnitsBurnt = 0;
	int32 NumBuildingsBurnt = 0;
};