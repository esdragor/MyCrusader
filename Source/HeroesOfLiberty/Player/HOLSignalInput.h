// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLSignalInput.generated.h"

/** SG3 (GDD 15, D386): a signal this player received (ClientReceiveSignal), shown on the minimap while it lives. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLReceivedSignal
{
	GENERATED_BODY()

	/** The seat that placed it. */
	UPROPERTY(BlueprintReadOnly, Category = "Signals")
	int32 Sender = INDEX_NONE;

	/** Its kind, as Script/Signals/HOLSignalRules.as: 0 neutral, 1 attack, 2 defend, 3 need help, 4 rally, 5 scout. */
	UPROPERTY(BlueprintReadOnly, Category = "Signals")
	int32 Kind = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Signals")
	FVector Location = FVector::ZeroVector;

	/** The game's clock when it arrived, seconds. */
	UPROPERTY(BlueprintReadOnly, Category = "Signals")
	double ReceivedAt = 0.0;

	/** Its icon's colour (hol.Signal.IconStyle, Retours/Q-signals-ui.md Q2). */
	UPROPERTY(BlueprintReadOnly, Category = "Signals")
	FLinearColor Color = FLinearColor::White;
};

/** SG3: the Alt + click's reading and the received signals' life (pure, tested by HeroesOfLiberty.Signals.AltClick). */
namespace HOLSignalInput
{
	/** The kind of an Alt + click pressed at Press and released at Release (screen pixels, y downwards). A click that moved less than
	 * hol.Signal.WheelPixels is the neutral signal; a drag picks one of the wheel's five kinds by its direction, clockwise from the
	 * screen's top: attack, defend, need help, rally, scout (Retours/Q-signals-ui.md Q1, default A). */
	HEROESOFLIBERTY_API int32 KindOfAltClick(const FVector2D& Press, const FVector2D& Release);

	/** Drops the signals older than hol.Signal.IconSeconds (5 s, D784) at Now (game clock, seconds). */
	HEROESOFLIBERTY_API void Expire(TArray<FHOLReceivedSignal>& Signals, double Now);

	/** The icon's colour of a kind (hol.Signal.IconStyle 0: one colour per kind; 1: all white). */
	HEROESOFLIBERTY_API FLinearColor ColorOf(int32 Kind);
}
