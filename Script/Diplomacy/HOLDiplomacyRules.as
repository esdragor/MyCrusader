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

	// The book of a Diplomacy match: Slots players, every stance at the default; IsAI marks the AIs (Enemy toward everybody, D367).
	FHOLDiplomacyBook NewBook(int Slots, const TArray<bool>& IsAI)
	{
		FHOLDiplomacyBook Book;
		Book.Slots = Math::Max(Slots, 0);
		for (int Index = 0; Index < Book.Slots * Book.Slots; Index++)
			Book.Stances.Add(DefaultStance);
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
		Change.After = Relation(Book, From, To);
		Change.bBroke = Change.Before == Ally && Change.After != Ally;
		Change.bFormed = Change.Before != Ally && Change.After == Ally;
		return Change;
	}
}
