// The hero's return rules (J8a.4, HOLHeroReturn.as): delay, price, return by itself, place.

// Level 1: 38 s = 760 steps; level 2: 46 s = 920 steps.
void Test_HeroReturnDelay(FUnitTest& T)
{
	T.AssertEquals(760, HOLHeroReturn::DelaySteps(1), "delay at level 1");
	T.AssertEquals(920, HOLHeroReturn::DelaySteps(2), "delay at level 2");
	T.AssertEquals(1000 + 760, HOLHeroReturn::ReadyStep(1000, 1), "ready step at level 1");
}

// Before the delay ends: not ready, no price, no paid return.
void Test_HeroReturnNotReadyBefore(FUnitTest& T)
{
	const int Death = 1000;
	T.AssertFalse(HOLHeroReturn::IsReady(Death, Death, 1), "not ready at death");
	T.AssertFalse(HOLHeroReturn::IsReady(Death + 759, Death, 1), "not ready one step before");
	T.AssertEquals(HOLHeroReturn::PriceNotReady, HOLHeroReturn::PriceAt(Death + 759, Death, 1), "no price before");
	T.AssertFalse(HOLHeroReturn::CanBuyReturn(Death + 759, Death, 1), "paid return refused before");
	T.AssertTrue(HOLHeroReturn::IsReady(Death + 760, Death, 1), "ready at the ready step");
	T.AssertTrue(HOLHeroReturn::CanBuyReturn(Death + 760, Death, 1), "paid return allowed at the ready step");
}

// 150 at the ready step, 75 half-way, rounded up in between, 1 just before the end, 0 from 1 200 steps after ready.
void Test_HeroReturnPrice(FUnitTest& T)
{
	const int Death = 1000;
	const int Ready = HOLHeroReturn::ReadyStep(Death, 1);
	T.AssertEquals(150, HOLHeroReturn::PriceAt(Ready, Death, 1), "150 at the ready step");
	T.AssertEquals(150, HOLHeroReturn::PriceAt(Ready + 1, Death, 1), "149.875 rounded up");
	T.AssertEquals(75, HOLHeroReturn::PriceAt(Ready + 600, Death, 1), "75 half-way");
	T.AssertEquals(75, HOLHeroReturn::PriceAt(Ready + 601, Death, 1), "74.875 rounded up");
	T.AssertEquals(1, HOLHeroReturn::PriceAt(Ready + 1199, Death, 1), "1 just before the end");
	T.AssertEquals(0, HOLHeroReturn::PriceAt(Ready + 1200, Death, 1), "0 at the end");
	T.AssertEquals(0, HOLHeroReturn::PriceAt(Ready + 5000, Death, 1), "0 after the end");
}

// J8b-Q5: the price starts at 100 + 50 x level (150 at level 1, 200 at level 2, 400 at level 6) and still drops to 0
// in 60 s; the delay 30 s + 8 s x level (78 s = 1 560 steps at level 6).
void Test_HeroReturnPriceByLevel(FUnitTest& T)
{
	const int Death = 1000;
	T.AssertEquals(1560, HOLHeroReturn::DelaySteps(6), "delay at level 6");
	const int Ready2 = HOLHeroReturn::ReadyStep(Death, 2);
	T.AssertEquals(200, HOLHeroReturn::PriceAt(Ready2, Death, 2), "200 at the ready step, level 2");
	T.AssertEquals(100, HOLHeroReturn::PriceAt(Ready2 + 600, Death, 2), "100 half-way, level 2");
	T.AssertEquals(0, HOLHeroReturn::PriceAt(Ready2 + 1200, Death, 2), "0 at the end, level 2");
	const int Ready6 = HOLHeroReturn::ReadyStep(Death, 6);
	T.AssertEquals(400, HOLHeroReturn::PriceAt(Ready6, Death, 6), "400 at the ready step, level 6");
	T.AssertEquals(200, HOLHeroReturn::PriceAt(Ready6 + 600, Death, 6), "200 half-way, level 6");
	T.AssertEquals(1, HOLHeroReturn::PriceAt(Ready6 + 1199, Death, 6), "1 just before the end, level 6");
	T.AssertTrue(HOLHeroReturn::ShouldAutoReturn(Ready6 + 1200, Death, 6), "back by itself at 0, level 6");
}

