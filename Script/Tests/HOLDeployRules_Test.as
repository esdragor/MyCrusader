// The deployable siege engine's pure rule, alone (Palier3InProgress.md G5, GDD D712): the Trebuchet is DEPLOYED (fires, does not walk) or
// PACKED (walks, does not fire), with a wait between the two; it cannot aim at a target nearer than its minimum range. The expectations
// come from D712 and from the rule's own profile and functions (the delays, the ranges), never from retyped numbers; the values of the
// profile are PROPOSED (Retours\Q-trebuchet-deploy.md), so no test pins one.

FHOLDeployState DeployState(EHOLDeployPhase Phase)
{
	FHOLDeployState State;
	State.Phase = Phase;
	State.ElapsedMs = 0;
	return State;
}

// Time passes in steps until the phase changes; the milliseconds it took (a safety stop at 10 minutes).
int MsUntilPhaseChange(FHOLDeployState Start, const FHOLDeployProfile& Profile, int Counterweight, int StepMs)
{
	FHOLDeployState State = Start;
	int Elapsed = 0;
	while (State.Phase == Start.Phase && Elapsed < 600000)
	{
		State = HOLDeploy::Advance(State, StepMs, Profile, Counterweight);
		Elapsed += StepMs;
	}
	return Elapsed;
}

// D712: packed it walks and does not fire, deployed it fires and does not walk; during a change of state it does neither (no action, no defence).
void Test_DeployWhoCanDoWhat(FUnitTest& T)
{
	T.AssertFalse(HOLDeploy::CanFire(DeployState(EHOLDeployPhase::Packed)), "packed: no fire");
	T.AssertTrue(HOLDeploy::CanFire(DeployState(EHOLDeployPhase::Deployed)), "deployed: fires");
	T.AssertFalse(HOLDeploy::CanFire(DeployState(EHOLDeployPhase::Deploying)), "deploying: no fire");
	T.AssertFalse(HOLDeploy::CanFire(DeployState(EHOLDeployPhase::Packing)), "packing: no fire");
	T.AssertTrue(HOLDeploy::CanMove(DeployState(EHOLDeployPhase::Packed)), "packed: walks");
	T.AssertFalse(HOLDeploy::CanMove(DeployState(EHOLDeployPhase::Deployed)), "deployed: does not walk");
	T.AssertFalse(HOLDeploy::CanMove(DeployState(EHOLDeployPhase::Deploying)), "deploying: does not walk");
	T.AssertFalse(HOLDeploy::CanMove(DeployState(EHOLDeployPhase::Packing)), "packing: does not walk");
	T.AssertTrue(HOLDeploy::CanAct(DeployState(EHOLDeployPhase::Packed)), "packed: can act (defend)");
	T.AssertTrue(HOLDeploy::CanAct(DeployState(EHOLDeployPhase::Deployed)), "deployed: can act");
	T.AssertFalse(HOLDeploy::CanAct(DeployState(EHOLDeployPhase::Deploying)), "deploying: no action, no defence");
	T.AssertFalse(HOLDeploy::CanAct(DeployState(EHOLDeployPhase::Packing)), "packing: no action, no defence");
}

// The deploying lasts exactly the profile's delay, the packing its own; time on a settled state changes nothing.
void Test_DeployDelaysAreTheProfiles(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeploy::DefaultProfile();
	T.AssertTrue(Profile.DeployMs > 0 && Profile.PackMs > 0, "the profile has real delays");
	FHOLDeployState Deploying = DeployState(EHOLDeployPhase::Deploying);
	FHOLDeployState Almost = HOLDeploy::Advance(Deploying, Profile.DeployMs - 1, Profile, 0);
	T.AssertTrue(Almost.Phase == EHOLDeployPhase::Deploying, "one millisecond short: still deploying");
	FHOLDeployState Done = HOLDeploy::Advance(Almost, 1, Profile, 0);
	T.AssertTrue(Done.Phase == EHOLDeployPhase::Deployed, "the delay passed: deployed");
	T.AssertEquals(0, Done.ElapsedMs, "a settled state starts at zero");
	FHOLDeployState Packing = DeployState(EHOLDeployPhase::Packing);
	T.AssertTrue(HOLDeploy::Advance(Packing, Profile.PackMs - 1, Profile, 0).Phase == EHOLDeployPhase::Packing, "one millisecond short: still packing");
	T.AssertTrue(HOLDeploy::Advance(Packing, Profile.PackMs, Profile, 0).Phase == EHOLDeployPhase::Packed, "the delay passed: packed");
	T.AssertTrue(HOLDeploy::Advance(DeployState(EHOLDeployPhase::Deployed), 100000, Profile, 0).Phase == EHOLDeployPhase::Deployed, "time on a deployed engine changes nothing");
	T.AssertTrue(HOLDeploy::Advance(DeployState(EHOLDeployPhase::Packed), 100000, Profile, 0).Phase == EHOLDeployPhase::Packed, "time on a packed engine changes nothing");
	// In small steps (a simulation's ticks) the change comes at the same moment.
	T.AssertTrue(MsUntilPhaseChange(Deploying, Profile, 0, 50) >= Profile.DeployMs && MsUntilPhaseChange(Deploying, Profile, 0, 50) < Profile.DeployMs + 50, "50 ms steps: the same moment, within a step");
}

