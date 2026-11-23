// S4-a (ETAPE0-1123): the AI's call of its faction powers (HOLAIPowerCall::Choose). One reserve per AI level, no cheating (D688):
// the stock left after paying must stay at or above the reserve of the level; a power without a coded effect is never cast (prepared,
// no effect). The rows and the reserves are fixtures; the book's state rule and the spend rule are the real ones.

FHOLPowerRow PowerCallRow(const FString& Key, int Cost, int MinTier, bool bCoded)
{
	FHOLPowerRow Row;
	Row.Key = Key;
	Row.Cost = Cost;
	Row.CooldownMs = 5000;
	Row.MinTier = MinTier;
	Row.bEffectCoded = bCoded;
	return Row;
}

TArray<FHOLPowerRow> PowerCallBook(bool bCoded)
{
	TArray<FHOLPowerRow> Rows;
	Rows.Add(PowerCallRow("Alpha", 30, 0, bCoded));
	return Rows;
}

TArray<int> PowerCallCooldowns()
{
	TArray<int> Result;
	Result.Add(0);
	return Result;
}

TArray<bool> PowerCallWanted(bool bWanted)
{
	TArray<bool> Result;
	Result.Add(bWanted);
	return Result;
}

FHOLAIPowerCallData PowerCallData()
{
	FHOLAIPowerCallData Data;
	Data.ReserveByLevel.Add(0);
	Data.ReserveByLevel.Add(40);
	Data.ReserveByLevel.Add(80);
	return Data;
}

void Test_AIPowerCallSpendsHonorByLevel(FUnitTest& T)
{
	const TArray<FHOLPowerRow> Rows = PowerCallBook(true);
	const TArray<int> Cooldowns = PowerCallCooldowns();
	const TArray<bool> Wanted = PowerCallWanted(true);
	const FHOLAIPowerCallData Data = PowerCallData();
	// Stock 100, cost 30: 70 left. Level 0 keeps 0, level 1 keeps 40: both cast; level 2 keeps 80: refused.
	const int Easy = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 100, 0, 0, Data);
	T.AssertEquals(0, Easy, "level 0 (reserve 0) casts with 70 left");
	const int Middle = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 100, 0, 1, Data);
	T.AssertEquals(0, Middle, "level 1 (reserve 40) casts with 70 left");
	const int Hard = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 100, 0, 2, Data);
	T.AssertEquals(-1, Hard, "level 2 (reserve 80) does not cast with 70 left");
	const int HardRich = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 120, 0, 2, Data);
	T.AssertEquals(0, HardRich, "level 2 casts once the stock leaves 90");
	// A level past the table reads the last reserve; a stock below the cost never casts (no cheating).
	const int Past = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 100, 0, 9, Data);
	T.AssertEquals(-1, Past, "a level past the table reads the last reserve (80)");
	const int Poor = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 20, 0, 0, Data);
	T.AssertEquals(-1, Poor, "the AI never casts what it cannot pay");
}

void Test_AIPowerCallNothingWithoutCodedEffect(FUnitTest& T)
{
	const TArray<FHOLPowerRow> Rows = PowerCallBook(false);
	const int Soon = HOLAIPowerCall::Choose(Rows, PowerCallCooldowns(), PowerCallWanted(true), 500, 0, 0, PowerCallData());
	T.AssertEquals(-1, Soon, "prepared without effect: a power whose effect is not coded is never cast");
	const TArray<FHOLPowerRow> Coded = PowerCallBook(true);
	const int Idle = HOLAIPowerCall::Choose(Coded, PowerCallCooldowns(), PowerCallWanted(false), 500, 0, 0, PowerCallData());
	T.AssertEquals(-1, Idle, "no trigger from the brain: nothing is cast");
}

