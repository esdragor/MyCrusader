// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLRelayReadLibrary.generated.h"

/**
 * What a relay's ring shows (FHOLRelayView, Match/HOLRelayView.h, not exposed to script itself): the same fields as plain values, so that the script can
 * read HOLRelay::MakeView (Angelscript: HOLRelayRead::MakeView). Description is FHOLRelayView::Describe. No rule here, a copy of what the rule returns.
 */
USTRUCT(BlueprintType)
struct FHOLRelayReadView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Relay")
	float X = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Relay")
	float Y = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Relay")
	float Radius = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Relay")
	int32 Owner = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Relay")
	int32 Taker = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Relay")
	int32 FillPercent = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Relay")
	FString Description;
};

/**
 * The relay rules that STAY in C++ (HOLRelay::FillPercent and MakeView, Simulation/HOLRelayRules.h: they fill the replicated FHOLRelayView) read by
 * Angelscript (namespace HOLRelayRead there): D836 keeps the rule in C++, so that its test (HOLRelayRingTests.cpp) can be ported to script
 * (socle step 1f, D830, D839). Wrappers only: no rule here. FHOLRelayState is passed as its three fields.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLRelayReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLRelay::FillPercent: the share of the capture held, 0 to 99; 0 with no hold steps. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static int32 FillPercent(int32 Owner, int32 Taker, int32 Held, int32 HoldSteps);

	/** HOLRelay::MakeView: what the server replicates of a relay at Location with a zone of Radius cm. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Relay")
	static FHOLRelayReadView MakeView(const FVector2D& Location, float Radius, int32 Owner, int32 Taker, int32 Held, int32 HoldSteps);
};