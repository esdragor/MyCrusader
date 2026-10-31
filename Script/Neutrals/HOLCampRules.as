// The neutral camps' rules (NeutralsInProgress.md N1, GDD 8.4, D32, D401): pure functions of a settings struct and what the camp sees,
// tested alone in script (HOLCampRules_Test.as). A camp is a group of neutral units guarding a post (a rich deposit or a pass). It
// guards, fights what comes, returns to its post when the fight is over, and once all its members are dead it is cleared for good
// (D32: no respawn). The two design choices left to the user (NQ1: aggro on sight or only when attacked; NQ2: who takes the XP of a
// cleared camp) are SETTINGS, both coded. Every number is provisional, tuned in test. The simulation (N3) calls these; nothing here
// touches the world.

// How a camp is drawn into a fight (NQ1).
enum EHOLCampAggro
{
	// Anyone of an adversary camp within AggroRadius of the post draws it.
	OnSight,
	// Only an attack on one of its members draws it.
	OnAttack,
}

// Who gets the XP of a cleared camp (NQ2).
enum EHOLCampCredit
{
	// The owner of the last blow.
	LastBlow,
	// The owner who dealt the most damage (the lowest owner on a tie).
	MostDamage,
	// Everyone who hit it, in proportion to the damage dealt.
	Shared,
}

enum EHOLCampState
{
	// Standing at the post, nothing near.
	Guarding,
	// Fighting an intruder.
	Fighting,
	// The fight is over: the members go back to the post.
	Returning,
	// Every member is dead: for the rest of the game (no respawn).
	Cleared,
}

struct FHOLCampSettings
{
	EHOLCampAggro Aggro = EHOLCampAggro::OnSight;
	EHOLCampCredit Credit = EHOLCampCredit::Shared;
	// Distance from the post at which an intruder draws the camp (OnSight), cm.
	float AggroRadius = 1500.f;
	// The leash: the camp gives up the chase when no intruder stands within this of the post, and a member farther than this goes back, cm.
	float LeashRadius = 3000.f;
	// The members are back at the post when all stand within this of it, cm.
	float ArriveRadius = 400.f;
	// The XP of a cleared camp (moderate, D32), shared out by the credit rule.
	int RewardXP = 100;
}

struct FHOLCampMember
{
	int Id = -1;
	FVector Location;
}

// A unit of an adversary camp near the post (the camp's owner is Neutral: everyone is an adversary, D401).
struct FHOLCampIntruder
{
	int Id = -1;
	int Owner = -1;
	FVector Location;
}

struct FHOLCampMemory
{
	EHOLCampState State = EHOLCampState::Guarding;
}

struct FHOLCampStep
{
	// The state changed in this step.
	bool bChanged = false;
	// The last member died in this step: the reward is due, once.
	bool bJustCleared = false;
	// The members to send back to the post.
	TArray<int> ReturnIds;
}

// One hit on a member of the camp, in the order it happened (the last is the last blow).
struct FHOLCampHit
{
	int Owner = -1;
	int Damage = 0;
}

struct FHOLCampShare
{
	int Owner = -1;
	int XP = 0;
}

namespace HOLCamp
{
	float FlatDistance(FVector A, FVector B)
	{
		FVector Line = A - B;
		Line.Z = 0;
		return Line.Size();
	}

