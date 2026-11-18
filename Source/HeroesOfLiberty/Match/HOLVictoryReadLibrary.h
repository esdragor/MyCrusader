// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLVictoryReadLibrary.generated.h"

/**
 * HOLVictoryRules (Match/HOLVictoryRules.h, C++, same module as this library) for Angelscript (namespace HOLVictoryRead there): D836 keeps the
 * rule in C++ and the script reads it here, so that the "Rules" tests of HOLVictoryTests.cpp (OneVsOne, Teams, FreeForAll, SingleCamp,
 * AllDefeated) can be ported to script (D830, D839). Wrappers only: no rule here, and no new struct. The slots are given as two arrays, the
 * way UHOLMatchConfigReadLibrary::DrawRandomColors does: States (0 Closed, 1 Human, 2 AI, EHOLSlotState's numbers) and Teams, the slot Index
 * of each; both are completed to the eight slots (closed, team 0), as the C++ test's MakeConfig does. Masks have the bit Slot set for that slot.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLVictoryReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * HOLVictoryRules::Evaluate(Config, DefeatedMask) on the configuration made of States and Teams: the return is FHOLVictoryResult::bOver,
	 * OutWinnerMask its WinnerMask, OutWinners its GetWinners().
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Victory")
	static bool Evaluate(const TArray<int32>& States, const TArray<int32>& Teams, int32 DefeatedMask, int32& OutWinnerMask, TArray<int32>& OutWinners);

	/** HOLVictoryRules::GetCamp on the configuration made of States and Teams (INDEX_NONE = -1 for a closed slot). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Victory")
	static int32 GetCamp(const TArray<int32>& States, const TArray<int32>& Teams, int32 Slot);

	/** HOLVictoryRules::AreHostile on the configuration made of States and Teams. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Victory")
	static bool AreHostile(const TArray<int32>& States, const TArray<int32>& Teams, int32 SlotA, int32 SlotB);
};
