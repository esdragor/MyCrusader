// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Match/HOLResources.h"
#include "HOLMatchSettings.generated.h"

/**
 * Rules of every match that the match configuration does not carry (D244: no lobby setting without its decision):
 * provisional values, balanced here (DefaultGame.ini).
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "HeroesOfLiberty Match"))
class HEROESOFLIBERTY_API UHOLMatchSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Stock of each player at the start of a match (D283: 200 of each resource, provisional). */
	UPROPERTY(config, EditAnywhere, Category = "Economy")
	FHOLResources StartingResources = FHOLResources(200, 200, 200, 200);

	/**
	 * IA1.1: Peasants each occupied slot (human or AI) gets in front of its main centre at the start of a real match
	 * (menu, lobby). A match configuration can ask for another number (FHOLMatchConfig::StartingPeasants: the test
	 * scenarios, which make their own Peasants, ask for 0 unless their .json says otherwise).
	 */
	UPROPERTY(config, EditAnywhere, Category = "Economy", meta = (ClampMin = "0"))
	int32 StartingPeasants = 5;
};
