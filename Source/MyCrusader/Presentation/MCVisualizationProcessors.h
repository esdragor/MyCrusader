// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassRepresentationProcessor.h"
#include "MassVisualizationLODProcessor.h"
#include "MCVisualizationProcessors.generated.h"

/** Per-frame representation LOD of light units (Mass' base processor is meant to be subclassed per project). */
UCLASS()
class MYCRUSADER_API UMCVisualizationLODProcessor : public UMassVisualizationLODProcessor
{
	GENERATED_BODY()

public:
	UMCVisualizationLODProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
};

/** Per-frame choice of the light units' representation (instances), from their LOD. */
UCLASS()
class MYCRUSADER_API UMCVisualizationProcessor : public UMassVisualizationProcessor
{
	GENERATED_BODY()

public:
	UMCVisualizationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
};
