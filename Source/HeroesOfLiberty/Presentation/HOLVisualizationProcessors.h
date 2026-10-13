// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassRepresentationProcessor.h"
#include "MassVisualizationLODProcessor.h"
#include "HOLVisualizationProcessors.generated.h"

/** Per-frame representation LOD of light units (Mass' base processor is meant to be subclassed per project). */
UCLASS()
class HEROESOFLIBERTY_API UHOLVisualizationLODProcessor : public UMassVisualizationLODProcessor
{
	GENERATED_BODY()

public:
	UHOLVisualizationLODProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
};

/** Per-frame choice of the light units' representation (instances), from their LOD. */
UCLASS()
class HEROESOFLIBERTY_API UHOLVisualizationProcessor : public UMassVisualizationProcessor
{
	GENERATED_BODY()

public:
	UHOLVisualizationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
};
