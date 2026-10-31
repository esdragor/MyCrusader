// The common duel kit, alone (DuelInProgress.md D6; GDD 10.3, D118, D119): each expectation comes from the preset or from the
// state returned; the order of the postures and the "strongly, without cancelling" of the parry are the GDD's statements.

FHOLDuelKitSettings KitSettings()
{
	return HOLDuelKit::Preset();
}

// Postures (D118): offensive deals more and has the shortest parry window, defensive deals less and has the widest.
void Test_DuelKitPostureOrder(FUnitTest& T)
{
	FHOLDuelKitSettings S = KitSettings();
	T.AssertEquals(100, HOLDuelKit::DamagePercentOf(S, EHOLDuelPosture::Balanced), "balanced is the reference");
	T.AssertTrue(HOLDuelKit::DamagePercentOf(S, EHOLDuelPosture::Offensive) > HOLDuelKit::DamagePercentOf(S, EHOLDuelPosture::Balanced), "offensive deals more");
	T.AssertTrue(HOLDuelKit::DamagePercentOf(S, EHOLDuelPosture::Defensive) < HOLDuelKit::DamagePercentOf(S, EHOLDuelPosture::Balanced), "defensive deals less");
	T.AssertTrue(HOLDuelKit::ParryWindowMsOf(S, EHOLDuelPosture::Offensive) < HOLDuelKit::ParryWindowMsOf(S, EHOLDuelPosture::Balanced), "offensive: shorter window");
	T.AssertTrue(HOLDuelKit::ParryWindowMsOf(S, EHOLDuelPosture::Defensive) > HOLDuelKit::ParryWindowMsOf(S, EHOLDuelPosture::Balanced), "defensive: wider window");
	T.AssertTrue(HOLDuelKit::ParryWindowMsOf(S, EHOLDuelPosture::Offensive) > 0, "a window is never nil");
}

// The announcement is read in 0.5 to 1 s (10.3) and the parry is never an immunity (D119).
void Test_DuelKitPresetInTheGddRanges(FUnitTest& T)
{
	FHOLDuelKitSettings S = KitSettings();
	T.AssertTrue(S.StrongAnnounceMs >= 500 && S.StrongAnnounceMs <= 1000, "announced 0.5 to 1 s");
	T.AssertTrue(S.ParryReducePercent >= 50 && S.ParryReducePercent < 100, "reduces strongly, without cancelling");
	T.AssertTrue(S.ParryCooldownMs > 0, "the parry has a recovery");
	T.AssertTrue(S.ParryCooldownMs > HOLDuelKit::ParryWindowMsOf(S, EHOLDuelPosture::Defensive), "the recovery outlasts even the widest window");
}

// Changing posture is immediate and leaves the parry alone.
void Test_DuelKitPostureChangeIsImmediate(FUnitTest& T)
{
	FHOLDuelKitSettings S = KitSettings();
	FHOLDuelKit Kit;
	Kit = HOLDuelKit::PressParry(S, Kit);
	FHOLDuelKit Changed = HOLDuelKit::SetPosture(Kit, EHOLDuelPosture::Defensive);
	T.AssertTrue(Changed.Posture == EHOLDuelPosture::Defensive, "the posture is the new one at once");
	T.AssertEquals(Kit.ParryWindowLeftMs, Changed.ParryWindowLeftMs, "the open window is not touched");
	T.AssertEquals(Kit.ParryCooldownLeftMs, Changed.ParryCooldownLeftMs, "nor the recovery");
}

// The parry opens the window of the posture and starts the recovery; it cannot be pressed during the recovery.
void Test_DuelKitParryWindowAndCooldown(FUnitTest& T)
{
	FHOLDuelKitSettings S = KitSettings();
	FHOLDuelKit Kit = HOLDuelKit::SetPosture(FHOLDuelKit(), EHOLDuelPosture::Defensive);
	FHOLDuelKit Pressed = HOLDuelKit::PressParry(S, Kit);
	T.AssertEquals(S.DefensiveParryWindowMs, Pressed.ParryWindowLeftMs, "the window of the defensive posture");
	T.AssertEquals(S.ParryCooldownMs, Pressed.ParryCooldownLeftMs, "the recovery starts");
	FHOLDuelKit Again = HOLDuelKit::PressParry(S, Pressed);
	T.AssertEquals(Pressed.ParryWindowLeftMs, Again.ParryWindowLeftMs, "pressed again during the recovery: nothing changes");
	FHOLDuelKit Later = HOLDuelKit::Tick(Pressed, S.DefensiveParryWindowMs);
	T.AssertEquals(0, Later.ParryWindowLeftMs, "the window closes after its time");
	T.AssertEquals(S.ParryCooldownMs - S.DefensiveParryWindowMs, Later.ParryCooldownLeftMs, "the recovery runs down");
	FHOLDuelKit Ready = HOLDuelKit::Tick(Pressed, S.ParryCooldownMs + 10);
	T.AssertEquals(0, Ready.ParryCooldownLeftMs, "never below zero");
	FHOLDuelKit Second = HOLDuelKit::PressParry(S, Ready);
	T.AssertEquals(S.DefensiveParryWindowMs, Second.ParryWindowLeftMs, "ready again: the parry opens again");
}

// A strong attack parried in the window loses ParryReducePercent and spends the window; unparried, it lands whole.
void Test_DuelKitParryReducesAStrongHit(FUnitTest& T)
{
	FHOLDuelKitSettings S = KitSettings();
	int Base = 200;
	FHOLDuelKit Bare;
	T.AssertEquals(Base, HOLDuelKit::TakeStrongHit(S, Bare, Base), "no window: the whole damage");
	FHOLDuelKit Kit = HOLDuelKit::PressParry(S, FHOLDuelKit());
	int Taken = HOLDuelKit::TakeStrongHit(S, Kit, Base);
	T.AssertEquals(Base - Math::IntegerDivisionTrunc(Base * S.ParryReducePercent, 100), Taken, "the reduction of the preset");
	T.AssertTrue(Taken > 0, "reduced, not cancelled");
	T.AssertEquals(0, Kit.ParryWindowLeftMs, "the window is spent");
	T.AssertEquals(Base, HOLDuelKit::TakeStrongHit(S, Kit, Base), "a second strong hit finds no window");
}

// The damage a duellist deals follows his posture's percent.
void Test_DuelKitDealDamageFollowsPosture(FUnitTest& T)
{
	FHOLDuelKitSettings S = KitSettings();
	FHOLDuelKit Kit;
	T.AssertEquals(100, HOLDuelKit::DealDamage(S, Kit, 100), "balanced: the reference");
	Kit = HOLDuelKit::SetPosture(Kit, EHOLDuelPosture::Offensive);
	T.AssertEquals(S.OffensiveDamagePercent, HOLDuelKit::DealDamage(S, Kit, 100), "offensive: its percent of 100");
	Kit = HOLDuelKit::SetPosture(Kit, EHOLDuelPosture::Defensive);
	T.AssertEquals(S.DefensiveDamagePercent, HOLDuelKit::DealDamage(S, Kit, 100), "defensive: its percent of 100");
}
