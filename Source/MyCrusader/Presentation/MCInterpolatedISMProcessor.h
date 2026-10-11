// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "MCInterpolatedISMProcessor.generated.h"

/**
 * Per-frame replacement of UMassUpdateISMProcessor (disabled in DefaultMass.ini): instances are drawn
 * between the two last fixed-step transforms, so 20 Hz movement looks smooth at any frame rate (D217).
 */
UCLASS()
class MYCRUSADER_API UMCInterpolatedISMProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMCInterpolatedISMProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
