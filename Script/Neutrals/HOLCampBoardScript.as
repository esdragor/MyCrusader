// The server's camp board (bascule Camp, D830/D836 point 3), the script side of UHOLCampBoard: UHOLCampSubsystem (C++, the units and
// the world) gives what a camp sees; the rule is HOLCampRules.as (HOLCamp::Step, HOLCamp::Rewards). The bridge's structures are
// converted here to the rule's own. Server and solo only. Tested by HeroesOfLiberty.Camp.ServerBoard.
class UHOLCampBoardScript : UHOLCampBoard
{
	UFUNCTION(BlueprintOverride)
	FHOLCampBoardStep CampStep(EHOLCampBoardState State, const FHOLCampBoardSettings& Settings, FVector2D Post, const TArray<FHOLCampBoardMember>& Members, const TArray<FHOLCampBoardIntruder>& Intruders, bool bAttacked)
	{
		FHOLCampMemory Memory;
		Memory.State = EHOLCampState(int(State));
		TArray<FHOLCampMember> RuleMembers;
		for (int Index = 0; Index < Members.Num(); Index++)
		{
			FHOLCampMember Member;
			Member.Id = Members[Index].Id;
			Member.Location = FVector(Members[Index].Location.X, Members[Index].Location.Y, 0);
			RuleMembers.Add(Member);
		}
		TArray<FHOLCampIntruder> RuleIntruders;
		for (int Index = 0; Index < Intruders.Num(); Index++)
		{
			FHOLCampIntruder Intruder;
			Intruder.Id = Intruders[Index].Id;
			Intruder.Owner = Intruders[Index].Owner;
			Intruder.Location = FVector(Intruders[Index].Location.X, Intruders[Index].Location.Y, 0);
			RuleIntruders.Add(Intruder);
		}
		const FHOLCampStep Step = HOLCamp::Step(Memory, RuleSettings(Settings), FVector(Post.X, Post.Y, 0), RuleMembers, RuleIntruders, bAttacked);
		FHOLCampBoardStep Answer;
		Answer.State = EHOLCampBoardState(int(Memory.State));
		Answer.bChanged = Step.bChanged;
		Answer.bJustCleared = Step.bJustCleared;
		Answer.ReturnIds = Step.ReturnIds;
		return Answer;
	}

	UFUNCTION(BlueprintOverride)
	TArray<FHOLCampBoardShare> CampRewards(const FHOLCampBoardSettings& Settings, const TArray<FHOLCampBoardHit>& Hits)
	{
		TArray<FHOLCampHit> RuleHits;
		for (int Index = 0; Index < Hits.Num(); Index++)
		{
			FHOLCampHit Hit;
			Hit.Owner = Hits[Index].Owner;
			Hit.Damage = Hits[Index].Damage;
			RuleHits.Add(Hit);
		}
		const TArray<FHOLCampShare> Shares = HOLCamp::Rewards(RuleSettings(Settings), RuleHits);
		TArray<FHOLCampBoardShare> Answer;
		for (int Index = 0; Index < Shares.Num(); Index++)
		{
			FHOLCampBoardShare Share;
			Share.Owner = Shares[Index].Owner;
			Share.XP = Shares[Index].XP;
			Answer.Add(Share);
		}
		return Answer;
	}

	FHOLCampSettings RuleSettings(const FHOLCampBoardSettings& Settings)
	{
		FHOLCampSettings Rule;
		Rule.Aggro = EHOLCampAggro(int(Settings.Aggro));
		Rule.Credit = EHOLCampCredit(int(Settings.Credit));
		Rule.AggroRadius = Settings.AggroRadius;
		Rule.LeashRadius = Settings.LeashRadius;
		Rule.ArriveRadius = Settings.ArriveRadius;
		Rule.RewardXP = Settings.RewardXP;
		return Rule;
	}
}
