// The server's relay board (bascule Relay, D830/D836 point 3: the script is the source), the script side of UHOLRelayBoard:
// UHOLRelaySubsystem (C++, by-step: it counts the units of each zone) asks it once per relay and per step for the relay after the step
// (HOLRelay::StepWith of HOLRelayRules.as, with the capture steps and the recession the server holds), and once per slot for the territory XP
// of the step (HOLRelay::TerritoryXPAt). HOLRelay::MakeView stays C++ (it fills the replicated FHOLRelayView). Server and solo only.
// Tested by HeroesOfLiberty.Relay.ServerBoard.
class UHOLRelayBoardScript : UHOLRelayBoard
{
	UFUNCTION(BlueprintOverride)
	FHOLRelayBoardState RelayStep(const FHOLRelayBoardState& State, const TArray<int>& UnitsBySlot, const TArray<int>& TeamBySlot, int HoldSteps, int DecaySteps)
	{
		HOLRelay::FHOLRelayState Rule;
		Rule.Owner = State.Owner;
		Rule.Taker = State.Taker;
		Rule.Held = State.Held;
		const HOLRelay::FHOLRelayState Next = HOLRelay::StepWith(Rule, UnitsBySlot, TeamBySlot, HoldSteps, DecaySteps);
		FHOLRelayBoardState Result;
		Result.Owner = Next.Owner;
		Result.Taker = Next.Taker;
		Result.Held = Next.Held;
		return Result;
	}

	UFUNCTION(BlueprintOverride)
	FHOLRelayXPResult RelayTerritoryXP(int Carry, int Relays, int Centres, int StepsPerSecond)
	{
		HOLRelay::FHOLTerritoryCarry Memory;
		Memory.Carry = Carry;
		FHOLRelayXPResult Result;
		Result.Award = HOLRelay::TerritoryXPAt(Memory, Relays, Centres, StepsPerSecond);
		Result.Carry = Memory.Carry;
		return Result;
	}
}