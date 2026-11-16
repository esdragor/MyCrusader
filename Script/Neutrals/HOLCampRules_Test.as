// The neutral camps' rules alone (NeutralsInProgress.md N1, GDD 8.4, D32): the state of a camp, its aggro and leash, the clearing
// that never reverses, the reward by credit rule. The distances come from the settings (no number retyped in the expectations).

const FVector CampPost = FVector(10000, 0, 0);

TArray<FHOLCampMember> CampMembers(int Count, FVector Where)
{
	TArray<FHOLCampMember> Members;
	for (int Index = 0; Index < Count; Index++)
	{
		FHOLCampMember Member;
		Member.Id = 500 + Index;
		Member.Location = Where;
		Members.Add(Member);
	}
	return Members;
}

TArray<FHOLCampIntruder> CampIntruderAt(float Distance)
{
	TArray<FHOLCampIntruder> Intruders;
	FHOLCampIntruder Intruder;
	Intruder.Id = 1;
	Intruder.Owner = 0;
	Intruder.Location = CampPost + FVector(Distance, 0, 0);
	Intruders.Add(Intruder);
	return Intruders;
}

// Nothing near: the camp guards. On sight, an intruder at the aggro radius (inclusive) draws it, one beyond does not.
void Test_CampAggroOnSight(FUnitTest& T)
{
	FHOLCampSettings S;
	S.Aggro = EHOLCampAggro::OnSight;
	const TArray<FHOLCampMember> Members = CampMembers(3, CampPost);
	FHOLCampMemory Memory;
	FHOLCampStep Quiet = HOLCamp::Step(Memory, S, CampPost, Members, TArray<FHOLCampIntruder>(), false);
	T.AssertTrue(Memory.State == EHOLCampState::Guarding && !Quiet.bChanged, "nobody near: the camp guards");

	FHOLCampStep Far = HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.AggroRadius + 1.f), false);
	T.AssertTrue(Memory.State == EHOLCampState::Guarding && !Far.bChanged, "an intruder beyond the aggro radius: still guarding");

	FHOLCampStep Near = HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.AggroRadius), false);
	T.AssertTrue(Memory.State == EHOLCampState::Fighting && Near.bChanged, "an intruder at the aggro radius draws the camp");
}

// Only when attacked: the intruder in sight does not draw it; the blow does.
void Test_CampAggroOnAttackOnly(FUnitTest& T)
{
	FHOLCampSettings S;
	S.Aggro = EHOLCampAggro::OnAttack;
	const TArray<FHOLCampMember> Members = CampMembers(3, CampPost);
	FHOLCampMemory Memory;
	HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(100.f), false);
	T.AssertTrue(Memory.State == EHOLCampState::Guarding, "an intruder on top of the post, no blow: the camp does not move");
	FHOLCampStep Hit = HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(100.f), true);
	T.AssertTrue(Memory.State == EHOLCampState::Fighting && Hit.bChanged, "a blow draws it");
}

// The leash: a member that follows beyond the leash radius is sent back; when no intruder stands within the leash radius the camp
// returns (everyone sent back), and it guards again only when all are at the post.
void Test_CampLeashAndReturn(FUnitTest& T)
{
	FHOLCampSettings S;
	S.Aggro = EHOLCampAggro::OnSight;
	FHOLCampMemory Memory;
	TArray<FHOLCampMember> Members = CampMembers(3, CampPost);
	HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.AggroRadius), false);
	T.AssertTrue(Memory.State == EHOLCampState::Fighting, "fighting");

	// One member chases past the leash radius: only it is sent back; the camp keeps fighting.
	Members[1].Location = CampPost + FVector(S.LeashRadius + 1.f, 0, 0);
	FHOLCampStep Chase = HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.AggroRadius), false);
	T.AssertTrue(Memory.State == EHOLCampState::Fighting && Chase.ReturnIds.Num() == 1 && Chase.ReturnIds.Contains(Members[1].Id), "the member past the leash goes back, the camp fights on");

	// The intruder leaves beyond the leash radius: the camp returns, every member is sent back.
	FHOLCampStep Gone = HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.LeashRadius + 1.f), false);
	T.AssertTrue(Memory.State == EHOLCampState::Returning && Gone.bChanged, "the intruder is gone: the camp returns");
	T.AssertEquals(Members.Num(), Gone.ReturnIds.Num(), "every member is sent back to the post");

	// Not guarding while one member is still away from the post.
	HOLCamp::Step(Memory, S, CampPost, Members, TArray<FHOLCampIntruder>(), false);
	T.AssertTrue(Memory.State == EHOLCampState::Returning, "one member is still out: still returning");
	Members[1].Location = CampPost + FVector(S.ArriveRadius + 1.f, 0, 0);
	HOLCamp::Step(Memory, S, CampPost, Members, TArray<FHOLCampIntruder>(), false);
	T.AssertTrue(Memory.State == EHOLCampState::Returning, "just outside the arrival radius: still returning");
	Members[1].Location = CampPost + FVector(S.ArriveRadius, 0, 0);
	FHOLCampStep Home = HOLCamp::Step(Memory, S, CampPost, Members, TArray<FHOLCampIntruder>(), false);
	T.AssertTrue(Memory.State == EHOLCampState::Guarding && Home.bChanged, "all within the arrival radius: guarding again");

	// A new intruder while returning draws it again.
	HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.LeashRadius + 1.f), false);
	T.AssertTrue(Memory.State == EHOLCampState::Guarding, "(an intruder far away does nothing)");
	Members[0].Location = CampPost + FVector(S.ArriveRadius + 100.f, 0, 0);
	FHOLCampMemory Back;
	Back.State = EHOLCampState::Returning;
	HOLCamp::Step(Back, S, CampPost, Members, CampIntruderAt(S.AggroRadius), false);
	T.AssertTrue(Back.State == EHOLCampState::Fighting, "an intruder in aggro while returning: fighting again");
}