// S4-a branch step: the brain's decision on the REAL Dawn book (HOLDawnPowers::Rows: Sacred Light, effect coded, 25 Honor, 20 s) and the real rules
// (book state, spend rule, reserve by level). The Honor stock, the wounded points and the cast origin are the fixtures; the choice and the ground point
// are computed by Plan (the point is the centre of the wounded units in range, never copied from a fixture).
TArray<FVector> PowerCallWounded()
{
	TArray<FVector> Result;
	Result.Add(FVector(1000.0, 1000.0, 0.0));
	Result.Add(FVector(1200.0, 1000.0, 0.0));
	Result.Add(FVector(1100.0, 1200.0, 0.0));
	return Result;
}

// GLORFINDEL-1123h, the real partie (scenario ai_sacred_light): the AI plants no banner, so its banner reads (0, 0) while its 7 units stand about 13700 cm away,
// 3 of them wounded, Honor 100, Easy level: the AI never cast (255 decisions cast=false). The origin must then be the army's centre.
void Test_AIBrainCastsWithoutAPlantedBanner(FUnitTest& T)
{
	const TArray<FHOLPowerRow> Rows = HOLDawnPowers::Rows();
	TArray<int> Ready;
	Ready.Add(0);
	TArray<FVector> Army;
	for (int Index = 0; Index < 7; Index++)
	{
		Army.Add(FVector(13700.0 + double(Index) * 50.0, double(Index % 3) * 100.0, 0.0));
	}
	TArray<FVector> Wounded;
	for (int Index = 0; Index < 3; Index++)
	{
		Wounded.Add(Army[Index]);
	}
	const FVector Origin = HOLFactionPowerServer::CastOriginOf(FIntPoint(0, 0), Army);
	const FHOLAIPowerOrder Order = HOLAIPowerCall::Plan(Rows, Ready, 100, 0, 0, HOLAIPowerCall::DefaultData(), Wounded, Origin, 800);
	T.AssertTrue(Order.bCast, "no banner planted, 3 of 7 wounded in the army, Honor 100: the AI casts the Sacred Light");
	// A planted banner stays the origin.
	const FVector Planted = HOLFactionPowerServer::CastOriginOf(FIntPoint(500, 700), Army);
	T.AssertTrue(Math::Abs(Planted.X - 500.0) < 1.0 && Math::Abs(Planted.Y - 700.0) < 1.0, "a planted banner is the origin");
}

void Test_AIBrainCastsSacredLightWhenHonorAllows(FUnitTest& T)
{
	const TArray<FHOLPowerRow> Rows = HOLDawnPowers::Rows();
	TArray<int> Ready;
	Ready.Add(0);
	const FHOLAIPowerCallData Data = PowerCallData();
	const FVector Origin(1000.0, 1000.0, 0.0);
	const int64 Range = 800;
	// Honor 100 (75 left), level 1 keeps 40: the Sacred Light is cast, on the centre of the three wounded units.
	const FHOLAIPowerOrder Order = HOLAIPowerCall::Plan(Rows, Ready, 100, 0, 1, Data, PowerCallWounded(), Origin, Range);
	T.AssertTrue(Order.bCast, "the AI casts the Sacred Light with 75 Honor left above its reserve of 40");
	T.AssertEquals(0, Order.PowerIndex, "the cast power is the book's first line (Sacred Light)");
	T.AssertTrue(Math::Abs(Order.Point.X - 1100.0) < 1.0 && Math::Abs(Order.Point.Y - 1066.0) < 2.0, "the point is the centre of the wounded units");
	// Honor 60 (35 left) is under the reserve of level 2 (80): nothing is cast (no cheating).
	const FHOLAIPowerOrder Saving = HOLAIPowerCall::Plan(Rows, Ready, 60, 0, 2, Data, PowerCallWounded(), Origin, Range);
	T.AssertFalse(Saving.bCast, "a stock whose rest stays under the reserve is kept");
	// No wounded unit: no trigger, nothing is cast. A cool-down not over: nothing either.
	TArray<FVector> None;
	const FHOLAIPowerOrder Idle = HOLAIPowerCall::Plan(Rows, Ready, 100, 0, 1, Data, None, Origin, Range);
	T.AssertFalse(Idle.bCast, "no wounded unit: the AI does not cast");
	TArray<int> Cooling;
	Cooling.Add(9000);
	const FHOLAIPowerOrder Cold = HOLAIPowerCall::Plan(Rows, Cooling, 100, 0, 1, Data, PowerCallWounded(), Origin, Range);
	T.AssertFalse(Cold.bCast, "a power still in cool-down is not cast");
	// Wounded units beyond the cast range (the server would refuse the point): not chosen.
	TArray<FVector> Far;
	Far.Add(FVector(5000.0, 5000.0, 0.0));
	const FHOLAIPowerOrder Away = HOLAIPowerCall::Plan(Rows, Ready, 100, 0, 1, Data, Far, Origin, Range);
	T.AssertFalse(Away.bCast, "wounded units out of range are not a target");
}