// The hero comes back by itself at price 0: 1 960 steps after death at level 1.
void Test_HeroReturnByItself(FUnitTest& T)
{
	const int Death = 1000;
	T.AssertFalse(HOLHeroReturn::ShouldAutoReturn(Death + 1959, Death, 1), "not yet at 1 959");
	T.AssertTrue(HOLHeroReturn::ShouldAutoReturn(Death + 1960, Death, 1), "by itself at 1 960");
	T.AssertTrue(HOLHeroReturn::ShouldAutoReturn(Death + 3000, Death, 1), "still by itself later");
}

// Main forum standing: main; down: the other forum nearest to the main; none: no return.
void Test_HeroReturnPlace(FUnitTest& T)
{
	const FIntPoint Main = FIntPoint(0, 0);
	TArray<FIntPoint> Others;
	Others.Add(FIntPoint(5000, 0));
	Others.Add(FIntPoint(-1000, 1000));
	T.AssertEquals(HOLHeroReturn::PlaceMainForum, HOLHeroReturn::ReturnPlace(true, Main, Others), "main standing");
	T.AssertEquals(1, HOLHeroReturn::ReturnPlace(false, Main, Others), "main down: nearest other forum");
	TArray<FIntPoint> None;
	T.AssertEquals(HOLHeroReturn::PlaceNone, HOLHeroReturn::ReturnPlace(false, Main, None), "main down, no other forum");
}

// The rules' rate is the simulation's, read from the C++ bridge (UHOLHeroLibrary, HOLHero:: in script).
void Test_HeroReturnStepsPerSecondFromBridge(FUnitTest& T)
{
	T.AssertEquals(HOLHero::GetSimulationRateHz(), HOLHeroReturn::StepsPerSecond, "steps per second: the simulation's (D217)");
	T.AssertEquals(20, HOLHero::GetSimulationRateHz(), "20 steps per second");
}

// The times' shortcut for the network test (hol.Hero.ReturnTimePercent, J8a.4 loop 5): at its default the real rules
// (760 steps then 1 960 at level 1); at 20 % a fifth of them, the price's formula unchanged (150 at the ready step, 75
// half-way, 0 at the end of the shorter drop).
void Test_HeroReturnTimePercent(FUnitTest& T)
{
	T.AssertEquals(100, HOLHero::GetReturnTimePercent(), "default: the rules' times");
	T.AssertEquals(760, HOLHeroReturn::DelaySteps(1), "default: 760 steps at level 1");
	T.AssertEquals(1960, HOLHeroReturn::DelaySteps(1) + HOLHeroReturn::PriceDropSteps(), "default: back by itself at 1 960");
	T.AssertEquals(152, HOLHeroReturn::DelayStepsAt(1, 20), "20 %: 152 steps at level 1");
	T.AssertEquals(240, HOLHeroReturn::PriceDropStepsAt(20), "20 %: the price drops in 240 steps");
	T.AssertEquals(1, HOLHeroReturn::PriceDropStepsAt(0), "never less than one step");
}

// Compiles only if a script object can listen to the C++ step end (UHOLHeroSubsystem::OnHeroStep), as loop 3's rule will.
class UHOLHeroStepListenCheck : UObject
{
	int NumHeard = 0;

	UFUNCTION()
	void HandleHeroStep(int Step)
	{
		NumHeard++;
	}

	void ListenToWorld()
	{
		UHOLHeroSubsystem Heroes = UHOLHeroSubsystem::Get();
		if (Heroes != nullptr)
			Heroes.OnHeroStep.AddUFunction(this, n"HandleHeroStep");
	}
}
