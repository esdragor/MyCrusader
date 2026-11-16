// Copyright Epic Games, Inc. All Rights Reserved.

#include "HOLVersionReadLibrary.h"

#include "HOLVersion.h"

FString UHOLVersionReadLibrary::GetDev() { return FString(HOLVersion::Dev); }
FString UHOLVersionReadLibrary::GetUnknown() { return FString(HOLVersion::Unknown); }
FString UHOLVersionReadLibrary::Get() { return HOLVersion::Get(); }
FString UHOLVersionReadLibrary::Parse(const FString& FileText) { return HOLVersion::Parse(FileText); }
FString UHOLVersionReadLibrary::Display(const FString& Version) { return HOLVersion::Display(Version); }

bool UHOLVersionReadLibrary::IsAccepted(const FString& Server, const FString& ClientOption, FString& OutError)
{
	return HOLVersion::IsAccepted(Server, ClientOption, &OutError);
}
