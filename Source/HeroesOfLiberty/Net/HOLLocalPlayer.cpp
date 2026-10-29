// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLLocalPlayer.h"

#include "GenericPlatform/GenericPlatformHttp.h"
#include "HOLVersion.h"
#include "Net/HOLAccessKey.h"
#include "Misc/CommandLine.h"

FString UHOLLocalPlayer::GetGameLoginOptions() const
{
	// One option string for the engine (FURL::AddOption), parsed back as ?Version=...?HOLKey=... by the server.
	const FString Options = FString::Printf(TEXT("%s=%s"), HOLVersion::UrlOption, *HOLVersion::Get());
	const FString KeyOption = HOLAccessKey::MakeLoginOption(HOLAccessKey::GetClientKey());
	return KeyOption.IsEmpty() ? Options : Options + TEXT("?") + KeyOption;
}

FString UHOLLocalPlayer::GetNickname() const
{
	if (!ChosenName.IsEmpty())
	{
		return FGenericPlatformHttp::UrlEncode(ChosenName);
	}
	FString Name;
	return FParse::Value(FCommandLine::Get(), TEXT("PlayerName="), Name) ? FGenericPlatformHttp::UrlEncode(Name.TrimStartAndEnd()) : FString();
}