// D712 (PQ-3): the order does the work. A packed engine given a far target walks, one in range deploys, a deployed one fires.
void Test_DeployAttackOrderDecisions(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeploy::DefaultProfile();
	int InRange = (Profile.MinRangeCm + Profile.MaxRangeCm) / 2;
	int TooFar = Profile.MaxRangeCm + 100;
	T.AssertTrue(HOLDeploy::Decide(DeployState(EHOLDeployPhase::Packed), EHOLDeployOrder::Attack, TooFar, Profile).Action == EHOLDeployAction::March, "packed, target too far: walk toward it");
	FHOLDeployDecision Start = HOLDeploy::Decide(DeployState(EHOLDeployPhase::Packed), EHOLDeployOrder::Attack, InRange, Profile);
	T.AssertTrue(Start.Action == EHOLDeployAction::Deploy && Start.NextPhase == EHOLDeployPhase::Deploying, "packed, target in range: starts deploying");
	T.AssertTrue(HOLDeploy::Decide(DeployState(EHOLDeployPhase::Deploying), EHOLDeployOrder::Attack, InRange, Profile).Action == EHOLDeployAction::Wait, "deploying, target in range: waits");
	T.AssertTrue(HOLDeploy::Decide(DeployState(EHOLDeployPhase::Deployed), EHOLDeployOrder::Attack, InRange, Profile).Action == EHOLDeployAction::Fire, "deployed, target in range: fires");
	FHOLDeployDecision Leave = HOLDeploy::Decide(DeployState(EHOLDeployPhase::Deployed), EHOLDeployOrder::Attack, TooFar, Profile);
	T.AssertTrue(Leave.Action == EHOLDeployAction::Pack && Leave.NextPhase == EHOLDeployPhase::Packing, "deployed, target too far: packs to go to it");
	T.AssertTrue(HOLDeploy::Decide(DeployState(EHOLDeployPhase::Packing), EHOLDeployOrder::Attack, InRange, Profile).Action == EHOLDeployAction::Wait, "packing, target in range: waits for the end");
	// The ends of the range are inside it.
	T.AssertTrue(HOLDeploy::Decide(DeployState(EHOLDeployPhase::Deployed), EHOLDeployOrder::Attack, Profile.MinRangeCm, Profile).Action == EHOLDeployAction::Fire, "exactly the minimum range: aimed");
	T.AssertTrue(HOLDeploy::Decide(DeployState(EHOLDeployPhase::Deployed), EHOLDeployOrder::Attack, Profile.MaxRangeCm, Profile).Action == EHOLDeployAction::Fire, "exactly the maximum range: aimed");
}

