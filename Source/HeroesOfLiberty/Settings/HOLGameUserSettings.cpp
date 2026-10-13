// Copyright Epic Games, Inc. All Rights Reserved.

#include "Settings/HOLGameUserSettings.h"

#include "AudioDevice.h"
#include "Engine/Engine.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

DEFINE_LOG_CATEGORY_STATIC(LogHOLSettings, Log, All);

namespace UE::HeroesOfLiberty::Settings
{
	// Made by Tools/Editor/make_sound_classes.py.
	const TCHAR* MixPath = TEXT("/Game/Audio/SCM_Volumes.SCM_Volumes");
	const TCHAR* MusicClassPath = TEXT("/Game/Audio/SC_Music.SC_Music");
	const TCHAR* EffectsClassPath = TEXT("/Game/Audio/SC_Effects.SC_Effects");
	const TCHAR* VoiceClassPath = TEXT("/Game/Audio/SC_Voice.SC_Voice");
}

UHOLGameUserSettings* UHOLGameUserSettings::Get()
{
	return GEngine ? Cast<UHOLGameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
}

float& UHOLGameUserSettings::VolumeRef(const EHOLVolume Volume)
{
	switch (Volume)
	{
	case EHOLVolume::Music:
		return MusicVolume;
	case EHOLVolume::Effects:
		return EffectsVolume;
	case EHOLVolume::Voice:
		return VoiceVolume;
	default:
		return MasterVolume;
	}
}

float UHOLGameUserSettings::GetVolume(const EHOLVolume Volume) const
{
	return const_cast<UHOLGameUserSettings*>(this)->VolumeRef(Volume);
}

void UHOLGameUserSettings::SetVolume(const EHOLVolume Volume, const float Value)
{
	VolumeRef(Volume) = FMath::Clamp(Value, 0.f, 1.f);
}

void UHOLGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();
	MasterVolume = MusicVolume = EffectsVolume = VoiceVolume = 1.f;
}

void UHOLGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();
	ApplySound();
}

void UHOLGameUserSettings::ApplySound()
{
	using namespace UE::HeroesOfLiberty::Settings;

	FAudioDeviceHandle Device = GEngine ? GEngine->GetMainAudioDevice() : FAudioDeviceHandle();
	if (!Device.IsValid())
	{
		return;
	}
	Device->SetTransientPrimaryVolume(MasterVolume);

	USoundMix* Mix = LoadObject<USoundMix>(nullptr, MixPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (!Mix)
	{
		UE_LOG(LogHOLSettings, Verbose, TEXT("No sound mix %s: music, effects and voice volumes not applied"), MixPath);
		return;
	}
	const TPair<const TCHAR*, float> Classes[] = { { MusicClassPath, MusicVolume }, { EffectsClassPath, EffectsVolume }, { VoiceClassPath, VoiceVolume } };
	for (const TPair<const TCHAR*, float>& Class : Classes)
	{
		if (USoundClass* SoundClass = LoadObject<USoundClass>(nullptr, Class.Key, nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			Device->SetSoundMixClassOverride(Mix, SoundClass, Class.Value, 1.f, 0.f, /*bApplyToChildren*/ true);
		}
	}
	if (!bMixPushed)
	{
		Device->PushSoundMixModifier(Mix);
		bMixPushed = true;
	}
	UE_LOG(LogHOLSettings, Log, TEXT("Volumes: master %.2f, music %.2f, effects %.2f, voice %.2f"), MasterVolume, MusicVolume,
		EffectsVolume, VoiceVolume);
}
