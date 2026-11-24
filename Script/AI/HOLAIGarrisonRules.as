// Garrison in the AI's rules (F7 lot 4c, FortificationsInProgress.md; D696: Normal and Hard garrison their towers, Hard reacts sooner,
// Easy never; D793: who may enter; D374: never an ally's building). When one of the AI's finished towers with free places is under attack,
// which of its units nearby enter, in which order, and when they come out. Pure functions on plain inputs, tested in
// HOLAIGarrisonRules_Test.as. The admission itself (own slot, kind, free places) is HOLGarrison::AdmittedWith, the same rule the server
// applies to the right click (HOLPlayerOrders::Garrison).
// WIRED to the brain (HOLAIBrain.ThinkGarrison: HOLAIGarrisonCall.as for the entry, HOLAIGarrisonExit.as for the exit, via the C++ bindings
// HOLAIGarrisonOrder::IssueAIGarrisonOrder / IssueAIUngarrisonOrder). Still inert in a match until a building type has GarrisonSlots > 0 (see BuildingTypes.csv).
// The design is not decided (Retours/Q-garrison-ia.md, [A REPONDRE]): the defaults below are the recommended ones, one setting each.

// Which units the AI sends in (Q-garrison-ia Q1): 0 = never, 1 = ranged foot only (A, default), 2 = ranged foot first then any admitted
// kind to fill (B), 3 = any admitted kind, nearest first (C).
const FConsoleVariable CVar_AIGarrisonRule("hol.AI.GarrisonRule", 1);
// A unit this close to the tower (cm, on the ground) is called in (15 m, proposed).
const FConsoleVariable CVar_AIGarrisonRadius("hol.AI.GarrisonRadius", 1500);
// A tower hit within these seconds is under attack (proposed).
const FConsoleVariable CVar_AIGarrisonAttackedSeconds("hol.AI.GarrisonAttackedSeconds", 5);
// Seconds between the first hit and the call, by level (D696: Hard reacts sooner; proposed).
const FConsoleVariable CVar_AIGarrisonReactNormal("hol.AI.GarrisonReactNormal", 3);
const FConsoleVariable CVar_AIGarrisonReactHard("hol.AI.GarrisonReactHard", 1);
// The occupants come out after this many seconds without a hit on the tower (Q2 A, default); 0 = they stay until the tower falls (Q2 B).
const FConsoleVariable CVar_AIGarrisonExitCalmSeconds("hol.AI.GarrisonExitCalmSeconds", 15);

// One tower of the AI as the rule sees it. Seconds since the first and the last hit of the current attack; -1 = never hit.
struct FHOLAIGarrisonTower
{
	int Slot = -1;
	FVector Place = FVector::ZeroVector;
	bool bFinished = false;
	int Slots = 0;
	int Occupants = 0;
	float SinceFirstHit = -1.f;
	float SinceLastHit = -1.f;
}

namespace HOLAIGarrison
{
	const int RuleNone = 0;
	const int RuleRangedFoot = 1;
	const int RuleRangedFirst = 2;
	const int RuleAnyNearest = 3;

	int RuleFromCVar()
	{
		return CVar_AIGarrisonRule.GetInt();
	}

	float RadiusFromCVar()
	{
		return float(Math::Max(0, CVar_AIGarrisonRadius.GetInt()));
	}

	float AttackedSecondsFromCVar()
	{
		return float(Math::Max(0, CVar_AIGarrisonAttackedSeconds.GetInt()));
	}

	float ExitCalmSecondsFromCVar()
	{
		return float(Math::Max(0, CVar_AIGarrisonExitCalmSeconds.GetInt()));
	}

	// Does this level garrison at all (D696: Normal and Hard; Easy never)? The caller passes the level played (HOLAILives::Played, so an
	// Evolving AI answers with its current level).
	bool LevelGarrisons(EHOLAIDifficulty Level)
	{
		return Level == EHOLAIDifficulty::Normal || Level == EHOLAIDifficulty::Hard;
	}

	// Seconds between the first hit and the call (D696: Hard sooner than Normal).
	float ReactSeconds(EHOLAIDifficulty Level)
	{
		if (Level == EHOLAIDifficulty::Hard)
			return float(Math::Max(0, CVar_AIGarrisonReactHard.GetInt()));
		return float(Math::Max(0, CVar_AIGarrisonReactNormal.GetInt()));
	}

