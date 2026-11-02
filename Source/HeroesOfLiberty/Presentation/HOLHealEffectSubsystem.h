// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLHealEffectSubsystem.generated.h"

/**
 * What the Monk's heal looks like (Q-chapelle-moine = A): for each heal of the last simulation step (UHOLHealProcessor::GetLinks) a green
 * line from the healer to the unit it heals and a green "+" over that unit, redrawn each frame while the heal lasts. Solo and host only (the
 * simulation runs there; a client sees the target's health rise, the heal is not replicated, no wire bit is free for it). Debug drawing:
 * a placeholder until the art of the effect exists.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLHealEffectSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Heals drawn since the world began (hol.Heal.Report). */
	int64 GetDrawnCount() const { return Drawn; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	/** A client's version: the heal is guessed from the health received (a unit whose health rose, a healer of its owner within reach). */
	void TickClient(UWorld& World);

	int64 Drawn = 0;
	/** A client's guess: each unit's health at the last frame, and the time its health last rose. */
	TMap<uint32, int32> LastHealth;
	TMap<uint32, double> LastRise;
};
