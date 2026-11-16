// Lot L17-10 of the migration (D830, D836, D839, ARWEN-1117b): the script port of HOLDeployRulesTests.cpp (9 tests), run against the read library of
// the deployable engine's rule that STAYS in C++ (HOLDeploy, read by Mass): UHOLDeployReadLibrary (namespace HOLDeployRead). Same vectors as the C++
// tests; the expectations come from the profile's own values (the delays, the ranges), never retyped numbers: the values are PROPOSED
// (Retours/Q-trebuchet-deploy.md), no test pins one. The C++ file stays until the rouge vu / vert is confirmed by the main session.
// (The old script test of this name was removed with the script rule by D836; this one is new, against the library.)

FHOLDeployState DeployStateOf(EHOLDeployPhase Phase)
{
	FHOLDeployState State;
	State.Phase = Phase;
	State.ElapsedMs = 0;
	return State;
}

// Time passes in steps until the phase changes; the milliseconds it took (a safety stop at 10 minutes).
int DeployMsUntilPhaseChange(const FHOLDeployState& Start, const FHOLDeployProfile& Profile, int Counterweight, int StepMs)
{
	FHOLDeployState State = Start;
	int Elapsed = 0;
	while (State.Phase == Start.Phase && Elapsed < 600000)
	{
		State = HOLDeployRead::Advance(State, StepMs, Profile, Counterweight);
		Elapsed += StepMs;
	}
	return Elapsed;
}

// HeroesOfLiberty.Siege.Deploy.WhoCanDoWhat
void Test_DeployWhoCanDoWhat(FUnitTest& T)
{
	T.AssertFalse(HOLDeployRead::CanFire(DeployStateOf(EHOLDeployPhase::Packed)), "packed: no fire");
	T.AssertTrue(HOLDeployRead::CanFire(DeployStateOf(EHOLDeployPhase::Deployed)), "deployed: fires");
	T.AssertFalse(HOLDeployRead::CanFire(DeployStateOf(EHOLDeployPhase::Deploying)), "deploying: no fire");
	T.AssertFalse(HOLDeployRead::CanFire(DeployStateOf(EHOLDeployPhase::Packing)), "packing: no fire");
	T.AssertTrue(HOLDeployRead::CanMove(DeployStateOf(EHOLDeployPhase::Packed)), "packed: walks");
	T.AssertFalse(HOLDeployRead::CanMove(DeployStateOf(EHOLDeployPhase::Deployed)), "deployed: does not walk");
	T.AssertFalse(HOLDeployRead::CanMove(DeployStateOf(EHOLDeployPhase::Deploying)), "deploying: does not walk");
	T.AssertFalse(HOLDeployRead::CanMove(DeployStateOf(EHOLDeployPhase::Packing)), "packing: does not walk");
	T.AssertTrue(HOLDeployRead::CanAct(DeployStateOf(EHOLDeployPhase::Packed)), "packed: can act (defend)");
	T.AssertTrue(HOLDeployRead::CanAct(DeployStateOf(EHOLDeployPhase::Deployed)), "deployed: can act");
	T.AssertFalse(HOLDeployRead::CanAct(DeployStateOf(EHOLDeployPhase::Deploying)), "deploying: no action, no defence");
	T.AssertFalse(HOLDeployRead::CanAct(DeployStateOf(EHOLDeployPhase::Packing)), "packing: no action, no defence");
}