// A blow from beyond the leash radius (an archer far off) draws no camp and does not hold a fight: the camp does not chase, nor fall back
// and come out again, for what it cannot reach (both aggro modes).
void Test_CampIgnoresWhatItCannotReach(FUnitTest& T)
{
	for (int Mode = 0; Mode < 2; Mode++)
	{
		FHOLCampSettings S;
		S.Aggro = Mode == 0 ? EHOLCampAggro::OnSight : EHOLCampAggro::OnAttack;
		const TArray<FHOLCampMember> Members = CampMembers(3, CampPost);
		FHOLCampMemory Memory;
		HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.LeashRadius + 1.f), true);
		T.AssertTrue(Memory.State == EHOLCampState::Guarding, f"mode {Mode}: a blow from beyond the leash does not draw the camp");

		Memory.State = EHOLCampState::Fighting;
		FHOLCampStep Gone = HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.LeashRadius + 1.f), true);
		T.AssertTrue(Memory.State == EHOLCampState::Returning && Gone.ReturnIds.Num() == Members.Num(), f"mode {Mode}: and a fight against what stands out of reach ends in the return");
	}
}

// The camp is cleared when its last member dies, once; it never comes back (D32: no respawn), whatever is listed afterwards.
void Test_CampClearedForGood(FUnitTest& T)
{
	FHOLCampSettings S;
	FHOLCampMemory Memory;
	HOLCamp::Step(Memory, S, CampPost, CampMembers(2, CampPost), CampIntruderAt(S.AggroRadius), false);
	T.AssertTrue(Memory.State == EHOLCampState::Fighting, "fighting");

	FHOLCampStep Last = HOLCamp::Step(Memory, S, CampPost, TArray<FHOLCampMember>(), CampIntruderAt(S.AggroRadius), false);
	T.AssertTrue(Memory.State == EHOLCampState::Cleared && Last.bChanged && Last.bJustCleared, "no member left: cleared, the reward is due");

	FHOLCampStep Again = HOLCamp::Step(Memory, S, CampPost, TArray<FHOLCampMember>(), CampIntruderAt(S.AggroRadius), false);
	T.AssertTrue(Memory.State == EHOLCampState::Cleared && !Again.bChanged && !Again.bJustCleared, "the reward is due once");

	FHOLCampStep Respawn = HOLCamp::Step(Memory, S, CampPost, CampMembers(2, CampPost), CampIntruderAt(S.AggroRadius), true);
	T.AssertTrue(Memory.State == EHOLCampState::Cleared && !Respawn.bChanged && Respawn.ReturnIds.Num() == 0, "no respawn: members listed again change nothing");

	// A camp wiped out without a fight (a spell from afar): cleared too.
	FHOLCampMemory Guarding;
	FHOLCampStep Spell = HOLCamp::Step(Guarding, S, CampPost, TArray<FHOLCampMember>(), TArray<FHOLCampIntruder>(), false);
	T.AssertTrue(Guarding.State == EHOLCampState::Cleared && Spell.bJustCleared, "wiped out while guarding: cleared");
}

