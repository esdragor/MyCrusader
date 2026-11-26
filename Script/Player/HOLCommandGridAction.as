// GLORFINDEL-1125ma (ENONCES-1125m section 1, G-1 / G-2): WHAT a click on a command grid cell does, decided in script (D839). The C++ keeps the
// execution (Request*, BeginPlacement) and the hook that asks this rule (G-4, BP_DecideGridAction, ELROND-1125ma); no twin of the switch of
// HOLPlayerController::ExecuteGridCommand stays once G-5 removes it. The input is the cell's command (HOLCommandGridRules::GetCommand / GetOrder)
// and the selection facts the command does not carry (the Monk to cast with). The output is one verdict: send a request, arm a "then click the
// ground" mode, place a building, refuse, or nothing.
// NOT COMPILED when written. Stub of the red: Decide answers None for everything.

enum EHOLGridActionKind
{
	None,
	Send,
	Arm,
	Place,
	Refuse,
}

// What the selection adds to the cell's command (filled by the hook from the selected units; the cell's own facts stay in FHOLGridCommandView).
struct FHOLGridActionContext
{
	bool bMonkUnit = false;
	// The id of the first selected Monk (the caster of the Seal of Light), -1 when none.
	int MonkUnitId = -1;
	// D923: a Virtuous Knight is selected and the first of them (the caster of the Challenge), -1 when none.
	bool bKnightUnit = false;
	int KnightUnitId = -1;
}

struct FHOLGridActionView
{
	EHOLGridActionKind Kind = EHOLGridActionKind::None;
	// For Arm: the mode armed (HOLCommandGridAction::GetAimAttackMove / GetAimBanner / GetAimSeal), -1 when none.
	int AimMode = -1;
	// For Arm of a ground ability: the farthest the caster throws it, in cm (read from HOLUnitAbilityTable).
	int RangeCm = 0;
	// For Arm of the Seal: the casting unit.
	int UnitId = -1;
	// For Send of a unit ability without aim (D923, the Knight's challenge): its id in HOLUnitAbilityTable, -1 none; the caster is UnitId.
	int AbilityId = -1;
	// The short name of the cell's action for hol.HUD.Report / the journal.
	FName Report;
	// For Refuse: why (HOLCommandGridKeys::CheckBuild / CheckProduction), for hol.HUD.Report through HOLCommandGridKeys::Describe.
	FHOLFeedbackView Feedback;
}

// What the cost checks need and neither the cell's command nor the selection carries (filled by the hook from the player state and the producing building).
// A struct of its own: FHOLGridActionContext is aligned with the bridge by ELROND-1125mb.
struct FHOLGridCostFacts
{
	FHOLResources Stock;
	FHOLResources Cost;
	int QueueLength = 0;
	int MaxQueue = 1;
	int PopulationUsed = 0;
	int PopulationCap = 0;
	int Places = 1;
}

namespace HOLCommandGridAction
{
	// The armed modes, 100 and up (0.. are the faction powers', ELROND-1125ma G-4 ConsumeArmedClick) (a constant of a file is not seen by another file, hence the accessors); -1 is no mode.
	int GetAimNone() { return -1; }
	int GetAimAttackMove() { return 100; }
	int GetAimBanner() { return 101; }
	int GetAimSeal() { return 102; }

	// The farthest whole cm the server lets the hero plant his Banner (HOLHeroAbilityRead::CanPlantBanner is the ONE source, no copied 1600; found by halving, as HOLAIPowerCall does).
	int GetBannerReachCm()
	{
		int Low = 0;
		int High = 100000;
		while (Low < High)
		{
			const int Mid = (Low + High + 1) / 2;
			if (HOLHeroAbilityRead::CanPlantBanner(float(Mid)))
				Low = Mid;
			else
				High = Mid - 1;
		}
		return Low;
	}

	// G-2 rest (GLORFINDEL-1125mb): Decide plus the cost checks. STUB of the red: answers Decide's verdict without checking the cost.
	FHOLGridActionView DecideCosted(const FHOLGridActionContext& Context, const FHOLGridCostFacts& Facts, const FHOLGridCommandView& Command)
	{
		FHOLGridActionView Result = Decide(Context, Command);
		if (Command.bGreyed)
		{
			return Result;
		}
		FHOLFeedbackView Feedback;
		if (Command.Command == EHOLGridCommandKind::Build)
		{
			Feedback = HOLCommandGridKeys::CheckBuild(Facts.Stock, Facts.Cost);
		}
		else if (Command.Command == EHOLGridCommandKind::Produce)
		{
			Feedback = HOLCommandGridKeys::CheckProduction(Facts.Stock, Facts.Cost, Facts.QueueLength, Facts.MaxQueue, Facts.PopulationUsed, Facts.PopulationCap, Facts.Places);
		}
		if (HOLCommandGridKeys::Blocks(Feedback))
		{
			Result.Kind = EHOLGridActionKind::Refuse;
			Result.Feedback = Feedback;
		}
		return Result;
	}

