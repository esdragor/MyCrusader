// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "HOLLocalPlayer.generated.h"

/**
 * Local player of the game: adds ?Version=<game version> to every login URL, so the server can check it (R3.3), and
 * ?HOLKey=<key> when the manager called has an access key (launcher L11, HOLAccessKey::GetClientKey); sends
 * the name given at the launch (-PlayerName=) instead of the system's nickname, which would replace the ?Name= of the URL
 * opened (FURL::AddOption): that name gives the slot (R3.2).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	virtual FString GetGameLoginOptions() const override;
	/**
	 * The name chosen on the Multiplayer screen (SetChosenName), else -PlayerName= of the command line, URL-encoded;
	 * else empty: the ?Name= of the URL opened is kept (none: a spectator).
	 */
	virtual FString GetNickname() const override;

	/**
	 * The name the player took on the Multiplayer screen (its lobby name): it goes over -PlayerName= at the login, or a
	 * player renamed there would join his match under the launch name, as a spectator (2026-10-29 trial).
	 */
	void SetChosenName(const FString& Name) { ChosenName = Name.TrimStartAndEnd(); }

private:
	FString ChosenName;
};
