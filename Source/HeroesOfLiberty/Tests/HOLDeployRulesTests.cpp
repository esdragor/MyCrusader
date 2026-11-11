// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Siege/HOLDeployRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// Palier 3 G5 (Palier3InProgress.md, GDD D712, D715): the C++ twin of the deployable engine's rule, tested with the SAME cases as
// Script/Tests/HOLDeployRules_Test.as (the script and the C++ must agree). The expectations come from D712 and from the profile's own
// values (the delays, the ranges), never retyped numbers: the values are PROPOSED (Retours/Q-trebuchet-deploy.md), no test pins one.
namespace UE::HeroesOfLiberty::Tests::Deploy
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	using namespace HOLDeploy;

	FState StateOf(EPhase Phase)
	{
		FState State;
		State.Phase = Phase;
		State.ElapsedMs = 0;
		return State;
	}

	/** Time passes in steps until the phase changes; the milliseconds it took (a safety stop at 10 minutes). */
	int32 MsUntilPhaseChange(const FState& Start, const FProfile& Profile, int32 Counterweight, int32 StepMs)
	{
		FState State = Start;
		int32 Elapsed = 0;
		while (State.Phase == Start.Phase && Elapsed < 600000)
		{
			State = Advance(State, StepMs, Profile, Counterweight);
			Elapsed += StepMs;
		}
		return Elapsed;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployWhoCanDoWhatTest, "HeroesOfLiberty.Siege.Deploy.WhoCanDoWhat", UE::HeroesOfLiberty::Tests::Deploy::Flags)
bool FHOLDeployWhoCanDoWhatTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Deploy;
	TestFalse(TEXT("packed: no fire"), CanFire(StateOf(EPhase::Packed)));
	TestTrue(TEXT("deployed: fires"), CanFire(StateOf(EPhase::Deployed)));
	TestFalse(TEXT("deploying: no fire"), CanFire(StateOf(EPhase::Deploying)));
	TestFalse(TEXT("packing: no fire"), CanFire(StateOf(EPhase::Packing)));
	TestTrue(TEXT("packed: walks"), CanMove(StateOf(EPhase::Packed)));
	TestFalse(TEXT("deployed: does not walk"), CanMove(StateOf(EPhase::Deployed)));
	TestFalse(TEXT("deploying: does not walk"), CanMove(StateOf(EPhase::Deploying)));
	TestFalse(TEXT("packing: does not walk"), CanMove(StateOf(EPhase::Packing)));
	TestTrue(TEXT("packed: can act (defend)"), CanAct(StateOf(EPhase::Packed)));
	TestTrue(TEXT("deployed: can act"), CanAct(StateOf(EPhase::Deployed)));
	TestFalse(TEXT("deploying: no action, no defence"), CanAct(StateOf(EPhase::Deploying)));
	TestFalse(TEXT("packing: no action, no defence"), CanAct(StateOf(EPhase::Packing)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployDelaysTest, "HeroesOfLiberty.Siege.Deploy.DelaysAreTheProfiles", UE::HeroesOfLiberty::Tests::Deploy::Flags)
bool FHOLDeployDelaysTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Deploy;
	const FProfile Profile = DefaultProfile();
	TestTrue(TEXT("the profile has real delays"), Profile.DeployMs > 0 && Profile.PackMs > 0);
	const FState Deploying = StateOf(EPhase::Deploying);
	const FState Almost = Advance(Deploying, Profile.DeployMs - 1, Profile, 0);
	TestTrue(TEXT("one millisecond short: still deploying"), Almost.Phase == EPhase::Deploying);
	const FState Done = Advance(Almost, 1, Profile, 0);
	TestTrue(TEXT("the delay passed: deployed"), Done.Phase == EPhase::Deployed);
	TestEqual(TEXT("a settled state starts at zero"), Done.ElapsedMs, 0);
	const FState Packing = StateOf(EPhase::Packing);
	TestTrue(TEXT("one millisecond short: still packing"), Advance(Packing, Profile.PackMs - 1, Profile, 0).Phase == EPhase::Packing);
	TestTrue(TEXT("the delay passed: packed"), Advance(Packing, Profile.PackMs, Profile, 0).Phase == EPhase::Packed);
	TestTrue(TEXT("time on a deployed engine changes nothing"), Advance(StateOf(EPhase::Deployed), 100000, Profile, 0).Phase == EPhase::Deployed);
	TestTrue(TEXT("time on a packed engine changes nothing"), Advance(StateOf(EPhase::Packed), 100000, Profile, 0).Phase == EPhase::Packed);
	const int32 Small = MsUntilPhaseChange(Deploying, Profile, 0, 50);
	TestTrue(TEXT("50 ms steps: the same moment, within a step"), Small >= Profile.DeployMs && Small < Profile.DeployMs + 50);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployAttackOrderTest, "HeroesOfLiberty.Siege.Deploy.AttackOrderDecisions", UE::HeroesOfLiberty::Tests::Deploy::Flags)
bool FHOLDeployAttackOrderTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Deploy;
	const FProfile Profile = DefaultProfile();
	const int32 InRange = (Profile.MinRangeCm + Profile.MaxRangeCm) / 2;
	const int32 TooFar = Profile.MaxRangeCm + 100;
	TestTrue(TEXT("packed, target too far: walk toward it"), Decide(StateOf(EPhase::Packed), EOrder::Attack, TooFar, Profile).Action == EAction::March);
	const FDecision Start = Decide(StateOf(EPhase::Packed), EOrder::Attack, InRange, Profile);
	TestTrue(TEXT("packed, target in range: starts deploying"), Start.Action == EAction::Deploy && Start.NextPhase == EPhase::Deploying);
	TestTrue(TEXT("deploying, target in range: waits"), Decide(StateOf(EPhase::Deploying), EOrder::Attack, InRange, Profile).Action == EAction::Wait);
	TestTrue(TEXT("deployed, target in range: fires"), Decide(StateOf(EPhase::Deployed), EOrder::Attack, InRange, Profile).Action == EAction::Fire);
	const FDecision Leave = Decide(StateOf(EPhase::Deployed), EOrder::Attack, TooFar, Profile);
	TestTrue(TEXT("deployed, target too far: packs to go to it"), Leave.Action == EAction::Pack && Leave.NextPhase == EPhase::Packing);
	TestTrue(TEXT("packing, target in range: waits for the end"), Decide(StateOf(EPhase::Packing), EOrder::Attack, InRange, Profile).Action == EAction::Wait);
	TestTrue(TEXT("exactly the minimum range: aimed"), Decide(StateOf(EPhase::Deployed), EOrder::Attack, Profile.MinRangeCm, Profile).Action == EAction::Fire);
	TestTrue(TEXT("exactly the maximum range: aimed"), Decide(StateOf(EPhase::Deployed), EOrder::Attack, Profile.MaxRangeCm, Profile).Action == EAction::Fire);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployMinimumRangeTest, "HeroesOfLiberty.Siege.Deploy.MinimumRange", UE::HeroesOfLiberty::Tests::Deploy::Flags)
bool FHOLDeployMinimumRangeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Deploy;
	const FProfile Profile = DefaultProfile();
	TestTrue(TEXT("the Trebuchet has a minimum range"), Profile.MinRangeCm > 0);
	TestTrue(TEXT("the minimum is under the maximum"), Profile.MinRangeCm < Profile.MaxRangeCm);
	for (int32 Index = 0; Index < 4; Index++)
	{
		const EPhase Phase = PhaseAt(Index);
		const FDecision Refused = Decide(StateOf(Phase), EOrder::Attack, Profile.MinRangeCm - 1, Profile);
		TestTrue(*FString::Printf(TEXT("phase %d: a target under the minimum is refused"), Index), Refused.Action == EAction::Refuse);
		TestEqual(*FString::Printf(TEXT("phase %d: the reason is the minimum range"), Index), Refused.Reason, TooClose);
		TestTrue(*FString::Printf(TEXT("phase %d: a refusal changes nothing"), Index), Refused.NextPhase == Phase);
		TestTrue(*FString::Printf(TEXT("phase %d: a target at distance 0 is refused too"), Index), Decide(StateOf(Phase), EOrder::Attack, 0, Profile).Action == EAction::Refuse);
	}
	TestFalse(TEXT("can aim: under the minimum, no"), CanAim(Profile, Profile.MinRangeCm - 1));
	TestTrue(TEXT("can aim: the minimum, yes"), CanAim(Profile, Profile.MinRangeCm));
	TestTrue(TEXT("can aim: the maximum, yes"), CanAim(Profile, Profile.MaxRangeCm));
	TestFalse(TEXT("can aim: past the maximum, no"), CanAim(Profile, Profile.MaxRangeCm + 1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployMoveOrderTest, "HeroesOfLiberty.Siege.Deploy.MoveOrderDecisions", UE::HeroesOfLiberty::Tests::Deploy::Flags)
bool FHOLDeployMoveOrderTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Deploy;
	const FProfile Profile = DefaultProfile();
	TestTrue(TEXT("packed: walks"), Decide(StateOf(EPhase::Packed), EOrder::Move, 5000, Profile).Action == EAction::March);
	const FDecision Pack = Decide(StateOf(EPhase::Deployed), EOrder::Move, 5000, Profile);
	TestTrue(TEXT("deployed: packs first"), Pack.Action == EAction::Pack && Pack.NextPhase == EPhase::Packing);
	TestTrue(TEXT("packing: waits for the end"), Decide(StateOf(EPhase::Packing), EOrder::Move, 5000, Profile).Action == EAction::Wait);
	const FDecision Cancel = Decide(StateOf(EPhase::Deploying), EOrder::Move, 5000, Profile);
	TestTrue(TEXT("deploying: the walk order cancels it, packed again"), Cancel.Action == EAction::CancelDeploy && Cancel.NextPhase == EPhase::Packed);
	TestTrue(TEXT("a walk order is never refused for the minimum range"), Decide(StateOf(EPhase::Packed), EOrder::Move, 0, Profile).Action == EAction::March);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployButtonTest, "HeroesOfLiberty.Siege.Deploy.Button", UE::HeroesOfLiberty::Tests::Deploy::Flags)
