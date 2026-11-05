// STUB (Palier3InProgress.md G5): behaviourless on purpose, so that HOLDeployRules_Test.as fails by assertion before the rule exists.

enum EHOLDeployPhase
{
	Packed,
	Deploying,
	Deployed,
	Packing,
}

enum EHOLDeployOrder
{
	Attack,
	Move,
}

enum EHOLDeployAction
{
	Wait,
	March,
	Deploy,
	Pack,
	Fire,
	CancelDeploy,
	Refuse,
}

struct FHOLDeployProfile
{
	int DeployMs = 0;
	int PackMs = 0;
	int MinRangeCm = 0;
	int MaxRangeCm = 0;
}

struct FHOLDeployState
{
	EHOLDeployPhase Phase = EHOLDeployPhase::Packed;
	int ElapsedMs = 0;
}

struct FHOLDeployDecision
{
	EHOLDeployAction Action = EHOLDeployAction::Wait;
	int Reason = 0;
	EHOLDeployPhase NextPhase = EHOLDeployPhase::Packed;
}

namespace HOLDeploy
{
	const int TooClose = 1;

	EHOLDeployPhase PhaseAt(int Index) { return EHOLDeployPhase::Packed; }
	FHOLDeployProfile DefaultProfile() { return FHOLDeployProfile(); }
	bool CanFire(const FHOLDeployState& State) { return false; }
	bool CanMove(const FHOLDeployState& State) { return false; }
	bool CanAct(const FHOLDeployState& State) { return false; }
	bool CanAim(const FHOLDeployProfile& Profile, int DistanceCm) { return false; }
	int DeployDelayMs(const FHOLDeployProfile& Profile, int CounterweightPercent) { return 0; }
	int PackDelayMs(const FHOLDeployProfile& Profile, int CounterweightPercent) { return 0; }
	FHOLDeployState Advance(const FHOLDeployState& State, int DtMs, const FHOLDeployProfile& Profile, int CounterweightPercent) { return State; }
	FHOLDeployDecision Decide(const FHOLDeployState& State, EHOLDeployOrder Order, int DistanceCm, const FHOLDeployProfile& Profile) { return FHOLDeployDecision(); }
	FHOLDeployDecision Toggle(const FHOLDeployState& State) { return FHOLDeployDecision(); }
	FHOLDeployState Apply(const FHOLDeployState& State, const FHOLDeployDecision& Decision) { return State; }
}
