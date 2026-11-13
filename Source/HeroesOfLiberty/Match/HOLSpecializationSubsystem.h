// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLSpecializationView.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLSpecializationSubsystem.generated.h"

class AHOLPlayerState;

/**
 * The specialization's board where the match runs (SP2, SpecializationInProgress.md; GDD 13, D07, D684, D785). The thinnest bridge,
 * modelled on UHOLSignalSubsystem: the server holds one FHOLSpecializationState per seat in its script subclass
 * (Script/Specialization/HOLSpecializationBoardScript.as, which applies HOLSpecialization::Start, Cancel, Advance), advances the
 * countdowns by the world's time (a paused world stops them) with the Forum's state (paused while it is down, answer A of Q2), and
 * copies each seat's view onto its AHOLPlayerState (owner only). The player controller's ServerStartSpecialization and
 * ServerCancelSpecialization call it. Abstract: only a script subclass is born. A client never decides (its tick does nothing).
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLSpecializationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The answer when no script board exists (HOLSpecialization answers are 0 Ok to 8 NoForumComplete). */
	static constexpr int32 NoScript = -1;

	/** Applied: the seat, the tier and the option (the factions' worksites listen, SP5). Server and solo only. */
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSpecializationApplied, int32 /*Slot*/, int32 /*Tier*/, int32 /*Option*/);
	FOnSpecializationApplied OnApplied;

	/** The world's specialization board (its script subclass), or null (no script compiled, or no world). */
	static UHOLSpecializationSubsystem* Find(const UWorld* World);

	/**
	 * The seat has a complete Forum (HOLNomad::HasCompleteForum, answer B of Q-nomad-forum Q2: the button stays off without one).
	 * Read on the world's board (HasCompleteForumNow): a standard seat is true, a Nomad seat false until its first Forum is complete.
	 */
	static bool IsForumComplete(const UWorld* World, int32 Slot);

	/** Slot plays the Nomad option (FHOLMatchConfig::bNomad, HOLNomad::NewState): no complete Forum until its first one (N2, at the start). */
	void SetNomadNow(int32 Slot, bool bNomad);

	/** Slot's first Forum is complete (HOLNomad::NoteForumComplete): the normal rules for good. */
	void NoteForumCompleteNow(int32 Slot);

	/** HOLNomad::HasCompleteForum of Slot's state: true for a standard seat, and true without script. */
	bool HasCompleteForumNow(int32 Slot);

	/** The seat's Forum stands (not down, AHOLGameState::IsForumDown): the countdown runs, else it waits (Q2 answer A). */
	static bool IsForumStanding(const UWorld* World, int32 Slot);

	/** Slot starts the unlock of Option at Tier (PlayerTier = AHOLGameState::GetTier): the HOLSpecialization answer, NoScript without script. */
	int32 StartNow(int32 Slot, int32 Tier, int32 Option, int32 PlayerTier, bool bForumComplete);

	/** Slot cancels its running unlock: the HOLSpecialization answer, NoScript without script. */
	int32 CancelNow(int32 Slot);

	/** Slot's choice of Tier is open (the button lights, HOLSpecialization::IsOpen); false without script. */
	bool IsOpenNow(int32 Slot, int32 Tier, int32 PlayerTier, bool bForumComplete);

	/** Slot's countdown runs ElapsedMs of match time: the tier applied (0 for none), broadcast on OnApplied. */
	int32 AdvanceNow(int32 Slot, int32 ElapsedMs, bool bForumStanding);

	/** What Slot's owner reads (copied onto its AHOLPlayerState). */
	FHOLSpecializationView ViewNow(int32 Slot);

	/** Every seated player runs ElapsedMs (its Forum's state read from the game state), then its view is copied onto its player state. */
	void AdvanceAllNow(int32 ElapsedMs);

	/** Copies Slot's view onto its player state (after a start or a cancel, so the owner reads it at once). */
	void RefreshView(AHOLPlayerState& PlayerState);

	// UTickableWorldSubsystem: the server's and the solo's world time drives the countdowns.
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Specialization")
	int32 StartSpecialization(int32 Slot, int32 Tier, int32 Option, int32 PlayerTier, bool bForumComplete);
	virtual int32 StartSpecialization_Implementation(int32 Slot, int32 Tier, int32 Option, int32 PlayerTier, bool bForumComplete) { return NoScript; }

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Specialization")
	int32 CancelSpecialization(int32 Slot);
	virtual int32 CancelSpecialization_Implementation(int32 Slot) { return NoScript; }

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Specialization")
	bool IsSpecializationOpen(int32 Slot, int32 Tier, int32 PlayerTier, bool bForumComplete);
	virtual bool IsSpecializationOpen_Implementation(int32 Slot, int32 Tier, int32 PlayerTier, bool bForumComplete) { return false; }

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Specialization")
	int32 AdvanceSpecialization(int32 Slot, int32 ElapsedMs, bool bForumStanding);
	virtual int32 AdvanceSpecialization_Implementation(int32 Slot, int32 ElapsedMs, bool bForumStanding) { return 0; }

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Specialization")
	FHOLSpecializationView ViewSpecialization(int32 Slot);
	virtual FHOLSpecializationView ViewSpecialization_Implementation(int32 Slot) { return FHOLSpecializationView(); }

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Specialization")
	void SetSeatNomad(int32 Slot, bool bNomad);
	virtual void SetSeatNomad_Implementation(int32 Slot, bool bNomad) {}

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Specialization")
	void NoteSeatForumComplete(int32 Slot);
	virtual void NoteSeatForumComplete_Implementation(int32 Slot) {}

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Specialization")
	bool SeatHasCompleteForum(int32 Slot);
	virtual bool SeatHasCompleteForum_Implementation(int32 Slot) { return true; }

private:
	/** The world time (ms) of the last tick, -1 before the first. */
	int64 LastTickMs = -1;
};
