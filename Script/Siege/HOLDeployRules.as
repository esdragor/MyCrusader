// The deployable siege engine's pure rule (Palier3InProgress.md G5, GDD D712, D715): the Trebuchet is PACKED (it walks, it does not
// fire), DEPLOYED (it fires, it does not walk), or changing from one to the other (DEPLOYING, PACKING: no action and no defence); the
// order does the work (an attack on a target in range deploys then fires, a walk order packs then walks) and a manual button toggles; a
// target nearer than the MINIMUM RANGE cannot be aimed. Whole numbers only (milliseconds, centimetres, percent: D468 determinism), no
// world, no unit, no position: the caller gives the engine's state, the order and the distance to the target, and applies the decision.
// Tested alone in script (HOLDeployRules_Test.as). The C++ twin and the simulation that applies it are G6.
//
// The VALUES are PROPOSED, not decided (Retours\Q-trebuchet-deploy.md): they sit in DefaultProfile and Counterweight's percentage is the
// caller's (the technology's table row); one line to change each. The interruption of a deploying by a walk order (it is cancelled, the
// engine is packed again at once) is the coded default of the same question.

// The four states of an engine that deploys.
enum EHOLDeployPhase
{
	// Walks, does not fire.
	Packed,
	// Setting up: no action, no defence, for the deploy delay.
	Deploying,
	// Fires, does not walk.
	Deployed,
	// Taking down: no action, no defence, for the pack delay.
	Packing,
}

// The orders that ask something of a deployable engine.
enum EHOLDeployOrder
{
	// Hit this target (a building: the Trebuchet never aims at a unit, D712).
	Attack,
	// Go to this point.
	Move,
}

// What the engine does now.
enum EHOLDeployAction
{
	// Nothing to do yet: a change of state is under way and the order waits for its end.
	Wait,
	// Walk (toward the target or the point).
	March,
	// Start deploying.
	Deploy,
	// Start packing.
	Pack,
	// Fire at the target.
	Fire,
	// Stop deploying: back to packed at once, the progress lost.
	CancelDeploy,
	// The order cannot be carried out; Reason says why.
	Refuse,
}

// What an engine is made of: the two delays (ms), the range it can aim at (cm, both ends inside it).
struct FHOLDeployProfile
{
	int DeployMs = 0;
	int PackMs = 0;
	int MinRangeCm = 0;
	int MaxRangeCm = 0;
}

// Where an engine is: its phase and, during a change of state, how long it has lasted (ms). Small and fixed, to be a simulation fragment.
struct FHOLDeployState
{
	EHOLDeployPhase Phase = EHOLDeployPhase::Packed;
	int ElapsedMs = 0;
}

// What to do with an order: the action, why when it is refused, and the phase right after the decision is applied.
struct FHOLDeployDecision
{
	EHOLDeployAction Action = EHOLDeployAction::Wait;
	int Reason = 0;
	EHOLDeployPhase NextPhase = EHOLDeployPhase::Packed;
}

namespace HOLDeploy
{
	// The reason of a refusal: the target is nearer than the minimum range.
	const int TooClose = 1;

	// PROPOSED values (Q-trebuchet-deploy.md), one line each: deploying 8 s, packing 5 s, the target no nearer than 12 m and no farther
	// than 50 m (the Mangonel's reach at buildings is 30 m).
	const int ProposedDeployMs = 8000;
	const int ProposedPackMs = 5000;
	const int ProposedMinRangeCm = 1200;
	const int ProposedMaxRangeCm = 5000;

	// The phase at an index (0 to 3, in the order of the enum).
	EHOLDeployPhase PhaseAt(int Index)
	{
		switch (Index)
		{
			case 0: return EHOLDeployPhase::Packed;
			case 1: return EHOLDeployPhase::Deploying;
			case 2: return EHOLDeployPhase::Deployed;
		}
		return EHOLDeployPhase::Packing;
	}

	// The Trebuchet's profile (until the unit table carries the columns: the caller may fill its own).
	FHOLDeployProfile DefaultProfile()
	{
		FHOLDeployProfile Profile;
		Profile.DeployMs = ProposedDeployMs;
		Profile.PackMs = ProposedPackMs;
		Profile.MinRangeCm = ProposedMinRangeCm;
		Profile.MaxRangeCm = ProposedMaxRangeCm;
		return Profile;
	}

	// D712: only a deployed engine fires.
	bool CanFire(const FHOLDeployState& State)
	{
		return State.Phase == EHOLDeployPhase::Deployed;
	}

	// D712: only a packed engine walks.
	bool CanMove(const FHOLDeployState& State)
	{
		return State.Phase == EHOLDeployPhase::Packed;
	}

	// D712: no action and no defence during a change of state.
	bool CanAct(const FHOLDeployState& State)
	{
		return State.Phase == EHOLDeployPhase::Packed || State.Phase == EHOLDeployPhase::Deployed;
	}

	// A target at this distance can be aimed: neither nearer than the minimum range nor farther than the maximum (both ends are inside).
	bool CanAim(const FHOLDeployProfile& Profile, int DistanceCm)
	{
		return DistanceCm >= Profile.MinRangeCm && DistanceCm <= Profile.MaxRangeCm;
	}

	// A delay shortened by Counterweight (D715): by a percentage of the table's, 0 to 100 (outside it, the nearest end), never below nothing.
	int Shortened(int BaseMs, int CounterweightPercent)
	{
		int Percent = Math::Clamp(CounterweightPercent, 0, 100);
		return Math::Max(0, BaseMs * (100 - Percent) / 100);
	}