	// G-1 / G-2 (ENONCES-1125m 1.3): the verdict of a click on the cell of this command. The caster, the range and the aim modes are DECIDED here; the C++ hook
	// (G-4) executes the verdict. Produce / Research / Trade (G-5) and Build's resource refusal (CheckBuildAnswer) are not decided here yet: Build answers Place.
	FHOLGridActionView Decide(const FHOLGridActionContext& Context, const FHOLGridCommandView& Command)
	{
		FHOLGridActionView Result;
		if (Command.bGreyed)
		{
			// A greyed cell (age not reached) does nothing. A greyed HERO ABILITY (not unlocked, cooling down) is REFUSED, not ignored (HALDIR-1125md: G-5a made it None,
			// so the player's notice of refusal was lost): the Report is the old journal word (banner / cry), the bridge writes the old line.
			if (Command.Command == EHOLGridCommandKind::HeroAbility)
			{
				Result.Kind = EHOLGridActionKind::Refuse;
				if (Command.Ability == 2)
				{
					Result.Report = n"banner";
				}
				else
				{
					Result.Report = n"cry";
				}
			}
			return Result;
		}
		EHOLGridCommandKind Kind = Command.Command;
		if (Kind == EHOLGridCommandKind::LightSeal)
		{
			// The Seal of Light: the first selected Monk throws it at the next ground click, from the table's range (HOLUnitAbilityTable, UnitAbilities.csv).
			if (!Context.bMonkUnit || Context.MonkUnitId < 0)
			{
				return Result;
			}
			HOLUnitAbilityTable::FHOLUnitAbilityRow Row = HOLUnitAbilityTable::Row(HOLUnitAbilityTable::LightSeal);
			Result.Kind = EHOLGridActionKind::Arm;
			Result.AimMode = GetAimSeal();
			Result.RangeCm = Row.RangeCm;
			Result.UnitId = Context.MonkUnitId;
			Result.Report = n"LightSeal";
		}
		else if (Kind == EHOLGridCommandKind::HeroAbility)
		{
			// Close Ranks is sent at once; the Banner waits for the ground (HOLCommandGridRules::HeroAbilityBanner = 2).
			if (Command.Ability == 2)
			{
				Result.Kind = EHOLGridActionKind::Arm;
				Result.AimMode = GetAimBanner();
				// ELROND-1125me: the Banner's own reach; 0 left the controller's last range (the Seal's 800 cm) in the aura report and the aim.
				Result.RangeCm = GetBannerReachCm();
				Result.Report = n"Banner";
			}
			else
			{
				Result.Kind = EHOLGridActionKind::Send;
				Result.Report = n"CloseRanks";
			}
		}
		else if (Kind == EHOLGridCommandKind::AttackMove)
		{
			Result.Kind = EHOLGridActionKind::Arm;
			Result.AimMode = GetAimAttackMove();
			Result.Report = n"AttackMove";
		}
		else if (Kind == EHOLGridCommandKind::Produce || Kind == EHOLGridCommandKind::Research || Kind == EHOLGridCommandKind::Trade)
		{
			Result.Kind = EHOLGridActionKind::Send;
		}
		else if (Kind == EHOLGridCommandKind::Build)
		{
			Result.Kind = EHOLGridActionKind::Place;
			Result.Report = n"Build";
		}
		else if (Kind == EHOLGridCommandKind::Stop || Kind == EHOLGridCommandKind::Hold || Kind == EHOLGridCommandKind::NecroticRage
			|| Kind == EHOLGridCommandKind::Deploy || Kind == EHOLGridCommandKind::UngarrisonAll || Kind == EHOLGridCommandKind::CancelConstruction
			|| Kind == EHOLGridCommandKind::Specialize || Kind == EHOLGridCommandKind::CancelSpecialization)
		{
			Result.Kind = EHOLGridActionKind::Send;
		}
		return Result;
	}
}