// D712 (PQ-13): a target nearer than the minimum range cannot be aimed, in any state: the order is refused with its reason, nothing changes.
void Test_DeployMinimumRange(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeploy::DefaultProfile();
	T.AssertTrue(Profile.MinRangeCm > 0, "the Trebuchet has a minimum range");
	T.AssertTrue(Profile.MinRangeCm < Profile.MaxRangeCm, "the minimum is under the maximum");
	for (int Index = 0; Index < 4; Index++)
	{
		EHOLDeployPhase Phase = HOLDeploy::PhaseAt(Index);
		FHOLDeployDecision Refused = HOLDeploy::Decide(DeployState(Phase), EHOLDeployOrder::Attack, Profile.MinRangeCm - 1, Profile);
		T.AssertTrue(Refused.Action == EHOLDeployAction::Refuse, f"phase {Index}: a target under the minimum is refused");
		T.AssertEquals(HOLDeploy::TooClose, Refused.Reason, f"phase {Index}: the reason is the minimum range");
		T.AssertTrue(Refused.NextPhase == Phase, f"phase {Index}: a refusal changes nothing");
		FHOLDeployDecision Point = HOLDeploy::Decide(DeployState(Phase), EHOLDeployOrder::Attack, 0, Profile);
		T.AssertTrue(Point.Action == EHOLDeployAction::Refuse, f"phase {Index}: a target at distance 0 is refused too");
	}
	T.AssertFalse(HOLDeploy::CanAim(Profile, Profile.MinRangeCm - 1), "can aim: under the minimum, no");
	T.AssertTrue(HOLDeploy::CanAim(Profile, Profile.MinRangeCm), "can aim: the minimum, yes");
	T.AssertTrue(HOLDeploy::CanAim(Profile, Profile.MaxRangeCm), "can aim: the maximum, yes");
	T.AssertFalse(HOLDeploy::CanAim(Profile, Profile.MaxRangeCm + 1), "can aim: past the maximum, no");
}

// D712: a walk order unpacks nothing and fires nothing: a packed engine walks, a deployed one packs first, a packing one finishes, and a
// deploying one is stopped (the interruption's rule is PROPOSED: the order cancels the deploying, back to packed at once).
void Test_DeployMoveOrderDecisions(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeploy::DefaultProfile();
	T.AssertTrue(HOLDeploy::Decide(DeployState(EHOLDeployPhase::Packed), EHOLDeployOrder::Move, 5000, Profile).Action == EHOLDeployAction::March, "packed: walks");
	FHOLDeployDecision Pack = HOLDeploy::Decide(DeployState(EHOLDeployPhase::Deployed), EHOLDeployOrder::Move, 5000, Profile);
	T.AssertTrue(Pack.Action == EHOLDeployAction::Pack && Pack.NextPhase == EHOLDeployPhase::Packing, "deployed: packs first");
	T.AssertTrue(HOLDeploy::Decide(DeployState(EHOLDeployPhase::Packing), EHOLDeployOrder::Move, 5000, Profile).Action == EHOLDeployAction::Wait, "packing: waits for the end");
	FHOLDeployDecision Cancel = HOLDeploy::Decide(DeployState(EHOLDeployPhase::Deploying), EHOLDeployOrder::Move, 5000, Profile);
	T.AssertTrue(Cancel.Action == EHOLDeployAction::CancelDeploy && Cancel.NextPhase == EHOLDeployPhase::Packed, "deploying: the walk order cancels it, packed again");
	// A walk order under the minimum range is a walk, not an aim: it is never refused.
	T.AssertTrue(HOLDeploy::Decide(DeployState(EHOLDeployPhase::Packed), EHOLDeployOrder::Move, 0, Profile).Action == EHOLDeployAction::March, "a walk order is never refused for the minimum range");
}

// D712 (PQ-4): the manual button, Mount / Dismount: deploys a packed engine, packs a deployed one, cancels a deploying one, waits on a packing one.
void Test_DeployButton(FUnitTest& T)
{
	T.AssertTrue(HOLDeploy::Toggle(DeployState(EHOLDeployPhase::Packed)).Action == EHOLDeployAction::Deploy, "button on a packed engine: deploy");
	T.AssertTrue(HOLDeploy::Toggle(DeployState(EHOLDeployPhase::Deployed)).Action == EHOLDeployAction::Pack, "button on a deployed engine: pack");
	T.AssertTrue(HOLDeploy::Toggle(DeployState(EHOLDeployPhase::Deploying)).Action == EHOLDeployAction::CancelDeploy, "button while deploying: cancel");
	T.AssertTrue(HOLDeploy::Toggle(DeployState(EHOLDeployPhase::Packing)).Action == EHOLDeployAction::Wait, "button while packing: nothing to do");
	FHOLDeployState Applied = HOLDeploy::Apply(DeployState(EHOLDeployPhase::Packed), HOLDeploy::Toggle(DeployState(EHOLDeployPhase::Packed)));
	T.AssertTrue(Applied.Phase == EHOLDeployPhase::Deploying && Applied.ElapsedMs == 0, "applying the button's decision starts the deploying at zero");
}