TArray<FHOLCampHit> CampHits()
{
	TArray<FHOLCampHit> Hits;
	FHOLCampHit A;
	A.Owner = 1;
	A.Damage = 60;
	Hits.Add(A);
	FHOLCampHit B;
	B.Owner = 0;
	B.Damage = 30;
	Hits.Add(B);
	FHOLCampHit C;
	C.Owner = 1;
	C.Damage = 10;
	Hits.Add(C);
	FHOLCampHit D;
	D.Owner = 2;
	D.Damage = 20;
	Hits.Add(D);
	return Hits;
}

int ShareOf(const TArray<FHOLCampShare>& Shares, int Owner)
{
	for (int Index = 0; Index < Shares.Num(); Index++)
	{
		if (Shares[Index].Owner == Owner)
		{
			return Shares[Index].XP;
		}
	}
	return 0;
}

// The XP goes to the credit rule's owner(s): owner 1 dealt 70 of 120 damage, owner 0 dealt 30, owner 2 dealt 20 and struck last.
void Test_CampRewardByCredit(FUnitTest& T)
{
	FHOLCampSettings S;
	S.RewardXP = 100;

	S.Credit = EHOLCampCredit::LastBlow;
	TArray<FHOLCampShare> Last = HOLCamp::Rewards(S, CampHits());
	T.AssertEquals(1, Last.Num(), "last blow: one share");
	T.AssertEquals(S.RewardXP, ShareOf(Last, 2), "last blow: everything to the owner who struck last");

	S.Credit = EHOLCampCredit::MostDamage;
	TArray<FHOLCampShare> Most = HOLCamp::Rewards(S, CampHits());
	T.AssertEquals(1, Most.Num(), "most damage: one share");
	T.AssertEquals(S.RewardXP, ShareOf(Most, 1), "most damage: everything to the owner who dealt the most (two hits added up)");

	S.Credit = EHOLCampCredit::Shared;
	TArray<FHOLCampShare> Shared = HOLCamp::Rewards(S, CampHits());
	T.AssertEquals(3, Shared.Num(), "shared: one share per owner who hit");
	T.AssertEquals(59, ShareOf(Shared, 1), "owner 1: 70 of 120 damage (58), plus the point lost by rounding down (the biggest takes it)");
	T.AssertEquals(25, ShareOf(Shared, 0), "owner 0: 30 of 120");
	T.AssertEquals(16, ShareOf(Shared, 2), "owner 2: 20 of 120");
	int Total = 0;
	for (int Index = 0; Index < Shared.Num(); Index++)
	{
		Total += Shared[Index].XP;
	}
	T.AssertEquals(S.RewardXP, Total, "the shares add up to the reward, to the last point");
	T.AssertTrue(Shared[0].Owner < Shared[1].Owner && Shared[1].Owner < Shared[2].Owner, "ascending by owner");
}

// A tie on the damage goes to the lowest owner; no hit, no reward.
void Test_CampRewardTiesAndNoHit(FUnitTest& T)
{
	FHOLCampSettings S;
	S.Credit = EHOLCampCredit::MostDamage;
	TArray<FHOLCampHit> Tie;
	FHOLCampHit X;
	X.Owner = 3;
	X.Damage = 50;
	Tie.Add(X);
	FHOLCampHit Y;
	Y.Owner = 1;
	Y.Damage = 50;
	Tie.Add(Y);
	TArray<FHOLCampShare> Most = HOLCamp::Rewards(S, Tie);
	T.AssertTrue(Most.Num() == 1 && Most[0].Owner == 1, "most damage, tie: the lowest owner");

	S.Credit = EHOLCampCredit::Shared;
	TArray<FHOLCampShare> Shared = HOLCamp::Rewards(S, Tie);
	T.AssertEquals(2, Shared.Num(), "shared, tie: both");
	T.AssertEquals(S.RewardXP, Shared[0].XP + Shared[1].XP, "and the sum is the reward even when it does not divide");
	T.AssertTrue(Shared[0].XP >= Shared[1].XP, "the remainder goes to the lowest owner of the biggest");

	T.AssertEquals(0, HOLCamp::Rewards(S, TArray<FHOLCampHit>()).Num(), "no hit: no reward (shared)");
	S.Credit = EHOLCampCredit::LastBlow;
	T.AssertEquals(0, HOLCamp::Rewards(S, TArray<FHOLCampHit>()).Num(), "no hit: no reward (last blow)");
	S.Credit = EHOLCampCredit::MostDamage;
	T.AssertEquals(0, HOLCamp::Rewards(S, TArray<FHOLCampHit>()).Num(), "no hit: no reward (most damage)");
}

