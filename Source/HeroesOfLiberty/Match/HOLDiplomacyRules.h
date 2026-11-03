// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * The stances of the Diplomacy mode where the simulation and the network run (D4 of DiplomacyInProgress.md; GDD 14.3, D369, D370, D371,
 * D374, D375, D401, D367, D373): Ally / Neutral / Enemy of each player toward each other, the relation of a pair (the most hostile of the
 * two), the immediate break, the vision given to an ally, the access of doors and ramparts, the allied victory. The same rule as
 * Script/Diplomacy/HOLDiplomacyRules.as (which the AI and the interface use), whole numbers only so that the server and the clients agree;
 * both are tested on the same cases. Nothing here reads the world: the state per match, the orders and the hooks come after (D5, D6).
 */
namespace HOLDiplomacy
{
	/** The stances, the lower the more hostile: the relation of a pair is the minimum of its two stances. */
	constexpr int32 Enemy = 0;
	constexpr int32 Neutral = 1;
	constexpr int32 Ally = 2;

	/** The stance at launch of a Diplomacy match (DQ-1, provisional: Enemy, pure free-for-all). */
	constexpr int32 DefaultStance = Enemy;

	/** The reasons of a refusal, in the order they are told (the first that applies). */
	enum class EResult : uint8
	{
		Ok = 0,
		NoSuchSlot = 1,
		Self = 2,
		BadStance = 3,
		SenderIsAI = 4,
		Locked = 5,
		/** Vision only: not allied (both must say Ally first). */
		NotAlly = 6,
	};

	/** What a SetStance did, for the announcement to all and for what the change cuts or opens. */
	struct HEROESOFLIBERTY_API FStanceChange
	{
		EResult Result = EResult::Ok;
		/** A stance really changed (the line told to all). */
		bool bAnnounce = false;
		int32 From = 0;
		int32 To = 0;
		int32 Stance = 0;
		/** The relation of the pair before and after. */
		int32 Before = 0;
		int32 After = 0;
		/** The pair was allied and no longer is / became allied. */
		bool bBroke = false;
		bool bFormed = false;
	};

	/** What a SetVision did. */
	struct HEROESOFLIBERTY_API FVisionChange
	{
		EResult Result = EResult::Ok;
		bool bAnnounce = false;
		int32 From = 0;
		int32 To = 0;
		bool bShare = false;
	};

	/** The end by allied victory: the winners, together, when it ends. */
	struct HEROESOFLIBERTY_API FAlliedVictory
	{
		bool bEnded = false;
		TArray<int32> Winners;
	};

	/** The stances of a match: the stance of From toward To is Stances[From * Slots + To]. */
	struct HEROESOFLIBERTY_API FBook
	{
		int32 Slots = 0;
		/** Outside the Diplomacy mode (teams fixed at launch): no stance can be set. */
		bool bLocked = false;
		TArray<int32> Stances;
		TArray<bool> bAI;
		/** "Share my vision" (D371): Vision[From * Slots + To] = From lets To see what it sees. Meaningful between allies only. */
		TArray<bool> Vision;
	};

	/** The book of a Diplomacy match: Slots players, every stance at the default; IsAI marks the AIs (Enemy toward everybody, D367). */
	HEROESOFLIBERTY_API FBook NewBook(int32 Slots, const TArray<bool>& IsAI);
	/** The book of a match with fixed teams: locked, same team (1 to 4) allies, the others enemies, team 0 alone; an AI on a team is a teammate. */
	HEROESOFLIBERTY_API FBook NewTeamBook(const TArray<int32>& TeamBySlot, const TArray<bool>& IsAI);

	HEROESOFLIBERTY_API bool InMatch(const FBook& Book, int32 Slot);
	/** The stance From has set toward To (Enemy outside the match). */
	HEROESOFLIBERTY_API int32 StanceOf(const FBook& Book, int32 From, int32 To);
	/** The relation of a pair: the most hostile of the two stances (D375). Oneself is an ally; an AI is Enemy to everybody (D367) in the Diplomacy mode. */
	HEROESOFLIBERTY_API int32 Relation(const FBook& Book, int32 A, int32 B);
	HEROESOFLIBERTY_API bool IsAlly(const FBook& Book, int32 A, int32 B);
	/** The units fire on their own at the other's: only at war (Neutral and Ally do not, D369). */
	HEROESOFLIBERTY_API bool FiresAtOnce(const FBook& Book, int32 A, int32 B);
	/** Counts as an adversary in the rules that say "enemy" (ruin, relay, altar, anti-construction zone, duel): Neutral included (D401). */
	HEROESOFLIBERTY_API bool IsAdversary(const FBook& Book, int32 A, int32 B);

	/** From says Stance toward To, at once (D370). Refused, nothing changed: slot outside the match, oneself, unknown stance, an AI, locked. */
	HEROESOFLIBERTY_API FStanceChange SetStance(FBook& Book, int32 From, int32 To, int32 Stance);
	/** From opens or closes its vision to To (D371): allied only, never in the standard modes (automatic there). */
	HEROESOFLIBERTY_API FVisionChange SetVision(FBook& Book, int32 From, int32 To, bool bShare);
	/** To sees what From sees: oneself, teammates of the standard modes, or allies who were given it. */
	HEROESOFLIBERTY_API bool SharesVision(const FBook& Book, int32 From, int32 To);

	/** Only in one's own building (D374). */
	HEROESOFLIBERTY_API bool CanGarrison(int32 UnitOwner, int32 BuildingOwner);
	/** The doors and ramparts of Owner are open to Other: its own and an ally's; shut to everybody else (D371, D374). */
	HEROESOFLIBERTY_API bool DoorsOpenFor(const FBook& Book, int32 Owner, int32 Other);
	/** A unit of UnitOwner on a rampart of RampartOwner can get down while the access is open (D374). */
	HEROESOFLIBERTY_API bool CanLeaveRampart(const FBook& Book, int32 UnitOwner, int32 RampartOwner);

	/** The survivors (not Defeated; a slot beyond the list is alive) all allied to each other and at least one beaten: they win together (D373). */
	HEROESOFLIBERTY_API FAlliedVictory AlliedVictory(const FBook& Book, const TArray<bool>& Defeated);
}
