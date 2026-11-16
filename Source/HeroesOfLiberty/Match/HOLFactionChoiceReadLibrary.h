// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLMatchConfig.h"
#include "HOLFactionChoiceReadLibrary.generated.h"

/** The state of the title's triple click (HOLFactionChoice::FClickCounter, a plain C++ struct there): the clicks counted and the time of the last. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLClickCounter
{
	GENERATED_BODY()

	/** Clicks counted in the current series. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|FactionChoice")
	int32 Count = 0;

	/** Time (seconds) of the last click. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|FactionChoice")
	double Last = 0.0;
};

/**
 * What the faction list of "Nouvelle partie" offers (HOLFactionChoice, Match/HOLFactionChoice.h, C++, same module as this library) for
 * Angelscript (namespace HOLFactionChoiceRead there): D836 keeps the rule in C++ and the script reads it here, so that the test of the
 * rule (HOLFactionChoiceTests.cpp) can be ported to script (socle step 1, D830, D839). Wrappers only: no rule here.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFactionChoiceReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLFactionChoice::IsChoosable: the list offers the faction, and the player can take it. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionChoice")
	static bool IsChoosable(EHOLFaction Faction, bool bCheat);

	/** HOLFactionChoice::IsListed: the list shows the faction (greyed or not). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionChoice")
	static bool IsListed(EHOLFaction Faction, bool bCheat);

	/** HOLFactionChoice::GetListed: the listed factions, in the order of the enum. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionChoice")
	static TArray<EHOLFaction> GetListed(bool bCheat);

	/** HOLFactionChoice::Sanitize: what a launch plays (the Templars without the cheat fall back to the Dawn). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionChoice")
	static EHOLFaction Sanitize(EHOLFaction Faction, bool bCheat);

	/** HOLFactionChoice::GetName: the enum's name as the manager writes it. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionChoice")
	static FString GetName(EHOLFaction Faction);

	/** HOLFactionChoice::TryFindByName: false for an unknown name (OutFaction is then left as the rule leaves it). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionChoice")
	static bool TryFindByName(const FString& Name, EHOLFaction& OutFaction);

	/** HOLFactionChoice::IsCheatOn: the session's cheat flag (process lifetime only). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionChoice")
	static bool IsCheatOn();

	/** HOLFactionChoice::SetCheatOn. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|FactionChoice")
	static void SetCheatOn(bool bOn);

	/** HOLFactionChoice::FClickCounter::Window: the seconds within which the clicks of a triple must follow each other. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionChoice")
	static double GetClickWindow();

	/** HOLFactionChoice::FClickCounter::Click: a click at Now (seconds); true on the third of a series. The counter is updated. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|FactionChoice")
	static bool Click(UPARAM(ref) FHOLClickCounter& Counter, double Now);
};