// HeroesOfLiberty.Siege.Deploy.DelaysAreTheProfiles
void Test_DeployDelaysAreTheProfiles(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeployRead::DefaultProfile();
	T.AssertTrue(Profile.DeployMs > 0 && Profile.PackMs > 0, "the profile has real delays");
	FHOLDeployState Deploying = DeployStateOf(EHOLDeployPhase::Deploying);
	FHOLDeployState Almost = HOLDeployRead::Advance(Deploying, Profile.DeployMs - 1, Profile, 0);
	T.AssertTrue(Almost.Phase == EHOLDeployPhase::Deploying, "one millisecond short: still deploying");
	FHOLDeployState Done = HOLDeployRead::Advance(Almost, 1, Profile, 0);
	T.AssertTrue(Done.Phase == EHOLDeployPhase::Deployed, "the delay passed: deployed");
	T.AssertEquals(0, Done.ElapsedMs, "a settled state starts at zero");
	FHOLDeployState Packing = DeployStateOf(EHOLDeployPhase::Packing);
	T.AssertTrue(HOLDeployRead::Advance(Packing, Profile.PackMs - 1, Profile, 0).Phase == EHOLDeployPhase::Packing, "one millisecond short: still packing");
	T.AssertTrue(HOLDeployRead::Advance(Packing, Profile.PackMs, Profile, 0).Phase == EHOLDeployPhase::Packed, "the delay passed: packed");
	T.AssertTrue(HOLDeployRead::Advance(DeployStateOf(EHOLDeployPhase::Deployed), 100000, Profile, 0).Phase == EHOLDeployPhase::Deployed, "time on a deployed engine changes nothing");
	T.AssertTrue(HOLDeployRead::Advance(DeployStateOf(EHOLDeployPhase::Packed), 100000, Profile, 0).Phase == EHOLDeployPhase::Packed, "time on a packed engine changes nothing");
	int Small = DeployMsUntilPhaseChange(Deploying, Profile, 0, 50);
	T.AssertTrue(Small >= Profile.DeployMs && Small < Profile.DeployMs + 50, "50 ms steps: the same moment, within a step");
}

// HeroesOfLiberty.Siege.Deploy.AttackOrderDecisions
void Test_DeployAttackOrderDecisions(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeployRead::DefaultProfile();
	int InRange = (Profile.MinRangeCm + Profile.MaxRangeCm) / 2;
	int TooFar = Profile.MaxRangeCm + 100;
	T.AssertTrue(HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Packed), EHOLDeployOrder::Attack, TooFar, Profile).Action == EHOLDeployAction::March, "packed, target too far: walk toward it");
	FHOLDeployDecision Start = HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Packed), EHOLDeployOrder::Attack, InRange, Profile);
	T.AssertTrue(Start.Action == EHOLDeployAction::Deploy && Start.NextPhase == EHOLDeployPhase::Deploying, "packed, target in range: starts deploying");
	T.AssertTrue(HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Deploying), EHOLDeployOrder::Attack, InRange, Profile).Action == EHOLDeployAction::Wait, "deploying, target in range: waits");
	T.AssertTrue(HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Deployed), EHOLDeployOrder::Attack, InRange, Profile).Action == EHOLDeployAction::Fire, "deployed, target in range: fires");
	FHOLDeployDecision Leave = HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Deployed), EHOLDeployOrder::Attack, TooFar, Profile);
	T.AssertTrue(Leave.Action == EHOLDeployAction::Pack && Leave.NextPhase == EHOLDeployPhase::Packing, "deployed, target too far: packs to go to it");
	T.AssertTrue(HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Packing), EHOLDeployOrder::Attack, InRange, Profile).Action == EHOLDeployAction::Wait, "packing, target in range: waits for the end");
	T.AssertTrue(HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Deployed), EHOLDeployOrder::Attack, Profile.MinRangeCm, Profile).Action == EHOLDeployAction::Fire, "exactly the minimum range: aimed");
	T.AssertTrue(HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Deployed), EHOLDeployOrder::Attack, Profile.MaxRangeCm, Profile).Action == EHOLDeployAction::Fire, "exactly the maximum range: aimed");
}

