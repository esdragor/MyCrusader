// MIG-C-PetitesRegles (ELROND-1117k): script port of HOLMonkTests.cpp (HealRules, HealOrderWalk, CostDiscount), run through UHOLHealReadLibrary
// (Simulation/HOLSmallRulesReadLibrary.*, namespace HOLHealRead, envelopes only; the rule HOLHealRules.h stays in C++). Same vectors as the C++ tests.
// NEEDS THE C++ LOT: compile only after the build. Not ported: Table (real unit table), RiseTracker (stateful struct, not exposed), kept in HOLMonkTests.cpp.
// HealOrderWalk: the Monk's HealRange (800) is the constant below (the C++ test reads it from the table and checks it there, in Table).
// The C++ test file is removed only after the red then green of this one (not done yet).

const int HOLMonkId = 1;
const double HOLMonkRange = 800.0;
const int HOLMonkKeep = 0;
const int HOLMonkWalk = 1;
const int HOLMonkStop = 2;

// The candidates of a heal as parallel arrays (Y = 0, maximum health 100, camp 1 unless told).
class FHOLMonkSet
{
	TArray<int> Ids;
	TArray<float32> Xs;
	TArray<float32> Ys;
	TArray<float32> Healths;
	TArray<float32> MaxHealths;
	TArray<int> Camps;
	TArray<int> Unhealables;

	void Add(int Id, double X, double Health, int Camp = 1, bool bUnhealable = false)
	{
		Ids.Add(Id);
		Xs.Add(float32(X));
		Ys.Add(float32(0.0));
		Healths.Add(float32(Health));
		MaxHealths.Add(float32(100.0));
		Camps.Add(Camp);
		Unhealables.Add(bUnhealable ? 1 : 0);
	}

	int Choose(int OrderedId, int KeptId) const
	{
		return HOLHealRead::ChooseTarget(HOLMonkId, 1, 0.0, 0.0, HOLMonkRange, OrderedId, KeptId, Ids, Xs, Ys, Healths, MaxHealths, Camps, Unhealables);
	}

	bool Healable(int Index) const
	{
		return HOLHealRead::IsHealable(HOLMonkId, 1, Ids[Index], Xs[Index], Ys[Index], Healths[Index], MaxHealths[Index], Camps[Index], Unhealables[Index] != 0);
	}
}