// D715: Counterweight shortens BOTH delays by a percentage; none leaves the table's; more is shorter; it never goes below nothing nor
// past 100 percent.
void Test_DeployCounterweight(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeploy::DefaultProfile();
	T.AssertEquals(Profile.DeployMs, HOLDeploy::DeployDelayMs(Profile, 0), "no Counterweight: the table's deploy delay");
	T.AssertEquals(Profile.PackMs, HOLDeploy::PackDelayMs(Profile, 0), "no Counterweight: the table's pack delay");
	T.AssertTrue(HOLDeploy::DeployDelayMs(Profile, 25) < Profile.DeployMs, "25 percent: the deploy delay is shorter");
	T.AssertTrue(HOLDeploy::PackDelayMs(Profile, 25) < Profile.PackMs, "25 percent: the pack delay is shorter");
	T.AssertTrue(HOLDeploy::DeployDelayMs(Profile, 50) < HOLDeploy::DeployDelayMs(Profile, 25), "more percent, shorter delay");
	T.AssertTrue(HOLDeploy::DeployDelayMs(Profile, 150) >= 0 && HOLDeploy::DeployDelayMs(Profile, 150) == HOLDeploy::DeployDelayMs(Profile, 100), "past 100 percent is 100 percent");
	T.AssertEquals(Profile.DeployMs, HOLDeploy::DeployDelayMs(Profile, -20), "a negative percentage is none");
	// The effect: the engine with Counterweight is deployed sooner, by the percentage.
	int Without = MsUntilPhaseChange(DeployState(EHOLDeployPhase::Deploying), Profile, 0, 10);
	int With = MsUntilPhaseChange(DeployState(EHOLDeployPhase::Deploying), Profile, 25, 10);
	T.AssertTrue(With < Without, "with Counterweight the engine is deployed sooner");
	T.AssertTrue(With >= HOLDeploy::DeployDelayMs(Profile, 25) && With < HOLDeploy::DeployDelayMs(Profile, 25) + 10, "and it takes the shortened delay, within a step");
	int PackWithout = MsUntilPhaseChange(DeployState(EHOLDeployPhase::Packing), Profile, 0, 10);
	int PackWith = MsUntilPhaseChange(DeployState(EHOLDeployPhase::Packing), Profile, 25, 10);
	T.AssertTrue(PackWith < PackWithout, "with Counterweight the engine is packed sooner too");
}

// The whole sequence on the real chain: an attack order on a target in range from packed; the first shot comes after the deploy delay and
// not before; then a walk order: the engine packs (no walking, no shot) and walks only after the pack delay.
void Test_DeploySequenceFromOrderToShot(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeploy::DefaultProfile();
	int Distance = (Profile.MinRangeCm + Profile.MaxRangeCm) / 2;
	FHOLDeployState State = DeployState(EHOLDeployPhase::Packed);
	int Step = 50;
	int Clock = 0;
	int FirstShot = -1;
	int FiredTooEarly = 0;
	for (int Tick = 0; Tick < 2000 && FirstShot < 0; Tick++)
	{
		FHOLDeployDecision Decision = HOLDeploy::Decide(State, EHOLDeployOrder::Attack, Distance, Profile);
		if (Decision.Action == EHOLDeployAction::Fire)
		{
			FirstShot = Clock;
		}
		else if (HOLDeploy::CanFire(State))
		{
			FiredTooEarly++;
		}
		State = HOLDeploy::Apply(State, Decision);
		State = HOLDeploy::Advance(State, Step, Profile, 0);
		Clock += Step;
	}
	T.AssertTrue(FirstShot >= Profile.DeployMs, "no shot before the deploy delay has passed");
	T.AssertTrue(FirstShot < Profile.DeployMs + 3 * Step, "the shot comes right after the deploy delay");
	T.AssertEquals(0, FiredTooEarly, "a deployed engine that could fire always did");
	// Now a walk order from the deployed state.
	int Packed = -1;
	int Walked = -1;
	int WalkedWhileNotPacked = 0;
	Clock = 0;
	for (int Tick = 0; Tick < 2000 && Walked < 0; Tick++)
	{
		FHOLDeployDecision Decision = HOLDeploy::Decide(State, EHOLDeployOrder::Move, 8000, Profile);
		if (Decision.Action == EHOLDeployAction::March)
		{
			Walked = Clock;
			if (State.Phase != EHOLDeployPhase::Packed)
			{
				WalkedWhileNotPacked++;
			}
		}
		State = HOLDeploy::Apply(State, Decision);
		State = HOLDeploy::Advance(State, Step, Profile, 0);
		Clock += Step;
		if (Packed < 0 && State.Phase == EHOLDeployPhase::Packed)
		{
			Packed = Clock;
		}
	}
	T.AssertEquals(0, WalkedWhileNotPacked, "it never walks before it is packed");
	T.AssertTrue(Walked >= Profile.PackMs, "it walks only after the pack delay");
	T.AssertTrue(Walked < Profile.PackMs + 3 * Step, "and right after it");
}