// HeroesOfLiberty.Siege.Deploy.MinimumRange
void Test_DeployMinimumRange(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeployRead::DefaultProfile();
	T.AssertTrue(Profile.MinRangeCm > 0, "the Trebuchet has a minimum range");
	T.AssertTrue(Profile.MinRangeCm < Profile.MaxRangeCm, "the minimum is under the maximum");
	for (int Index = 0; Index < 4; Index++)
	{
		EHOLDeployPhase Phase = HOLDeployRead::PhaseAt(Index);
		FHOLDeployDecision Refused = HOLDeployRead::Decide(DeployStateOf(Phase), EHOLDeployOrder::Attack, Profile.MinRangeCm - 1, Profile);
		T.AssertTrue(Refused.Action == EHOLDeployAction::Refuse, "phase " + Index + ": a target under the minimum is refused");
		T.AssertEquals(HOLDeployRead::GetTooClose(), Refused.Reason, "phase " + Index + ": the reason is the minimum range");
		T.AssertTrue(Refused.NextPhase == Phase, "phase " + Index + ": a refusal changes nothing");
		T.AssertTrue(HOLDeployRead::Decide(DeployStateOf(Phase), EHOLDeployOrder::Attack, 0, Profile).Action == EHOLDeployAction::Refuse, "phase " + Index + ": a target at distance 0 is refused too");
	}
	T.AssertFalse(HOLDeployRead::CanAim(Profile, Profile.MinRangeCm - 1), "can aim: under the minimum, no");
	T.AssertTrue(HOLDeployRead::CanAim(Profile, Profile.MinRangeCm), "can aim: the minimum, yes");
	T.AssertTrue(HOLDeployRead::CanAim(Profile, Profile.MaxRangeCm), "can aim: the maximum, yes");
	T.AssertFalse(HOLDeployRead::CanAim(Profile, Profile.MaxRangeCm + 1), "can aim: past the maximum, no");
}

// HeroesOfLiberty.Siege.Deploy.MoveOrderDecisions
void Test_DeployMoveOrderDecisions(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeployRead::DefaultProfile();
	T.AssertTrue(HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Packed), EHOLDeployOrder::Move, 5000, Profile).Action == EHOLDeployAction::March, "packed: walks");
	FHOLDeployDecision Pack = HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Deployed), EHOLDeployOrder::Move, 5000, Profile);
	T.AssertTrue(Pack.Action == EHOLDeployAction::Pack && Pack.NextPhase == EHOLDeployPhase::Packing, "deployed: packs first");
	T.AssertTrue(HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Packing), EHOLDeployOrder::Move, 5000, Profile).Action == EHOLDeployAction::Wait, "packing: waits for the end");
	FHOLDeployDecision Cancel = HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Deploying), EHOLDeployOrder::Move, 5000, Profile);
	T.AssertTrue(Cancel.Action == EHOLDeployAction::CancelDeploy && Cancel.NextPhase == EHOLDeployPhase::Packed, "deploying: the walk order cancels it, packed again");
	T.AssertTrue(HOLDeployRead::Decide(DeployStateOf(EHOLDeployPhase::Packed), EHOLDeployOrder::Move, 0, Profile).Action == EHOLDeployAction::March, "a walk order is never refused for the minimum range");
}

// HeroesOfLiberty.Siege.Deploy.Button
void Test_DeployButton(FUnitTest& T)
{
	T.AssertTrue(HOLDeployRead::Toggle(DeployStateOf(EHOLDeployPhase::Packed)).Action == EHOLDeployAction::Deploy, "button on a packed engine: deploy");
	T.AssertTrue(HOLDeployRead::Toggle(DeployStateOf(EHOLDeployPhase::Deployed)).Action == EHOLDeployAction::Pack, "button on a deployed engine: pack");
	T.AssertTrue(HOLDeployRead::Toggle(DeployStateOf(EHOLDeployPhase::Deploying)).Action == EHOLDeployAction::CancelDeploy, "button while deploying: cancel");
	T.AssertTrue(HOLDeployRead::Toggle(DeployStateOf(EHOLDeployPhase::Packing)).Action == EHOLDeployAction::Wait, "button while packing: nothing to do");
	FHOLDeployState Packed = DeployStateOf(EHOLDeployPhase::Packed);
	FHOLDeployState Applied = HOLDeployRead::Apply(Packed, HOLDeployRead::Toggle(Packed));
	T.AssertTrue(Applied.Phase == EHOLDeployPhase::Deploying && Applied.ElapsedMs == 0, "applying the button's decision starts the deploying at zero");
}

