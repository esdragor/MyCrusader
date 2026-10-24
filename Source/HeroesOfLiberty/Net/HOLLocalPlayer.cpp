// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLLocalPlayer.h"

#include "GenericPlatform/GenericPlatformHttp.h"
#include "HOLVersion.h"
#include "Misc/CommandLine.h"

FString UHOLLocalPlayer::GetGameLoginOptions() const
{
	return FString::Printf(TEXT("%s=%s"), HOLVersion::UrlOption, *HOLVersion::Get());
}

FString UHOLLocalPlayer::GetNickname() const
{
	FString Name;
	return FParse::Value(FCommandLine::Get(), TEXT("PlayerName="), Name) ? FGenericPlatformHttp::UrlEncode(Name.TrimStartAndEnd()) : FString();
}