bool FHOLDeployButtonTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Deploy;
	TestTrue(TEXT("button on a packed engine: deploy"), Toggle(StateOf(EPhase::Packed)).Action == EAction::Deploy);
	TestTrue(TEXT("button on a deployed engine: pack"), Toggle(StateOf(EPhase::Deployed)).Action == EAction::Pack);
	TestTrue(TEXT("button while deploying: cancel"), Toggle(StateOf(EPhase::Deploying)).Action == EAction::CancelDeploy);
	TestTrue(TEXT("button while packing: nothing to do"), Toggle(StateOf(EPhase::Packing)).Action == EAction::Wait);
	const FState Applied = Apply(StateOf(EPhase::Packed), Toggle(StateOf(EPhase::Packed)));
	TestTrue(TEXT("applying the button's decision starts the deploying at zero"), Applied.Phase == EPhase::Deploying && Applied.ElapsedMs == 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployCounterweightTest, "HeroesOfLiberty.Siege.Deploy.Counterweight", UE::HeroesOfLiberty::Tests::Deploy::Flags)
bool FHOLDeployCounterweightTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Deploy;
	const FProfile Profile = DefaultProfile();
	TestEqual(TEXT("no Counterweight: the table's deploy delay"), DeployDelayMs(Profile, 0), Profile.DeployMs);
	TestEqual(TEXT("no Counterweight: the table's pack delay"), PackDelayMs(Profile, 0), Profile.PackMs);
	TestTrue(TEXT("25 percent: the deploy delay is shorter"), DeployDelayMs(Profile, 25) < Profile.DeployMs);
	TestTrue(TEXT("25 percent: the pack delay is shorter"), PackDelayMs(Profile, 25) < Profile.PackMs);
	TestTrue(TEXT("more percent, shorter delay"), DeployDelayMs(Profile, 50) < DeployDelayMs(Profile, 25));
	TestTrue(TEXT("past 100 percent is 100 percent"), DeployDelayMs(Profile, 150) >= 0 && DeployDelayMs(Profile, 150) == DeployDelayMs(Profile, 100));
	TestEqual(TEXT("a negative percentage is none"), DeployDelayMs(Profile, -20), Profile.DeployMs);
	const int32 Without = MsUntilPhaseChange(StateOf(EPhase::Deploying), Profile, 0, 10);
	const int32 With = MsUntilPhaseChange(StateOf(EPhase::Deploying), Profile, 25, 10);
	TestTrue(TEXT("with Counterweight the engine is deployed sooner"), With < Without);
	TestTrue(TEXT("and it takes the shortened delay, within a step"), With >= DeployDelayMs(Profile, 25) && With < DeployDelayMs(Profile, 25) + 10);
	const int32 PackWithout = MsUntilPhaseChange(StateOf(EPhase::Packing), Profile, 0, 10);
	const int32 PackWith = MsUntilPhaseChange(StateOf(EPhase::Packing), Profile, 25, 10);
	TestTrue(TEXT("with Counterweight the engine is packed sooner too"), PackWith < PackWithout);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeploySequenceTest, "HeroesOfLiberty.Siege.Deploy.SequenceFromOrderToShot", UE::HeroesOfLiberty::Tests::Deploy::Flags)