// HeroesOfLiberty.Siege.Deploy.Counterweight
void Test_DeployCounterweight(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeployRead::DefaultProfile();
	T.AssertEquals(Profile.DeployMs, HOLDeployRead::DeployDelayMs(Profile, 0), "no Counterweight: the table's deploy delay");
	T.AssertEquals(Profile.PackMs, HOLDeployRead::PackDelayMs(Profile, 0), "no Counterweight: the table's pack delay");
	T.AssertTrue(HOLDeployRead::DeployDelayMs(Profile, 25) < Profile.DeployMs, "25 percent: the deploy delay is shorter");
	T.AssertTrue(HOLDeployRead::PackDelayMs(Profile, 25) < Profile.PackMs, "25 percent: the pack delay is shorter");
	T.AssertTrue(HOLDeployRead::DeployDelayMs(Profile, 50) < HOLDeployRead::DeployDelayMs(Profile, 25), "more percent, shorter delay");
	T.AssertTrue(HOLDeployRead::DeployDelayMs(Profile, 150) >= 0 && HOLDeployRead::DeployDelayMs(Profile, 150) == HOLDeployRead::DeployDelayMs(Profile, 100), "past 100 percent is 100 percent");
	T.AssertEquals(Profile.DeployMs, HOLDeployRead::DeployDelayMs(Profile, -20), "a negative percentage is none");
	int Without = DeployMsUntilPhaseChange(DeployStateOf(EHOLDeployPhase::Deploying), Profile, 0, 10);
	int With = DeployMsUntilPhaseChange(DeployStateOf(EHOLDeployPhase::Deploying), Profile, 25, 10);
	T.AssertTrue(With < Without, "with Counterweight the engine is deployed sooner");
	T.AssertTrue(With >= HOLDeployRead::DeployDelayMs(Profile, 25) && With < HOLDeployRead::DeployDelayMs(Profile, 25) + 10, "and it takes the shortened delay, within a step");
	int PackWithout = DeployMsUntilPhaseChange(DeployStateOf(EHOLDeployPhase::Packing), Profile, 0, 10);
	int PackWith = DeployMsUntilPhaseChange(DeployStateOf(EHOLDeployPhase::Packing), Profile, 25, 10);
	T.AssertTrue(PackWith < PackWithout, "with Counterweight the engine is packed sooner too");
}

// HeroesOfLiberty.Siege.Deploy.SequenceFromOrderToShot
void Test_DeploySequenceFromOrderToShot(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeployRead::DefaultProfile();
	int Distance = (Profile.MinRangeCm + Profile.MaxRangeCm) / 2;
	FHOLDeployState State = DeployStateOf(EHOLDeployPhase::Packed);
	int Step = 50;
	int Clock = 0;
	int FirstShot = -1;
	int FiredTooEarly = 0;
	for (int Tick = 0; Tick < 2000 && FirstShot < 0; Tick++)
	{
		FHOLDeployDecision Decision = HOLDeployRead::Decide(State, EHOLDeployOrder::Attack, Distance, Profile);
		if (Decision.Action == EHOLDeployAction::Fire)
		{
			FirstShot = Clock;
		}
		else if (HOLDeployRead::CanFire(State))
		{
			FiredTooEarly++;
		}
		State = HOLDeployRead::Apply(State, Decision);
		State = HOLDeployRead::Advance(State, Step, Profile, 0);
		Clock += Step;
	}
	T.AssertTrue(FirstShot >= Profile.DeployMs, "no shot before the deploy delay has passed");
	T.AssertTrue(FirstShot >= 0 && FirstShot < Profile.DeployMs + 3 * Step, "the shot comes right after the deploy delay");
	T.AssertEquals(0, FiredTooEarly, "a deployed engine that could fire always did");
	int Walked = -1;
	int WalkedWhileNotPacked = 0;
	Clock = 0;
	for (int Tick = 0; Tick < 2000 && Walked < 0; Tick++)
	{
		FHOLDeployDecision Decision = HOLDeployRead::Decide(State, EHOLDeployOrder::Move, 8000, Profile);
		if (Decision.Action == EHOLDeployAction::March)
		{
			Walked = Clock;
			if (State.Phase != EHOLDeployPhase::Packed)
			{
				WalkedWhileNotPacked++;
			}
		}
		State = HOLDeployRead::Apply(State, Decision);
		State = HOLDeployRead::Advance(State, Step, Profile, 0);
		Clock += Step;
	}
	T.AssertEquals(0, WalkedWhileNotPacked, "it never walks before it is packed");
	T.AssertTrue(Walked >= Profile.PackMs, "it walks only after the pack delay");
	T.AssertTrue(Walked >= 0 && Walked < Profile.PackMs + 3 * Step, "and right after it");
}

