// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "HOLLocalPlayer.generated.h"

/**
 * Local player of the game: adds ?Version=<game version> to every login URL, so the server can check it (R3.3), and sends
 * the name given at the launch (-PlayerName=) instead of the system's nickname, which would replace the ?Name= of the URL
 * opened (FURL::AddOption): that name gives the slot (R3.2).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	virtual FString GetGameLoginOptions() const override;
	/** -PlayerName= of the command line, URL-encoded; else empty: the ?Name= of the URL opened is kept (none: a spectator). */
	virtual FString GetNickname() const override;
};