// The Monk itself is wounded and alone: it heals nobody; an ally in reach and wounded is the target; 8 m; nearest; kept; ordered.
void Test_MonkHealRules(FUnitTest& T)
{
	FHOLMonkSet Alone;
	Alone.Add(HOLMonkId, 0.0, 40.0);
	T.AssertEquals(-1, Alone.Choose(0, 0), "a wounded Monk alone heals nobody: no self-heal");
	T.AssertEquals(-1, Alone.Choose(0, HOLMonkId), "... even if it kept itself as its target");

	// A healthy ally, an enemy, a King, a siege engine or a dying unit is not a target.
	FHOLMonkSet Mixed;
	Mixed.Add(HOLMonkId, 0.0, 40.0);
	Mixed.Add(2, 300.0, 100.0);
	Mixed.Add(3, 300.0, 50.0, 2);
	Mixed.Add(4, 300.0, 50.0, 1, true);	// the King (HOLCanBeHealed) or an engine
	Mixed.Add(5, 300.0, 50.0, 1, true);
	Mixed.Add(6, 300.0, 0.0);
	Mixed.Add(7, 300.0, 60.0);
	T.AssertEquals(6, Mixed.Choose(0, 0), "the wounded ally is the target (not a healthy one, an enemy, a King, an engine, a dead unit)");
	T.AssertFalse(Mixed.Healable(1), "a healthy ally is not healable");
	T.AssertFalse(Mixed.Healable(2), "an enemy is not healable");
	T.AssertFalse(Mixed.Healable(3), "an unhealable unit is not healable");
	T.AssertFalse(Mixed.Healable(5), "a dead unit is not healable");
	T.AssertTrue(Mixed.Healable(6), "a wounded ally is healable");
	T.AssertFalse(Mixed.Healable(0), "the healer is not healable by itself");

	// 8 m, centre to centre: 800 is in, 801 out.
	FHOLMonkSet EdgeIn;
	EdgeIn.Add(HOLMonkId, 0.0, 100.0);
	EdgeIn.Add(2, 800.0, 50.0);
	T.AssertEquals(1, EdgeIn.Choose(0, 0), "an ally at 8 m is healed");
	FHOLMonkSet EdgeOut;
	EdgeOut.Add(3, 801.0, 50.0);
	T.AssertEquals(-1, EdgeOut.Choose(0, 0), "an ally past 8 m is not");

	// The nearest, the lowest identifier on a tie, and the kept target (one at a time) before a nearer one.
	FHOLMonkSet Two;
	Two.Add(HOLMonkId, 0.0, 100.0);
	Two.Add(8, 500.0, 50.0);
	Two.Add(9, 200.0, 50.0);
	Two.Add(10, -200.0, 50.0);
	T.AssertEquals(2, Two.Choose(0, 0), "the nearest wounded ally, the lowest identifier on a tie");
	T.AssertEquals(1, Two.Choose(0, 8), "the target it healed before is kept while healable and in reach, though another is nearer");
	Two.Healths[1] = 100.0;
	T.AssertEquals(2, Two.Choose(0, 8), "the kept target healed to full: the nearest wounded one, the next");
	Two.Xs[2] = 5000.0;
	T.AssertEquals(3, Two.Choose(0, 9), "the kept target out of reach: another");

	// An order (a right click on an ally): the ordered unit first, though another is nearer or kept; ignored out of reach or healed.
	Two.Healths[1] = 50.0;
	Two.Xs[2] = 200.0;
	T.AssertEquals(1, Two.Choose(8, 0), "the ordered ally, though another is nearer");
	T.AssertEquals(1, Two.Choose(8, 9), "the ordered ally, though the kept one is another");
	T.AssertEquals(1, Two.Choose(0, 8), "the kept one without an order");
	Two.Xs[1] = 5000.0;
	T.AssertEquals(2, Two.Choose(8, 0), "the ordered ally out of reach: the nearest, the order waits");
	Two.Xs[1] = 500.0;
	Two.Healths[1] = 100.0;
	T.AssertEquals(2, Two.Choose(8, 0), "the ordered ally already healed: the nearest");
	Two.Healths[1] = 50.0;
	Two.Camps[1] = 2;
	T.AssertEquals(2, Two.Choose(8, 0), "an enemy cannot be ordered healed");

	// 5 health per second on 50 ms steps: 0.25 a step, 5 after 20 steps, held at the maximum.
	double Health = 40.0;
	for (int Step = 0; Step < 20; ++Step)
	{
		Health = HOLHealRead::GetHealedHealth(float32(Health), 100.0, 5.0, 50);
	}
	T.AssertTrue(Math::Abs(Health - 45.0) <= 0.001, "5 health per second: one second of steps heals 5");
	T.AssertTrue(Math::Abs(HOLHealRead::GetHealedHealth(99.9, 100.0, 5.0, 50) - 100.0) <= 0.001, "held at the maximum");
}

