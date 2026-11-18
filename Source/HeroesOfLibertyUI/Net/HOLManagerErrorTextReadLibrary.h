// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLManagerErrorTextReadLibrary.generated.h"

/** The kind of a failed call to the lobby manager (FHOLManagerError::EKind, a plain C++ enum there). */
UENUM(BlueprintType)
enum class EHOLManagerErrorKind : uint8
{
	None,
	Unreachable,
	Unreadable,
	Protocol,
};

/** Why a call to the manager failed (FHOLManagerError, Net/HOLManagerClientRules.h, a plain C++ struct there): the same four fields. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTYUI_API FHOLManagerErrorValue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Manager")
	EHOLManagerErrorKind Kind = EHOLManagerErrorKind::None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Manager")
	int32 HttpCode = 0;

	/** The protocol's stable code (lobby_full, ...). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Manager")
	FString Code;

	/** English, for the log. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|Manager")
	FString Message;
};

/**
 * The text shown for a failed call to the lobby manager (Net/HOLManagerErrorText.h, free functions, C++, same module as this library) for
 * Angelscript (namespace HOLManagerErrorTextRead there): D836 keeps the rule in C++ and the script reads it here, so that the test of the
 * rule (HOLManagerErrorTextTests.cpp) can be ported to script (socle step 1, D830, D839). Wrappers only: no rule here; the error is
 * converted to and from FHOLManagerError field by field.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLManagerErrorTextReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** GetManagerErrorText: the text shown for the error, chosen from its kind and its protocol code. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Manager")
	static FText GetManagerErrorText(const FHOLManagerErrorValue& Error);

	/** MakeVersionMismatchError: the version_mismatch of a manager whose /status serves another version than ours. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Manager")
	static FHOLManagerErrorValue MakeVersionMismatchError(const FString& Served, const FString& Ours);

	/** MakeMatchLossError: why a match opened from a lobby was lost (from UEngine::OnNetworkFailure). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Manager")
	static FHOLManagerErrorValue MakeMatchLossError(TEnumAsByte<ENetworkFailure::Type> Failure, const FString& Message);

	/** MakeMatchTravelError: the same for UEngine::OnTravelFailure (match_failed). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Manager")
	static FHOLManagerErrorValue MakeMatchTravelError(const FString& Message);

	/** MakeReturnToLobbyError: a refused "Retour au salon" (lobby_in_game becomes lobby_relaunched). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Manager")
	static FHOLManagerErrorValue MakeReturnToLobbyError(const FHOLManagerErrorValue& JoinError);

	/** CanCreateLobbyAfter: whether "Créer un salon" stays enabled after the list's answer (a bad_key greys it). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Manager")
	static bool CanCreateLobbyAfter(const FHOLManagerErrorValue& ListError);
};
