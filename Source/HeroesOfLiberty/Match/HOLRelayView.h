// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLRelayView.generated.h"

/**
 * R5: what every machine knows of a neutral relay (GDD 9.4, D388): where it is, its zone, its owner, who is filling its count and how far.
 * Public (D203/D353 style: the relays are on the map for everyone), replicated by the game state; the server writes it each step from the
 * relay's real state (UHOLRelaySubsystem), the markers draw the ring from it and nothing else.
 */
USTRUCT()
struct FHOLRelayView
{
	GENERATED_BODY()

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

	bool operator==(const FHOLRelayView& Other) const
	{
		return X == Other.X && Y == Other.Y && Radius == Other.Radius && Owner == Other.Owner && Taker == Other.Taker && FillPercent == Other.FillPercent;
	}

	/** "(1500, -2500) r1200 owner 2 taker 1 fill 50": the report's line. */
	FString Describe() const
	{
		return FString::Printf(TEXT("(%.0f, %.0f) r%.0f owner %d taker %d fill %d"), X, Y, Radius, static_cast<int32>(Owner), static_cast<int32>(Taker), static_cast<int32>(FillPercent));
	}
};
