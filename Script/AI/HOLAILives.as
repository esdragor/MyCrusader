// The weighing of the lives by level (IA4, D579, D329): what the AI does when its forum is attacked and it has a life to give. Easy (in
// HOLAIForum.as) gives the army whatever the ratio. NORMAL feints (falls back, goes round) when it knows the fight is lost, and keeps its
// economy (the Peasants run to a refuge, not to the attacked centre). HARD evaluates the loss of the fight, may let the life go to keep
// its army, snipes the weak ones meanwhile, and punishes the attacker's home. EVOLVING passes from Easy to Normal to Hard with time.
// Pure functions over the summary, tested alone on the REAL type tables (HOLAILives_Test.as); every number is a setting, tuned in test.
// The last life stays the common rule for every level (everything on the ruin): none of this applies on it.

// What Normal does with its economy when it knows it loses (IA4-Q1 in Auto_Journey_Retours.md, a design choice left to the user).
enum EHOLAILossPlan
{
	// The Peasants near the enemy run to the centre as ever (the attacked place).
	StayAndFlee,
	// They run to a refuge away from the attackers: the economy is kept for later ("refait l'economie plus loin"). No second centre exists
	// in the building table (D384), so there is no building at the refuge: the Peasants wait there and the brain sends them to work again.
	Withdraw,
}

struct FHOLAILivesData
{
	// ----- Normal (feint) -----
	// Below this power ratio over the attackers, with a life to spare, Normal knows it loses and feints instead of charging.
	float NormalLoseRatio = 0.3f;
	// The feint: falls back to the refuge for this long (s), then goes round to the flank of the attackers for this long (s), then strikes.
	float FeintBackSeconds = 6.f;
	float FeintAroundSeconds = 10.f;
	// Its orders are sent again at most this often (s).
	float FeintEvery = 3.f;
	// The refuge: this far from the centre, away from the attackers (cm); the flank point: this far from the attackers, to their side (cm).
	float RefugeDistance = 4000.f;
	float FlankDistance = 2500.f;
	EHOLAILossPlan LossPlan = EHOLAILossPlan::Withdraw;

	// ----- Hard (evaluation of the loss, snipe, punish) -----
	// Letting the life go keeps the army (IA4-Q2, a design choice left to the user): false = Hard defends like Normal's rules, and only snipes.
	bool bHardLetsLifeGo = true;
	// A fight costing more than this share of the army (expected loss) is not taken when the army kept is enough to crush the attacker's home.
	float HardMaxLoss = 0.5f;
	// "Enough army": its power over the enemy soldiers that are NOT at our base (the attacker's home) is at least this.
	float HardCrushRatio = 1.5f;
	// Sniping: a soldier goes to kill an enemy soldier within SnipeRange when the duel (at the remaining health of both) is won with
	// SnipeMargin and no other enemy soldier it would not beat stands within StrikeIsolation of it; ordered again at most every SnipeEvery (s).
	float SnipeMargin = 1.5f;
	float SnipeRange = 3000.f;
	float SnipeEvery = 4.f;

	// ----- Evolving (D329) -----
	// Easy until the first time (s), Normal until the second, Hard afterwards.
	float EvolveNormalAt = 600.f;
	float EvolveHardAt = 1500.f;
}

enum EHOLAIFeintPhase
{
	None,
	Back,
	Around,
	Strike,
}

// What the feint remembers between two thoughts of a slot (the brain keeps one per slot).
struct FHOLAIFeintMemory
{
	EHOLAIFeintPhase Phase = EHOLAIFeintPhase::None;
	float Since = 0.f;
	float LastOrder = -1000.f;
}

// What the feint does now: the soldiers to move (not attack-move) to Where, and the phase change that happened.
struct FHOLAIFeintStep
{
	TArray<int> MoveIds;
	FVector Where;
	bool bPhaseChanged = false;
	// The feint is over: the army meets the attackers (the rules defend it).
	bool bStrike = false;
}

