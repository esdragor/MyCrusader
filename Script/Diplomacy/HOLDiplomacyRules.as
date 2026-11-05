// The stances of the Diplomacy mode, pure rules (D369, D370, D375, D401, D367; GDD 14.3): each player sets a stance, Ally / Neutral / Enemy,
// toward each other player (one-sided, immediate, announced to all); the RELATION of a pair is the most hostile of the two stances
// (D375), so two players are allies only if both say Ally. The break is immediate (D370): the relation moves at the very call, straight to
// Enemy if asked, without notice. Neutral is an adversary that does not fire on its own (D401): it takes part in every "enemy" rule
// (ruin, relay, altar, anti-construction zone, duel) but not in the automatic fire. The AIs are alone against all (D367, D433): Enemy
// toward everybody and they take no stance. Outside the Diplomacy mode the teams are fixed and the stances locked (D387).
// Integers and arrays only (client and server agree); nothing here reads the world. Tested in HOLDiplomacyRules_Test.as.
// The stance of the pairs lives in a flat array: the stance of From toward To is Stances[From * Slots + To].
struct FHOLDiplomacyBook
{
	int Slots = 0;
	// Outside the Diplomacy mode (teams fixed at launch): no stance can be set.
	bool bLocked = false;
	TArray<int> Stances;
	TArray<bool> bAI;
	// "Share my vision" (D371): Vision[From * Slots + To] = From lets To see what it sees. Meaningful between allies only.
	TArray<bool> Vision;
}

// What a SetStance did, for the announcement to all and for what the change cuts or opens.
struct FHOLStanceChange
{
	// HOLDiplomacy::Ok, or the reason of the refusal (nothing changed).
	int Result = 0;
	// A stance really changed (the line told to all): who set which stance toward whom.
	bool bAnnounce = false;
	int From = 0;
	int To = 0;
	int Stance = 0;
	// The relation of the pair before and after (it does not move while the other side stays the most hostile).
	int Before = 0;
	int After = 0;
	// The pair was allied and no longer is (everything shared is cut at that second, D370, D371); the pair became allied.
	bool bBroke = false;
	bool bFormed = false;
}

// What a SetVision did: the line told to all (who opens or closes the vision toward whom).
struct FHOLVisionChange
{
	int Result = 0;
	bool bAnnounce = false;
	int From = 0;
	int To = 0;
	bool bShare = false;
}

// The end by allied victory (D373): the winners, together, when it ends.
struct FHOLAlliedVictory
{
	bool bEnded = false;
	TArray<int> Winners;
}

namespace HOLDiplomacy
{
	// The stances, the lower the more hostile: the relation of a pair is the minimum of its two stances.
	const int Enemy = 0;
	const int Neutral = 1;
	const int Ally = 2;

	// The stance of each player toward each other at launch of a Diplomacy match (DQ-1, provisional: Enemy, pure free-for-all).
	const int DefaultStance = Enemy;

	// The reasons of a refusal, told in this order (the first that applies).
	const int Ok = 0;
	const int NoSuchSlot = 1;
	const int Self = 2;
	const int BadStance = 3;
	const int SenderIsAI = 4;
	const int Locked = 5;
	// Vision only: not allied (both must say Ally first).
	const int NotAlly = 6;

	// The book of a Diplomacy match: Slots players, every stance at the default; IsAI marks the AIs (Enemy toward everybody, D367).
	FHOLDiplomacyBook NewBook(int Slots, const TArray<bool>& IsAI)
	{
		FHOLDiplomacyBook Book;
		Book.Slots = Math::Max(Slots, 0);
		for (int Index = 0; Index < Book.Slots * Book.Slots; Index++)
		{
			Book.Stances.Add(DefaultStance);
			Book.Vision.Add(false);
		}
		for (int Slot = 0; Slot < Book.Slots; Slot++)
			Book.bAI.Add(Slot < IsAI.Num() && IsAI[Slot]);
		return Book;
	}

