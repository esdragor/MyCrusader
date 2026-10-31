// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLRelayRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLRelaySubsystem.generated.h"

/**
 * The neutral relays of a match (R3, R4; GDD 9.4, D388): where the simulation runs (solo, server), each step it counts the living units of
 * each slot in each relay's zone, applies HOLRelay::Step and gives the owner's hero its territory XP by the minute (the Exploration source).
 * The relays are laid by the map (R2, not done yet: for now hol.Relay.Add, development only). Nothing is replicated yet (R5).
 * Not decided yet (RelaysInProgress.md): the rates and several adverse sides (provisional values in HOLRelayRules.h), whether the XP goes on
 * while the hero is dead (here it does), neutral units (they never take a relay), secondary centres (none in the game yet: 0 counted).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLRelaySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	struct FRelaySite
	{
		FVector2D Location = FVector2D::ZeroVector;
		/** The zone's radius, cm. */
		float Radius = 1200.f;
		FHOLRelayState State;
	};

	/** A relay at Location with a zone of Radius cm; its index. Server or solo only (INDEX_NONE on a client). */
	int32 AddRelay(const FVector2D& Location, float Radius);
	void ClearRelays();
	const TArray<FRelaySite>& GetRelays() const { return Relays; }
	/** Relays the slot owns. */
	int32 CountOwned(int32 Slot) const;
	/** The step the simulation is on is applied: counts, rule, XP. Public for the tests. */
	void OnStepEnded(int64 StepIndex);
	bool BindSteps();

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	void UnbindSteps();

	TArray<FRelaySite> Relays;
	/** Territory XP fractions carried per slot. */
	int32 Carries[8] = {};
	FDelegateHandle StepEndedHandle;
};
