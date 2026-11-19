// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLMatchFlowSubsystem.generated.h"

/**
 * The pause and diplomacy decisions of AHOLGameState where the match runs (MIG-CPP step 5, lot S1; D839: a binding to expose, the replicated fields
 * and their replication stay in AHOLGameState). The rule is the script's (Script/Match/HOLMatchFlowBoard.as, namespace HOLMatchFlow, tested by
 * Script/Match/HOLMatchFlow_Test.as); this world subsystem asks its script subclass (UHOLMatchFlowBoardScript). Abstract: only a script subclass is
 * born. Every Ask* answers a whole number, or -1 when the world has no script board (AHOLGameState then keeps the C++ rule: FHOLPauseRules and the
 * inline gates, still the twin of the script until the agreement test HOLMatchFlow_Test.as has run green and the twin is removed). The pause answers
 * are the order of FHOLPauseRules::EAnswer (EHOLPauseAnswer of the script), the stance gate 0 Open, 1 not the Diplomacy mode, 2 no such player.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLMatchFlowSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's match flow board (its script subclass), or null (no script compiled, or no world). */
	static UHOLMatchFlowSubsystem* Find(const UWorld* World);

	static int32 AskCanPause(const UWorld* World, bool bSolo, int32 PausedBySlot, int32 PausesLeft, bool bRequesterInMatch);
	static int32 AskCanResume(const UWorld* World, bool bSolo, int32 PausedBySlot, double PausedAtSeconds, int32 RequesterSlot, bool bRequesterInMatch, double Now);
	/** The pauses left once an allowed pause is taken, or -1 (no board). */
	static int32 AskPausesLeftAfterPause(const UWorld* World, bool bSolo, int32 PausesLeft);
	static int32 AskStanceGate(const UWorld* World, bool bAuthority, bool bDiplomacyMode, bool bFromOccupied, bool bToOccupied);
	/** 1 open, 0 shut, -1 no board. */
	static int32 AskVisionGate(const UWorld* World, bool bAuthority, bool bDiplomacyMode);
	/** 1 yes, 0 no, -1 no board. */
	static int32 AskEvaluateMatchEnd(const UWorld* World, bool bAnnounce, bool bFormed);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** The script subclass overrides these (HOLMatchFlow::...); the default is -1 (no rule here). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|MatchFlow")
	int32 PauseAnswer(bool bSolo, int32 PausedBySlot, int32 PausesLeft, bool bRequesterInMatch);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|MatchFlow")
	int32 ResumeAnswer(bool bSolo, int32 PausedBySlot, double PausedAtSeconds, int32 RequesterSlot, bool bRequesterInMatch, double Now);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|MatchFlow")
	int32 PausesAfterPause(bool bSolo, int32 PausesLeft);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|MatchFlow")
	int32 StanceGateAnswer(bool bAuthority, bool bDiplomacyMode, bool bFromOccupied, bool bToOccupied);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|MatchFlow")
	int32 VisionGateAnswer(bool bAuthority, bool bDiplomacyMode);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|MatchFlow")
	int32 EvaluateEndAnswer(bool bAnnounce, bool bFormed);
	virtual int32 PauseAnswer_Implementation(bool bSolo, int32 PausedBySlot, int32 PausesLeft, bool bRequesterInMatch);
	virtual int32 ResumeAnswer_Implementation(bool bSolo, int32 PausedBySlot, double PausedAtSeconds, int32 RequesterSlot, bool bRequesterInMatch, double Now);
	virtual int32 PausesAfterPause_Implementation(bool bSolo, int32 PausesLeft);
	virtual int32 StanceGateAnswer_Implementation(bool bAuthority, bool bDiplomacyMode, bool bFromOccupied, bool bToOccupied);
	virtual int32 VisionGateAnswer_Implementation(bool bAuthority, bool bDiplomacyMode);
	virtual int32 EvaluateEndAnswer_Implementation(bool bAnnounce, bool bFormed);
};
