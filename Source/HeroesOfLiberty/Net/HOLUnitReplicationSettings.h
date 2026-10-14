// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Net/HOLUnitReplicationRules.h"
#include "HOLUnitReplicationSettings.generated.h"

/** Unit replication settings (D269, D270), tuned in S5. */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "HeroesOfLiberty Unit Replication"))
class HEROESOFLIBERTY_API UHOLUnitReplicationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Prediction error (cm) past which a unit is sent again (D269: ~25 cm). Console override: hol.Net.ErrorThreshold (server). */
	UPROPERTY(config, EditAnywhere, Category = "Send rule", meta = (ClampMin = "1", ForceUnits = "cm"))
	float ErrorThreshold = 25.f;

	/** Cycle of the slow refresh, in simulation steps (D269: ~2 s, 40 steps at 20 Hz). */
	UPROPERTY(config, EditAnywhere, Category = "Send rule", meta = (ClampMin = "4"))
	int32 RefreshSteps = 40;

	/** Network updates per second of the shared actor (D217: 10 or 20 Hz, decided by S5). */
	UPROPERTY(config, EditAnywhere, Category = "Transport", meta = (ClampMin = "1", ClampMax = "60"))
	float NetUpdateFrequency = 20.f;

	FHOLUnitSendRules GetSendRules() const
	{
		FHOLUnitSendRules Rules;
		Rules.ErrorThreshold = ErrorThreshold;
		Rules.RefreshSteps = RefreshSteps;
		return Rules;
	}
};
