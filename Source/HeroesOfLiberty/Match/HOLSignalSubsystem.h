// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLSignalSubsystem.generated.h"

/**
 * The stances the signal rules read (HOLSignal::Post and Sees take a FHOLDiplomacyBook, Script/Diplomacy/HOLDiplomacyRules.as): the
 * number of slots, the locked flag (teams fixed outside the Diplomacy mode), the flat stance array (From * Slots + To) and the AIs.
 */
USTRUCT(BlueprintType)
struct FHOLSignalBookInput
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Signals")
	int32 Slots = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Signals")
	bool bLocked = true;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Signals")
	TArray<int32> Stances;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Signals")
	TArray<bool> IsAI;
};

/**
 * The map signals' board where the match runs (SG2, SignalsInProgress.md; GDD 15, D386). The thinnest bridge: the server holds the
 * board (its script subclass, Script/Signals/HOLSignalBoardScript.as, applies HOLSignal::Post: kind, rate, a defeated player silenced,
 * D784) and tells which seats see a signal (HOLSignal::Sees, spectators included: answer A of Q-signals-spectators). The player
 * controller's ServerPostSignal sends the signal to those seats only. Abstract: only a script subclass is born. A client never decides.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLSignalSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The answer when no script board exists (HOLSignal answers are 0 Ok to 4 TooMany). */
	static constexpr int32 NoScript = -1;

	/** The world's signal board (its script subclass), or null (no script compiled, or no world). */
	static UHOLSignalSubsystem* Find(const UWorld* World);

	/** The stances as this machine has them (the replicated state of AHOLGameState, through UHOLDiplomacyReadLibrary). */
	static FHOLSignalBookInput ReadBook(const UWorld* World);

	/** Viewer watches as a spectator for the signals: no seat, or defeated without a teammate (free-for-all, Diplomacy; D376, D784). */
	static bool IsSignalSpectator(const UWorld* World, int32 Viewer, bool bNoSeat);

	/** Sender places a signal of Kind at Location at NowMs (server): the HOLSignal answer, NoScript without the script board. */
	int32 PostNow(const FHOLSignalBookInput& Book, int32 Sender, bool bDefeated, int32 Kind, const FVector& Location, int32 NowMs);

	/** Viewer (a spectator with bViewerSpectator) sees the signals of Sender (HOLSignal::Sees); false without the script board. */
	bool SeesNow(const FHOLSignalBookInput& Book, int32 Sender, int32 Viewer, bool bViewerSpectator);

	/** The signals on the board (a test reads it). */
	int32 GetNumSignalsNow() { return NumSignals(); }

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Signals")
	int32 PostSignal(const FHOLSignalBookInput& Book, int32 Sender, bool bDefeated, int32 Kind, FVector Location, int32 NowMs);
	virtual int32 PostSignal_Implementation(const FHOLSignalBookInput& Book, int32 Sender, bool bDefeated, int32 Kind, FVector Location, int32 NowMs) { return NoScript; }

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Signals")
	bool SeesSignal(const FHOLSignalBookInput& Book, int32 Sender, int32 Viewer, bool bViewerSpectator);
	virtual bool SeesSignal_Implementation(const FHOLSignalBookInput& Book, int32 Sender, int32 Viewer, bool bViewerSpectator) { return false; }

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Signals")
	int32 NumSignals();
	virtual int32 NumSignals_Implementation() { return 0; }
};