	// How long the deploying lasts, with Counterweight's percentage (0 = none).
	int DeployDelayMs(const FHOLDeployProfile& Profile, int CounterweightPercent)
	{
		return Shortened(Profile.DeployMs, CounterweightPercent);
	}

	// How long the packing lasts, with Counterweight's percentage (0 = none).
	int PackDelayMs(const FHOLDeployProfile& Profile, int CounterweightPercent)
	{
		return Shortened(Profile.PackMs, CounterweightPercent);
	}

	// Time passes: a change of state ends when its delay has passed (the time left over is not carried); a settled state stays.
	FHOLDeployState Advance(const FHOLDeployState& State, int DtMs, const FHOLDeployProfile& Profile, int CounterweightPercent)
	{
		FHOLDeployState Next = State;
		if (State.Phase == EHOLDeployPhase::Deploying)
		{
			Next.ElapsedMs = State.ElapsedMs + Math::Max(0, DtMs);
			if (Next.ElapsedMs >= DeployDelayMs(Profile, CounterweightPercent))
			{
				Next.Phase = EHOLDeployPhase::Deployed;
				Next.ElapsedMs = 0;
			}
		}
		else if (State.Phase == EHOLDeployPhase::Packing)
		{
			Next.ElapsedMs = State.ElapsedMs + Math::Max(0, DtMs);
			if (Next.ElapsedMs >= PackDelayMs(Profile, CounterweightPercent))
			{
				Next.Phase = EHOLDeployPhase::Packed;
				Next.ElapsedMs = 0;
			}
		}
		else
		{
			Next.ElapsedMs = 0;
		}
		return Next;
	}

	// A decision with its next phase.
	FHOLDeployDecision Make(EHOLDeployAction Action, EHOLDeployPhase NextPhase, int Reason = 0)
	{
		FHOLDeployDecision Decision;
		Decision.Action = Action;
		Decision.NextPhase = NextPhase;
		Decision.Reason = Reason;
		return Decision;
	}

	// What an order asks of the engine now (D712). An attack: refused under the minimum range; a packed engine walks to a far target or
	// deploys for one in range, a deployed one fires at one in range or packs to go to a far one, a change of state under way ends first (a
	// deploying that must go elsewhere is cancelled). A walk order: a packed engine walks, a deployed one packs first, a packing one ends,
	// a deploying one is cancelled (the PROPOSED interruption rule). DistanceCm is the distance to the target; it means nothing for a walk.
	FHOLDeployDecision Decide(const FHOLDeployState& State, EHOLDeployOrder Order, int DistanceCm, const FHOLDeployProfile& Profile)
	{
		EHOLDeployPhase Phase = State.Phase;
		if (Order == EHOLDeployOrder::Move)
		{
			switch (Phase)
			{
				case EHOLDeployPhase::Packed: return Make(EHOLDeployAction::March, Phase);
				case EHOLDeployPhase::Deployed: return Make(EHOLDeployAction::Pack, EHOLDeployPhase::Packing);
				case EHOLDeployPhase::Deploying: return Make(EHOLDeployAction::CancelDeploy, EHOLDeployPhase::Packed);
			}
			return Make(EHOLDeployAction::Wait, Phase);
		}
		if (DistanceCm < Profile.MinRangeCm)
		{
			return Make(EHOLDeployAction::Refuse, Phase, TooClose);
		}
		bool InReach = DistanceCm <= Profile.MaxRangeCm;
		switch (Phase)
		{
			case EHOLDeployPhase::Packed:
				return InReach ? Make(EHOLDeployAction::Deploy, EHOLDeployPhase::Deploying) : Make(EHOLDeployAction::March, Phase);
			case EHOLDeployPhase::Deploying:
				return InReach ? Make(EHOLDeployAction::Wait, Phase) : Make(EHOLDeployAction::CancelDeploy, EHOLDeployPhase::Packed);
			case EHOLDeployPhase::Deployed:
				return InReach ? Make(EHOLDeployAction::Fire, Phase) : Make(EHOLDeployAction::Pack, EHOLDeployPhase::Packing);
		}
		return Make(EHOLDeployAction::Wait, Phase);
	}

	// The manual button, Mount / Dismount (D712): deploys a packed engine, packs a deployed one, cancels a deploying one; a packing one
	// has nothing to do.
	FHOLDeployDecision Toggle(const FHOLDeployState& State)
	{
		switch (State.Phase)
		{
			case EHOLDeployPhase::Packed: return Make(EHOLDeployAction::Deploy, EHOLDeployPhase::Deploying);
			case EHOLDeployPhase::Deployed: return Make(EHOLDeployAction::Pack, EHOLDeployPhase::Packing);
			case EHOLDeployPhase::Deploying: return Make(EHOLDeployAction::CancelDeploy, EHOLDeployPhase::Packed);
		}
		return Make(EHOLDeployAction::Wait, State.Phase);
	}

	// The state once a decision is applied: the next phase, the change of state starting at zero.
	FHOLDeployState Apply(const FHOLDeployState& State, const FHOLDeployDecision& Decision)
	{
		FHOLDeployState Next = State;
		if (Decision.NextPhase != State.Phase)
		{
			Next.Phase = Decision.NextPhase;
			Next.ElapsedMs = 0;
		}
		return Next;
	}
}
