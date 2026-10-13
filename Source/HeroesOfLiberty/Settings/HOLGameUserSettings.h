// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "HOLGameUserSettings.generated.h"

/** Volumes of the Sound tab (D245). */
UENUM()
enum class EHOLVolume : uint8
{
	Master,
	Music,
	Effects,
	Voice,
};

/**
 * Player settings (D245), saved in GameUserSettings.ini next to the engine's video settings, which it keeps: the four
 * volumes. The master volume acts on the audio device; music, effects and voice go through the sound classes and the
 * mix made by Tools/Editor/make_sound_classes.py (/Game/Audio), heard once sounds use those classes. The language is
 * the engine's own setting ([Internationalization] in the same file). Declared in DefaultEngine.ini
 * (GameUserSettingsClassName); in the main module because the engine creates it early, dedicated server included.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	/** The engine's settings object, or null if the class is not the one in DefaultEngine.ini. */
	static UHOLGameUserSettings* Get();

	/** 0 to 1. */
	float GetVolume(EHOLVolume Volume) const;
	void SetVolume(EHOLVolume Volume, float Value);

	virtual void SetToDefaults() override;
	virtual void ApplyNonResolutionSettings() override;

	/** Gives the volumes to the audio device (nothing without one: dedicated server, -nosound). */
	void ApplySound();

private:
	float& VolumeRef(EHOLVolume Volume);

	UPROPERTY(Config)
	float MasterVolume = 1.f;

	UPROPERTY(Config)
	float MusicVolume = 1.f;

	UPROPERTY(Config)
	float EffectsVolume = 1.f;

	UPROPERTY(Config)
	float VoiceVolume = 1.f;

	/** The mix is pushed once on the audio device, which outlives the maps. */
	bool bMixPushed = false;
};
