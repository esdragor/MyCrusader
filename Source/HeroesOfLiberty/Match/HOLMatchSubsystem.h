// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMatchConfig.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HOLMatchSubsystem.generated.h"

/**
 * Carries the match configuration across map changes (D243). One reader, the game mode; three sources: the menu
 * in single player, the lobby of M2.5 on the server, and -MatchConfig=<JSON file> on the command line for a
 * dedicated server started without a lobby.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLMatchSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void SetConfig(const FHOLMatchConfig& InConfig);
	void ClearConfig();
	bool HasConfig() const { return bHasConfig; }

	/** The configuration that was set, or the default one (one human in slot 0). */
	FHOLMatchConfig GetConfigOrDefault() const;

	/** Reads a JSON configuration file; logs and returns false on failure, keeping the current configuration. */
	bool LoadFromFile(const FString& Path);

private:
	FHOLMatchConfig Config;
	bool bHasConfig = false;
};