// The settings hold together: the post is drawn before the leash lets go, and arriving is closer than either.
void Test_CampSettingsAreCoherent(FUnitTest& T)
{
	FHOLCampSettings S;
	T.AssertTrue(S.ArriveRadius < S.AggroRadius, "arrival is closer than the aggro radius");
	T.AssertTrue(S.AggroRadius < S.LeashRadius, "the leash is longer than the aggro radius (the camp does not drop a fight it has just begun)");
	T.AssertTrue(S.RewardXP > 0, "a cleared camp pays");
}

// MIG-J-Camp (D830/D835, 2026-11-16): the vectors of the C++ twin's test HeroesOfLiberty.Neutrals.CampRules
// (Source/HeroesOfLiberty/Tests/HOLCampRulesTests.cpp), case by case, so that both copies answer the same cases while two exist.
FHOLCampHit CampVectorHit(int Owner, int Damage)
{
	FHOLCampHit Hit;
	Hit.Owner = Owner;
	Hit.Damage = Damage;
	return Hit;
}

void Test_CampRulesVectors(FUnitTest& T)
{
	FHOLCampSettings S;
	S.Aggro = EHOLCampAggro::OnSight;
	const TArray<FHOLCampMember> Members = CampMembers(3, CampPost);

	// On sight: an intruder at the aggro radius (inclusive) draws the camp, one beyond does not.
	FHOLCampMemory Memory;
	FHOLCampStep Step = HOLCamp::Step(Memory, S, CampPost, Members, TArray<FHOLCampIntruder>(), false);
	T.AssertTrue(Memory.State == EHOLCampState::Guarding && !Step.bChanged, "nobody near: guarding");
	HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.AggroRadius + 1.f), false);
	T.AssertTrue(Memory.State == EHOLCampState::Guarding, "beyond the aggro radius: still guarding");
	Step = HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.AggroRadius), false);
	T.AssertTrue(Memory.State == EHOLCampState::Fighting && Step.bChanged, "at the aggro radius: fighting");

	// The leash: nobody within it, everyone goes back; then at the post, guarding again.
	Step = HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.LeashRadius + 1.f), false);
	T.AssertTrue(Memory.State == EHOLCampState::Returning, "nobody within the leash: returning");
	T.AssertEquals(3, Step.ReturnIds.Num(), "all sent back");
	HOLCamp::Step(Memory, S, CampPost, CampMembers(3, CampPost + FVector(S.ArriveRadius + 1.f, 0, 0)), TArray<FHOLCampIntruder>(), false);
	T.AssertTrue(Memory.State == EHOLCampState::Returning, "not yet at the post: still returning");
	HOLCamp::Step(Memory, S, CampPost, CampMembers(3, CampPost + FVector(S.ArriveRadius, 0, 0)), TArray<FHOLCampIntruder>(), false);
	T.AssertTrue(Memory.State == EHOLCampState::Guarding, "at the post: guarding");

	// A member chasing past the leash is sent back alone.
	Memory.State = EHOLCampState::Fighting;
	TArray<FHOLCampMember> Chasers = CampMembers(2, CampPost);
	Chasers[1].Location = CampPost + FVector(S.LeashRadius + 1.f, 0, 0);
	Step = HOLCamp::Step(Memory, S, CampPost, Chasers, CampIntruderAt(100.f), false);
	T.AssertTrue(Memory.State == EHOLCampState::Fighting && Step.ReturnIds.Num() == 1, "one member past the leash is sent back");
	T.AssertEquals(Chasers[1].Id, Step.ReturnIds.Num() == 1 ? Step.ReturnIds[0] : -1, "the one past the leash");

	// Only when attacked: the sight does not draw it, a blow from one it can reach does, a blow from beyond the leash does not.
	S.Aggro = EHOLCampAggro::OnAttack;
	Memory.State = EHOLCampState::Guarding;
	HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(100.f), false);
	T.AssertTrue(Memory.State == EHOLCampState::Guarding, "on attack: an intruder on the post, no blow: guarding");
	HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(S.LeashRadius + 1.f), true);
	T.AssertTrue(Memory.State == EHOLCampState::Guarding, "a blow from beyond the leash is not answered");
	HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(100.f), true);
	T.AssertTrue(Memory.State == EHOLCampState::Fighting, "a blow draws it");

	// Cleared for good: no member left = cleared once; members listed later do not bring it back.
	Memory.State = EHOLCampState::Fighting;
	Step = HOLCamp::Step(Memory, S, CampPost, TArray<FHOLCampMember>(), TArray<FHOLCampIntruder>(), false);
	T.AssertTrue(Memory.State == EHOLCampState::Cleared && Step.bJustCleared, "no member left: cleared, once");
	Step = HOLCamp::Step(Memory, S, CampPost, Members, CampIntruderAt(10.f), true);
	T.AssertTrue(Memory.State == EHOLCampState::Cleared && !Step.bChanged && !Step.bJustCleared, "cleared for good (D32)");

	// The reward by credit rule, on the C++ hits (owner 1: 60 + 10, owner 0: 30 + 10 and the last blow): the shares add up.
	TArray<FHOLCampHit> Hits;
	Hits.Add(CampVectorHit(1, 60));
	Hits.Add(CampVectorHit(0, 30));
	Hits.Add(CampVectorHit(1, 10));
	Hits.Add(CampVectorHit(0, 10));
	S.RewardXP = 100;
	S.Credit = EHOLCampCredit::LastBlow;
	TArray<FHOLCampShare> Shares = HOLCamp::Rewards(S, Hits);
	T.AssertTrue(Shares.Num() == 1 && Shares[0].Owner == 0 && Shares[0].XP == 100, "last blow: all to the owner of the last hit");
	S.Credit = EHOLCampCredit::MostDamage;
	Shares = HOLCamp::Rewards(S, Hits);
	T.AssertTrue(Shares.Num() == 1 && Shares[0].Owner == 1 && Shares[0].XP == 100, "most damage: all to owner 1 (70 against 40)");
	S.Credit = EHOLCampCredit::Shared;
	Shares = HOLCamp::Rewards(S, Hits);
	T.AssertEquals(2, Shares.Num(), "shared: two owners");
	T.AssertEquals(0, Shares.Num() == 2 ? Shares[0].Owner : -1, "shared: owners ascending");
	T.AssertEquals(36, Shares.Num() == 2 ? Shares[0].XP : -1, "shared: owner 0, 40 of 110");
	T.AssertEquals(64, Shares.Num() == 2 ? Shares[1].XP : -1, "shared: owner 1, 70 of 110 plus the rounding");
	T.AssertEquals(0, HOLCamp::Rewards(S, TArray<FHOLCampHit>()).Num(), "no hit, no share");
}

