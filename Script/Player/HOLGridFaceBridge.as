// SAURON-1125md (ENONCES-1125m G-5b, D839): the script side of UHOLGridFaceBridge (Source/HeroesOfLiberty/Player/HOLGridFaceBridge.h).
// UHOLGameScreen::RefreshGrid asks GetCellFace with the cell's command as ints and the facts it read from the game state; this class builds the
// view the rule wants (HOLCommandGridFace::GetCellFace, GLORFINDEL-1125mc) and copies its FHOLGridCellLookView into FHOLGridFaceAnswer.
// Envelope only: the rule has ONE source, GetCellFace. NOT COMPILED when written.
// Lives in Script/Player (never Dev / Developers / Examples / Editor: those folders are skipped on the server and the client).

namespace HOLGridFaceBridgeScript
{
	FHOLGridFaceAnswer FaceFromBridge(int Command, int Ability, bool bGreyed, int CommandCooldownSeconds, int FactCooldownSeconds, int UnlockLevel, const TArray<int>& Numbers)
	{
		// The C++ EHOLGridCommand and the script EHOLGridCommandKind list the same commands in the same order (see HOLGridActionBridge.as).
		FHOLGridCommandView CellView;
		CellView.Command = EHOLGridCommandKind(Command);
		CellView.Ability = Ability;
		CellView.bGreyed = bGreyed;
		CellView.CooldownSeconds = CommandCooldownSeconds;

		FHOLGridFaceFacts Facts;
		Facts.CooldownSeconds = FactCooldownSeconds;
		Facts.UnlockLevel = UnlockLevel;
		Facts.Numbers = Numbers;

		FHOLGridCellLookView Look = HOLCommandGridFace::GetCellFace(CellView, Facts);
		FHOLGridFaceAnswer Answer;
		Answer.Label = Look.Label;
		Answer.State = int(Look.State);
		Answer.TooltipName = Look.TooltipName;
		Answer.TooltipEffect = Look.TooltipEffect;
		Answer.ReportName = Look.ReportName;
		return Answer;
	}
}

class UHOLGridFaceScript : UHOLGridFaceBridge
{
	UFUNCTION(BlueprintOverride)
	FHOLGridFaceAnswer GetCellFace(int Command, int Ability, bool bGreyed, int CommandCooldownSeconds, int FactCooldownSeconds, int UnlockLevel, const TArray<int>& Numbers)
	{
		return HOLGridFaceBridgeScript::FaceFromBridge(Command, Ability, bGreyed, CommandCooldownSeconds, FactCooldownSeconds, UnlockLevel, Numbers);
	}
}
