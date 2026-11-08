// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLSiteView.generated.h"

/**
 * S5 (SacredSitesInProgress.md, D701): what one player knows of a sacred site: its number, where it is, its zone, its owner, who is filling
 * its count and how far. NOT public (unlike FHOLRelayView): the server writes to each player's state only the sites his camp may know (taken
 * or being taken by it, or explored by it), frozen as he last saw them while the fog hides them (UHOLSiteSubsystem), and replicates them to
 * that player only. The ring, the minimap and the messages read this and nothing else.
 */
USTRUCT()
struct FHOLSiteView
{
	GENERATED_BODY()

	/** The site's number in the match (its index in the subsystem), named in the messages: "site 2". */
	UPROPERTY()
	uint8 Index = 0;

	UPROPERTY()
	float X = 0.f;

	UPROPERTY()
	float Y = 0.f;

	/** The zone's radius, cm. */
	UPROPERTY()
	float Radius = 1200.f;

	/** The owner's slot, INDEX_NONE for nobody. */
	UPROPERTY()
	int8 Owner = INDEX_NONE;

	/** The slot filling the count, INDEX_NONE while nobody does. */
	UPROPERTY()
	int8 Taker = INDEX_NONE;

	/** Share of the capture held, 0 to 99. */
	UPROPERTY()
	uint8 FillPercent = 0;

	bool operator==(const FHOLSiteView& Other) const
	{
		return Index == Other.Index && X == Other.X && Y == Other.Y && Radius == Other.Radius && Owner == Other.Owner && Taker == Other.Taker && FillPercent == Other.FillPercent;
	}

	/** "#1 (1500, -2500) r1200 owner 2 taker 1 fill 50": the report's line. */
	FString Describe() const
	{
		return FString::Printf(TEXT("#%d (%.0f, %.0f) r%.0f owner %d taker %d fill %d"), static_cast<int32>(Index), X, Y, Radius, static_cast<int32>(Owner), static_cast<int32>(Taker), static_cast<int32>(FillPercent));
	}
};