// MIG-J-Camp: the script side of HeroesOfLiberty.Neutrals.Owner. The neutral owner is the first number past the slots (D401), hostile
// to every slot: an intruder of any slot draws the camp and the blow of any slot earns its owner the reward. The constants of
// FHOLMatchConfig, HOLVictoryRules::AreHostile and the HOLCamps statics of the combat processor have no script copy: they stay in C++.
void Test_CampOwner(FUnitTest& T)
{
	FHOLCampSettings S;
	S.Credit = EHOLCampCredit::LastBlow;
	const TArray<FHOLCampMember> Members = CampMembers(2, CampPost);
	int Drawn = 0;
	int Paid = 0;
	for (int Slot = 0; Slot < HOLAIArmy::NeutralOwnerSlot; Slot++)
	{
		TArray<FHOLCampIntruder> Intruders = CampIntruderAt(S.AggroRadius);
		Intruders[0].Owner = Slot;
		FHOLCampMemory Memory;
		HOLCamp::Step(Memory, S, CampPost, Members, Intruders, false);
		Drawn += Memory.State == EHOLCampState::Fighting ? 1 : 0;

		TArray<FHOLCampHit> Hits;
		FHOLCampHit Hit;
		Hit.Owner = Slot;
		Hit.Damage = 5;
		Hits.Add(Hit);
		TArray<FHOLCampShare> Shares = HOLCamp::Rewards(S, Hits);
		Paid += (Shares.Num() == 1 && Shares[0].Owner == Slot && Shares[0].XP == S.RewardXP) ? 1 : 0;
	}
	T.AssertEquals(8, HOLAIArmy::NeutralOwnerSlot, "the neutral owner is the first number past the 8 slots (FHOLMatchConfig::NeutralOwner = MaxSlots)");
	T.AssertEquals(HOLAIArmy::NeutralOwnerSlot, Drawn, "an intruder of every slot draws the camp");
	T.AssertEquals(HOLAIArmy::NeutralOwnerSlot, Paid, "the blow of every slot earns its owner the reward");
}
