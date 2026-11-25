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
	// The short name of the cell's action for hol.HUD.Report / the journal.
	FName Report;
}

namespace HOLCommandGridAction
{
	// The armed modes (a constant of a file is not seen by another file, hence the accessors); -1 is no mode.
	int GetAimNone() { return -1; }
	int GetAimAttackMove() { return 0; }
	int GetAimBanner() { return 1; }
	int GetAimSeal() { return 2; }

	FHOLGridActionView Decide(const FHOLGridActionContext& Context, const FHOLGridCommandView& Command)
	{
		FHOLGridActionView Result;
		return Result;
	}
}
