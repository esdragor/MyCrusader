// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLAccessKeyReadLibrary.h"

#include "Net/HOLAccessKey.h"

FString UHOLAccessKeyReadLibrary::ParseServerKey(const FString& CommandLine) { return HOLAccessKey::ParseServerKey(*CommandLine); }

bool UHOLAccessKeyReadLibrary::IsAccepted(const FString& Expected, const FString& ClientOption, FString& OutError)
{
	return HOLAccessKey::IsAccepted(Expected, ClientOption, &OutError);
}

FString UHOLAccessKeyReadLibrary::MakeLoginOption(const FString& Key) { return HOLAccessKey::MakeLoginOption(Key); }
