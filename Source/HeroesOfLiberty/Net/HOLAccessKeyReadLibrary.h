// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLAccessKeyReadLibrary.generated.h"

/**
 * The shared access key of a match server (HOLAccessKey, Net/HOLAccessKey.h, C++, applied in PreLogin) for Angelscript (namespace
 * HOLAccessKeyRead there): D836 keeps the rule in C++ and the script reads it here, so that the test of the rule (HOLVersionTests.cpp,
 * AccessKey) can be ported to script (socle step 1, D830, D839). Wrappers only: no rule here. The key is never logged.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLAccessKeyReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLAccessKey::ParseServerKey: -HOLAccessKey= of a command line, trimmed; empty without one. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AccessKey")
	static FString ParseServerKey(const FString& CommandLine);

	/** HOLAccessKey::IsAccepted: whether a server expecting Expected lets in a client that sent ClientOption; OutError is the mismatch message when not. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AccessKey")
	static bool IsAccepted(const FString& Expected, const FString& ClientOption, FString& OutError);

	/** HOLAccessKey::MakeLoginOption: "HOLKey=<key URL-encoded>", empty without a key. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AccessKey")
	static FString MakeLoginOption(const FString& Key);
};
