// ELROND-1125mb (ENONCES-1125m G-4 / raccord): the script side of UHOLGridActionBridge (Source/HeroesOfLiberty/Player/HOLGridActionBridge.h, ELROND-1125ma).
// AHOLPlayerController::ExecuteGridCommand asks DecideGridAction with the cell's command as ints and the own selection; this class builds the context the rule
// wants (HOLCommandGridAction::Decide, GLORFINDEL-1125ma) and copies its verdict into FHOLGridActionAnswer. Envelope only: the rule has ONE source, Decide.
// Lives in Script/Player (never Dev / Developers / Examples / Editor: those folders are skipped on the server and the client).
// STUB OF THE RED: DecideFromBridge answers an empty answer (Kind None) for everything.

namespace HOLGridActionBridgeScript
{
	FHOLGridActionAnswer DecideFromBridge(int Command, int Ability, bool bGreyed, int CooldownSeconds, FName TypeName, const TArray<int>& SelectedUnitIds)
	{
		FHOLGridActionAnswer Answer;
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