	// Does the tower call its defenders in: a level that garrisons, a finished building with free places, hit lately (the attack still on)
	// and for long enough for this level to have reacted?
	bool TowerCalls(EHOLAIDifficulty Level, FHOLAIGarrisonTower Tower)
	{
		if (!LevelGarrisons(Level) || !Tower.bFinished || HOLGarrison::FreeSlots(Tower.Slots, Tower.Occupants) <= 0)
			return false;
		if (Tower.SinceLastHit < 0.f || Tower.SinceFirstHit < 0.f)
			return false;
		return Tower.SinceLastHit <= AttackedSecondsFromCVar() && Tower.SinceFirstHit >= ReactSeconds(Level);
	}

	float FlatDistance(FVector A, FVector B)
	{
		return FVector(A.X - B.X, A.Y - B.Y, 0.f).Size();
	}

	// May this kind be called under this rule (before the garrison's own admission, which still refuses siege, D793)?
	bool RuleCalls(int Rule, int Kind)
	{
		if (Rule == RuleRangedFoot)
			return Kind == HOLGarrison::KindRangedInfantry;
		return Rule == RuleRangedFirst || Rule == RuleAnyNearest;
	}

	// The units that enter (indexes into UnitSlots / UnitKinds / UnitPlaces, in the order they are sent): within Radius of the tower, of a
	// kind the rule calls, sorted (ranged foot first under rule 2, then) nearest first, and admitted by the garrison rule with its default
	// settings (own slot only, kind, up to the free places). Nobody when the tower does not call (TowerCalls).
	TArray<int> Entrants(int Rule, EHOLAIDifficulty Level, FHOLAIGarrisonTower Tower, const TArray<int>& UnitSlots,
		const TArray<int>& UnitKinds, const TArray<FVector>& UnitPlaces, float Radius)
	{
		TArray<int> Result;
		if (Rule == RuleNone || !TowerCalls(Level, Tower))
			return Result;
		TArray<int> Order;
		TArray<float> Keys;
		for (int Index = 0; Index < UnitSlots.Num(); Index++)
		{
			if (!UnitKinds.IsValidIndex(Index) || !UnitPlaces.IsValidIndex(Index) || !RuleCalls(Rule, UnitKinds[Index]))
				continue;
			const float Distance = FlatDistance(UnitPlaces[Index], Tower.Place);
			if (Distance > Radius)
				continue;
			// Rule 2: every ranged foot before any other kind (a key beyond any distance in reach).
			const float Key = (Rule == RuleRangedFirst && UnitKinds[Index] != HOLGarrison::KindRangedInfantry) ? Distance + Radius + 1.f : Distance;
			int At = Order.Num();
			while (At > 0 && Keys[At - 1] > Key)
				At--;
			Order.Insert(Index, At);
			Keys.Insert(Key, At);
		}
		TArray<int> Slots;
		TArray<int> Kinds;
		for (int Index = 0; Index < Order.Num(); Index++)
		{
			Slots.Add(UnitSlots[Order[Index]]);
			Kinds.Add(UnitKinds[Order[Index]]);
		}
		const TArray<int> In = HOLGarrison::AdmittedWith(Slots, Kinds, Tower.Slot, Tower.Slots, Tower.Occupants,
			HOLGarrison::KindMayEnterByDefault(HOLGarrison::KindMeleeInfantry), HOLGarrison::KindMayEnterByDefault(HOLGarrison::KindHero),
			HOLGarrison::KindMayEnterByDefault(HOLGarrison::KindCavalry));
		for (int Index = 0; Index < In.Num(); Index++)
			Result.Add(Order[In[Index]]);
		return Result;
	}

	// Do the occupants come out: CalmSeconds without a hit (Q2 A); CalmSeconds 0 = never by themselves, only ejected at the tower's fall
	// (Q2 B). A tower never hit (SinceLastHit -1) counts as calm.
	bool OccupantsLeave(float SinceLastHit, float CalmSeconds)
	{
		if (CalmSeconds <= 0.f)
			return false;
		return SinceLastHit < 0.f || SinceLastHit >= CalmSeconds;
	}
}
