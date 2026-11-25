// ELROND-1125mb (ENONCES-1125m G-4 / raccord): the script side of UHOLGridActionBridge (Source/HeroesOfLiberty/Player/HOLGridActionBridge.h, ELROND-1125ma).
// AHOLPlayerController::ExecuteGridCommand asks DecideGridAction with the cell's command as ints and the own selection; this class builds the context the rule
// wants (HOLCommandGridAction::Decide, GLORFINDEL-1125ma) and copies its verdict into FHOLGridActionAnswer. Envelope only: the rule has ONE source, Decide.
// Lives in Script/Player (never Dev / Developers / Examples / Editor: those folders are skipped on the server and the client).
// DecideFromBridge copies the verdict of Decide into the answer (the stub answered Kind None for everything: RED 2237/2239).

namespace HOLGridActionBridgeScript
{
	FHOLGridActionAnswer DecideFromBridge(int Command, int Ability, bool bGreyed, int CooldownSeconds, FName TypeName, const TArray<int>& SelectedUnitIds)
	{
		// The cell's own facts (the C++ EHOLGridCommand and the script EHOLGridCommandKind list the same commands in the same order).
		FHOLGridCommandView CellView;
		CellView.Command = EHOLGridCommandKind(Command);
		CellView.Ability = Ability;
		CellView.bGreyed = bGreyed;
		CellView.CooldownSeconds = CooldownSeconds;
		CellView.TypeName = TypeName;

		// What the selection adds: the Seal's cell only exists for a selection holding a Monk, the caster is the first selected unit.
		FHOLGridActionContext Context;
		if (SelectedUnitIds.Num() > 0)
		{
			Context.bMonkUnit = true;
			Context.MonkUnitId = SelectedUnitIds[0];
		}

		FHOLGridActionView Verdict = HOLCommandGridAction::Decide(Context, CellView);
		FHOLGridActionAnswer Answer;
		Answer.Kind = int(Verdict.Kind);
		Answer.AimMode = Verdict.AimMode;
		Answer.RangeCm = Verdict.RangeCm;
		Answer.UnitId = Verdict.UnitId >= 0 ? Verdict.UnitId : 0;
		Answer.Report = Verdict.Report;
		return Answer;
	}
}

class UHOLGridActionScript : UHOLGridActionBridge
{
	UFUNCTION(BlueprintOverride)
	FHOLGridActionAnswer DecideGridAction(int Command, int Ability, bool bGreyed, int CooldownSeconds, FName TypeName, const TArray<int>& SelectedUnitIds)
	{
		return HOLGridActionBridgeScript::DecideFromBridge(Command, Ability, bGreyed, CooldownSeconds, TypeName, SelectedUnitIds);
	}
}