// A soldier sent to kill a weak enemy soldier.
struct FHOLAISnipe
{
	int SoldierId = -1;
	int TargetId = -1;
}

struct FHOLAISnipeMemory
{
	TMap<int, float> Last;
}

namespace HOLAILives
{
	// The level that plays now: Evolving is Easy, then Normal, then Hard with the game time; the other levels are themselves.
	EHOLAIDifficulty Effective(EHOLAIDifficulty Level, float Now, const FHOLAILivesData& Data)
	{
		if (Level != EHOLAIDifficulty::Evolving)
		{
			return Level;
		}
		if (Now >= Data.EvolveHardAt)
		{
			return EHOLAIDifficulty::Hard;
		}
		if (Now >= Data.EvolveNormalAt)
		{
			return EHOLAIDifficulty::Normal;
		}
		return EHOLAIDifficulty::Easy;
	}

	// The share of the army a straight fight costs at a power ratio (ours over theirs): Lanchester's square law, the survivors being
	// sqrt(1 - 1/R^2) of the army for R >= 1; at an even ratio or below the army is lost.
	float ExpectedLoss(float Ratio)
	{
		if (Ratio <= 1.f)
		{
			return 1.f;
		}
		return 1.f - Math::Sqrt(1.f - 1.f / (Ratio * Ratio));
	}

