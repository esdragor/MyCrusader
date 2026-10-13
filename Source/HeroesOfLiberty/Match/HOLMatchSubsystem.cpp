// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchSubsystem.h"

#include "HeroesOfLibertyLog.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"

void UHOLMatchSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FString Path;
	if (FParse::Value(FCommandLine::Get(), TEXT("MatchConfig="), Path))
	{
		LoadFromFile(Path);
	}
}

void UHOLMatchSubsystem::SetConfig(const FHOLMatchConfig& InConfig)
{
	Config = InConfig;
	Config.Normalize();
	bHasConfig = true;
}

void UHOLMatchSubsystem::ClearConfig()
{
	Config = FHOLMatchConfig();
	bHasConfig = false;
}

FHOLMatchConfig UHOLMatchSubsystem::GetConfigOrDefault() const
{
	return bHasConfig ? Config : FHOLMatchConfig::MakeDefault();
}

bool UHOLMatchSubsystem::LoadFromFile(const FString& Path)
{
	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *Path))
	{
		UE_LOG(LogHOLMatch, Error, TEXT("Match config: cannot read %s"), *Path);
		return false;
	}

	FHOLMatchConfig Loaded;
	FString Error;
	if (!FHOLMatchConfig::FromJson(Json, Loaded, &Error))
	{
		UE_LOG(LogHOLMatch, Error, TEXT("Match config: %s is invalid (%s)"), *Path, *Error);
		return false;
	}

	SetConfig(Loaded);
	UE_LOG(LogHOLMatch, Log, TEXT("Match config read from %s: %s, %d occupied slots"), *Path,
		*Config.Map.ToString(), Config.GetNumOccupiedSlots());
	return true;
}