// HeroesOfLiberty.Siege.Deploy.EveryCombination
void Test_DeployEveryCombination(FUnitTest& T)
{
	FHOLDeployProfile Profile = HOLDeployRead::DefaultProfile();
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
		EHOLDeployPhase Phase = HOLDeployRead::PhaseAt(PhaseIndex);
		for (int OrderIndex = 0; OrderIndex < 2; OrderIndex++)
		{
			EHOLDeployOrder Order = OrderIndex == 0 ? EHOLDeployOrder::Attack : EHOLDeployOrder::Move;
			for (int DistanceIndex = 0; DistanceIndex < Distances.Num(); DistanceIndex++)
			{
				int Distance = Distances[DistanceIndex];
				Cases++;
				FHOLDeployState State = DeployStateOf(Phase);
				FHOLDeployDecision Decision = HOLDeployRead::Decide(State, Order, Distance, Profile);
				FHOLDeployState Next = HOLDeployRead::Apply(State, Decision);
				bool bOk = true;
				if (Decision.Action == EHOLDeployAction::Fire)
				{
					bOk = bOk && Phase == EHOLDeployPhase::Deployed && Order == EHOLDeployOrder::Attack && HOLDeployRead::CanAim(Profile, Distance);
				}
				if (Decision.Action == EHOLDeployAction::March)
				{
					bOk = bOk && Phase == EHOLDeployPhase::Packed;
				}
				if (Decision.Action == EHOLDeployAction::Refuse)
				{
					bOk = bOk && Order == EHOLDeployOrder::Attack && Distance < Profile.MinRangeCm;
				}
				if (Order == EHOLDeployOrder::Attack && Distance < Profile.MinRangeCm)
				{
					bOk = bOk && Decision.Action == EHOLDeployAction::Refuse;
				}
				if (Decision.Action == EHOLDeployAction::Deploy)
				{
					bOk = bOk && Phase == EHOLDeployPhase::Packed && Next.Phase == EHOLDeployPhase::Deploying;
				}
				if (Decision.Action == EHOLDeployAction::Pack)
				{
					bOk = bOk && Phase == EHOLDeployPhase::Deployed && Next.Phase == EHOLDeployPhase::Packing;
				}
				if (Decision.Action == EHOLDeployAction::CancelDeploy)
				{
					bOk = bOk && Phase == EHOLDeployPhase::Deploying && Next.Phase == EHOLDeployPhase::Packed;
				}
				if (Decision.Action == EHOLDeployAction::Fire || Decision.Action == EHOLDeployAction::March || Decision.Action == EHOLDeployAction::Wait || Decision.Action == EHOLDeployAction::Refuse)
				{
					bOk = bOk && Next.Phase == Phase;
				}
				if (Phase == EHOLDeployPhase::Packing)
				{
					bOk = bOk && (Decision.Action == EHOLDeployAction::Wait || Decision.Action == EHOLDeployAction::Refuse);
				}
				if (!bOk)
				{
					Broken++;
				}
			}
		}
	}
	T.AssertEquals(4 * 2 * 7, Cases, "all 56 combinations were visited");
	T.AssertEquals(0, Broken, "no combination breaks the rule");
}
