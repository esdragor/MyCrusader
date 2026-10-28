// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLHeroSheet.h"
#include "HOLForumMessages.generated.h"

struct FHOLForumState;
struct FHOLMatchConfig;
enum class EHOLAIDifficulty : uint8;

/** V4.1 (V4-Q1): the events of the ruin every player hears. */
UENUM()
enum class EHOLForumEventKind : uint8
{
	/** A main forum fell (V2): Lives left. */
	Fell,
	/** An adversary (OtherSlot) began to hold a forum down, its hold reaching Holding, once per attacker since the count was at 0 (V3, V4.2: ShouldAnnounceHold). */
	HoldStarted,
	/** The foundations of a forum down razed (V3, D348): Lives left. */
	Razed,
	/** A slot defeated, whatever the reason. */
	Defeated,
	/** V5.2 (Regicide, D355): a slot's King died; its defeat (reason King) follows at the end of the step. */
	KingDied,
};

/** V4.1: one event, sent once by the server to every machine (AHOLGameState::MulticastForumEvent). */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLForumEvent
{
	GENERATED_BODY()

	UPROPERTY()
	EHOLForumEventKind Kind = EHOLForumEventKind::Fell;

	/** The slot touched: the forum's owner, or the slot defeated. */
	UPROPERTY()
	int8 Slot = INDEX_NONE;

	/** HoldStarted: the attacker holding (FHOLForumState::HolderSlot); INDEX_NONE otherwise. */
	UPROPERTY()
	int8 OtherSlot = INDEX_NONE;

	/** Fell, Razed: the lives left. */
	UPROPERTY()
	int8 Lives = 0;

	static FHOLForumEvent Make(EHOLForumEventKind Kind, int32 Slot, int32 OtherSlot = INDEX_NONE, int32 Lives = 0);
};

/** V4.1 (V4-Q3, V4-Q4): what a machine shows of an event, as its viewer sees it. */
struct FHOLForumMessage
{
	/** « Le forum de Joueur 2 est tombé — 1 vie », « Votre forum est tombé — 1 vie »... */
	FText Text;
	/** The viewer is the slot touched or its ally (its camp, D357): a major alert (the banner and sound of V4.3). */
	bool bAlert = false;
	/** The slots named in Text, in their order (V4.2 colours their names as the players' table does). */
	TArray<int32> NamedSlots;
	/** For the logs and the tests: "Message: Votre forum est tombé — 1 vie (alert)". */
	FString Shown;
};

/** V4.1: the messages of the ruin, pure: no world, no actor. */
namespace HOLForumMessages
{
	/** « Vous » for the viewer's own slot, « Joueur N » otherwise (as the players' table, HOLForumRules::GetPlayersTable). */
	HEROESOFLIBERTY_API FText GetSlotName(int32 Slot, int32 ViewerSlot);

	/** IA0.1-Q1: a level's name, « Facile », « Normal », « Difficile », « Évolutive » (D422, D329). */
	HEROESOFLIBERTY_API FText GetDifficultyName(EHOLAIDifficulty Difficulty);

	/** IA0.1-Q1: an AI slot's name, « IA <level> N » (N = Slot + 1), ex. « IA Difficile 3 ». */
	HEROESOFLIBERTY_API FText GetAIName(int32 Slot, EHOLAIDifficulty Difficulty);

	/** As GetSlotName, an AI slot of Config named by GetAIName (an AI's player state). */
	HEROESOFLIBERTY_API FText GetSlotName(int32 Slot, int32 ViewerSlot, const FHOLMatchConfig& Config);

	/**
	 * A slot's name where a player reads it (players' table, end banner, statistics, the thread's messages): « Vous » for ViewerSlot, the lobby's
	 * name when the slot has one (M6-Q12), else GetSlotName with Config (an AI's « IA Difficile 3 »).
	 */
	HEROESOFLIBERTY_API FText GetPlayerName(const FHOLMatchConfig& Config, int32 Slot, int32 ViewerSlot);

	/**
	 * The message of Event for the viewer at ViewerSlot (INDEX_NONE: a spectator, never alerted); the camps from Config
	 * (HOLVictoryRules::GetCamp).
	 */
	HEROESOFLIBERTY_API FHOLForumMessage GetMessage(const FHOLForumEvent& Event, const FHOLMatchConfig& Config, int32 ViewerSlot);

	/** J8a.4 (D397, D202): the minor alert « Votre héros est prêt à revenir », a thread's line, no banner nor sound. */
	HEROESOFLIBERTY_API FHOLForumMessage GetHeroReadyMessage();

	HEROESOFLIBERTY_API const TCHAR* GetKindName(EHOLForumEventKind Kind);

	/**
	 * V4.2, server only: whether the hold of State (after its step) starts a HoldStarted message. Announced, a bit per
	 * attacker's slot, the attackers already announced for this ruin since its count was at 0: one message for an attacker
	 * new to Holding (from None, from Contested, or taking the lead from another), none for the same one again after a
	 * contest or a recession. Forgotten when the count is back at 0 (None), and at the next fall and once standing (the
	 * caller clears it).
	 */
	HEROESOFLIBERTY_API bool ShouldAnnounceHold(const FHOLForumState& State, uint8& Announced);

	/** V4.2: one piece of a message's line, its text and the slot it names (its colour), INDEX_NONE for the plain words. */
	struct FPiece
	{
		FString Text;
		int32 Slot = INDEX_NONE;
	};

