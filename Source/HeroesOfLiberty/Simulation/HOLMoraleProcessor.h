// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "HOLMoraleProcessor.generated.h"

/**
 * The morale processor (PLAN-MORAL-1120 step 4, D866), on the server and in solo: each fixed step, every unit that carries a
 * FHOLMoraleFragment sees time pass on its morale effects (HOLMorale::Tick: temporary effects run down and end at 0, auras do not run down,
 * the immunity runs down). The auras are taken off each step and the Dawn aura (the first row of the table, SourceKind Hero) is laid again by every living hero on the units of his own camp within the row radius (D705: military and heroes, never a Peasant; once, never doubled); the other auras, the combat's read of the levers come in the next slices. Units are independent, so the outcome does not depend on the chunks. Runs before the combat of the
 * step so the combat reads the levers of the step that just began.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLMoraleProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLMoraleProcessor();

	/**
	 * PLAN-MORAL-1120 step 6 (S2-b): queues a morale order of the duel (called by UHOLMoraleLibrary::ApplyEffectAround). The next Execute of this
	 * world's processor lays the row EffectId (duration DurationMs, 0 = the row's) once, as a source of the camp OrderSlot at the point, on the units
	 * of that camp within RadiusCm (0 = the row's), through the same pass as the cry and the banner (D705: never a Peasant).
	 */
	static void QueueEffectAround(const UWorld* World, int32 EffectId, int32 CenterXCm, int32 CenterYCm, int32 OrderSlot, int32 DurationMs, int32 RadiusCm);

	/**
	 * PLAN-MORAL-1120 step 7 (S2-c, PROVISIONAL stand-in for the Luminous Monk's passive aura, which does not exist before A3; console command
	 * hol.Morale.Attenuate <slot> <percent>): the attenuation percent (D125, D706) the camp Slot gives to every malus it is about to take. The
	 * processor hands it to HOLMorale::Apply, which holds the whole rule (a temporary malus loses time, an aura malus loses intensity, a bonus
	 * is never touched, 75 % at most); 0 = none. The value is clamped to 0..100.
	 */
	static void SetSlotAttenuation(int32 Slot, int32 Percent);

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