// The heal order's walk: walks only until within the heal range (8 m), not to contact; a target that moves off is followed.
void Test_MonkHealOrderWalk(FUnitTest& T)
{
	const double Margin = HOLHealRead::GetOrderStopMargin();
	const double Repath = HOLHealRead::GetOrderRepathDistance();

	// A Monk 20 m from its target walks 20 cm a step: it stops once the target is within the heal range.
	double Distance = 2000.0;
	bool bWalking = true;	// the first look at the order
	double GoalDrift = 1.0e9;
	int Steps = 0;
	while (Steps < 2000)
	{
		const int Walk = HOLHealRead::GetOrderWalk(Distance, HOLMonkRange, bWalking, GoalDrift);
		if (Walk == HOLMonkStop)
		{
			break;
		}
		if (Walk == HOLMonkWalk)
		{
			bWalking = true;
			GoalDrift = 0.0;
		}
		Distance -= 20.0;
		++Steps;
	}
	T.AssertTrue(Distance <= HOLMonkRange - Margin + 0.001, "20 m away: the Monk stops once the target is within the heal range");
	T.AssertTrue(Distance > 600.0, "... and stops at the range, not at contact (> 6 m left)");
	T.AssertEquals(HOLMonkStop, HOLHealRead::GetOrderWalk(500.0, HOLMonkRange, true, 1.0e9), "the first look at an order in reach ends any earlier walk");
	T.AssertEquals(HOLMonkKeep, HOLHealRead::GetOrderWalk(799.0, HOLMonkRange, false, 0.0), "standing in reach: nothing to do");
	T.AssertEquals(HOLMonkWalk, HOLHealRead::GetOrderWalk(801.0, HOLMonkRange, false, 0.0), "standing out of reach: walk");
	T.AssertEquals(HOLMonkKeep, HOLHealRead::GetOrderWalk(1500.0, HOLMonkRange, true, 0.0), "walking, still far, the target where it was: go on");
	T.AssertEquals(HOLMonkWalk, HOLHealRead::GetOrderWalk(1500.0, HOLMonkRange, true, Repath + 1.0), "walking, the target moved off its goal: a new walk (it follows)");
	T.AssertEquals(HOLMonkKeep, HOLHealRead::GetOrderWalk(1500.0, HOLMonkRange, true, Repath), "walking, a small step of the target: go on");
	T.AssertEquals(HOLMonkStop, HOLHealRead::GetOrderWalk(700.0, HOLMonkRange, true, 5000.0), "walking, in reach of a moving target: stop, not follow");
	T.AssertEquals(HOLMonkWalk, HOLHealRead::GetOrderWalk(1000.0, HOLMonkRange, false, 0.0), "stopped, the target walks off out of reach: follow");
}

// The Vow of Poverty (GDD 11.1, table: MonkCostDiscount 20 %): the Monk costs less, and only the Monk.
void Test_MonkCostDiscount(FUnitTest& T)
{
	T.AssertEquals("MonkCostDiscount", HOLHealRead::GetCostDiscountStat("Monk"), "the Monk's cost is lowered by MonkCostDiscount");
	T.AssertEquals("", HOLHealRead::GetCostDiscountStat("Spearman"), "no other unit's is: Spearman");
	T.AssertEquals("", HOLHealRead::GetCostDiscountStat("Peasant"), "no other unit's is: Peasant");
	int Food = 0;
	int Wood = 0;
	int Stone = 0;
	int Gold = 0;
	HOLHealRead::GetDiscountedCost(100, 0, 0, 100, 20, Food, Wood, Stone, Gold);
	T.AssertTrue(Food == 80 && Wood == 0 && Stone == 0 && Gold == 80, "20 %: 80 food + 80 gold");
	HOLHealRead::GetDiscountedCost(100, 0, 0, 100, 0, Food, Wood, Stone, Gold);
	T.AssertTrue(Food == 100 && Wood == 0 && Stone == 0 && Gold == 100, "0 %: unchanged");
	HOLHealRead::GetDiscountedCost(0, 0, 0, 50, 20, Food, Wood, Stone, Gold);
	T.AssertEquals(0, Food, "a free resource stays free");
	HOLHealRead::GetDiscountedCost(50, 0, 0, 0, 15, Food, Wood, Stone, Gold);
	T.AssertEquals(43, Food, "rounded to the nearest unit: 15 % of 50 = 42.5 -> 43");
	HOLHealRead::GetDiscountedCost(100, 0, 0, 100, 400, Food, Wood, Stone, Gold);
	T.AssertEquals(10, Food, "never below 10 % of the cost");
}