	/** V4.2: Message's text cut at the names of its NamedSlots, in their order (as ViewerSlot reads them), for their colours. */
	HEROESOFLIBERTY_API TArray<FPiece> SplitNames(const FHOLForumMessage& Message, int32 ViewerSlot);

	/** As SplitNames, Config's lobby names and AI slots named as GetMessage writes them (GetPlayerName). */
	HEROESOFLIBERTY_API TArray<FPiece> SplitNames(const FHOLForumMessage& Message, int32 ViewerSlot, const FHOLMatchConfig& Config);
}

/**
 * J8a.4 (D397, D202): when the viewer's own hero is ready to return. Fed with the viewer's public sheet state at each
 * change: true once when it turns ReadyToReturn (once per death: Alive or Dead in between makes the next one new). A
 * spectator (INDEX_NONE) never. Pure: the game state feeds it (AHOLGameState::OnHeroReadyMessage).
 */
struct HEROESOFLIBERTY_API FHOLHeroReadyWatch
{
	EHOLHeroState Seen = EHOLHeroState::None;

	bool Update(int32 ViewerSlot, EHOLHeroState State);
};

/** V4.2: a line of the thread of messages, and when it came (seconds, the HUD's real time). */
struct FHOLForumThreadLine
{
	FHOLForumMessage Message;
	double AddedSeconds = 0.0;
	/** V4.3: the slot touched (FHOLForumEvent::Slot): a click on a line carrying an alert centres the camera on its forum. */
	int32 Slot = INDEX_NONE;
};

/**
 * V4.2 (V4-Q2): the thread of messages, right of the screen under the players' table: at most MaxLines, the newest at the
 * bottom (the oldest dropped), each LineSeconds then gone, its last FadeSeconds fading. Pure: the HUD gives the time.
 */
struct HEROESOFLIBERTY_API FHOLForumThread
{
	static constexpr double LineSeconds = 8.0;
	static constexpr double FadeSeconds = 0.5;
	static constexpr int32 MaxLines = 5;

	/** The oldest first. */
	TArray<FHOLForumThreadLine> Lines;

	void Add(const FHOLForumMessage& Message, double NowSeconds, int32 Slot = INDEX_NONE);

	/** Drops the lines LineSeconds old or more; true if a line went. */
	bool Expire(double NowSeconds);

	/** 1, then down to 0 over the line's last FadeSeconds. */
	static float GetOpacity(const FHOLForumThreadLine& Line, double NowSeconds);

	/** For the HUD's report: "[Votre forum est tombé — 1 vie | Joueur 2 est vaincu]", the oldest first. */
	FString Describe() const;
};

/**
 * V4.3 (D353, D202): the major alert, a banner top centre under the resources' bar: the message of an event touching
 * the viewer or its ally (FHOLForumMessage::bAlert), ShowSeconds then gone, its last FadeSeconds fading. None once the
 * match is over for the viewer (its Victory / Defeat banner due, HOLGameMenuRules::GetEndBanner): the thread's line is
 * enough. V4.5b: a shown banner stays MinSeconds at least before a new one replaces it; an alert coming sooner waits
 * (at most MaxWaiting, the oldest in excess dropped), then is shown its ShowSeconds; one sound for alerts coming less than
 * SoundGapSeconds apart, played with the banner shown (bSound), not on arrival. Pure: the HUD gives the time.
 */
struct HEROESOFLIBERTY_API FHOLForumAlert
{
	static constexpr double ShowSeconds = 4.0;
	static constexpr double FadeSeconds = 0.5;
	static constexpr double MinSeconds = 1.5;
	static constexpr int32 MaxWaiting = 2;
	static constexpr double SoundGapSeconds = 1.0;
	/** The alert's short sound, 2D (the thread's lines are silent); provisional, the herald's voice will replace it. */
	static constexpr const TCHAR* SoundPath = TEXT("/Engine/EngineSounds/1kSineTonePing.1kSineTonePing");

	FHOLForumMessage Message;
	/** The slot touched (FHOLForumEvent::Slot): a click on the banner centres the camera on its forum. */
	int32 Slot = INDEX_NONE;
	double AddedSeconds = 0.0;
	bool bShown = false;
	/** The shown banner has the sound (the first of its burst). */
	bool bSound = false;

	struct FWaiting
	{
		FHOLForumMessage Message;
		int32 Slot = INDEX_NONE;
		bool bSound = false;
	};
	/** The alerts come while the shown banner had not its MinSeconds, the oldest first. */
	TArray<FWaiting> Waiting;
	/** When the last alert came (for SoundGapSeconds). */
	double LastArrivalSeconds = -1.0e9;

	/**
	 * Takes InMessage for InSlot if it is an alert and the match goes on for the viewer: shown at once if no banner is or
	 * the shown one had its MinSeconds, waiting otherwise; true if a banner was shown now (the HUD fills it, plays its sound).
	 */
	bool Show(const FHOLForumMessage& InMessage, int32 InSlot, bool bMatchOverForViewer, double NowSeconds);

	/** Shows the next waiting alert once the shown one had its MinSeconds (none waiting once the match is over); true if shown. */
	bool Advance(double NowSeconds, bool bMatchOverForViewer);

	/** Hides the banner ShowSeconds old or more, none waiting; true if it went. */
	bool Expire(double NowSeconds);

	/** 1, then down to 0 over the banner's last FadeSeconds; 0 hidden. */
	float GetOpacity(double NowSeconds) const;

	/** For the HUD's report: "Votre forum est tombé — 1 vie (slot 0)", or "none". */
	FString Describe() const;
};
