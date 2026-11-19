// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLScriptLogLibrary.h"

#include "HeroesOfLibertyLog.h"

namespace
{
	/** The category object of a name of HeroesOfLibertyLog.h, or null. */
	const FLogCategoryBase* FindCategory(const FName Name)
	{
		if (Name == FName(TEXT("LogHOLSimulation"))) { return &LogHOLSimulation; }
		if (Name == FName(TEXT("LogHOLPresentation"))) { return &LogHOLPresentation; }
		if (Name == FName(TEXT("LogHOLCommands"))) { return &LogHOLCommands; }
		if (Name == FName(TEXT("LogHOLStressTest"))) { return &LogHOLStressTest; }
		if (Name == FName(TEXT("LogHOLMatch"))) { return &LogHOLMatch; }
		if (Name == FName(TEXT("LogHOLTest"))) { return &LogHOLTest; }
		if (Name == FName(TEXT("LogHOLNet"))) { return &LogHOLNet; }
		return nullptr;
	}
}

bool UHOLScriptLogLibrary::IsKnownCategory(const FName Category)
{
	return FindCategory(Category) != nullptr;
}

bool UHOLScriptLogLibrary::Emit(const FName Category, const ELogVerbosity::Type Verbosity, const FString& Line)
{
	const FLogCategoryBase* Found = FindCategory(Category);
	if (!Found)
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("ScriptLog: unknown log category %s (line dropped: %s)"), *Category.ToString(), *Line);
		return false;
	}
#if !NO_LOGGING
	if (!Found->IsSuppressed(Verbosity))
	{
		FMsg::Logf(__FILE__, __LINE__, Found->GetCategoryName(), Verbosity, TEXT("%s"), *Line);
	}
#endif
	return true;
}

bool UHOLScriptLogLibrary::LogDisplay(const FName Category, const FString& Line)
{
	return Emit(Category, ELogVerbosity::Display, Line);
}

bool UHOLScriptLogLibrary::LogWarning(const FName Category, const FString& Line)
{
	return Emit(Category, ELogVerbosity::Warning, Line);
}

bool UHOLScriptLogLibrary::LogError(const FName Category, const FString& Line)
{
	return Emit(Category, ELogVerbosity::Error, Line);
}
