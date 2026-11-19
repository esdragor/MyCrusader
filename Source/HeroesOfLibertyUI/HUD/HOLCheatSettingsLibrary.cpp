// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLCheatSettingsLibrary.h"

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/ConfigCacheIni.h"

namespace HOLCheatSettingsPrivate
{
	const TCHAR* const ConfigSection = TEXT("HOL.CheatPanel");
	const TCHAR* const ConfigKey = TEXT("bOpen");

	float ReadVariable(const TCHAR* Name, const float Absent)
	{
		const IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name);
		return Variable ? Variable->GetFloat() : Absent;
	}
}

bool UHOLCheatSettingsLibrary::GetSavedOpen()
{
	bool bOpen = false;
	if (GConfig)
	{
		GConfig->GetBool(HOLCheatSettingsPrivate::ConfigSection, HOLCheatSettingsPrivate::ConfigKey, bOpen, GGameUserSettingsIni);
	}
	return bOpen;
}

void UHOLCheatSettingsLibrary::SetSavedOpen(const bool bOpen)
{
	if (GConfig)
	{
		GConfig->SetBool(HOLCheatSettingsPrivate::ConfigSection, HOLCheatSettingsPrivate::ConfigKey, bOpen, GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}
}

float UHOLCheatSettingsLibrary::GetWindowHeight()
{
	FVector2D Size = FVector2D::ZeroVector;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(Size);
	}
	return static_cast<float>(Size.Y);
}

bool UHOLCheatSettingsLibrary::IsServerLocal(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World && World->GetNetMode() != NM_Client;
}

float UHOLCheatSettingsLibrary::GetServerBuildTime()
{
	return HOLCheatSettingsPrivate::ReadVariable(TEXT("hol.Prod.BuildTime"), 0.f);
}

float UHOLCheatSettingsLibrary::GetServerSpeed(const int32 Index)
{
	switch (Index)
	{
	case 0: return HOLCheatSettingsPrivate::ReadVariable(TEXT("hol.Units.SpeedScale"), 1.f);
	case 1: return HOLCheatSettingsPrivate::ReadVariable(TEXT("hol.Gather.SpeedScale"), 1.f);
	default: return 1.f;
	}
}
