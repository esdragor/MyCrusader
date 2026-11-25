// GLORFINDEL-1125mc (ENONCES-1125m G-6, D839): the FACE of a command grid cell (label, greyed or ready, tooltip, report name), decided in script.
// Today HOLGameScreen.cpp 1300-1372 decides it (HeroAbility, LightSeal, NecroticRage); the C++ hook that asks this rule and keeps only
// GridCell->SetCommand / SetTooltip comes in a later C++ lot (no twin: the C++ cases go when the hook is wired). NOT COMPILED when written.
// STUB of the red: GetCellFace answers an empty face for everything.

enum EHOLGridFaceState
{
	Ready,
	Greyed,
}

// What the cell's command does not carry (filled by the hook from the game state): the seconds left of the cool-down shown under the label,
// the hero level an ability needs (0 when it is unlocked), and the numbers of the tooltip (HOLZoneEffect's for the Seal: radius m, duration s, slow %;
// the simulation's for the rage: duration s, +damage %, +rate %, HP lost per hit, refused under HP, stops at HP, recharge s).
struct FHOLGridFaceFacts
{
	int CooldownSeconds = 0;
	int UnlockLevel = 0;
	TArray<int> Numbers;
}

struct FHOLGridCellLookView
{
	FText Label;
	EHOLGridFaceState State = EHOLGridFaceState::Ready;
	FText TooltipName;
	FText TooltipEffect;
	FName ReportName;
}

namespace HOLCommandGridFace
{
	FHOLGridCellLookView GetCellFace(const FHOLGridCommandView& Command, const FHOLGridFaceFacts& Facts)
	{
		FHOLGridCellLookView Result;
		return Result;
	}
}
