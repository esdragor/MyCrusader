// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLMatchFlowSubsystem.generated.h"

/**
 * The pause and diplomacy decisions of AHOLGameState where the match runs (MIG-CPP step 5, lot S1; D839: a binding to expose, the replicated fields
 * and their replication stay in AHOLGameState). The rule is the script's (Script/Match/HOLMatchFlowBoard.as, namespace HOLMatchFlow, tested by
 * Script/Match/HOLMatchFlow_Test.as); this world subsystem asks its script subclass (UHOLMatchFlowBoardScript). Abstract: only a script subclass is
 * born. Every Ask* answers a whole number, or -1 when the world has no script board (AHOLGameState then keeps the inline gates of the stances; for the
 * pause there is no C++ rule left: no board = the request is refused with an error line). The pause answers
 * are the order of EHOLPauseAnswer of the script (HOLMatchFlow_Test.as: Test_MatchFlowAnswerOrder), the stance gate 0 Open, 1 not the Diplomacy mode, 2 no such player.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLMatchFlowSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's match flow board (its script subclass), or null (no script compiled, or no world). */
	static UHOLMatchFlowSubsystem* Find(const UWorld* World);

	/** EHOLPauseAnswer::Allowed of the script. */
	static constexpr int32 PauseAnswerAllowed = 0;
	/** The journal's reason for a refused pause answer (the order of EHOLPauseAnswer of the script). */
	static const TCHAR* PauseAnswerToText(const int32 Answer)
	{
		static const TCHAR* const Texts[] = { TEXT("allowed"), TEXT("not in the match"), TEXT("no pause left"), TEXT("already paused"), TEXT("not paused"), TEXT("timer running") };
		return Answer >= 0 && Answer < static_cast<int32>(UE_ARRAY_COUNT(Texts)) ? Texts[Answer] : TEXT("?");
	}

	static int32 AskCanPause(const UWorld* World, bool bSolo, int32 PausedBySlot, int32 PausesLeft, bool bRequesterInMatch);
	static int32 AskCanResume(const UWorld* World, bool bSolo, int32 PausedBySlot, double PausedAtSeconds, int32 RequesterSlot, bool bRequesterInMatch, double Now);
	/** The pauses left once an allowed pause is taken, or -1 (no board). */
	static int32 AskPausesLeftAfterPause(const UWorld* World, bool bSolo, int32 PausesLeft);
	static int32 AskStanceGate(const UWorld* World, bool bAuthority, bool bDiplomacyMode, bool bFromOccupied, bool bToOccupied);
	/** 1 open, 0 shut, -1 no board. */
	static int32 AskVisionGate(const UWorld* World, bool bAuthority, bool bDiplomacyMode);
	/** 1 yes, 0 no, -1 no board. */
	static int32 AskEvaluateMatchEnd(const UWorld* World, bool bAnnounce, bool bFormed);
	/** S2a: 1 open, 0 shut, -1 no board (HOLMatchFlow::DefeatGateOpen of the script, the gate of AHOLGameState::DefeatSlot). */
	static int32 AskDefeatGate(const UWorld* World, bool bAuthority, bool bMatchOver, bool bReasonNone, bool bSlotValid, bool bDefeated, bool bCampOccupied);
	/** S2c: 1 open, 0 shut, -1 no board (HOLMatchFlow::FallGateOpen, the gate of AHOLGameState::HandleForumFall). Game thread only. */
	static int32 AskFallGate(const UWorld* World, bool bAuthority, bool bPrimaryForum, bool bSlotValid, bool bDefeated, bool bPendingForum, bool bCampOccupied);
	/** S2c: 1 open, 0 shut, -1 no board (HOLMatchFlow::KingGateOpen, the gate of AHOLGameState::HandleKingDeath). Game thread only. */
	static int32 AskKingGate(const UWorld* World, bool bAuthority, bool bMatchOver, bool bSlotValid, bool bDefeated, bool bPendingKing, bool bCampOccupied);
	/** S2a: the winner mask 0..255 when the match is over, MatchNotOver when not, -1 no board (HOLMatchFlow::MatchEndVerdict). */
	static int32 AskMatchEnd(const UWorld* World, const TArray<int32>& States, const TArray<int32>& Teams, int32 DefeatedMask, bool bDiplomacyMode, int32 AlliedWinnerMask);

	/** The script's HOLMatchFlow::MatchNotOver() (the verdict of a match that goes on; a winner mask is below it). */
	static constexpr int32 MatchNotOver = 256;

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
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|MatchFlow")
	int32 DefeatGateAnswer(bool bAuthority, bool bMatchOver, bool bReasonNone, bool bSlotValid, bool bDefeated, bool bCampOccupied);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|MatchFlow")
	int32 FallGateAnswer(bool bAuthority, bool bPrimaryForum, bool bSlotValid, bool bDefeated, bool bPendingForum, bool bCampOccupied);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|MatchFlow")
	int32 KingGateAnswer(bool bAuthority, bool bMatchOver, bool bSlotValid, bool bDefeated, bool bPendingKing, bool bCampOccupied);
	virtual int32 FallGateAnswer_Implementation(bool bAuthority, bool bPrimaryForum, bool bSlotValid, bool bDefeated, bool bPendingForum, bool bCampOccupied);
	virtual int32 KingGateAnswer_Implementation(bool bAuthority, bool bMatchOver, bool bSlotValid, bool bDefeated, bool bPendingKing, bool bCampOccupied);
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|MatchFlow")
	int32 MatchEndAnswer(const TArray<int32>& States, const TArray<int32>& Teams, int32 DefeatedMask, bool bDiplomacyMode, int32 AlliedWinnerMask);
	virtual int32 DefeatGateAnswer_Implementation(bool bAuthority, bool bMatchOver, bool bReasonNone, bool bSlotValid, bool bDefeated, bool bCampOccupied);
	virtual int32 MatchEndAnswer_Implementation(const TArray<int32>& States, const TArray<int32>& Teams, int32 DefeatedMask, bool bDiplomacyMode, int32 AlliedWinnerMask);
	virtual int32 PauseAnswer_Implementation(bool bSolo, int32 PausedBySlot, int32 PausesLeft, bool bRequesterInMatch);
	virtual int32 ResumeAnswer_Implementation(bool bSolo, int32 PausedBySlot, double PausedAtSeconds, int32 RequesterSlot, bool bRequesterInMatch, double Now);
	virtual int32 PausesAfterPause_Implementation(bool bSolo, int32 PausesLeft);
	virtual int32 StanceGateAnswer_Implementation(bool bAuthority, bool bDiplomacyMode, bool bFromOccupied, bool bToOccupied);
	virtual int32 VisionGateAnswer_Implementation(bool bAuthority, bool bDiplomacyMode);
	virtual int32 EvaluateEndAnswer_Implementation(bool bAnnounce, bool bFormed);
};
