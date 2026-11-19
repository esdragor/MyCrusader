// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLFactionResourceLibrary.generated.h"

/**
 * The replicated state of the faction resource gauge (AHOLGameState::FactionResource, one integer per player slot, the same value on every
 * client) for Angelscript (namespace HOLFactionResource:: there, shared with the rule's script namespace of Script/Factions/HOLFactionResourceRules.as).
 * Wrappers only: no rule here. The Floor and Cap bounds stay script parameters of that rule; this library stores what the rule computed.
 * Reading works everywhere (a client reads its copy); Set is for the authority (server, solo) and does nothing on a client.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFactionResourceLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The slot's gauge value as this machine has it; 0 for a slot out of the table (or without a game state). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Faction", meta = (WorldContext = "WorldContextObject"))
	static int32 Get(const UObject* WorldContextObject, int32 Slot);

	/** Authority only: stores the slot's gauge value, replicated to every client. No effect on a client or for a slot out of the table. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Faction", meta = (WorldContext = "WorldContextObject"))
	static void Set(const UObject* WorldContextObject, int32 Slot, int32 Value);
};
