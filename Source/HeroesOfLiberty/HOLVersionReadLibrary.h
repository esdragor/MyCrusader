// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLVersionReadLibrary.generated.h"

/**
 * The version of the game and its check at login (HOLVersion, HOLVersion.h, C++, applied by the server) for Angelscript (namespace
 * HOLVersionRead there): D836 keeps the rule in C++ and the script reads it here, so that the test of the rule (HOLVersionTests.cpp)
 * can be ported to script (socle step 1, D830, D839). Wrappers only: no rule here. The "inline const TCHAR*" constants are
 * republished as functions.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLVersionReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLVersion::Dev: the version outside a package. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Version")
	static FString GetDev();

	/** HOLVersion::Unknown: a package without its Version.txt. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Version")
	static FString GetUnknown();

	/** HOLVersion::Get: the version of this game, read once ("dev" in the editor). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Version")
	static FString Get();

	/** HOLVersion::Parse: the version in the text of a Version.txt (first line, trimmed; Unknown if empty). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Version")
	static FString Parse(const FString& FileText);

	/** HOLVersion::Display: how a version is shown ("V 0.Y.X" for a numbered one, as it is for any other). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Version")
	static FString Display(const FString& Version);

	/** HOLVersion::IsAccepted: whether a server of version Server lets in a client that sent ClientOption; OutError says why not. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Version")
	static bool IsAccepted(const FString& Server, const FString& ClientOption, FString& OutError);
};