// Every combination of phase, order and distance keeps the rule's promises (the whole table, not the cases above).
void Test_DeployEveryCombination(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeploy::DefaultProfile();
	TArray<int> Distances;
	Distances.Add(0);
	Distances.Add(Profile.MinRangeCm - 1);
	Distances.Add(Profile.MinRangeCm);
	Distances.Add((Profile.MinRangeCm + Profile.MaxRangeCm) / 2);
	Distances.Add(Profile.MaxRangeCm);
	Distances.Add(Profile.MaxRangeCm + 1);
	Distances.Add(Profile.MaxRangeCm * 10);
	int Broken = 0;
	int Cases = 0;
	for (int PhaseIndex = 0; PhaseIndex < 4; PhaseIndex++)
	{
		EHOLDeployPhase Phase = HOLDeploy::PhaseAt(PhaseIndex);
		for (int OrderIndex = 0; OrderIndex < 2; OrderIndex++)
		{
			EHOLDeployOrder Order = OrderIndex == 0 ? EHOLDeployOrder::Attack : EHOLDeployOrder::Move;
			for (int Distance : Distances)
			{
				Cases++;
				FHOLDeployState State = DeployState(Phase);
				FHOLDeployDecision Decision = HOLDeploy::Decide(State, Order, Distance, Profile);
				FHOLDeployState Next = HOLDeploy::Apply(State, Decision);
				bool Ok = true;
				// Fire: deployed, attacking, in range.
				if (Decision.Action == EHOLDeployAction::Fire)
				{
					Ok = Ok && Phase == EHOLDeployPhase::Deployed && Order == EHOLDeployOrder::Attack && HOLDeploy::CanAim(Profile, Distance);
				}
				// March: packed only.
				if (Decision.Action == EHOLDeployAction::March)
				{
					Ok = Ok && Phase == EHOLDeployPhase::Packed;
				}
				// Refuse: an attack under the minimum, nothing else.
				if (Decision.Action == EHOLDeployAction::Refuse)
				{
					Ok = Ok && Order == EHOLDeployOrder::Attack && Distance < Profile.MinRangeCm;
				}
				if (Order == EHOLDeployOrder::Attack && Distance < Profile.MinRangeCm)
				{
					Ok = Ok && Decision.Action == EHOLDeployAction::Refuse;
				}
				// Deploy and Pack are started from their own settled phase; the others keep or cancel.
				if (Decision.Action == EHOLDeployAction::Deploy)
				{
					Ok = Ok && Phase == EHOLDeployPhase::Packed && Next.Phase == EHOLDeployPhase::Deploying;
				}
				if (Decision.Action == EHOLDeployAction::Pack)
				{
					Ok = Ok && Phase == EHOLDeployPhase::Deployed && Next.Phase == EHOLDeployPhase::Packing;
				}
				if (Decision.Action == EHOLDeployAction::CancelDeploy)
				{
					Ok = Ok && Phase == EHOLDeployPhase::Deploying && Next.Phase == EHOLDeployPhase::Packed;
				}
				if (Decision.Action == EHOLDeployAction::Fire || Decision.Action == EHOLDeployAction::March || Decision.Action == EHOLDeployAction::Wait || Decision.Action == EHOLDeployAction::Refuse)
				{
					Ok = Ok && Next.Phase == Phase;
				}
				// A change of state is never decided in the middle of another one (except the cancel of a deploying).
				if (Phase == EHOLDeployPhase::Packing)
				{
					Ok = Ok && (Decision.Action == EHOLDeployAction::Wait || Decision.Action == EHOLDeployAction::Refuse);
				}
				if (!Ok)
				{
					Broken++;
				}
			}
		}
	}
	T.AssertEquals(4 * 2 * 7, Cases, "all 56 combinations were visited");
	T.AssertEquals(0, Broken, "no combination breaks the rule");
}
