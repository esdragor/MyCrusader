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
	/** Prediction error (cm) past which a unit is sent again (D269, set to 35 cm by S5: D274). Console override: hol.Net.ErrorThreshold (server). */
	UPROPERTY(config, EditAnywhere, Category = "Send rule", meta = (ClampMin = "1", ForceUnits = "cm"))
	float ErrorThreshold = 35.f;

	/** Cycle of the slow refresh, in simulation steps (D269: ~2 s, 40 steps at 20 Hz). */
	UPROPERTY(config, EditAnywhere, Category = "Send rule", meta = (ClampMin = "4"))
	int32 RefreshSteps = 40;

	/** Network updates per second of the shared actor and of its chunk actors (D217: 10 or 20 Hz; S5 chose 10: D274). */
	UPROPERTY(config, EditAnywhere, Category = "Transport", meta = (ClampMin = "1", ClampMax = "60"))
	float NetUpdateFrequency = 10.f;

	FHOLUnitSendRules GetSendRules() const
	{
		FHOLUnitSendRules Rules;
		Rules.ErrorThreshold = ErrorThreshold;
		Rules.RefreshSteps = RefreshSteps;
		return Rules;
	}
};

/** Console knobs of S5 (D273), defined in HOLUnitReplicationSubsystem.cpp. */
namespace UE::HeroesOfLiberty::Net
{
	/** hol.Net.ErrorThreshold (server): overrides the settings' threshold when > 0. */
	extern float ErrorThresholdOverride;
	/** hol.Net.UpdateFrequency (server): overrides the settings' updates per second when > 0. */
	extern float UpdateFrequencyOverride;
	/** hol.Net.RefreshSteps (server): overrides the settings' refresh cycle when > 0. */
	extern int32 RefreshStepsOverride;
	/** hol.Net.ReportInterval (server and client): a report line every this many seconds, 0 = none. */
	extern float ReportInterval;
	/** hol.Net.Sample (server and client): logs the positions of the units whose identifier is a multiple of it, 0 = none. */
	extern int32 SampleEvery;
}
