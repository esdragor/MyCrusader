// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Player/HOLSignalInput.h"
#include "HOLSignalInputReadLibrary.generated.h"

/**
 * The SG3 Alt + click's reading and the received signals' life (HOLSignalInput:: in Player/HOLSignalInput.h, the C++ rule) read by
 * Angelscript (namespace HOLSignalInputRead there), so that its test (HOLSignalInputTests.cpp) can be ported to script (E3-11, D830,
 * D839). Wrappers only: no rule here, nothing moved out of HOLSignalInput.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLSignalInputReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLSignalInput::KindOfAltClick: the kind (0 neutral, 1 attack, 2 defend, 3 need help, 4 rally, 5 scout) of an Alt + click pressed at Press and released at Release (screen pixels, y downwards). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Signals")
	static int32 KindOfAltClick(const FVector2D& Press, const FVector2D& Release);

	/** HOLSignalInput::Expire: drops from Signals those older than hol.Signal.IconSeconds at Now (game clock, seconds). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Signals")
	static void Expire(UPARAM(ref) TArray<FHOLReceivedSignal>& Signals, double Now);

	/** HOLSignalInput::ColorOf: the icon's colour of a kind (hol.Signal.IconStyle). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Signals")
	static FLinearColor ColorOf(int32 Kind);

	/** HOLSignalInput::PulseScale: the factor a mark is drawn at AgeSeconds after its signal arrived (D860 H2; 1 when fixed, born, expired). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Signals")
	static double PulseScale(double AgeSeconds);

	/** HOLSignalInput::MarkLifetime: how long a mark lives, seconds. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Signals")
	static double MarkLifetime();

	/** HOLSignalInput::PulseMin / PulseMax: the bounds PulseScale stays within. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Signals")
	static double PulseMin();

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Signals")
	static double PulseMax();

	/** A received signal of this Kind that arrived at ReceivedAt (the other fields at their defaults), so that the script can fill the list Expire works on (the struct's fields are read only). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Signals")
	static FHOLReceivedSignal MakeSignal(int32 Kind, double ReceivedAt);
};
