// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "HOLMoraleProcessor.generated.h"

/**
 * The morale processor (PLAN-MORAL-1120 step 4, D866), on the server and in solo: each fixed step, every unit that carries a
 * FHOLMoraleFragment sees time pass on its morale effects (HOLMorale::Tick: temporary effects run down and end at 0, auras do not run down,
 * the immunity runs down). This is the first rule only; the auras cleared and posed again each step, the recipients and the combat's read of
 * the levers come in the next slices. Units are independent, so the outcome does not depend on the chunks. Runs before the combat of the
 * step so the combat reads the levers of the step that just began.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLMoraleProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHOLMoraleProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