	// One thought of a camp. Members are the living ones; bAttacked: one of them was hit by an adversary in this step.
	FHOLCampStep Step(FHOLCampMemory& Memory, const FHOLCampSettings& Settings, FVector Post, const TArray<FHOLCampMember>& Members, const TArray<FHOLCampIntruder>& Intruders, bool bAttacked)
	{
		FHOLCampStep Result;
		if (Memory.State == EHOLCampState::Cleared)
		{
			return Result;
		}
		if (Members.Num() == 0)
		{
			Memory.State = EHOLCampState::Cleared;
			Result.bChanged = true;
			Result.bJustCleared = true;
			return Result;
		}

		// What draws the camp: an intruder within the aggro radius (OnSight), or a blow from one it can reach (within the leash radius:
		// the camp does not chase, nor fall back and come again for, what it cannot reach).
		bool bSighted = false;
		bool bReachable = false;
		for (int Index = 0; Index < Intruders.Num(); Index++)
		{
			const float Distance = FlatDistance(Intruders[Index].Location, Post);
			bSighted = bSighted || Distance <= Settings.AggroRadius;
			bReachable = bReachable || Distance <= Settings.LeashRadius;
		}
		const bool bDrawn = (bAttacked && bReachable) || (Settings.Aggro == EHOLCampAggro::OnSight && bSighted);

		const EHOLCampState Before = Memory.State;
		if (Memory.State == EHOLCampState::Guarding)
		{
			if (bDrawn)
			{
				Memory.State = EHOLCampState::Fighting;
			}
		}
		else if (Memory.State == EHOLCampState::Fighting)
		{
			if (!bReachable)
			{
				// Nobody within the leash radius any more (gone, or dead): everyone goes back to the post.
				Memory.State = EHOLCampState::Returning;
				for (int Index = 0; Index < Members.Num(); Index++)
				{
					Result.ReturnIds.Add(Members[Index].Id);
				}
			}
			else
			{
				// A member that chases past the leash radius gives up and goes back; the others fight on.
				for (int Index = 0; Index < Members.Num(); Index++)
				{
					if (FlatDistance(Members[Index].Location, Post) > Settings.LeashRadius)
					{
						Result.ReturnIds.Add(Members[Index].Id);
					}
				}
			}
		}
		else if (Memory.State == EHOLCampState::Returning)
		{
			bool bAllHome = true;
			for (int Index = 0; Index < Members.Num(); Index++)
			{
				bAllHome = bAllHome && FlatDistance(Members[Index].Location, Post) <= Settings.ArriveRadius;
			}
			if (bDrawn)
			{
				Memory.State = EHOLCampState::Fighting;
			}
			else if (bAllHome)
			{
				Memory.State = EHOLCampState::Guarding;
			}
		}
		Result.bChanged = Memory.State != Before;
		return Result;
	}

	// The XP of a cleared camp by owner (ascending), from the hits it took; the shares add up to RewardXP. Nothing without a hit.
	TArray<FHOLCampShare> Rewards(const FHOLCampSettings& Settings, const TArray<FHOLCampHit>& Hits)
	{
		TArray<FHOLCampShare> Shares;
		if (Hits.Num() == 0)
		{
			return Shares;
		}

		// The damage by owner, ascending by owner.
		TArray<int> Owners;
		TArray<int> Damages;
		for (int Index = 0; Index < Hits.Num(); Index++)
		{
			int Slot = Owners.FindIndex(Hits[Index].Owner);
			if (Slot < 0)
			{
				Slot = 0;
				while (Slot < Owners.Num() && Owners[Slot] < Hits[Index].Owner)
				{
					Slot++;
				}
				Owners.Insert(Hits[Index].Owner, Slot);
				Damages.Insert(0, Slot);
			}
			Damages[Slot] += Hits[Index].Damage;
		}

		// The owner who dealt the most (the lowest on a tie: the owners are ascending and only a strictly greater total replaces).
		int Biggest = 0;
		int Total = 0;
		for (int Index = 0; Index < Owners.Num(); Index++)
		{
			Total += Damages[Index];
			if (Damages[Index] > Damages[Biggest])
			{
				Biggest = Index;
			}
		}

		if (Settings.Credit == EHOLCampCredit::LastBlow || (Settings.Credit == EHOLCampCredit::Shared && Total <= 0))
		{
			FHOLCampShare Whole;
			Whole.Owner = Hits[Hits.Num() - 1].Owner;
			Whole.XP = Settings.RewardXP;
			Shares.Add(Whole);
		}
		else if (Settings.Credit == EHOLCampCredit::MostDamage)
		{
			FHOLCampShare Whole;
			Whole.Owner = Owners[Biggest];
			Whole.XP = Settings.RewardXP;
			Shares.Add(Whole);
		}
		else
		{
			// In proportion to the damage, rounded down; the points lost by rounding go to the biggest, so the shares add up to the reward.
			int Given = 0;
			for (int Index = 0; Index < Owners.Num(); Index++)
			{
				FHOLCampShare Share;
				Share.Owner = Owners[Index];
				Share.XP = Settings.RewardXP * Damages[Index] / Total;
				Given += Share.XP;
				Shares.Add(Share);
			}
			Shares[Biggest].XP += Settings.RewardXP - Given;
		}
		return Shares;
	}
}