bool FHOLDeploySequenceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Deploy;
	const FProfile Profile = DefaultProfile();
	const int32 Distance = (Profile.MinRangeCm + Profile.MaxRangeCm) / 2;
	FState State = StateOf(EPhase::Packed);
	const int32 Step = 50;
	int32 Clock = 0;
	int32 FirstShot = -1;
	int32 FiredTooEarly = 0;
	for (int32 Tick = 0; Tick < 2000 && FirstShot < 0; Tick++)
	{
		const FDecision Decision = Decide(State, EOrder::Attack, Distance, Profile);
		if (Decision.Action == EAction::Fire)
		{
			FirstShot = Clock;
		}
		else if (CanFire(State))
		{
			FiredTooEarly++;
		}
		State = Apply(State, Decision);
		State = Advance(State, Step, Profile, 0);
		Clock += Step;
	}
	TestTrue(TEXT("no shot before the deploy delay has passed"), FirstShot >= Profile.DeployMs);
	TestTrue(TEXT("the shot comes right after the deploy delay"), FirstShot >= 0 && FirstShot < Profile.DeployMs + 3 * Step);
	TestEqual(TEXT("a deployed engine that could fire always did"), FiredTooEarly, 0);
	int32 Walked = -1;
	int32 WalkedWhileNotPacked = 0;
	Clock = 0;
	for (int32 Tick = 0; Tick < 2000 && Walked < 0; Tick++)
	{
		const FDecision Decision = Decide(State, EOrder::Move, 8000, Profile);
		if (Decision.Action == EAction::March)
		{
			Walked = Clock;
			if (State.Phase != EPhase::Packed)
			{
				WalkedWhileNotPacked++;
			}
		}
		State = Apply(State, Decision);
		State = Advance(State, Step, Profile, 0);
		Clock += Step;
	}
	TestEqual(TEXT("it never walks before it is packed"), WalkedWhileNotPacked, 0);
	TestTrue(TEXT("it walks only after the pack delay"), Walked >= Profile.PackMs);
	TestTrue(TEXT("and right after it"), Walked >= 0 && Walked < Profile.PackMs + 3 * Step);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployEveryCombinationTest, "HeroesOfLiberty.Siege.Deploy.EveryCombination", UE::HeroesOfLiberty::Tests::Deploy::Flags)
bool FHOLDeployEveryCombinationTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Deploy;
	const FProfile Profile = DefaultProfile();
	const TArray<int32> Distances = {0, Profile.MinRangeCm - 1, Profile.MinRangeCm, (Profile.MinRangeCm + Profile.MaxRangeCm) / 2, Profile.MaxRangeCm, Profile.MaxRangeCm + 1, Profile.MaxRangeCm * 10};
	int32 Broken = 0;
	int32 Cases = 0;
	for (int32 PhaseIndex = 0; PhaseIndex < 4; PhaseIndex++)
	{
		const EPhase Phase = PhaseAt(PhaseIndex);
		for (int32 OrderIndex = 0; OrderIndex < 2; OrderIndex++)
		{
			const EOrder Order = OrderIndex == 0 ? EOrder::Attack : EOrder::Move;
			for (const int32 Distance : Distances)
			{
				Cases++;
				const FState State = StateOf(Phase);
				const FDecision Decision = Decide(State, Order, Distance, Profile);
				const FState Next = Apply(State, Decision);
				bool bOk = true;
				if (Decision.Action == EAction::Fire)
				{
					bOk = bOk && Phase == EPhase::Deployed && Order == EOrder::Attack && CanAim(Profile, Distance);
				}
				if (Decision.Action == EAction::March)
				{
					bOk = bOk && Phase == EPhase::Packed;
				}
				if (Decision.Action == EAction::Refuse)
				{
					bOk = bOk && Order == EOrder::Attack && Distance < Profile.MinRangeCm;
				}
				if (Order == EOrder::Attack && Distance < Profile.MinRangeCm)
				{
					bOk = bOk && Decision.Action == EAction::Refuse;
				}
				if (Decision.Action == EAction::Deploy)
				{
					bOk = bOk && Phase == EPhase::Packed && Next.Phase == EPhase::Deploying;
				}
				if (Decision.Action == EAction::Pack)
				{
					bOk = bOk && Phase == EPhase::Deployed && Next.Phase == EPhase::Packing;
				}
				if (Decision.Action == EAction::CancelDeploy)
				{
					bOk = bOk && Phase == EPhase::Deploying && Next.Phase == EPhase::Packed;
				}
				if (Decision.Action == EAction::Fire || Decision.Action == EAction::March || Decision.Action == EAction::Wait || Decision.Action == EAction::Refuse)
				{
					bOk = bOk && Next.Phase == Phase;
				}
				if (Phase == EPhase::Packing)
				{
					bOk = bOk && (Decision.Action == EAction::Wait || Decision.Action == EAction::Refuse);
				}
				if (!bOk)
				{
					Broken++;
				}
			}
		}
	}
	TestEqual(TEXT("all 56 combinations were visited"), Cases, 4 * 2 * 7);
	TestEqual(TEXT("no combination breaks the rule"), Broken, 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
