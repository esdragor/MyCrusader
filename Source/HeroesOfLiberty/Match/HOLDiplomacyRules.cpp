// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLDiplomacyRules.h"

namespace HOLDiplomacy
{
	FBook NewBook(const int32 Slots, const TArray<bool>& IsAI)
	{
		FBook Book;
		Book.Slots = FMath::Max(Slots, 0);
		for (int32 Index = 0; Index < Book.Slots * Book.Slots; ++Index)
		{
			Book.Stances.Add(DefaultStance);
			Book.Vision.Add(false);
		}
		for (int32 Slot = 0; Slot < Book.Slots; ++Slot)
		{
			Book.bAI.Add(IsAI.IsValidIndex(Slot) && IsAI[Slot]);
		}
		return Book;
	}

	FBook NewTeamBook(const TArray<int32>& TeamBySlot, const TArray<bool>& IsAI)
	{
		FBook Book = NewBook(TeamBySlot.Num(), IsAI);
		Book.bLocked = true;
		for (int32 From = 0; From < Book.Slots; ++From)
		{
			for (int32 To = 0; To < Book.Slots; ++To)
			{
				const bool bSameTeam = TeamBySlot[From] > 0 && TeamBySlot[From] == TeamBySlot[To];
				Book.Stances[From * Book.Slots + To] = bSameTeam ? Ally : Enemy;
			}
		}
		return Book;
	}

	bool InMatch(const FBook& Book, const int32 Slot)
	{
		return Slot >= 0 && Slot < Book.Slots;
	}

	int32 StanceOf(const FBook& Book, const int32 From, const int32 To)
	{
		if (!InMatch(Book, From) || !InMatch(Book, To))
		{
			return Enemy;
		}
		return Book.Stances[From * Book.Slots + To];
	}

	int32 Relation(const FBook& Book, const int32 A, const int32 B)
	{
		if (!InMatch(Book, A) || !InMatch(Book, B))
		{
			return Enemy;
		}
		if (A == B)
		{
			return Ally;
		}
		if (!Book.bLocked && (Book.bAI[A] || Book.bAI[B]))
		{
			return Enemy;
		}
		return FMath::Min(StanceOf(Book, A, B), StanceOf(Book, B, A));
	}

	bool IsAlly(const FBook& Book, const int32 A, const int32 B)
	{
		return InMatch(Book, A) && InMatch(Book, B) && A != B && Relation(Book, A, B) == Ally;
	}

	bool FiresAtOnce(const FBook& Book, const int32 A, const int32 B)
	{
		return InMatch(Book, A) && InMatch(Book, B) && A != B && Relation(Book, A, B) == Enemy;
	}

	bool IsAdversary(const FBook& Book, const int32 A, const int32 B)
	{
		return InMatch(Book, A) && InMatch(Book, B) && A != B && Relation(Book, A, B) != Ally;
	}

	FStanceChange SetStance(FBook& Book, const int32 From, const int32 To, const int32 Stance)
	{
		FStanceChange Change;
		Change.From = From;
		Change.To = To;
		Change.Stance = Stance;
		if (!InMatch(Book, From) || !InMatch(Book, To))
		{
			Change.Result = EResult::NoSuchSlot;
		}
		else if (From == To)
		{
			Change.Result = EResult::Self;
		}
		else if (Stance < Enemy || Stance > Ally)
		{
			Change.Result = EResult::BadStance;
		}
		else if (Book.bAI[From])
		{
			Change.Result = EResult::SenderIsAI;
		}
		else if (Book.bLocked)
		{
			Change.Result = EResult::Locked;
		}
		if (Change.Result != EResult::Ok)
		{
			return Change;
		}

		Change.Before = Relation(Book, From, To);
		Change.bAnnounce = StanceOf(Book, From, To) != Stance;
		Book.Stances[From * Book.Slots + To] = Stance;
		Change.After = Relation(Book, From, To);
		Change.bBroke = Change.Before == Ally && Change.After != Ally;
		Change.bFormed = Change.Before != Ally && Change.After == Ally;
		// Everything shared is cut at that second (D370, D371), and not given back by itself if the alliance is formed again.
		if (Change.bBroke)
		{
			Book.Vision[From * Book.Slots + To] = false;
			Book.Vision[To * Book.Slots + From] = false;
		}
		return Change;
	}

	FVisionChange SetVision(FBook& Book, const int32 From, const int32 To, const bool bShare)
	{
		FVisionChange Change;
		Change.From = From;
		Change.To = To;
		Change.bShare = bShare;
		if (!InMatch(Book, From) || !InMatch(Book, To))
		{
			Change.Result = EResult::NoSuchSlot;
		}
		else if (From == To)
		{
			Change.Result = EResult::Self;
		}
		else if (Book.bAI[From])
		{
			Change.Result = EResult::SenderIsAI;
		}
		else if (Book.bLocked)
		{
			Change.Result = EResult::Locked;
		}
		else if (!IsAlly(Book, From, To))
		{
			Change.Result = EResult::NotAlly;
		}
		if (Change.Result != EResult::Ok)
		{
			return Change;
		}

		Change.bAnnounce = Book.Vision[From * Book.Slots + To] != bShare;
		Book.Vision[From * Book.Slots + To] = bShare;
		return Change;
	}

	bool SharesVision(const FBook& Book, const int32 From, const int32 To)
	{
		if (!InMatch(Book, From) || !InMatch(Book, To))
		{
			return false;
		}
		if (From == To)
		{
			return true;
		}
		if (!IsAlly(Book, From, To))
		{
			return false;
		}
		return Book.bLocked || Book.Vision[From * Book.Slots + To];
	}

	bool CanGarrison(const int32 UnitOwner, const int32 BuildingOwner)
	{
		return UnitOwner >= 0 && UnitOwner == BuildingOwner;
	}

	bool DoorsOpenFor(const FBook& Book, const int32 Owner, const int32 Other)
	{
		return InMatch(Book, Owner) && InMatch(Book, Other) && (Owner == Other || IsAlly(Book, Owner, Other));
	}

	bool CanLeaveRampart(const FBook& Book, const int32 UnitOwner, const int32 RampartOwner)
	{
		return DoorsOpenFor(Book, RampartOwner, UnitOwner);
	}

	FAlliedVictory AlliedVictory(const FBook& Book, const TArray<bool>& Defeated)
	{
		FAlliedVictory Victory;
		bool bSomeoneBeaten = false;
		TArray<int32> Survivors;
		for (int32 Slot = 0; Slot < Book.Slots; ++Slot)
		{
			if (Defeated.IsValidIndex(Slot) && Defeated[Slot])
			{
				bSomeoneBeaten = true;
			}
			else
			{
				Survivors.Add(Slot);
			}
		}
		if (!bSomeoneBeaten || Survivors.Num() == 0)
		{
			return Victory;
		}
		for (int32 First = 0; First < Survivors.Num(); ++First)
		{
			for (int32 Second = First + 1; Second < Survivors.Num(); ++Second)
			{
				if (!IsAlly(Book, Survivors[First], Survivors[Second]))
				{
					return Victory;
				}
			}
		}
		Victory.bEnded = true;
		Victory.Winners = Survivors;
		return Victory;
	}
}