// GLORFINDEL-1123pa (D891): the AI plants a banner before casting the Sacred Light; the centre-of-army origin stays only while none is planted.
// The decision (PlanBanner) is a pure rule: the wounded points, the hero's place, whether he may plant (alive, level, cool-down), whether a banner stands.
void Test_AIPlantsABannerBeforeCasting(FUnitTest& T)
{
	TArray<FVector> Wounded;
	Wounded.Add(FVector(13700.0, 0.0, 0.0));
	Wounded.Add(FVector(13800.0, 100.0, 0.0));
	Wounded.Add(FVector(13900.0, 200.0, 0.0));
	const FVector Hero = FVector(13600.0, 0.0, 0.0);
	// A hero able to plant, no banner standing, wounded units: the AI plants, at the wounded units' centre when it is within the hero's reach.
	const FHOLAIBannerOrder Order = HOLAIPowerCall::PlanBanner(Wounded, Hero, true, false);
	T.AssertTrue(Order.bPlant, "wounded units, no banner standing, the hero can plant: the AI plants a banner");
	T.AssertTrue(Math::Abs(Order.Point.X - 13800.0) < 1.0 && Math::Abs(Order.Point.Y - 100.0) < 1.0, "the banner goes on the centre of the wounded units");
	// A wounded group beyond the hero's reach: the banner is put at the reach's edge, on the line toward them (the server would refuse a farther point).
	TArray<FVector> Far;
	Far.Add(FVector(16600.0, 0.0, 0.0));
	const FHOLAIBannerOrder Edge = HOLAIPowerCall::PlanBanner(Far, Hero, true, false);
	T.AssertTrue(Edge.bPlant, "a far wounded group still asks for a banner");
	const double Reach = Math::Sqrt(Math::Square(Edge.Point.X - Hero.X) + Math::Square(Edge.Point.Y - Hero.Y));
	T.AssertTrue(HOLHeroAbilityRead::CanPlantBanner(float(Reach)), "the banner stays within the hero's planting reach");
	T.AssertTrue(Edge.Point.X > Hero.X + 1000.0, "and it is pushed toward the wounded units, not left at the hero's feet");
	// Nothing to plant: a banner already stands, the hero cannot, or nobody is wounded (the origin then stays the banner or the army's centre).
	T.AssertFalse(HOLAIPowerCall::PlanBanner(Wounded, Hero, true, true).bPlant, "a standing banner is not planted again");
	T.AssertFalse(HOLAIPowerCall::PlanBanner(Wounded, Hero, false, false).bPlant, "a hero who cannot plant (dead, low level, cool-down) plants nothing");
	TArray<FVector> None;
	T.AssertFalse(HOLAIPowerCall::PlanBanner(None, Hero, true, false).bPlant, "no wounded unit: no banner");
}