	// The book of a match with fixed teams (standard modes): the stances are locked, the same team (1 to 4) are allies, the others enemies,
	// team 0 is alone. An AI on a team is a teammate there: D367 is the rule of the Diplomacy mode only.
	FHOLDiplomacyBook NewTeamBook(const TArray<int>& TeamBySlot, const TArray<bool>& IsAI)
	{
		FHOLDiplomacyBook Book = NewBook(TeamBySlot.Num(), IsAI);
		Book.bLocked = true;
		for (int From = 0; From < Book.Slots; From++)
		{
			for (int To = 0; To < Book.Slots; To++)
			{
				const bool bSameTeam = TeamBySlot[From] > 0 && TeamBySlot[From] == TeamBySlot[To];
				Book.Stances[From * Book.Slots + To] = bSameTeam ? Ally : Enemy;
			}
		}
		return Book;
	}

	bool InMatch(const FHOLDiplomacyBook& Book, int Slot)
	{
		return Slot >= 0 && Slot < Book.Slots;
	}

	// The stance From has set toward To (Enemy outside the match).
	int StanceOf(const FHOLDiplomacyBook& Book, int From, int To)
	{
		if (!InMatch(Book, From) || !InMatch(Book, To))
			return Enemy;
		return Book.Stances[From * Book.Slots + To];
	}

	// The relation of a pair: the most hostile of the two stances (D375), the same from either side. Oneself is an ally; an AI is Enemy to
	// everybody in the Diplomacy mode (D367); a slot outside the match is Enemy.
	int Relation(const FHOLDiplomacyBook& Book, int A, int B)
	{
		if (!InMatch(Book, A) || !InMatch(Book, B))
			return Enemy;
		if (A == B)
			return Ally;
		if (!Book.bLocked && (Book.bAI[A] || Book.bAI[B]))
			return Enemy;
		return Math::Min(StanceOf(Book, A, B), StanceOf(Book, B, A));
	}

	// Allies: both said Ally (or the same team). Shares the doors, the heals and the auras (D371); the vision is a box of its own.
	bool IsAlly(const FHOLDiplomacyBook& Book, int A, int B)
	{
		return InMatch(Book, A) && InMatch(Book, B) && A != B && Relation(Book, A, B) == Ally;
	}

	// The units fire on their own at the other's (only at war: Neutral and Ally do not, D369). Nobody fires on oneself.
	bool FiresAtOnce(const FHOLDiplomacyBook& Book, int A, int B)
	{
		return InMatch(Book, A) && InMatch(Book, B) && A != B && Relation(Book, A, B) == Enemy;
	}

	// Counts as an adversary in the rules that say "enemy" (ruin, relay, altar, anti-construction zone, duel): Neutral included (D401).
	bool IsAdversary(const FHOLDiplomacyBook& Book, int A, int B)
	{
		return InMatch(Book, A) && InMatch(Book, B) && A != B && Relation(Book, A, B) != Ally;
	}

	// From says Stance toward To. Takes effect at once, from the very call (D370): a break is not a notice. Refused (nothing changed): a
	// slot outside the match, oneself, a stance that does not exist, an AI (it takes none), the locked stances of the standard modes.
	FHOLStanceChange SetStance(FHOLDiplomacyBook& Book, int From, int To, int Stance)
	{
		FHOLStanceChange Change;
		Change.From = From;
		Change.To = To;
		Change.Stance = Stance;
		if (!InMatch(Book, From) || !InMatch(Book, To))
			Change.Result = NoSuchSlot;
		else if (From == To)
			Change.Result = Self;
		else if (Stance < Enemy || Stance > Ally)
			Change.Result = BadStance;
		else if (Book.bAI[From])
			Change.Result = SenderIsAI;
		else if (Book.bLocked)
			Change.Result = Locked;
		if (Change.Result != Ok)
			return Change;

		Change.Before = Relation(Book, From, To);
		Change.bAnnounce = StanceOf(Book, From, To) != Stance;
		Book.Stances[From * Book.Slots + To] = Stance;
		// Lowering the relation is mutual and at once (user answer of 2026-11-04): the other side's stance toward From drops to this stance too.
		if (Stance < Change.Before)
			Book.Stances[To * Book.Slots + From] = Math::Min(Book.Stances[To * Book.Slots + From], Stance);
		Change.After = Relation(Book, From, To);
		Change.bBroke = Change.Before == Ally && Change.After != Ally;
		Change.bFormed = Change.Before != Ally && Change.After == Ally;
		// Everything shared is cut at that second (D370, D371), and it is not given back by itself if the alliance is formed again.
		if (Change.bBroke)
		{
			Book.Vision[From * Book.Slots + To] = false;
			Book.Vision[To * Book.Slots + From] = false;
		}
		return Change;
	}