	// The power ratio of the army over the enemy soldiers that are not at the base (farther than DefenceRadius from the centre): the
	// attacker's home guard. The large ratio when none is seen.
	float HomeRatio(const FHOLAISummary& Summary, const FHOLAIArmyData& Data)
	{
		FHOLAISummary Away;
		Away.Army = Summary.Army;
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			if (HOLAIArmy::IsThreatening(Summary.Enemies[Index]) && HOLAIArmy::FlatDistance(Summary.Enemies[Index].Location, Summary.CentreLocation) > Data.DefenceRadius)
			{
				Away.Enemies.Add(Summary.Enemies[Index]);
			}
		}
		return HOLAIArmy::PowerRatio(Data, Away);
	}

	// Weighs a "forum attacked" situation for a level (D579): sets the decision, and the loss and the army kept (Hard). The last life is the
	// common rule (everything on the ruin): the rules decide. Easy is decided in HOLAIForum::Judge.
	void Weigh(FHOLAIForumSituation& Situation, EHOLAIDifficulty Level, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIThreat& Threat)
	{
		Situation.Call = EHOLAILifeCall::Rules;
		Situation.Loss = ExpectedLoss(Threat.Ratio);
		if (Situation.Lives <= 1)
		{
			return;
		}
		if (Level == EHOLAIDifficulty::Normal && Threat.Ratio < Data.Lives.NormalLoseRatio)
		{
			Situation.Call = EHOLAILifeCall::Feint;
		}
		if (Level == EHOLAIDifficulty::Hard)
		{
			Situation.HomeRatio = HomeRatio(Summary, Data);
			if (Data.Lives.bHardLetsLifeGo && Summary.bHasEnemyCentre && Situation.Loss > Data.Lives.HardMaxLoss && Situation.HomeRatio >= Data.Lives.HardCrushRatio)
			{
				Situation.Call = EHOLAILifeCall::Let;
			}
		}
	}

	// The army is held back from the attackers (the ratio rules do not send it): the feint, or the life let go.
	bool HoldsBack(const FHOLAIForumSituation& Situation)
	{
		return Situation.bAttacked && (Situation.Call == EHOLAILifeCall::Feint || Situation.Call == EHOLAILifeCall::Let);
	}

	// The army is still held back this thought: held by the decision, and the feint has not reached its strike.
	bool ArmyHeld(const FHOLAIForumSituation& Situation, const FHOLAIFeintStep& Feint)
	{
		return HoldsBack(Situation) && !Feint.bStrike;
	}

	// The soldiers to send back to the assembly point (RegroupOrders), except while the army is held back: the feint's retreat and the
	// soldiers kept out of the fight must not be sent back to the assembly point, which stands by the attackers (found in a real game: the
	// regroup order undid the feint's first order in the same thought). Nothing is marked as sent when held.
	TArray<int> RegroupUnlessHeld(FHOLAIDefence& State, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, FVector AssemblyPoint, bool bHeld, float Now)
	{
		TArray<int> Held;
		if (bHeld)
		{
			return Held;
		}
		return HOLAIArmy::RegroupOrders(State, Summary, Data, AssemblyPoint, Now);
	}

	// The refuge: from the centre, away from the attackers, RefugeDistance far (the origin side when they stand on the centre).
	FVector RefugePoint(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIThreat& Threat)
	{
		FVector Away = Summary.CentreLocation - Threat.Location;
		Away.Z = 0;
		if (Away.Size() < 1.f)
		{
			Away = FVector(1, 0, 0);
		}
		return Summary.CentreLocation + Away / Away.Size() * Data.Lives.RefugeDistance;
	}

	// The flank point: FlankDistance from the attackers, at a right angle to the line from the centre to them.
	FVector FlankPoint(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIThreat& Threat)
	{
		FVector Line = Threat.Location - Summary.CentreLocation;
		Line.Z = 0;
		if (Line.Size() < 1.f)
		{
			Line = FVector(1, 0, 0);
		}
		const FVector Side = FVector(-Line.Y, Line.X, 0) / Line.Size();
		return Threat.Location + Side * Data.Lives.FlankDistance;
	}

	// Where the Peasants that flee go: the refuge when Normal knows it loses and the plan is to withdraw, else the centre.
	FVector FleeTarget(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIForumSituation& Situation, const FHOLAIThreat& Threat)
	{
		if (Situation.bAttacked && Situation.Call == EHOLAILifeCall::Feint && Data.Lives.LossPlan == EHOLAILossPlan::Withdraw)
		{
			return RefugePoint(Summary, Data, Threat);
		}
		return Summary.CentreLocation;
	}

	// One thought of the feint (Normal): back to the refuge, then round to the flank, then the strike (the army meets the attackers by the
	// rules). The memory is cleared as soon as the decision is no longer a feint. Soldiers move (not attack-move) until the strike.
	FHOLAIFeintStep FeintStep(FHOLAIFeintMemory& Memory, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIThreat& Threat, const FHOLAIForumSituation& Situation, float Now)
	{
		FHOLAIFeintStep Step;
		if (!Situation.bAttacked || Situation.Call != EHOLAILifeCall::Feint)
		{
			Memory.Phase = EHOLAIFeintPhase::None;
			return Step;
		}
		bool bOrder = false;
		if (Memory.Phase == EHOLAIFeintPhase::None)
		{
			Memory.Phase = EHOLAIFeintPhase::Back;
			Memory.Since = Now;
			Step.bPhaseChanged = true;
			bOrder = true;
		}
		else if (Memory.Phase == EHOLAIFeintPhase::Back && Now - Memory.Since >= Data.Lives.FeintBackSeconds)
		{
			Memory.Phase = EHOLAIFeintPhase::Around;
			Memory.Since = Now;
			Step.bPhaseChanged = true;
			bOrder = true;
		}
		else if (Memory.Phase == EHOLAIFeintPhase::Around && Now - Memory.Since >= Data.Lives.FeintAroundSeconds)
		{
			Memory.Phase = EHOLAIFeintPhase::Strike;
			Memory.Since = Now;
			Step.bPhaseChanged = true;
		}
		Step.bStrike = Memory.Phase == EHOLAIFeintPhase::Strike;
		if (Step.bStrike)
		{
			return Step;
		}
		Step.Where = Memory.Phase == EHOLAIFeintPhase::Back ? RefugePoint(Summary, Data, Threat) : FlankPoint(Summary, Data, Threat);
		if (!bOrder && Now - Memory.LastOrder < Data.Lives.FeintEvery)
		{
			return Step;
		}
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			const FHOLAIUnit Soldier = Summary.Army[Index];
			if (bOrder || (Soldier.bIdle && Soldier.TargetId < 0 && HOLAIArmy::FlatDistance(Soldier.Location, Step.Where) > Data.AssemblyRadius))
			{
				Step.MoveIds.Add(Soldier.Id);
			}
		}
		if (Step.MoveIds.Num() > 0)
		{
			Memory.LastOrder = Now;
		}
		return Step;
	}

	// How a soldier fares one on one against an enemy soldier, at the health each has NOW: the matchup of the figures (the same one the
	// power ratio is built on, D553), 0 for a type the data does not know.
	float DuelOf(const FHOLAIArmyData& Data, const FHOLAIUnit& Mine, const FHOLAIUnit& Enemy)
	{
		const int Own = HOLAIArmy::TypeIndex(Data, Mine.Type);
		const int At = HOLAIArmy::TypeIndex(Data, Enemy.Type);
		if (Own < 0 || At < 0 || Mine.Health <= 0 || Enemy.Health <= 0)
		{
			return 0.f;
		}
		FHOLAITypeFigures Ours = Data.Figures[Own];
		Ours.Health = Math::Min(Mine.Health, Ours.Health);
		FHOLAITypeFigures Theirs = Data.Figures[At];
		Theirs.Health = Math::Min(Enemy.Health, Theirs.Health);
		return HOLAIArmy::Matchup(Data, Ours, Theirs);
	}

	// The enemy soldier a soldier may snipe: the nearest within SnipeRange whose duel is won with SnipeMargin and who stands alone (no other
	// enemy soldier that this soldier would not beat within StrikeIsolation of him); -1 for none.
	int SnipeTargetFor(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIUnit& Soldier)
	{
		int Best = -1;
		float BestDistance = 0.f;
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			const FHOLAIUnit Enemy = Summary.Enemies[Index];
			if (!HOLAIArmy::IsThreatening(Enemy))
			{
				continue;
			}
			const float Distance = HOLAIArmy::FlatDistance(Enemy.Location, Soldier.Location);
			if (Distance > Data.Lives.SnipeRange || DuelOf(Data, Soldier, Enemy) < Data.Lives.SnipeMargin)
			{
				continue;
			}
			bool bAlone = true;
			for (int Each = 0; Each < Summary.Enemies.Num() && bAlone; Each++)
			{
				const FHOLAIUnit Other = Summary.Enemies[Each];
				bAlone = Other.Id == Enemy.Id || !HOLAIArmy::IsThreatening(Other) || HOLAIArmy::FlatDistance(Other.Location, Enemy.Location) > Data.StrikeIsolation
					|| DuelOf(Data, Soldier, Other) >= Data.Lives.SnipeMargin;
			}
			if (bAlone && (Best < 0 || Distance < BestDistance))
			{
				Best = Enemy.Id;
				BestDistance = Distance;
			}
		}
		return Best;
	}

	// The soldiers to send sniping now: only while the life is let go (Hard keeps its army out of the fight and picks the weak ones off to
	// slow the attackers down); idle soldiers, not ordered in the last SnipeEvery seconds.
	TArray<FHOLAISnipe> SnipeOrders(FHOLAISnipeMemory& Memory, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIForumSituation& Situation, float Now)
	{
		TArray<FHOLAISnipe> Snipes;
		if (!Situation.bAttacked || Situation.Call != EHOLAILifeCall::Let)
		{
			return Snipes;
		}
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			const FHOLAIUnit Soldier = Summary.Army[Index];
			float Last = -1000.f;
			Memory.Last.Find(Soldier.Id, Last);
			if (!Soldier.bIdle || Soldier.TargetId >= 0 || Now - Last < Data.Lives.SnipeEvery)
			{
				continue;
			}
			const int Target = SnipeTargetFor(Summary, Data, Soldier);
			if (Target >= 0)
			{
				FHOLAISnipe Snipe;
				Snipe.SoldierId = Soldier.Id;
				Snipe.TargetId = Target;
				Snipes.Add(Snipe);
				Memory.Last.Add(Soldier.Id, Now);
			}
		}
		return Snipes;
	}
}