// GLORFINDEL-1123pd (recette 1123pa, defect b): the banner was refused ("banner at (1599, -2) refused") because ThinkPowers read the hero's place as (0, 0).
// PlanBannerOf is the pure seam fed with the hero's real place (the script test world has no living hero yet, so the C++ reader is run by the main session).
void Test_AIPlantsBannerFromTheHeroRealPlace(FUnitTest& T)
{
	TArray<FVector> Wounded;
	Wounded.Add(FVector(14940.0, -40.0, 0.0));
	Wounded.Add(FVector(14960.0, -60.0, 0.0));
	const FVector Hero = FVector(14900.0, 0.0, 0.0);
	const FHOLAIBannerOrder Order = HOLAIPowerCall::PlanBannerOf(Wounded, true, Hero, true, false);
	T.AssertTrue(Order.bPlant, "a hero found, able to plant, wounded units: the AI plants");
	const double Reach = Math::Sqrt(Math::Square(Order.Point.X - Hero.X) + Math::Square(Order.Point.Y - Hero.Y));
	T.AssertTrue(HOLHeroAbilityRead::CanPlantBanner(float(Reach)), "the banner is within the hero's REAL reach (a (0, 0) hero gave (1599, -2), 13000 cm away)");
	T.AssertTrue(Math::Abs(Order.Point.X - 14950.0) < 1.0 && Math::Abs(Order.Point.Y + 50.0) < 1.0, "the banner goes on the centre of the wounded units");
	TArray<FVector> Far;
	Far.Add(FVector(17900.0, 0.0, 0.0));
	const FHOLAIBannerOrder Edge = HOLAIPowerCall::PlanBannerOf(Far, true, Hero, true, false);
	T.AssertTrue(Edge.bPlant && Edge.Point.X > Hero.X + 1500.0 && Edge.Point.X <= Hero.X + 1600.0, "a far group: the banner is pushed from the hero's real place toward it, at the edge of his reach (1600 cm: HOLHeroAbility_Test.as:44-45)");
	T.AssertFalse(HOLAIPowerCall::PlanBannerOf(Wounded, false, Hero, true, false).bPlant, "no hero found: no banner");
	T.AssertFalse(HOLAIPowerCall::PlanBannerOf(Wounded, true, Hero, false, false).bPlant, "his ability not ready: no banner");
}

// GLORFINDEL-1123pg (D890, D891): the AI's Sacred Light zone reads the Sacred Light's OWN range (HOLDawnPowers::SacredLightRangeCm, 1200 cm), not the Seal's 800 cm:
// a wounded unit 1000 cm from the cast origin is a target (it was not with the Seal's range), one at 1300 cm is still out of range.
void Test_AISacredLightCastsOnAWoundedUnitAt1000cm(FUnitTest& T)
{
	const TArray<FHOLPowerRow> Rows = HOLDawnPowers::Rows();
	TArray<int> Ready;
	Ready.Add(0);
	const FVector Origin(1000.0, 1000.0, 0.0);
	const int64 Range = int64(HOLDawnPowers::SacredLightRangeCm());
	TArray<FVector> Near;
	Near.Add(FVector(2000.0, 1000.0, 0.0));
	const FHOLAIPowerOrder InRange = HOLAIPowerCall::Plan(Rows, Ready, 100, 0, 1, PowerCallData(), Near, Origin, Range);
	T.AssertTrue(InRange.bCast, "a wounded unit 1000 cm from the origin is inside the Sacred Light's 1200 cm: the AI casts");
	const FHOLAIPowerOrder Seal = HOLAIPowerCall::Plan(Rows, Ready, 100, 0, 1, PowerCallData(), Near, Origin, HOLFactionPowerCast::SealRangeCm());
	T.AssertFalse(Seal.bCast, "with the Seal's 800 cm the same wounded unit is out of range (the old behaviour)");
	TArray<FVector> Far;
	Far.Add(FVector(2300.0, 1000.0, 0.0));
	const FHOLAIPowerOrder OutOfRange = HOLAIPowerCall::Plan(Rows, Ready, 100, 0, 1, PowerCallData(), Far, Origin, Range);
	T.AssertFalse(OutOfRange.bCast, "a wounded unit 1300 cm away is past the Sacred Light's 1200 cm: no cast");
}