	// From opens (or closes) its vision to To: a box ticked toward one ally, one-sided, announced to all (D371). Allied only (both said
	// Ally); in the standard modes the vision of teammates is automatic, there is no box (Locked). Same refusals as a stance, in order.
	FHOLVisionChange SetVision(FHOLDiplomacyBook& Book, int From, int To, bool bShare)
	{
		FHOLVisionChange Change;
		Change.From = From;
		Change.To = To;
		Change.bShare = bShare;
		if (!InMatch(Book, From) || !InMatch(Book, To))
			Change.Result = NoSuchSlot;
		else if (From == To)
			Change.Result = Self;
		else if (Book.bAI[From])
			Change.Result = SenderIsAI;
		else if (Book.bLocked)
			Change.Result = Locked;
		else if (!IsAlly(Book, From, To))
			Change.Result = NotAlly;
		if (Change.Result != Ok)
			return Change;

		Change.bAnnounce = Book.Vision[From * Book.Slots + To] != bShare;
		Book.Vision[From * Book.Slots + To] = bShare;
		return Change;
	}

	// To sees what From sees: oneself always; teammates of the standard modes always (automatic); otherwise allies who were given it.
	bool SharesVision(const FHOLDiplomacyBook& Book, int From, int To)
	{
		if (!InMatch(Book, From) || !InMatch(Book, To))
			return false;
		if (From == To)
			return true;
		if (!IsAlly(Book, From, To))
			return false;
		return Book.bLocked || Book.Vision[From * Book.Slots + To];
	}

	// No garrison in an ally's building, in the standard modes as in the Diplomacy mode (D374, revising D371): only in one's own.
	bool CanGarrison(int UnitOwner, int BuildingOwner)
	{
		return UnitOwner >= 0 && UnitOwner == BuildingOwner;
	}

	// The doors and ramparts of Owner are open to Other: its own, and an ally's (D371); shut to everybody else, at the very second an
	// alliance breaks (D370, D374).
	bool DoorsOpenFor(const FHOLDiplomacyBook& Book, int Owner, int Other)
	{
		return InMatch(Book, Owner) && InMatch(Book, Other) && (Owner == Other || IsAlly(Book, Owner, Other));
	}

	// A unit of UnitOwner standing on a rampart of RampartOwner can get down: it can while the access is open. After a break the units
	// of the former ally stay up there for good (D374: "tant pis").
	bool CanLeaveRampart(const FHOLDiplomacyBook& Book, int UnitOwner, int RampartOwner)
	{
		return DoorsOpenFor(Book, RampartOwner, UnitOwner);
	}

	// The allied victory (D373): the survivors (not Defeated; a slot beyond the list is alive) are all allied to each other and at least one
	// player has been beaten: they win together, at once. Nobody beaten: a general alliance ends nothing. No survivor: not an allied victory.
	// An AI alive is Enemy to everybody in the Diplomacy mode (D367) and so blocks it. The standard modes' fixed teams give the team victory.
	FHOLAlliedVictory AlliedVictory(const FHOLDiplomacyBook& Book, const TArray<bool>& Defeated)
	{
		FHOLAlliedVictory Victory;
		bool bSomeoneBeaten = false;
		TArray<int> Survivors;
		for (int Slot = 0; Slot < Book.Slots; Slot++)
		{
			if (Slot < Defeated.Num() && Defeated[Slot])
				bSomeoneBeaten = true;
			else
				Survivors.Add(Slot);
		}
		if (!bSomeoneBeaten || Survivors.Num() == 0)
			return Victory;
		for (int First = 0; First < Survivors.Num(); First++)
		{
			for (int Second = First + 1; Second < Survivors.Num(); Second++)
			{
				if (!IsAlly(Book, Survivors[First], Survivors[Second]))
					return Victory;
			}
		}
		Victory.bEnded = true;
		Victory.Winners = Survivors;
		return Victory;
	}
}
