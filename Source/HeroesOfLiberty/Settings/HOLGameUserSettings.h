// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "Player/HOLRTSCamera.h"
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
 * volumes, the camera speed and edge scrolling (D253). The master volume acts on the audio device; music, effects and voice go through the sound classes and the
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
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	static UHOLGameUserSettings* Get();

	/** 0 to 1. */
	float GetVolume(EHOLVolume Volume) const;
	void SetVolume(EHOLVolume Volume, float Value);

	virtual void SetToDefaults() override;
	virtual void ApplyNonResolutionSettings() override;

	/** Gives the volumes to the audio device (nothing without one: dedicated server, -nosound). */
	void ApplySound();

	EHOLCameraSpeed GetCameraSpeed() const { return static_cast<EHOLCameraSpeed>(FMath::Clamp<int32>(CameraSpeed, 0, 2)); }
	void SetCameraSpeed(EHOLCameraSpeed Speed) { CameraSpeed = static_cast<int32>(Speed); }
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	bool IsEdgeScrollEnabled() const { return bEdgeScroll; }
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	void SetEdgeScrollEnabled(bool bEnabled) { bEdgeScroll = bEnabled; }

	/** For the script (the settings screen): a volume by its index in EHOLVolume (0 Master, 1 Music, 2 Effects, 3 Voice), 0 to 1. */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	float GetVolumeAt(int32 Volume) const { return GetVolume(static_cast<EHOLVolume>(FMath::Clamp(Volume, 0, 3))); }
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	void SetVolumeAt(int32 Volume, float Value) { SetVolume(static_cast<EHOLVolume>(FMath::Clamp(Volume, 0, 3)), Value); }
	/** The camera speed by its index in EHOLCameraSpeed (0 Slow, 1 Normal, 2 Fast). */
	UFUNCTION(BlueprintPure, Category = "HOL|Settings")
	int32 GetCameraSpeedIndex() const { return static_cast<int32>(GetCameraSpeed()); }
	UFUNCTION(BlueprintCallable, Category = "HOL|Settings")
	void SetCameraSpeedIndex(int32 Index) { SetCameraSpeed(static_cast<EHOLCameraSpeed>(FMath::Clamp(Index, 0, 2))); }

	/** The name given to the lobby manager (Multiplayer screen, R4.2); empty until the player gives one. */
	const FString& GetPlayerName() const { return PlayerName; }
	void SetPlayerName(const FString& Name) { PlayerName = Name; }

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

	/** EHOLCameraSpeed, saved as a number. */
	UPROPERTY(Config)
	int32 CameraSpeed = static_cast<int32>(EHOLCameraSpeed::Normal);

	UPROPERTY(Config)
	bool bEdgeScroll = true;

	UPROPERTY(Config)
	FString PlayerName;

	/** The mix is pushed once on the audio device, which outlives the maps. */
	bool bMixPushed = false;
};
